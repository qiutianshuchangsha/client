// ProcManageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ProcManageDlg.h"
#include "afxdialogex.h"
#include "comm.h"
#include "TcpTran.h"
#include "resource.h"


static int nrItemNum = 0; 
static DWORD procitem = 0;

CProcManageDlg *m_procmanagedlg = NULL;

DWORD WINAPI InitList(std::vector<PROCESSINFO *> pVecTor)
{
	
	//std::vector<PROCESSINFO> *pVecTor = (std::vector<PROCESSINFO>) lp;
    //std::vector<PROCESSINFO> *pVecTor =(vector<PROCESSINFO> ) lp;
 
	m_procmanagedlg->m_list.DeleteAllItems();
	for(DWORD i = 0; i < pVecTor.size(); i++)
	{
        CString tmp = _T("");

		tmp.Format("%d",pVecTor[i]->PID); //****得到vector[i] 里的结构的一部分**** //pVecTor->at(i).PID
	
	    m_procmanagedlg->m_list.InsertItem(i,"");
	    m_procmanagedlg->m_list.SetItemText(i,0,tmp);
    
	    tmp.Format("%s",pVecTor[i]->ProcName);
	
	//    m_procmanagedlg->m_list.InsertItem(i,(const char *)1);
 	    m_procmanagedlg->m_list.SetItemText(i,1,tmp);
	
	     tmp.Format("%s",pVecTor[i]->ProcPath); 
	
	 //   m_procmanagedlg->m_list.InsertItem(i,(const char *)2);
	    m_procmanagedlg->m_list.SetItemText(i,2,tmp);
			
	}

	return 0;
}

// CProcManageDlg 对话框

IMPLEMENT_DYNAMIC(CProcManageDlg, CDialogEx)

CProcManageDlg::CProcManageDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CProcManageDlg::IDD, pParent)
{
	m_procmanagedlg=this;
	m_procmanagedlg->ClientSocket = s;
//	CString str;
//	str.Format("%d",m_procmanagedlg->ClientSocket);
//	AfxMessageBox(str);
}

CProcManageDlg::~CProcManageDlg()
{
}

void OnStart()
{
	COMMAND m_command;
	
	int len = 0;

    //extern  SOCKET g__clientsocket ;
	memset((char  *)&m_command, 0,sizeof(m_command));

    m_command.wCmd = CMD_PROCESS_MANAGE;
    m_command.DataSize = 0;
	CTcpTran m_tcptran ; 

    int buf = 0; 
    
	len = m_tcptran.mysend(m_procmanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);

    if (len<0)
	{
		len = m_tcptran.mysend(m_procmanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);
    }
	
	
	int processlen = m_tcptran.myrecv(m_procmanagedlg->ClientSocket,(char *)&buf,sizeof(int),0,60,NULL,false);
	
	if (processlen>0)
	{
		
		std::vector<PROCESSINFO *> pProcInfo;

        PROCESSINFO *tmp = new PROCESSINFO; //一样的问题 
		// 
 
        for(int i=0;i<buf;i++)
		{
			tmp = new PROCESSINFO;
			memset(tmp, 0,sizeof(PROCESSINFO));
			m_tcptran.myrecv(m_procmanagedlg->ClientSocket,(char *)tmp,sizeof(PROCESSINFO),0,60,0,false);			
			
			pProcInfo.push_back(tmp);

			//delete tmp;
			
		}
		
		InitList(pProcInfo);
		
	}
}

BOOL CProcManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	LONG lStyle = m_list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
	lStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	m_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0,(LPARAM)lStyle);
	LV_COLUMN lvc;
	
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH                            /*| LVCF_FMT*/;
	
	m_list.DeleteAllItems();
 	m_list.InsertColumn(0,"进程ID",LVCFMT_LEFT,50);  
 	m_list.InsertColumn(1,"进程名称",LVCFMT_LEFT,110);
 	m_list.InsertColumn(2,"路径",LVCFMT_LEFT,360); 
	//m_procmanagedlg->m_list.InsertItem(0,"xxxxxxxxxxxx");
	ShowWindow(SW_NORMAL);
	OnStart();

	return TRUE;

}

void CProcManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CProcManageDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CProcManageDlg::OnClickList1)
	ON_COMMAND(ID_PROC_KILL, &CProcManageDlg::OnProcKill)
	ON_COMMAND(ID_PROC_REFRESH, &CProcManageDlg::OnProcRefresh)
END_MESSAGE_MAP()


// CProcManageDlg 消息处理程序


void CProcManageDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_PROC));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		CWnd* pWndPopupOwner = this;

		while (pWndPopupOwner->GetStyle() & WS_CHILD)
			pWndPopupOwner = pWndPopupOwner->GetParent();

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
}


void CProcManageDlg::OnClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
//	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: 在此添加控件通知处理程序代码
	if(pNMListView->iItem != -1)
    {
		//调试用代码
		////////////////////////////////////////////////////////////////////////
        //        CString strtemp;
        //        strtemp.Format("单击的是第%d行第%d列",
        //                        pNMListView->iItem, pNMListView->iSubItem);
        //        AfxMessageBox(strtemp);
		////////////////////////////////////////////////////////////////////////
                procitem  = pNMListView->iItem;
    }
	*pResult = 0;
}


void CProcManageDlg::OnProcKill()
{
	// TODO: 在此添加命令处理程序代码
	COMMAND m_command;

	int len = 0;

    //extern  SOCKET g__clientsocket ;
	memset((char  *)&m_command, 0,sizeof(m_command));

    m_command.wCmd = CMD_PROCESS_KILL;
   
    // get PID 		
	CString id;

	DWORD tmp_pid = 0;

	id = m_list.GetItemText(procitem,0);

//	id.Format("%d",tmp_pid);

	tmp_pid = _ttoi(id);

	m_command.DataSize = tmp_pid;

	CTcpTran m_tcptran ; 

    int buf = 0; 
    
	len = m_tcptran.mysend(m_procmanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);

    if (len<0)
	{
		len = m_tcptran.mysend(m_procmanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);    
    }

	int processlen = m_tcptran.myrecv(m_procmanagedlg->ClientSocket,(char *)&buf,sizeof(int),0,60,NULL,false);

	if (processlen>0)
	{
		
		std::vector<PROCESSINFO *> pProcInfo;

        PROCESSINFO *tmp = new PROCESSINFO; //一样的问题 
		// 
 
        for(int i=0;i<buf;i++)
		{
			tmp = new PROCESSINFO;
			memset(tmp, 0,sizeof(PROCESSINFO));
			m_tcptran.myrecv(m_procmanagedlg->ClientSocket,(char *)tmp,sizeof(PROCESSINFO),0,60,0,false);			
			
			pProcInfo.push_back(tmp);

			//delete tmp;
			
		}

		InitList(pProcInfo);
		
	}
}


void CProcManageDlg::OnProcRefresh()
{
	// TODO: 在此添加命令处理程序代码
	OnStart();
}
