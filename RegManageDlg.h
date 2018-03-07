#pragma once
#include "afxcmn.h"


// CRegManageDlg 对话框

class CRegManageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRegManageDlg)

public:
	BOOL OnInitDialog();
	SOCKET ClientSocket;
	CRegManageDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegManageDlg();

// 对话框数据
	enum { IDD = IDD_REG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	CTreeCtrl m_tree;
};
