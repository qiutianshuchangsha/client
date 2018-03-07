// FileManageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client.h"
#include "FileManageDlg.h"
#include "afxdialogex.h"
#include "TcpTran.h"
#include "resource.h"
#include "comm.h"
#include "shellapi.h"

// CFileManageDlg 对话框
CWinThread *pDriverInfoThread			= NULL;
CWinThread *pListDirThread              = NULL;

CFileManageDlg *m_filemanageDlg = NULL;

UINT ListDirThread(LPVOID lparam)
{
	CTcpTran m_tcptran;
	CFileManageDlg *pDlg=(CFileManageDlg *)lparam;
    pDlg->DeleteTreeChildItem(pDlg->TreeNode);
	pDlg->m_list.DeleteAllItems();

	if(pDlg->ClientSocket!=INVALID_SOCKET)
	{
		pDlg->m_list.SetRedraw(FALSE);
  //      pDlg->m_list.DrawSearch(TRUE);
		COMMAND m_command;
		memset(&m_command,0,sizeof(COMMAND));
		m_command.wCmd = CMD_FILE_GETSUBFILE;

        strcpy(m_command.szCurDir , pDlg->chrPath );

		m_tcptran.mysend(pDlg->ClientSocket,(char *)&m_command,sizeof(COMMAND),0,60);//枚举文件
		FILEINFO fileinfo;
	    do
		{
		   m_tcptran.myrecv(pDlg->ClientSocket,(char *)&fileinfo,sizeof(FILEINFO),0,60,NULL,false);//接收目标数据
		   if(fileinfo.next ==0)
			  break;

		   HTREEITEM hit;
		   if(fileinfo.isdirectory ==1)
		   {
			    if(fileinfo.filename [0]=='.')
				    continue;
			    hit=pDlg->m_tree.InsertItem(fileinfo.filename,3,4,pDlg->TreeNode,TVI_LAST);
			    pDlg->m_tree.InsertItem(NULL,hit,TVI_LAST);			
		   }
		   else
		   {
			   int iIcon;
			   CString icon;
			   icon.Format("%s",fileinfo.filename);
			   pDlg->GetFileIcon(icon,&iIcon);
			   int n =pDlg->m_list.GetItemCount();
	           pDlg->m_list.InsertItem(n,"",iIcon);
		       CString kb;
			   kb.Format("%dK",fileinfo.filesize);
	           pDlg->m_list.SetItemText(n,0,fileinfo.filename);
			   pDlg->m_list.SetItemText(n,1,kb);
			   pDlg->m_list.SetItemText(n,2,fileinfo.time);			
		   }
		}while(fileinfo.invalidir!=1);
        pDlg->PostMessage(STOP_MESSAGE);
		pDlg->m_list.SetRedraw(TRUE);
		pDlg->m_list.Invalidate();
	}
	pListDirThread = NULL;
	return 0;	
}

//DriverInfo线程
UINT DriverInfoThread(LPVOID lparam)
{
	CTcpTran m_tcptran;
	CFileManageDlg *pDlg=(CFileManageDlg *)lparam;

	if(pDlg->ClientSocket!=INVALID_SOCKET)
	{  
	    int ret;
		DRIVER driver;

		COMMAND m_command;
		m_command.wCmd = CMD_FILE_MANAGE;

		ret=m_tcptran.mysend(pDlg->ClientSocket,(char *)&m_command,sizeof(COMMAND),0,60);//获得盘符信息

	    pDlg->m_tree.DeleteAllItems();
	    pDlg->m_list.DeleteAllItems();
		while(ret>0)
		{   HTREEITEM hit;
			ret=m_tcptran.myrecv(pDlg->ClientSocket,(char *)&driver,sizeof(DRIVER),0,60,NULL,false);//接收目标数据
			if(ret<=0||driver.end)
				break;
            //表示接收到数据
			if(driver.drivertype==DRIVE_FIXED)
			{
            hit=pDlg->m_tree.InsertItem(driver.driver,0,0,TVI_ROOT,TVI_LAST);
			pDlg->m_tree.InsertItem(NULL,hit,TVI_LAST);
			}
			if(driver.drivertype==DRIVE_CDROM)
			{
            hit=pDlg->m_tree.InsertItem(driver.driver,1,1,TVI_ROOT,TVI_LAST);
			}
			if(driver.drivertype==DRIVE_REMOVABLE)
			{
            hit=pDlg->m_tree.InsertItem(driver.driver,2,2,TVI_ROOT,TVI_LAST);
			pDlg->m_tree.InsertItem(NULL,hit,TVI_LAST);
			}
		}
	}
	pDriverInfoThread = NULL;
	return 0;	
}

IMPLEMENT_DYNAMIC(CFileManageDlg, CDialogEx)

CFileManageDlg::CFileManageDlg(SOCKET s,CWnd* pParent /*=NULL*/)
	: CDialogEx(CFileManageDlg::IDD, pParent)
{
	m_filemanageDlg = this;
	m_filemanageDlg->ClientSocket = s;
}

CFileManageDlg::~CFileManageDlg()
{
}

void CFileManageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_tree);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CFileManageDlg, CDialogEx)
	
	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CFileManageDlg::OnClickTree1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CFileManageDlg::OnRclickList1)

    ON_MESSAGE(BEGIN_MESSAGE, &CFileManageDlg::OnBeginMessage)
    ON_MESSAGE(STOP_MESSAGE, &CFileManageDlg::OnStopMessage)
	ON_COMMAND(ID_FILE_DEL, &CFileManageDlg::OnFileDel)
END_MESSAGE_MAP()


// CFileManageDlg 消息处理程序



void CFileManageDlg::OnClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	POINT ptMouse;
	GetCursorPos(&ptMouse);
	m_tree.ScreenToClient(&ptMouse);
	HTREEITEM TreeSel=m_tree.HitTest(ptMouse,0);
	if(!TreeSel)
		return;
	GetItPath(TreeSel,chrPath);//得到当前展开的路径
	TreeNode=TreeSel;
	CString str;
	str.Format("%s",chrPath);
	str=str.Left(str.ReverseFind('*'));
	GetDlgItem(IDC_CURRENT)->SetWindowText(str);

	if(pDriverInfoThread != NULL)
	{
	  ::TerminateThread(pDriverInfoThread->m_hThread, 0);
	  ::WaitForSingleObject(pDriverInfoThread->m_hThread, INFINITE);
	  CloseHandle(pDriverInfoThread->m_hThread);
	  pDriverInfoThread = NULL;
	}

    if(pListDirThread != NULL)
	{
	  ::TerminateThread(pListDirThread->m_hThread, 0);
	  ::WaitForSingleObject(pListDirThread->m_hThread, INFINITE);
	  CloseHandle(pListDirThread->m_hThread);
	  pListDirThread = NULL;
	}

    pListDirThread = AfxBeginThread(ListDirThread, (LPVOID)this);
	PostMessage(BEGIN_MESSAGE);
	*pResult = 0;
}


void CFileManageDlg::OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POINT ptMouse;
	POINT ptClientMouse;

	GetCursorPos(&ptMouse);

	ptClientMouse=ptMouse;
	m_list.ScreenToClient(&ptClientMouse);
	int ListSel=m_list.HitTest(ptClientMouse,0);
	if(ListSel==-1)
		return;

	UpdateData(TRUE);
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_FILE));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);	
	*pResult = 0;
}


afx_msg LRESULT CFileManageDlg::OnBeginMessage(WPARAM wParam, LPARAM lParam)
{
//	m_list.DrawSearch(TRUE);
	return 1;
}


afx_msg LRESULT CFileManageDlg::OnStopMessage(WPARAM wParam, LPARAM lParam)
{
//	m_list.DrawSearch(FALSE);
	return 1;
}



void CFileManageDlg::OnFileDel()
{
	// TODO: 在此添加命令处理程序代码
	 char FilePath[MAX_PATH];
	CString path;
	GetDlgItem(IDC_CURRENT)->GetWindowText(path);
	strcpy(FilePath,path);
	POSITION pos =m_list.GetFirstSelectedItemPosition();
	int i=m_list.GetNextSelectedItem(pos);
	if(i>=0)//选择某项			
	strcat(FilePath,m_list.GetItemText(i,0));
	// TODO: Add your command handler code here
    CTcpTran m_tcptran;
	COMMAND m_command;
    m_command.wCmd = CMD_FILE_DEL;
	strcpy(m_command.szCurDir,FilePath); 
	m_tcptran.mysend(ClientSocket,(char *)&m_command,sizeof(COMMAND),0,60);

	if(pDriverInfoThread != NULL)
	{
	  ::TerminateThread(pDriverInfoThread->m_hThread, 0);
	  ::WaitForSingleObject(pDriverInfoThread->m_hThread, INFINITE);
	  CloseHandle(pDriverInfoThread->m_hThread);
	  pDriverInfoThread = NULL;
	}
   if(pListDirThread != NULL)
	{
	  ::TerminateThread(pListDirThread->m_hThread, 0);
	  ::WaitForSingleObject(pListDirThread->m_hThread, INFINITE);
	  CloseHandle(pListDirThread->m_hThread);
	  pListDirThread = NULL;
	}

	pListDirThread = AfxBeginThread(ListDirThread, (LPVOID)this);
}

void CFileManageDlg::DeleteTreeChildItem(HTREEITEM hit)
{
	HTREEITEM hchild;
	hchild=m_tree.GetChildItem(hit);
	while(hchild!=0)
	{
		m_tree.DeleteItem(hchild);
		hchild=m_tree.GetChildItem(hit);
	}	

}

void CFileManageDlg::GetFileIcon(CString &fileName, int *iIcon)
{
	CString str=fileName;
	SHFILEINFO shfi;
	memset(&shfi,0,sizeof(shfi));
	if(!SHGetFileInfo(str,FILE_ATTRIBUTE_NORMAL,&shfi,sizeof(shfi),SHGFI_ICON|SHGFI_SMALLICON|SHGFI_USEFILEATTRIBUTES))
	{
		AfxMessageBox("@@@@");
		return;
	}
	*iIcon=shfi.iIcon;
	DestroyIcon(shfi.hIcon);
	return ;
	
}

void CFileManageDlg::GetItPath(HTREEITEM hitem, char *path)
{
	char tpath[MAX_PATH]="\0";
	char chrtmp[MAX_PATH]="\0";
	do{
		strcpy(chrtmp,m_tree.GetItemText(hitem));
		strcat(chrtmp,"\\");
		strcat(chrtmp,tpath);
		strcpy(tpath,chrtmp);
		hitem=m_tree.GetParentItem(hitem);
	}while(hitem!=0);
	strcat(tpath,"*");
	strcpy(path,tpath);	

}

BOOL CFileManageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_ImageList1.Create(16,16,ILC_COLOR16|ILC_MASK,2,2);
	HICON hIcon0 = ::LoadIcon (::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON1));
	m_ImageList1.Add(hIcon0);
	HICON hIcon1 = ::LoadIcon (::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON2));
	m_ImageList1.Add(hIcon1);
	HICON hIcon2 = ::LoadIcon (::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON3));
	m_ImageList1.Add(hIcon2);
	HICON hIcon3 = ::LoadIcon (::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON4));
	m_ImageList1.Add(hIcon3);
	HICON hIcon4 = ::LoadIcon (::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON5));
	m_ImageList1.Add(hIcon4);
	m_tree.SetImageList(&m_ImageList1,TVSIL_NORMAL);//添加图标
	
	// TODO: Add extra initialization here
	LONG lStyle = m_list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
	lStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	m_list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0,(LPARAM)lStyle);
	LV_COLUMN lvc;
	
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH /*| LVCF_FMT*/;
	
	m_list.DeleteAllItems();
 	m_list.InsertColumn(0,"文件名",LVCFMT_LEFT,130);  
 	m_list.InsertColumn(1,"文件大小",LVCFMT_LEFT,90);
 	m_list.InsertColumn(2,"修改时间",LVCFMT_LEFT,120);


//备注：字符串结尾\0设置的问题 初始化时要更新

	//创建ImageList
    HIMAGELIST hImageList;
	//定义文件信息变量
	SHFILEINFO shFi;
	//获取文件图标信息
	char cSysDir[100],dir[4]={0};
    GetSystemDirectory(cSysDir,100);
	strncpy(dir,cSysDir,3);

    //得到系统盘符
	hImageList = (HIMAGELIST)SHGetFileInfo(dir,0,&shFi,sizeof(shFi),
		SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
	//将文件图象列表存入m_ctImageList
	m_ImageList2.m_hImageList = hImageList;
	m_list.SetImageList(&m_ImageList2,LVSIL_SMALL);//添加图标


	pDriverInfoThread = AfxBeginThread(DriverInfoThread, (LPVOID)this);

	return TRUE;
}
