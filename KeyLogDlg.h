#pragma once
#include "afxwin.h"


// CKeyLogDlg 对话框

class CKeyLogDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CKeyLogDlg)

public:
	BOOL OnInitDialog();
	CKeyLogDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CKeyLogDlg();
	SOCKET ClientSocket;

// 对话框数据
	enum { IDD = IDD_KEYLOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_logEdit;
	CButton m_btn_startkeylog;
	CButton m_btn_show;
	CButton m_btn_stopkeylog;
	afx_msg void OnClickedStart();
	afx_msg void OnClickedStop();
	afx_msg void OnClickedShow();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
