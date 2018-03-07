#pragma once

#if !defined(AFX_JPEGFILE_H__7B4DD98B_945B_11D2_815A_444553540000__INCLUDED_)
#define AFX_JPEGFILE_H__7B4DD98B_945B_11D2_815A_444553540000__INCLUDED_

//#define HAVE_BOOLEAN
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

class JpegFile
{
public:
	unsigned long m_lWidth;
	unsigned long m_lHeight;
	unsigned long  m_dwPoint;
	DWORD m_dwOutMaxSize;
    BYTE * m_lpOutBuffer;
    BYTE * m_lpPreData;
	DWORD m_dwScreenMaxSize;
    BYTE * m_lpScreenBuffer;
	DWORD m_dwFrame;
    bool m_bOpening;
	CFile m_rFile;

	JpegFile(void);
	~JpegFile(void);
	 BYTE * JpegFileToRGB(BYTE *dataBuf,			// path to image
		                       unsigned long size,
							   UINT *width,					// image width in pixels
							   UINT *height);				// image height

	  BOOL RGBToJpegFile(				// path
							BYTE *dataBuf,					// RGB buffer
							BYTE *outBuf,
							UINT width,						// pixels
							UINT height,					// rows
							BOOL color,						// TRUE = RGB
															// FALSE = Grayscale
							int quality,
							unsigned long *size);					// 0 - 100

	  BOOL GetJPGDimensions(CString fileName,			// path
								UINT *width,				// pixels
								UINT *height);

	  BYTE * MakeDwordAlignedBuf(BYTE *dataBuf,			// input buf
									 UINT widthPix,				// input pixels
									 UINT height,				// lines
									 UINT *uiOutWidthBytes);	// new width bytes

	  BYTE * JpegFile::MovetoBuf(BYTE *dataBuf,
						 CRect *prcRect,				// pixels!!
						 UINT widthNew,
						 UINT heightNew);		// bytes!!!

	  BYTE *RGBFromDWORDAligned(BYTE *inBuf,				// input buf
									UINT widthPix,				// input size
									UINT widthBytes,			// input size
									UINT height);

	  BOOL VertFlipBuf(BYTE * inbuf,						// input buf
					   UINT widthBytes,							// input width bytes
					   UINT height);							// height

	   BOOL MakeGrayScale(BYTE *buf,						// input buf 
						UINT widthPix,							// width in pixels
						UINT height,
						BOOL turn = false);							// height

	    BOOL BGRFromRGB(BYTE *buf,							// input buf
					UINT widthPix,								// width in pixels
					UINT height);								// lines
     BYTE *BMPtoFix(unsigned char * lpData, 
		            unsigned long *lWidth ,
					unsigned long *lHeight, 
					unsigned long *lHeadSize, 
					unsigned long *lSize);
    void DeletePreData();
    BYTE *FixtoBMP(unsigned char * lpInData, 
						  unsigned long lWidth ,
						  unsigned long lHeight, 
						  unsigned char * lpHeadData,
						  unsigned long lHeadSize, 
						  unsigned long w, 
						  unsigned long h) ;

     BYTE * LoadBMP(LPSTR lpData,
		                unsigned long *lWidth,
						unsigned long *lHeight);

};

#endif