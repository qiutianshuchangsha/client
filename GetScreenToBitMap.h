

#if !defined(AFX_GETSCREENTOBITMAP_H__576574FF_E7C0_4C9D_8258_90A53B957D89__INCLUDED_)
#define AFX_GETSCREENTOBITMAP_H__576574FF_E7C0_4C9D_8258_90A53B957D89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Huffman.H"
class CGetScreenToBitMap
{
public:
	LPSTR FAR	m_lpBmpData;
	DWORD		m_dwBmpSize;
	DWORD		m_dwBmpInfoSize;
	CRect m_rcArea;
	void SetArea(CRect rcArea,int nArea);
	DWORD GetImageSize(void);
	LPSTR FAR GetImage(void);
	void ResetVariable(void);
	BOOL GetScreen(CRect rcArea, int nBits, int nArea);
	CGetScreenToBitMap(void);
	~CGetScreenToBitMap(void);
};
#endif

