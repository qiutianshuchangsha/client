
// ClientDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = NULL);	// 标准构造函数
//	CClientDlg *m_clientdlg;

// 对话框数据
	enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFileCtrl();
	afx_msg void OnKeyLog();
	afx_msg void OnProcCtrl();
	afx_msg void OnRegCtrl();
	afx_msg void OnRemoteDeskCtrl();
	afx_msg void OnRemoteShell();
	afx_msg void OnServCtrl();
	afx_msg void OnVideo();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CButton m_btn_start;
	CButton m_btn_stop;
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
