// RemoteDesktopDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "RemoteDesktopDlg.h"
#include "afxdialogex.h"
#include "comm.h"
#include "TcpTran.h"


HANDLE  g_hImageCapThread =NULL;
//BOOL    m_bDlgScreen = FALSE;  //已打开抓图窗口
BOOL    g_bImageLogging = FALSE;
BOOL    g_bImageExit = FALSE;

DWORD   g_dwImageCapID;
// CRemoteDesktopDlg 对话框

CRemoteDesktopDlg *m_remotedesktopdlg= NULL;

IMPLEMENT_DYNAMIC(CRemoteDesktopDlg, CDialogEx)

CRemoteDesktopDlg::CRemoteDesktopDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CRemoteDesktopDlg::IDD, pParent)
{
	m_remotedesktopdlg = this;
	m_remotedesktopdlg->ClientSocket = s;
	m_bChkContinue = FALSE;
}

CRemoteDesktopDlg::~CRemoteDesktopDlg()
{
}

void CRemoteDesktopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_picStartArea);
}


BEGIN_MESSAGE_MAP(CRemoteDesktopDlg, CDialogEx)
	ON_BN_CLICKED(IDC_FULL_SCR, &CRemoteDesktopDlg::OnClickedFullScr)
	ON_BN_CLICKED(IDC_SCR_CAP, &CRemoteDesktopDlg::OnClickedScrCap)
	ON_BN_CLICKED(IDC_SCR, &CRemoteDesktopDlg::OnClickedScr)
	ON_COMMAND(ID_MENUITEM_CLOSE, &CRemoteDesktopDlg::OnMenuitemClose)
END_MESSAGE_MAP()



DWORD WINAPI Set_BackGroud_Image()
{
   	if (m_remotedesktopdlg->m_lpImageData==NULL) return 1;

    HWND   hWnd=m_remotedesktopdlg->GetSafeHwnd();

	CRect rc;
	HWND Handle = GetDlgItem(hWnd,IDC_STATIC_PICTURE);//获取窗口主句柄  
	 CWnd *hwnd = CWnd::FromHandle(Handle);  
    hwnd->GetWindowRect(&rc);//获取picture control的指针，得到区间范围rect  
	CDC *theDC = m_remotedesktopdlg->m_picStatArea.GetWindowDC (); 
	if(theDC!= NULL)
    {
        int left = m_remotedesktopdlg->m_nLeft; //-m_hScrollBar.GetScrollPos();
        int top =  m_remotedesktopdlg->m_nTop;  //-m_vScrollBar.GetScrollPos();

		BYTE *tmp = m_remotedesktopdlg->m_lpImageData;

	// set up a DIB 
		BITMAPINFOHEADER bmiHeader;
		bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiHeader.biWidth = m_remotedesktopdlg->m_nBmpWidth; //1024/2;//
		bmiHeader.biHeight =m_remotedesktopdlg->m_nBmpHeight; // 768/2;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 24; //24
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = 0;
		bmiHeader.biXPelsPerMeter = 0;
		bmiHeader.biYPelsPerMeter = 0;
		bmiHeader.biClrUsed = 0;
		bmiHeader.biClrImportant = 0;
		int lines = StretchDIBits(theDC->m_hDC,
					rc.left,
					rc.top,
					rc.Width(),//bmiHeader.biWidth,//1024/2,//rc.Width(),//bmiHeader.biWidth/2,
				 	rc.Height(),//bmiHeader.biHeight,//768/2,//rc.Height(),///bmiHeader.biHeight/2,
					0,0,
					bmiHeader.biWidth,
					bmiHeader.biHeight,
					tmp,
					(LPBITMAPINFO)&bmiHeader,
					DIB_RGB_COLORS,
					SRCCOPY);
	
		ReleaseDC(hWnd,*theDC);
    }

	return 0;
}

int Get_Screen_Data(LPVOID lpBmpData,LPCOMMAND lpMsg)
{

	//step 1:设置显示区域大小

    BYTE *lpData = NULL, *lpData1 = NULL;
	COMMAND msg;
	CRect rcOld;

	lpData = (BYTE *)lpBmpData;

	rcOld.top = m_remotedesktopdlg->m_nTop;//lpMsg->rcArea.top;
	rcOld.bottom = m_remotedesktopdlg->m_nbottom;
	rcOld.left = m_remotedesktopdlg->m_nLeft;
	rcOld.right = m_remotedesktopdlg->m_nRight;
    
    
	if (rcOld.left < 0 || rcOld.top < 0 )
		int i = 0;

    //step 2: jpeg 解码

    lpData = m_remotedesktopdlg->m_Jpeg.JpegFileToRGB((BYTE *)lpBmpData+lpMsg->dwBmpInfoSize,
			                   lpMsg->dwBmpSize,
							   (UINT *)&m_remotedesktopdlg->m_nBmpWidth,
							   (UINT *)&m_remotedesktopdlg->m_nBmpHeight);

	if (lpData == NULL)	return 1;
	
    m_remotedesktopdlg->m_Jpeg.BGRFromRGB(lpData, m_remotedesktopdlg->m_nBmpWidth, m_remotedesktopdlg->m_nBmpHeight);
    m_remotedesktopdlg->m_Jpeg.VertFlipBuf(lpData, m_remotedesktopdlg->m_nBmpWidth * 3, m_remotedesktopdlg->m_nBmpHeight);



	m_remotedesktopdlg->m_nBmpTop= lpMsg->rcArea.top;
	m_remotedesktopdlg->m_nBmpLeft = lpMsg->rcArea.left;

	UINT m_widthDW;
	m_remotedesktopdlg->m_lpImageData = m_remotedesktopdlg->m_Jpeg.MakeDwordAlignedBuf((BYTE *)lpData,
							m_remotedesktopdlg->m_nBmpWidth,
							m_remotedesktopdlg->m_nBmpHeight,
							&m_widthDW);


	 //step 3:进入bmp图片显示线程
//	AfxMessageBox("进入bmp图片显示线程");
     Set_BackGroud_Image ();

	 return 0;
}

//打开抓图线程
DWORD WINAPI ImageCapThread()
{
	COMMAND SendMsg;

    CRect ClientRect = CRect(0,0,1024,768);
	CTcpTran m_tcptran;

	g_bImageLogging = TRUE;
	g_bImageExit = FALSE;
	
	HGLOBAL	hPackData = NULL, hBmpData = NULL, hBmpScr = NULL;
	LPVOID  lpPackData = NULL, lpBmpData = NULL, lpBmpScr = NULL;
	
	DWORD  dwOldSize = 0, dwSize,  dwBmpInfoSize, dwMaxSize = -1;

	char *lpData;
	CString strtmp;

	int nFirst = 0;

loop001:
	while(g_bImageLogging)
	{
		//Step 2:Send Get image command    
		memset(&SendMsg, 0, sizeof(COMMAND));
		SendMsg.wCmd = CMD_SCREEN_MANAGE;
		SendMsg.rcArea = ClientRect;
		SendMsg.nArea = 0; 
        SendMsg.nBits = 24;
		SendMsg.nDelay = 1000;
		SendMsg.nCompress = 2;
		SendMsg.nCell = nFirst;
		SendMsg.nJpegQ = 80;

		//Step 1: Send command
		if(!g_bImageLogging) goto Exit01;
		if (m_tcptran.mysend(m_remotedesktopdlg->ClientSocket,(char *)&SendMsg, sizeof(COMMAND),0,5) < 0)
		{
//			AfxMessageBox("1");
//			CString str;
//			str.Format("%d",m_remotedesktopdlg->ClientSocket);
//			AfxMessageBox(str);
			goto Err01;
		}
		//Step 2: Get image struct
//		AfxMessageBox("success");
		memset(&SendMsg,0, sizeof(COMMAND));
		if(!g_bImageLogging) goto Exit01;

		if (m_tcptran.myrecv(m_remotedesktopdlg->ClientSocket, (char *)&SendMsg, sizeof(SendMsg),0,60,0,false) < 0)
		{
//			AfxMessageBox("2");
			goto Err01;
		}

		if(SendMsg.dwBmpSize == 0) //图象无变化
		{	
			
			Get_Screen_Data(0,&SendMsg);
			goto loop001;
        }
		//分配内存
		dwBmpInfoSize = SendMsg.dwBmpInfoSize;
		dwSize = SendMsg.dwBmpSize;

		if (dwSize > dwMaxSize || lpBmpData == NULL)
		{
			if (lpBmpData) GlobalFree (lpBmpData);
			lpBmpData = GlobalAlloc (GMEM_FIXED, dwSize);
			dwMaxSize = dwSize;
		}
		if(lpBmpData == NULL) goto Err02;
		lpData = (char *)lpBmpData;


		if(!g_bImageLogging) goto Exit01;
		if (m_tcptran.myrecv(m_remotedesktopdlg->ClientSocket, lpData, dwSize,0,60,0,false) < 0) 
		{
//			AfxMessageBox("3");
			goto Err01;
		}
			nFirst = 1;
		
		//Step 5: Show image
		lpData = (char *)lpBmpData;
//	    AfxMessageBox("show image");
		Get_Screen_Data(lpBmpData, &SendMsg);

		if(!g_bImageLogging) goto Exit01;
		//Sleep(m_remotedesktopdlg->m_nEdtSecond);
		  Sleep(1500);

		
	} //end while
	
Exit01:
//	
	Sleep(1000);
	if (lpBmpData)
		GlobalFree(lpBmpData);
	g_bImageExit = TRUE;
	return 0;
	
Err02:		
    goto Exit01;
Err01:		
//	AfxMessageBox("ddddddddddddddddd");
    goto Exit01;   
}


// CRemoteDesktopDlg 消息处理程序

BOOL CRemoteDesktopDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	IsFullScreen=FALSE;
	m_bChkContinue = FALSE;
	
	
	GetDlgItem(IDC_SCR_CAP)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SCR)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_FULL_SCR)->ShowWindow(SW_SHOW);
	return true;
}


int SysEndImage()
{
    int i = 100;
	if(g_bImageLogging==FALSE)
		return 0;
	g_bImageLogging=FALSE; //退出
	if(WaitForSingleObject(g_hImageCapThread,5000)==WAIT_OBJECT_0)

	while(g_bImageExit==FALSE) {
		Sleep(15);  //等待退出
	    i--;
		if (i == 0) TerminateProcess(g_hImageCapThread, 0);
	}
	 return -1;	
}


int SysStartImage()
{

	if(g_bImageLogging == TRUE)
		return -1;
	
	g_hImageCapThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ImageCapThread,
					0,0,
					&g_dwImageCapID);

	if(g_hImageCapThread==NULL) 
	{
		return -1;
	}

	return 0;
}
void CRemoteDesktopDlg::OnClickedFullScr()
{
	// TODO: 在此添加控件通知处理程序代码
	TitleBar=new CTitleBar(AfxGetInstanceHandle(), this->GetSafeHwnd());
	if(IsFullScreen==FALSE)
		SetFullScreen(TRUE); //Set fullscreen on!
	else
		SetFullScreen(FALSE); //Set fullscreen on!
}


void CRemoteDesktopDlg::OnClickedScrCap()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bChkContinue=TRUE;
	if(m_bChkContinue)
	{
		 SysStartImage();
		//bStatus = FALSE;
	}
	else
	{
		 SysEndImage();
		//bStatus =TRUE;
	}
}


void CRemoteDesktopDlg::OnClickedScr()
{
	// TODO: 在此添加控件通知处理程序代码
	CTcpTran m_tcptran;

	COMMAND SendMsg;
	memset(&SendMsg, 0, sizeof(COMMAND));
	SendMsg.wCmd = CMD_GET_SCREEN_INFO;

	int tmp = m_tcptran.mysend(m_remotedesktopdlg->ClientSocket,(char *)&SendMsg,sizeof(COMMAND),0,60);

	if (tmp)
	{
		memset(&SendMsg, 0, sizeof(COMMAND));
        m_tcptran.myrecv(m_remotedesktopdlg->ClientSocket,(char *)&SendMsg,sizeof(COMMAND),0,60,0,false);

		m_nTop    = SendMsg.rcArea.top;
		m_nLeft   = SendMsg.rcArea.left;
        m_nbottom  = SendMsg.rcArea.bottom;
        m_nRight = SendMsg.rcArea.right;

		CString ffff;
		ffff.Format("%d - %d - %d - %d",m_nTop,m_nLeft,m_nbottom,m_nRight);

		AfxMessageBox(ffff);

	}
}


void CRemoteDesktopDlg::OnMenuitemClose()
{
	// TODO: 在此添加命令处理程序代码
	::SendMessage((HWND)this,WM_CLOSE,0,0);
}

void CRemoteDesktopDlg::SetFullScreen(BOOL OnOff)
{
	LONG style = GetWindowLong(m_hWnd, GWL_STYLE);

	if(OnOff==TRUE)
	{
		::ShowWindow(m_hWnd, SW_MAXIMIZE);
		style = GetWindowLong(m_hWnd, GWL_STYLE);
		style &= ~(WS_DLGFRAME | WS_THICKFRAME);
		SetWindowLong(m_hWnd, GWL_STYLE, style);
		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, -1, -1, cx+3, cy+3, SWP_FRAMECHANGED);
		IsFullScreen=TRUE;
		TitleBar->DisplayWindow(TRUE, TRUE);
		::InvalidateRect(m_hWnd,NULL,TRUE);
	}
	else //OFF!
	{
		style |= WS_DLGFRAME | WS_THICKFRAME;
		SetWindowLong(m_hWnd, GWL_STYLE, style);
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0,0,100,100, SWP_NOMOVE | SWP_NOSIZE);
		::ShowWindow(m_hWnd, SW_NORMAL);
		IsFullScreen=FALSE;
		TitleBar->DisplayWindow(FALSE, TRUE);
		::InvalidateRect(m_hWnd,NULL,TRUE);
	}
	

}
