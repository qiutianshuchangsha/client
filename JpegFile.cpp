#include "StdAfx.h"
#include "JpegFile.h"
#define MAX_SIZE 1000000
#include <stdio.h>

#ifdef __cplusplus
	extern "C" {
#endif // __cplusplus

#include "jpeglib\\jpeglib.h"

#ifdef __cplusplus
	}
#endif // __cplusplus

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>

// error handler, to avoid those pesky exit(0)'s

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

//
//
//

METHODDEF(void) my_error_exit (j_common_ptr cinfo);

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	/* Always display the message. */
	//MessageBox(NULL,buffer,"JPEG Fatal Error",MB_ICONSTOP);


	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

// store a scanline to our data buffer
void j_putRGBScanline(BYTE *jpegline, 
						 int widthPix,
						 BYTE *outBuf,
						 int row);

void j_putGrayScanlineToRGB(BYTE *jpegline, 
						 int widthPix,
						 BYTE *outBuf,
						 int row);



JpegFile::JpegFile(void)
{
	m_dwPoint = 0;
//	m_lpBuffer = NULL;
	m_bOpening = false;
	m_dwOutMaxSize = 0;
    m_lpOutBuffer = NULL;
	m_dwScreenMaxSize = 0;
    m_lpScreenBuffer = NULL;
    m_lpPreData = NULL;
}


JpegFile::~JpegFile(void)
{
	 if (m_lpPreData) GlobalFree (m_lpPreData);
	 if (m_lpOutBuffer) GlobalFree (m_lpOutBuffer);
	 if (m_lpScreenBuffer) GlobalFree (m_lpScreenBuffer);
}

BYTE * JpegFile::JpegFileToRGB(BYTE *inBuf,
							   unsigned long size,
							   UINT *width,
							   UINT *height)

{
	// get our buffer set to hold data
	BYTE *dataBuf = NULL;

	// basic code from IJG Jpeg Code v6 example.c

	*width=0;
	*height=0;

	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile=NULL;		/* source file */

	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */
//	char buf[250];

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

/*	if ((infile = fopen(fileName, "rb")) == NULL) {
		sprintf(buf, "JPEG :\nCan't open %s\n", fileName);
		AfxMessageBox(buf);
		return NULL;
	}
*/
	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;


	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		jpeg_destroy_decompress(&cinfo);
/*
		if (infile!=NULL)
			fclose(infile);

      if (dataBuf!=NULL)
      {
         delete [] dataBuf;
      }
*/
		return NULL;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);
    cinfo.l_size = size;
 	cinfo.l_point = 0;
 	cinfo.i_stream = 1;
 	cinfo.buffer = inBuf;

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/ 

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
//	dataBuf=(BYTE *)new BYTE[cinfo.output_width * 3 * cinfo.output_height];
	DWORD dwSize = cinfo.output_width * 3 * cinfo.output_height;
	
	if (m_lpOutBuffer==NULL || m_dwOutMaxSize < dwSize) 
	{
 	   if (m_lpOutBuffer) GlobalFree (m_lpOutBuffer);
	   m_lpOutBuffer = (BYTE *)GlobalAlloc (GMEM_FIXED, dwSize);
	   m_dwOutMaxSize = dwSize;
	   if (m_lpOutBuffer == NULL)
	   {
	     AfxMessageBox("JpegFile :\nOut of memory",MB_ICONSTOP);
		 jpeg_destroy_decompress(&cinfo);
		
//DEL		fclose(infile);
   		  return NULL;
	   }
	}
    dataBuf = m_lpOutBuffer;
	// how big is this thing gonna be?
	*width = cinfo.output_width;
	*height = cinfo.output_height;
	
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */

		// asuumer all 3-components are RGBs
		if (cinfo.out_color_components==3) {
			
			j_putRGBScanline(buffer[0], 
								*width,
								dataBuf,
								cinfo.output_scanline-1);

		} else if (cinfo.out_color_components==1) {

			// assume all single component images are grayscale
			j_putGrayScanlineToRGB(buffer[0], 
								*width,
								dataBuf,
								cinfo.output_scanline-1);

		}

	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
//	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	return dataBuf;
}

BOOL JpegFile::GetJPGDimensions(CString fileName,
								UINT *width,
								UINT *height)

{
	// basic code from IJG Jpeg Code v6 example.c

	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	FILE * infile=NULL;		/* source file */
	char buf[250];

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

	if ((infile = fopen(fileName, "rb")) == NULL) {
		sprintf(buf, "JPEG :\nCan't open %s\n", fileName);
		AfxMessageBox(buf);
		return FALSE;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;


	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		jpeg_destroy_decompress(&cinfo);

		if (infile!=NULL)
			fclose(infile);
		return FALSE;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/


	// how big is this thing ?
	*width = cinfo.image_width;
	*height = cinfo.image_height;

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	return TRUE;
}

//
//
//

BYTE *JpegFile::RGBFromDWORDAligned(BYTE *inBuf,
									UINT widthPix, 
									UINT widthBytes,
									UINT height)
{
	if (inBuf==NULL)
		return NULL;


	BYTE *tmp;
	tmp=(BYTE *)new BYTE[height * widthPix * 3];
	if (tmp==NULL)
		return NULL;

	UINT row;

	for (row=0;row<height;row++) {
		memcpy((tmp+row * widthPix * 3), 
				(inBuf + row * widthBytes), 
				widthPix * 3);
	}

	return tmp;
}

//
//
//

BOOL JpegFile::RGBToJpegFile(BYTE *dataBuf,
							  BYTE *outBuf,
							UINT widthPix,
							UINT height,
							BOOL color, 
							int quality,
							unsigned long *size)
{
  BOOL b_return = true;
  FILE *outfile = NULL;			/* target file */
/*  FILE outtmp;
  FILE *outfile = &outtmp;
  memset(outfile, 0, sizeof(FILE));
*/	if (dataBuf==NULL) goto err01;
	if (outBuf == NULL) goto err01;
	if (widthPix==0)  goto err01;
	if (height==0) goto err01;
	// we vertical flip for display. undo that.
//	VertFlipBuf(dataBuf, widthPix * 3, height);

	// we swap red and blue for display, undo that.
//	BGRFromRGB(dataBuf, widthPix, height);

	LPBYTE tmp;
	if (!color) {
		tmp = (BYTE*)new BYTE[widthPix*height];
		if (tmp==NULL) goto err01;

		UINT row,col;
		for (row=0;row<height;row++) {
			for (col=0;col<widthPix;col++) {
				LPBYTE pRed, pGrn, pBlu;
				pRed = dataBuf + row * widthPix * 3 + col * 3;
				pGrn = dataBuf + row * widthPix * 3 + col * 3 + 1;
				pBlu = dataBuf + row * widthPix * 3 + col * 3 + 2;

				// luminance
				int lum = (int)(.299 * (double)(*pRed) + .587 * (double)(*pGrn) + .114 * (double)(*pBlu));
				LPBYTE pGray;
				pGray = tmp + row * widthPix + col;
				*pGray = (BYTE)lum;
			}
		}
	}

	struct jpeg_compress_struct cinfo;
	/* More stuff */
	int row_stride;			/* physical row widthPix in image buffer */

	struct my_error_mgr jerr;

	/* Step 1: allocate and initialize JPEG compression object */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		jpeg_destroy_compress(&cinfo);

//DEL		if (outfile!=NULL)
//DEL			fclose(outfile);

		if (!color) delete [] tmp;
//DEL		return FALSE;
//DEL		if (!color)
		  goto err01;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);
	cinfo.l_size = *size;
 	cinfo.l_point = 0;
 	cinfo.i_stream = 1;
 	cinfo.buffer = (unsigned char *)outBuf;

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */
/*
	if ((outfile = fopen("c:\\111.jpg", "wb")) == NULL) 
	{
  	   char buf[250];
		//sprintf(buf, "JpegFile :\nCan't open %s\n", fileName);
	AfxMessageBox(buf);
		return FALSE;
	}
*/
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */
												    
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = widthPix; 	/* image widthPix and height, in pixels */
	cinfo.image_height = height;
	if (color) {
		cinfo.input_components = 3;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	} else {
		cinfo.input_components = 1;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	}

 
/* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */

  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = widthPix * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
	LPBYTE outRow;
	if (color) {
		outRow = dataBuf + (cinfo.next_scanline * widthPix * 3);
	} else {
		outRow = tmp + (cinfo.next_scanline * widthPix);
	}

    (void) jpeg_write_scanlines(&cinfo, &outRow, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  *size = cinfo.l_point;
  /* After finish_compress, we can close the output file. */
  //DEL fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  if (cinfo.l_point > cinfo.l_size)
 	  b_return = false;

  jpeg_destroy_compress(&cinfo);
//  fclose(outfile);
  if (!color)  delete [] tmp;
  /* And we're done! */
  return b_return;
err01:
   return false;
//  return TRUE;
}

//
//	stash a scanline
//

void j_putRGBScanline(BYTE *jpegline, 
					 int widthPix,
					 BYTE *outBuf,
					 int row)
{
	int offset = row * widthPix * 3;
	int count;
	for (count=0;count<widthPix;count++) 
	{
		*(outBuf + offset + count * 3 + 0) = *(jpegline + count * 3 + 0);
		*(outBuf + offset + count * 3 + 1) = *(jpegline + count * 3 + 1);
		*(outBuf + offset + count * 3 + 2) = *(jpegline + count * 3 + 2);
	}
}

//
//	stash a gray scanline
//

void j_putGrayScanlineToRGB(BYTE *jpegline, 
							 int widthPix,
							 BYTE *outBuf,
							 int row)
{
	int offset = row * widthPix * 3;
	int count;
	for (count=0;count<widthPix;count++) {

		BYTE iGray;

		// get our grayscale value
		iGray = *(jpegline + count);

		*(outBuf + offset + count * 3 + 0) = iGray;
		*(outBuf + offset + count * 3 + 1) = iGray;
		*(outBuf + offset + count * 3 + 2) = iGray;
	}
}
//
// copies BYTE buffer into DWORD-aligned BYTE buffer
// return addr of new buffer
//

BYTE * JpegFile::MovetoBuf(BYTE *dataBuf,
						 CRect *prcRect,				// pixels!!
						 UINT widthNew,
						 UINT heightNew)		// bytes!!!
{
	////////////////////////////////////////////////////////////
	// what's going on here? this certainly means trouble 
	if (dataBuf==NULL)
		return NULL;

	////////////////////////////////////////////////////////////
	// how big is the smallest DWORD-aligned buffer that we can use?
	UINT uiWidthBytes, uiWidthBytesNew;
	uiWidthBytes = WIDTHBYTES(prcRect->Width() * 24);
	uiWidthBytes = 3 * prcRect->Width();
	uiWidthBytesNew = 3 * widthNew;

	DWORD dwNewsize=(DWORD)((DWORD)uiWidthBytesNew * 
							(DWORD)heightNew);
	BYTE *pNew;

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
	pNew=(BYTE *)new BYTE[dwNewsize];
	if (pNew==NULL) {
		return NULL;
	}
	UINT row,col;
	memset(pNew, 0,dwNewsize);
	for (row=0;(int)row<prcRect->Height();row++) {
		for (col=0;(int)col<prcRect->Width();col++) {
			BYTE pRed, pGrn, pBlu;
			pRed = dataBuf[row * prcRect->Width() * 3 + col * 3];
			pGrn = dataBuf[row * prcRect->Width() * 3 + col * 3+1];
			pBlu = dataBuf[row * prcRect->Width() * 3 + col * 3+2];
			pNew[(heightNew - prcRect->bottom + row) * widthNew * 3 + (col+prcRect->left) * 3] = (BYTE)pRed;
			pNew[(heightNew - prcRect->bottom + row) * widthNew * 3 + (col+prcRect->left) * 3+1] = (BYTE)pGrn;
			pNew[(heightNew - prcRect->bottom + row) * widthNew * 3 + (col+prcRect->left) * 3+2] = (BYTE)pBlu;

		}
	}

	return pNew;
}

//
// copies BYTE buffer into DWORD-aligned BYTE buffer
// return addr of new buffer
//

BYTE * JpegFile::MakeDwordAlignedBuf(BYTE *dataBuf,
									 UINT widthPix,				// pixels!!
									 UINT height,
									 UINT *uiOutWidthBytes)		// bytes!!!
{
	////////////////////////////////////////////////////////////
	// what's going on here? this certainly means trouble 
	if (dataBuf==NULL)
		return NULL;

	////////////////////////////////////////////////////////////
	// how big is the smallest DWORD-aligned buffer that we can use?
	UINT uiWidthBytes;
	uiWidthBytes = WIDTHBYTES(widthPix * 24);

	DWORD dwNewsize=(DWORD)((DWORD)uiWidthBytes * 
							(DWORD)height);
	BYTE *pNew;

	////////////////////////////////////////////////////////////
	// alloc and open our new buffer
/*	pNew=(BYTE *)new BYTE[dwNewsize];
	if (pNew==NULL) {
		return NULL;
	}
*/
	if (m_lpScreenBuffer==NULL || m_dwScreenMaxSize < dwNewsize) 
	{
 	   if (m_lpScreenBuffer) GlobalFree (m_lpScreenBuffer);
	   m_lpScreenBuffer = (BYTE *)GlobalAlloc (GMEM_FIXED, dwNewsize);
	   m_dwScreenMaxSize = dwNewsize;
	   if (m_lpScreenBuffer == NULL) return NULL;
    }
	pNew = m_lpScreenBuffer;
	////////////////////////////////////////////////////////////
	// copy row-by-row
	UINT uiInWidthBytes = widthPix * 3;
	UINT uiCount;
	for (uiCount=0;uiCount < height;uiCount++) {
		BYTE * bpInAdd;
		BYTE * bpOutAdd;
		ULONG lInOff;
		ULONG lOutOff;

		lInOff=uiInWidthBytes * uiCount;
		lOutOff=uiWidthBytes * uiCount;

		bpInAdd= dataBuf + lInOff;
		bpOutAdd= pNew + lOutOff;

		memcpy(bpOutAdd,bpInAdd,uiInWidthBytes);
	}

	*uiOutWidthBytes=uiWidthBytes;
	return pNew;
}

BOOL JpegFile::VertFlipBuf(BYTE  * inbuf, 
					   UINT widthBytes, 
					   UINT height)
{   
	BYTE  *tb1;
	BYTE  *tb2;

	if (inbuf==NULL)
		return FALSE;

	UINT bufsize;

	bufsize=widthBytes;

	tb1= (BYTE *)new BYTE[bufsize];
	if (tb1==NULL) {
		return FALSE;
	}

	tb2= (BYTE *)new BYTE [bufsize];
	if (tb2==NULL) {
		delete [] tb1;
		return FALSE;
	}
	
	UINT row_cnt;     
	ULONG off1=0;
	ULONG off2=0;

	for (row_cnt=0;row_cnt<(height+1)/2;row_cnt++) {
		off1=row_cnt*bufsize;
		off2=((height-1)-row_cnt)*bufsize;   
		
		memcpy(tb1,inbuf+off1,bufsize);
		memcpy(tb2,inbuf+off2,bufsize);	
		memcpy(inbuf+off1,tb2,bufsize);
		memcpy(inbuf+off2,tb1,bufsize);
	}	

	delete [] tb1;
	delete [] tb2;

	return TRUE;
}        


//
//	swap Rs and Bs
//
//	Note! this does its stuff on buffers with a whole number of pixels
//	per data row!!
//


BOOL JpegFile::BGRFromRGB(BYTE *buf, UINT widthPix, UINT height)
{
	if (buf==NULL)
		return FALSE;

	UINT col, row;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 3 + col * 3;
			pGrn = buf + row * widthPix * 3 + col * 3 + 1;
			pBlu = buf + row * widthPix * 3 + col * 3 + 2;

			// swap red and blue
			BYTE tmp;
			tmp = *pRed;
			*pRed = *pBlu;
			*pBlu = tmp;
		}
	}
	return TRUE;
}

//
//	Note! this does its stuff on buffers with a whole number of pixels
//	per data row!!
//

BOOL JpegFile::MakeGrayScale(BYTE *buf, UINT widthPix, UINT height, BOOL trun)
{
	if (buf==NULL)
		return FALSE;

	UINT row,col;
	for (row=0;row<height;row++) {
		for (col=0;col<widthPix;col++) {
			LPBYTE pRed, pGrn, pBlu;
			pRed = buf + row * widthPix * 3 + col * 3;
			pGrn = buf + row * widthPix * 3 + col * 3 + 1;
			pBlu = buf + row * widthPix * 3 + col * 3 + 2;

			// luminance
			int lum = (int)(.299 * (double)(*pRed) + .587 * (double)(*pGrn) + .114 * (double)(*pBlu));
			if (trun) lum = ~lum;
			*pRed = (BYTE)lum;
			*pGrn = (BYTE)lum;
			*pBlu = (BYTE)lum;
		}
	}
	return TRUE;
}

BYTE * JpegFile::LoadBMP(LPSTR lpData, unsigned long *lWidth ,unsigned long *lHeight) 
{
    
	LPBITMAPINFOHEADER	lpbi;		//指向位图信息头结构
	BYTE *outBuf=NULL;
	
	long m_bytesRead=0;
    long row=0;
	long rowOffset=0;
    long pixoff;
	    lpbi = (LPBITMAPINFOHEADER)lpData;
        lpData += sizeof(BITMAPINFOHEADER);

//导入调色板
		RGBQUAD szColormap[256];
		RGBQUAD *colormap = szColormap;
		switch (lpbi->biBitCount) {
		case 24:
			break;
// read pallete 
		case 1:
		case 4:
		case 8:
			//colormap = new RGBQUAD[lpbi->biClrUsed];
			//if (colormap==NULL) return NULL;
			int i;
			for (i=0; i < (int)lpbi->biClrUsed;i++) {
				colormap[i].rgbBlue=*(lpData+m_bytesRead);
				m_bytesRead++;
				colormap[i].rgbGreen=*(lpData+m_bytesRead);
				m_bytesRead++;
				colormap[i].rgbRed=*(lpData+m_bytesRead);
				m_bytesRead++;
				m_bytesRead++; //去空格
			}
			break;
		}

		int w = lpbi->biWidth;
		int h = lpbi->biHeight;
		*lWidth = w;
		*lHeight = h;
		long row_size = w * 3;

		long bufsize = (long)w * 3 * (long)h;

		////////////////////////////////////////////////////////////////////////////
		// alloc our buffer
	DWORD dwSize = bufsize;
	if (m_lpOutBuffer==NULL || m_dwOutMaxSize < dwSize) 
	{
 	   if (m_lpOutBuffer) GlobalFree (m_lpOutBuffer);
	   m_lpOutBuffer = (BYTE *)GlobalAlloc (GMEM_FIXED, dwSize);
	   m_dwOutMaxSize = dwSize;
	   if (m_lpOutBuffer == NULL) goto err01;
	}
        outBuf = m_lpOutBuffer;
		////////////////////////////////////////////////////////////////////////////
		//	read it
		row=0;
	    rowOffset=0;
        pixoff = m_bytesRead;
			// read rows in reverse order
		for (row=lpbi->biHeight-1;row>=0;row--) {

				// which row are we working on?
				rowOffset=(long unsigned)row*row_size;						      

				if (lpbi->biBitCount==24) {
					for (int col=0;col<w;col++) {
						long offset = col * 3;
						//char pixel[3];
						// we swap red and blue here
							*(outBuf + rowOffset + offset + 2)=*(lpData + m_bytesRead);		// b
							m_bytesRead++;
							*(outBuf + rowOffset + offset + 1)=*(lpData + m_bytesRead);		// g
							m_bytesRead++;
							*(outBuf + rowOffset + offset + 0)=*(lpData + m_bytesRead);		// r
							m_bytesRead++;
					}
					// read DWORD padding
					while ((m_bytesRead-pixoff)&3) {
						m_bytesRead++;
					}
 					
				} else {	// 1, 4, or 8 bit image

					////////////////////////////////////////////////////////////////
					// pixels are packed as 1 , 4 or 8 bit vals. need to unpack them

					int bit_count = 0;
					UINT mask = (1 << lpbi->biBitCount) - 1;

					BYTE inbyte=0;

					for (int col=0;col<w;col++) {
						
						int pix=0;

						// if we need another byte
						if (bit_count <= 0) {
							bit_count = 8;
							inbyte = *(lpData+m_bytesRead);
							m_bytesRead++;
						}

						// keep track of where we are in the bytes
						bit_count -= lpbi->biBitCount;
						pix = ( inbyte >> bit_count) & mask;

						// lookup the color from the colormap - stuff it in our buffer
						// swap red and blue
						*(outBuf + rowOffset + col * 3 + 2) = colormap[pix].rgbBlue;
						*(outBuf + rowOffset + col * 3 + 1) = colormap[pix].rgbGreen;
						*(outBuf + rowOffset + col * 3 + 0) = colormap[pix].rgbRed;
					}

					// read DWORD padding
					while ((m_bytesRead-pixoff)&3) {
						m_bytesRead++;
					}
				}
			}
		

//	if (colormap) delete [] colormap;
	//BGRFromRGB(outBuf, lpbi->biWidth, lpbi->biHeight);
	// vertical flip for display
	//VertFlipBuf(outBuf, lpbi->biWidth * 3, lpbi->biHeight);

	return outBuf;
err01:
//	if (colormap) delete [] colormap;
	return NULL;
}

void JpegFile::DeletePreData()
{
  if (m_lpPreData) GlobalFree (m_lpPreData);
  m_lpPreData = NULL;
}
#define GETBIT(p, x) ((unsigned char)p[x/8] >> (x%8) & 1)

BYTE * JpegFile::FixtoBMP(unsigned char * lpInData, 
						  unsigned long lWidth ,
						  unsigned long lHeight, 
						  unsigned char * lpHeadData,
						  unsigned long lHeadSize, 
						  unsigned long w, 
						  unsigned long h) 
{
	if (w % 16) return NULL;  //必需能被16整除
	int hCell = h / 16;
	if (hCell % 16) hCell++;
	int wCell = w / 16;
	int hSize = (wCell*hCell)/8;  
	if ((wCell*hCell)%8)
		hSize++;
	if (hSize != lHeadSize) return NULL;
	DWORD lBufSize = (long)w * 3 * (long)h;
	if (m_lpPreData==NULL) 
	{
	   m_lpPreData = (BYTE *)GlobalAlloc (GMEM_FIXED, lBufSize);
	   if (m_lpPreData == NULL) return NULL;
	   memset(m_lpPreData,0, lBufSize);
	}

   DWORD lFullRow, row ;
   DWORD lFullRowOffset = 0, lFullPoint = 0;
   DWORD lPreRowOffset = 0, lPrePoint;
   for (lFullRow = 0; lFullRow < wCell*hCell; lFullRow++)
   {
//       lPrePoint = ((lFullRow%(w/16))*16 + (lFullRow/(w/16))*w*16) * 3;
       lPrePoint = ((lFullRow%wCell) + (lFullRow/wCell)*w)*48;
	   if (GETBIT(lpHeadData, lFullRow))//不相同
	   {
	    for (row = 0 ; row < 16 ; row ++)
		{
	      if (lPrePoint+1 < lBufSize)
		  {
		     memcpy(m_lpPreData + lPrePoint, lpInData + lFullPoint, 48);
		  }
	      lPrePoint  += w * 3;
	      lFullPoint += 48; //16 * 3
		}//end for
	   }//end if
  }//end for
 return m_lpPreData;
}
#define SETBIT(p, x) ((unsigned char)p[x/8] |= (1 << (x%8)))
BYTE *JpegFile::BMPtoFix(unsigned char * lpInData, unsigned long *lWidth ,unsigned long *lHeight, 
						  unsigned long *lHeadSize, unsigned long *lSize) 
{
    
	LPBITMAPINFOHEADER	lpbi;		//指向位图信息头结构
	BOOL bPre = true;               //前图有效
	lpbi = (LPBITMAPINFOHEADER)lpInData;
    lpInData += sizeof(BITMAPINFOHEADER);
	if (lpbi->biBitCount != 24) return NULL; //只支持真彩

	int w = lpbi->biWidth;
	int h = lpbi->biHeight;
	if (w % 16) return NULL;  //必需能被16整除
//	if (h % 8 )return NULL;   //必需能被8整除
//	unsigned long  hSize = (long)w*(long)h/1024;
	int hCell = h / 16;
	if (hCell % 16) hCell++;
	int wCell = w / 16;
	int hSize = (wCell*hCell)/8;  
	if ((wCell*hCell)%8)
		hSize++;
//	DWORD lInRowSize = w * 3;
	DWORD lBufSize = wCell*hCell*768 + hSize;

////////////////////////////////////////////////////////////////////////////
//图象缓存
	if (m_lpOutBuffer==NULL || m_dwOutMaxSize < lBufSize) 
	{
 	   if (m_lpOutBuffer) GlobalFree (m_lpOutBuffer);
	   m_lpOutBuffer = (BYTE *)GlobalAlloc (GMEM_FIXED, lBufSize);
	   m_dwOutMaxSize = lBufSize;
	   if (m_lpOutBuffer == NULL) return NULL;
	}
	if (m_lpPreData==NULL) 
	{
	   m_lpPreData = (BYTE *)GlobalAlloc (GMEM_FIXED, lBufSize-hSize);
	   if (m_lpPreData == NULL) return NULL;
	   memset(m_lpPreData,0, lBufSize-hSize);
	   bPre = false;
	}
   memset(m_lpOutBuffer, 0, hSize);
   BYTE *lpOutData = m_lpOutBuffer + hSize;
   DWORD  lPreRow, row ;
   DWORD lOutRowOffset = 0, lPreRowOffset = 0, lPrePoint;
   DWORD lInPoint ;
   int iFound, iHeightPoint = 0;
  for (lPreRow = 0; lPreRow < wCell*hCell; lPreRow++, lPreRowOffset += 768)
   {
	   iFound = 0;
       lPrePoint = lPreRowOffset;
//       lInPoint = ((lPreRow%wCell)*16 + (lPreRow/wCell)*w*16)*3;
      lInPoint = ((lPreRow%wCell) + (lPreRow/wCell)*w)*48;

	   for (row = 0 ; row < 16 ; row ++)
	   {
//比较
		if (lInPoint+1 > lBufSize)
		{
           memset(m_lpPreData + lPrePoint, 0,48); 
        }
		else
		{
           if (iFound == 0 && bPre == true) 
		   {
//			   iFound = memcmp(m_lpPreData + lPrePoint, lpInData + lInPoint, 48);
			for (int i = 0 ; i < 48; i++)
			{  
				if (*(m_lpPreData + lPrePoint + i) != *(lpInData + lInPoint+i))
				{
	               iHeightPoint++;
				   iFound++;
				   break;
				}
			      
			}//end for
		   }//end if
           memcpy(m_lpPreData + lPrePoint, lpInData + lInPoint, 48); //保存	       
		}
		lPrePoint += 48; //16 * 3;
		lInPoint += w * 3;
   }//end for
 	   if (iFound == 0 && bPre == true) continue;
	   SETBIT(m_lpOutBuffer, lPreRow);
//输出
	   memcpy(lpOutData + lOutRowOffset, 
	          m_lpPreData + lPreRowOffset, 768); //16*16*3
	   lOutRowOffset += 768;
  }//end for
 *lWidth = 16;
 *lHeight = lOutRowOffset/(16*3);
 //*lHeight = iHeightPoint;
 *lHeadSize = hSize;
 *lSize = lOutRowOffset;
 return m_lpOutBuffer;
}

