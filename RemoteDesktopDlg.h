#pragma once
#include "Jpegfile.h"
#include "FullScreenTitleBar.h"
#include "afxwin.h"


// CRemoteDesktopDlg 对话框

class CRemoteDesktopDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRemoteDesktopDlg)

public:
	void SetFullScreen(BOOL OnOff);
	BOOL OnInitDialog();
	SOCKET ClientSocket;
	CRemoteDesktopDlg(SOCKET s,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRemoteDesktopDlg();

// 对话框数据
	CTitleBar *TitleBar;
	JpegFile m_Jpeg;
	HDC	 m_hMemDC;

	CRect       m_rcArea;
	HGLOBAL		m_lpBmpData;
	BYTE        *m_lpImageData;

	DWORD		m_dwBmpSize;
	DWORD	    m_dwBmpInfoSize;

	int         m_nBmpTop, m_nBmpLeft;
    int			m_nBmpWidth, m_nBmpHeight;
	int         m_nTop, m_nLeft;
	//int         m_nWidth, m_nHeight;
	int         m_nbottom, m_nRight;
	enum { IDD = IDD_REMOTEDESKTOP };
	CStatic	m_picStatArea;
	BOOL	m_bChkContinue;

protected:
	BOOL IsFullScreen;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_picStartArea;
	afx_msg void OnClickedFullScr();
	afx_msg void OnClickedScrCap();
	afx_msg void OnClickedScr();
	afx_msg void OnMenuitemClose();
};
