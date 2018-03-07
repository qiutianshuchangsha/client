
#pragma once
#include "afxcmn.h"
#include "searchlistctrl.h"

//自定义消息
#define BEGIN_MESSAGE			    WM_USER + 0x77
#define STOP_MESSAGE			    WM_USER + 0x88


// CFileManageDlg 对话框

class CFileManageDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileManageDlg)

public:
	void GetFileIcon(CString &fileName, int *iIcon);
	void DeleteTreeChildItem(HTREEITEM hit);
	void GetItPath(HTREEITEM hitem, char *path);
	BOOL OnInitDialog();
	CFileManageDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileManageDlg();

	char chrPath[MAX_PATH];
	HTREEITEM TreeNode;
	CImageList m_ImageList1;
	CImageList m_ImageList2;

	SOCKET ClientSocket;
	CImageList	m_ImageList;


// 对话框数据
	enum { IDD = IDD_FILE_MANAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CTreeCtrl m_tree;
	CSearchListCtrl m_list;
	afx_msg void OnClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFileDel();
	afx_msg LRESULT OnBeginMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopMessage(WPARAM wParam, LPARAM lParam);
	
};
