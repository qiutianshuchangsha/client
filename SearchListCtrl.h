#pragma once
#include "afxcmn.h"
class CSearchListCtrl :
	public CListCtrl
{
public:
	CSearchListCtrl(void);
//	void DrawSearch(BOOL flag);
	CAnimateCtrl *pavi;

	~CSearchListCtrl(void);
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

