#pragma once
#include "afxcmn.h"


// CServiceManageDlg 对话框

class CServiceManageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServiceManageDlg)

public:
	BOOL OnInitDialog();
	CServiceManageDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServiceManageDlg();

// 对话框数据
	enum { IDD = IDD_SERVICE_MANAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	SOCKET ClientSocket;
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnServiceRefresh();
	afx_msg void OnServSet();
	afx_msg void OnServDelete();
	afx_msg void OnClickList1(NMHDR *pNMHDR, LRESULT *pResult);
};
