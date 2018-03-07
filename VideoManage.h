#pragma once


// VideoManage 对话框

class CVideoManage : public CDialogEx
{
//	DECLARE_DYNAMIC(CVideoManage)

public:
	SOCKET ClientSocket;
	CVideoManage(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CVideoManage();

// 对话框数据
	enum { IDD = IDD_VIDEO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
