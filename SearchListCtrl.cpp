#include "StdAfx.h"
#include "SearchListCtrl.h"


CSearchListCtrl::CSearchListCtrl(void)
{
	 pavi=NULL;
}


CSearchListCtrl::~CSearchListCtrl(void)
{
}
BEGIN_MESSAGE_MAP(CSearchListCtrl, CListCtrl)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


HBRUSH CSearchListCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CListCtrl::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	pDC->SetBkMode(TRANSPARENT);
	HBRUSH hb=CreateSolidBrush(RGB(255,255,255));
	pDC->SetBkColor(RGB(255,255,255));
	return hb;
	
}
