#pragma once
#include "afxwin.h"


// CServicManageEditDlg 对话框

class CServicManageEditDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServicManageEditDlg)

public:
	BOOL OnInitDialog();
	SOCKET m_ClientSocket;

	CString  ServiceName;
	CString  ServiceState;
    CString  ServiceStartType;
	CServicManageEditDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServicManageEditDlg();

// 对话框数据
	enum { IDD = IDD_SERVICE_MANAGE_SUB };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_combox_state;
	CComboBox m_combox_autoload;
};
