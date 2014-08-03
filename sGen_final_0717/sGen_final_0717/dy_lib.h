/********************************************************************
*																	*
*	VISION IMAGE PROCESSING LAB.									*
*	IMAGE PRCESSING LIBRARY FOR VIP LAB.							*
*	last modified : 17 / 07 / 2014	by Do-young Park				*
*																	*
*	Copyright(c) 2014												*
*	All rights reserved by Do-young Park							*
*																	*
********************************************************************/

#ifndef _DOYOUNG_LIB_
#define _DOYOUNG_LIB_

#include	<cmath>
#include	<memory>
#include	<Windows.h>
#include	<fstream>
#include	<iostream>

#define PI acos(-1.0)
#define FORWARD 1
#define BACKWARD -1
#define PLANAR 1
#define INTERLEAVED 2

/*********************************** HEADER 등. *********************************/
/* BMP FILE & INFORMATION HEADER*/
typedef struct BMPFILEHEADER
{
	WORD	TypeId;			// "BM" - begin of bmp file
	INT		FileSize;		// file size
	WORD	Reserved1;		// for application program
	WORD	Reserved2;		// for application program
	INT		Offset;			// distance to image data
}BmpFileHeader;

typedef struct BMPINFOHEADER
{
	INT		BmpInfoHeaderSize;			//size of BITMAPINFOHEADER struct
	INT		Width;						//image column
	INT		Height;						//image row
	WORD	ColorPlane;					//color plane(always 1)
	WORD	BitCount;					//number of bits per pixel - typical:1, 4, 8, 16, 24, 32
	INT		CompressionFlag;			//compression method - (0:BI_RGB none, 1:BI_RLE8 for 8bit, 2:BIRLE4 for 4bit, 3:BI_BITFIELDS bit 16,32 bit, 4:BI_JPEG for jpg, 5:BI_PNG for png)
	INT		RawDataSize;				//size of raw bitmap data(image size)
	INT		WidthResolution;			//column resolution of image (pixel per meter)
	INT		HeightResolusion;			//row resolution of image (pixel per meter)
	INT		NumOfColor;					//number of colors
	INT		NumOfImportantColor;		//number of important color used(0 when every color is important)
}BmpInforHearder;
/* END BMP FILE & INFORMATION HEADER */

/*********************************** 신호처리 - 영상처리 기술 *********************************/
/* 1D DISCRETE COSINE TRANSFORM - INPUT , OUTPUT , LENGTH OF SIGNAL */
template <class T>
VOID DCT_1D(T *in, T *out, CONST INT N);
/* 1D INVERSE DISCRETE COSINE TRANSFORM - INPUT , OUTPUT , LENGTH OF SIGNAL */
template <class T>
VOID IDCT_1D(T *in, T *out, CONST INT N);
/* 1D (INVERSE) DISCRETE FOURIER TRANSFORM - INPUT , OUTPUT , LENGTH OF SIGNAL , INVERSE(1 - FORWARD, 2 - BACKWARD) */
template <class T>
VOID DFT_1D(T* real, T* imagine, CONST INT Length, CONST INT Forward = FORWARD);

/* PSNR CALCULATE - ORIGINAL IMAGE DATA, TARGET IMAGE DATA, SIZE OF IMAGE, BIT PER PIXEL(DEFAULT : 8BIT=256) */
template<typename T>
DOUBLE PSNR(T* origin, T* target, CONST INT size, CONST INT bitPerPixel = 8);

/* CLIPPING - ARRAY DATA, MIN VALUE, MAX VALUE (DEFAULT : 0 <= data <= 255) */
template<typename T, typename C>
VOID CLIPPING(T* data, CONST INT size, CONST C min = 0.0, CONST C max = 255.0);

/* ROUND - ORIGINAL VALUE, POSITION OF ROUND(DEFAULT : 1) */
template<typename T, typename C>
T ROUND(C value, CONST INT position = 1);
template<typename T, typename C>
T* ROUND(C* value, CONST INT size, CONST INT position = 1);
template<typename T, typename C>
T* ROUND(C* value, CONST INT row, CONST INT col, CONST INT position = 1);

/* REVERSE IMAGE - ORIGINAL IMAGE, SIZE OF IMAGE, BIT PER PIXEL(DEFAULT : 8BIT=256) */
template <typename T>
VOID REVERSE_IMAGE(T* data, CONST INT size, CONST INT bitPerPixel = 8);
template <typename T>
VOID REVERSE_IMAGE(T* data, CONST INT row, CONST INT col, CONST INT bitPerPixel = 8);

/* READ & WRITE IMAGE FILE( FORMAT TO RAW DATA ) - READ IMAGE - ROW(HEIGHT), COL(WIDTH), OUTPUT DATA(RAW), IMAGE FILE NAME - WRITE IMAGE - ROW(HEIGHT), COL(WIDTH), IMAGE RAW DATA, IMAGE FILE NAME < color image => rgb plane > */
/* PGM FILE FORMAT */
BYTE* ReadPgm(INT *row, INT *col, LPCSTR  filename);
VOID WritePgm(INT row, INT col, BYTE* img, LPCSTR  filename); //완

/* PBM FILE FORMAT*/
BYTE* ReadPbm(INT *row, INT *col, LPCSTR  filename);
VOID WritePbm(INT row, INT col, BYTE* img, LPCSTR  filename);

/* PPM FILE FORMAT */
BYTE* ReadPpm(INT *row, INT *col, LPCSTR  filename, INT order = 1);
VOID WritePpm(INT row, INT col, BYTE* img, LPCSTR  filename, INT order = 1);

/* BMP FILE FORMAT */
BYTE* ReadBmp(INT *row, INT *col, LPCSTR  filename);
VOID WriteBmp(INT row, INT col, BYTE* img, LPCSTR  filename);

/* END READ & WRITE IMAGE FILE - READ IMAGE - ROW(HEIGHT), COL(WIDTH), OUTPUT DATA(RAW), IMAGE FILE NAME - WRITE IMAGE - ROW(HEIGHT), COL(WIDTH), IMAGE RAW DATA, IMAGE FILE NAME*/

/* PSNR - GET PSNR - */
template <class T>
DOUBLE gPSNR(T* origin, T* target, CONST INT length, CONST INT max = 255);
template <class T>
DOUBLE gPSNR(T* origin, T* target, CONST INT height, CONST INT width, CONST INT max = 255, CONST INT boundary = 0);


/* 개발 준비중인 함수들 */
template<typename T>
DOUBLE SNR(T* origin, T* target, CONST INT size);

BOOL ReadPng(INT *row, INT *col, BYTE* img, LPCSTR  filename);//http://noteroom.tistory.com/157
BOOL WritePng(INT row, INT col, BYTE* img, LPCSTR  filename);//http://www.fastgraph.com/help/jpeg_header_format.html
BOOL ReadJpg(INT *row, INT *col, BYTE* img, LPCSTR  filename);
BOOL WriteJpg(INT row, INT col, BYTE* img, LPCSTR  filename);
BOOL ReadRaw(INT *row, INT *col, BYTE* img, LPCSTR  filename);//있을거고 - 이건 그냥 가져오면 됨 그냥뺄까..
BOOL WriteRaw(INT row, INT col, BYTE* img, LPCSTR  filename);

// color space conversion method도 
// rgb cmyk hsv cie yiq hls 
// 필터- 기본필터 가우시안 등등... 탭수 알아서 - 세퍼러블로 

#endif