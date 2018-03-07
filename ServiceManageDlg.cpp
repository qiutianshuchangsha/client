// ServiceManageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "ServiceManageDlg.h"
#include "afxdialogex.h"
#include "comm.h"
#include "TcpTran.h"
#include "ServicManageEditDlg.h"

CServiceManageDlg  *m_servicemanagedlg = NULL;

static int Svritem = 0;

DWORD WINAPI InitListService(std::vector<SERVICEINFO *> pServInfo)
{
	m_servicemanagedlg->m_list.DeleteAllItems();
	for(DWORD i = 0; i < pServInfo.size(); i++)
	{
        CString tmp = _T("");
		m_servicemanagedlg->m_list.InsertItem(i,"");
   
	    tmp.Format("%s",pServInfo[i]->ServiceName);
 	    m_servicemanagedlg->m_list.SetItemText(i,0,tmp);
	
	    tmp.Format("%s",pServInfo[i]->ServiceDetail); 
	    m_servicemanagedlg->m_list.SetItemText(i,1,tmp);

	    tmp.Format("%s",pServInfo[i]->ServiceState); 
	    m_servicemanagedlg->m_list.SetItemText(i,2,tmp);
	     
		tmp.Format("%s",pServInfo[i]->ServiceStartType); 
	    m_servicemanagedlg->m_list.SetItemText(i,3,tmp);

	}

	return 0;
}

// CServiceManageDlg 对话框

IMPLEMENT_DYNAMIC(CServiceManageDlg, CDialogEx)

void OnServiceStart()
{

	COMMAND m_command;

	int len = 0;

    //extern  SOCKET g__clientsocket ;
	memset((char  *)&m_command, 0,sizeof(m_command));

    m_command.wCmd = CMD_SERVICE_MANAGE;
    m_command.DataSize = 0;
	CTcpTran m_tcptran ; 

    int sbuf = 0; 
    
	len = m_tcptran.mysend(m_servicemanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);

    if (len<0)
	{
		len = m_tcptran.mysend(m_servicemanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);    
    }

	int servicelen = m_tcptran.myrecv(m_servicemanagedlg->ClientSocket,(char *)&sbuf,sizeof(int),0,60,NULL,false);

	if (servicelen>0)
	{
		std::vector<SERVICEINFO *> pServiceInfo;
        SERVICEINFO *tmp = new SERVICEINFO; 
        for(int i=0;i<sbuf;i++)
		{
			tmp = new SERVICEINFO;
			memset(tmp, 0,sizeof(SERVICEINFO));
			m_tcptran.myrecv(m_servicemanagedlg->ClientSocket,(char *)tmp,sizeof(SERVICEINFO),0,60,0,false);			
			pServiceInfo.push_back(tmp);		
		}
		InitListService(pServiceInfo);		
	}
}

CServiceManageDlg::CServiceManageDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CServiceManageDlg::IDD, pParent)
{
	m_servicemanagedlg = this;
	m_servicemanagedlg->ClientSocket = s;
}

CServiceManageDlg::~CServiceManageDlg()
{
}

void CServiceManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CServiceManageDlg, CDialogEx)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID__SERVICE_REFRESH, &CServiceManageDlg::OnServiceRefresh)
	ON_COMMAND(ID__SERV_SET, &CServiceManageDlg::OnServSet)
	ON_COMMAND(ID__SERV_DELETE, &CServiceManageDlg::OnServDelete)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CServiceManageDlg::OnClickList1)
END_MESSAGE_MAP()


// CServiceManageDlg 消息处理程序


void CServiceManageDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_SERVICE));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		CWnd* pWndPopupOwner = this;

		while (pWndPopupOwner->GetStyle() & WS_CHILD)
			pWndPopupOwner = pWndPopupOwner->GetParent();

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
	}

}


void CServiceManageDlg::OnServiceRefresh()
{
	// TODO: 在此添加命令处理程序代码
	OnServiceStart();
}


void CServiceManageDlg::OnServSet()
{
	// TODO: 在此添加命令处理程序代码
	CServicManageEditDlg m_servicemanageeditdlg;

	m_servicemanageeditdlg.m_ClientSocket = ClientSocket;
	
	CString id;
    m_servicemanageeditdlg.ServiceName=m_list.GetItemText(Svritem,0); 
	m_servicemanageeditdlg.ServiceState=m_list.GetItemText(Svritem,2); 
	m_servicemanageeditdlg.ServiceStartType=m_list.GetItemText(Svritem,3); 
	m_servicemanageeditdlg.DoModal();

}


void CServiceManageDlg::OnServDelete()
{
	// TODO: 在此添加命令处理程序代码
	COMMAND m_command;
	int len = 0;
	memset((char  *)&m_command, 0,sizeof(m_command));
    m_command.wCmd = CMD_SERVICE_DEL;		
	CString id;
	DWORD tmp_Serviceid = 0;
	id = m_list.GetItemText(Svritem,0);
	char *m_chsvr = (char *)(LPCSTR)m_list.GetItemText(Svritem,0); 
	strcpy(m_command.tmp,m_chsvr);
	CTcpTran m_tcptran ; 
    int sbuf = 0; 
	len = m_tcptran.mysend(m_servicemanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);
    if (len<0)
	{
		len = m_tcptran.mysend(m_servicemanagedlg->ClientSocket,(char *)&m_command,sizeof(m_command),0,60);    
    }

	int Svrlen = m_tcptran.myrecv(m_servicemanagedlg->ClientSocket,(char *)&sbuf,sizeof(int),0,60,NULL,false);

	if (Svrlen>0)
	{
		std::vector<SERVICEINFO *> pServiceInfo;
        SERVICEINFO *tmp = new SERVICEINFO; 
        for(int i=0;i<sbuf;i++)
		{
			tmp = new SERVICEINFO;
			memset(tmp, 0,sizeof(SERVICEINFO));
			m_tcptran.myrecv(m_servicemanagedlg->ClientSocket,(char *)tmp,sizeof(SERVICEINFO),0,60,0,false);						
			pServiceInfo.push_back(tmp);		
		}	
		InitListService(pServiceInfo);	
	}
}



BOOL CServiceManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	LONG mlStyle = m_list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
	mlStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	m_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0,(LPARAM)mlStyle);
	LV_COLUMN lvc;
	
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH /*| LVCF_FMT*/;
	
	m_list.DeleteAllItems();
    m_list.InsertColumn(0,"服务名称",LVCFMT_LEFT,120);
	m_list.InsertColumn(1,"描述",LVCFMT_LEFT,270);
	m_list.InsertColumn(2,"状态",LVCFMT_LEFT,80);
	m_list.InsertColumn(3,"启动类型",LVCFMT_LEFT,80);

	OnServiceStart();

	return TRUE;
}



void CServiceManageDlg::OnClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	*pResult = 0;
}
