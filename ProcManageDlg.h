#pragma once

#include <vector>
#include "afxcmn.h"

typedef struct tagProcessInfo
{
	DWORD PID;
	char  ProcName[64];
	char  ProcPath[128];

}PROCESSINFO,*LPPROCESSINFO;
// CProcManageDlg 对话框

class CProcManageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CProcManageDlg)

public:
	CProcManageDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcManageDlg();


// 对话框数据
	enum { IDD = IDD_PROC_MANAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProcKill();
	afx_msg void OnProcRefresh();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	SOCKET ClientSocket;
	
};
