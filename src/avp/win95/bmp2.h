/****************************************************************************\
**  Title:   BMP.H                                                          **
**  Purpose: BMP Header file                                                **
**  Version: 1.0                                                            **
**  Date:    October 1991                                                   **
**  Author:  James D. Murray, Anaheim, CA, USA                              **
**                                                                          **
**  This header file contains the structures for the three flavors of the   **
**  BMP image file format (OS/2 1.x, WIndows 3.0, and OS/2 2.0).  Each BMP  **
**  file will contain a BMPINFO header followed by either a PMINFPHEAD,     **
**  WININFOHEAD, or PM2INFOHEAD header.  To simplify reading and writing    **
**  BMP files the BMP file format structure defined in BMP.H contains       **
**  structures for all three flavors of the BMP image file format.          **
**                                                                          **
**  Copyright (C) 1991 Graphics Software Labs.  All rights reserved.        **
\****************************************************************************/
#ifndef BMP2_H
#define BMP2_H   1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "datatype.h"        /* Include the data type definitions */

#define COMPRESS_RGB        0L      /* No compression               */
#define COMPRESS_RLE8       1L      /* 8 bits per pixel compression */
#define COMPRESS_RLE4       2L      /* 4 bits per pixel compression */
#define BMP_ID              0x4d42  /* BMP "magic" number           */

#define LSN(value)	((value) & 0x0f)	    /* Least-significant nibble */
#define MSN(value)	(((value) & 0xf0) >> 4)	/* Most-significant nibble  */

/*
**  BMP File Format Bitmap Header.
*/
typedef struct _BmpInfo     /* Offset   Description                      */
{
    WORD   Type;            /*  00h     File Type Identifier             */
    DWORD  FileSize;        /*  02h     Size of File                     */
    WORD   Reserved1;       /*  06h     Reserved (should be 0)           */
    WORD   Reserved2;       /*  08h     Reserved (should be 0)           */
    DWORD  Offset;          /*  0Ah     Offset to bitmap data            */
} BMPINFO;

/*
**  Presentation Manager (OS/2 1.x) Information Header Format.
*/
typedef struct _PmInfoHeader   /* Offset   Description                     */
{
    DWORD   Size;               /*  0Eh     Size of Remianing Header        */
    WORD    Width;              /*  12h     Width of Bitmap in Pixels       */
    WORD    Height;             /*  14h     Height of Bitmap in Pixels      */
    WORD    Planes;             /*  16h     Number of Planes                */
    WORD    BitCount;           /*  18h     Color Bits Per Pixel            */
} PMINFOHEAD;

/*
**  Windows 3.x Information Header Format.
*/
typedef struct _WinInfoHeader   /* Offset  Description                      */
{
    DWORD  Size;                /*  0Eh    Size of Remianing Header         */
    DWORD  Width;               /*  12h    Width of Bitmap in Pixels        */
    DWORD  Height;              /*  16h    Height of Bitmap in Pixels       */
    WORD   Planes;              /*  1Ah    Number of Planes                 */
    WORD   BitCount;            /*  1Ch    Bits Per Pixel                   */
    DWORD  Compression;         /*  1Eh    Compression Scheme (0=none)      */
    DWORD  SizeImage;           /*  22h    Size of bitmap in bytes          */
    DWORD  XPelsPerMeter;       /*  26h    Horz. Resolution in Pixels/Meter */
    DWORD  YPelsPerMeter;       /*  2Ah    Vert. Resolution in Pixels/Meter */
    DWORD  ClrUsed;             /*  2Eh    Number of Colors in Color Table  */
    DWORD  ClrImportant;        /*  32h    Number of Important Colors       */
} WININFOHEAD;

/*
**  Presentation Manager (OS/2 2.0) Information Header Format.
*/
typedef struct _Pm2InfoHeader   /* Offset  Description                      */
{  
    DWORD   Size;               /*  0Eh    Size of Info Header (always 64)  */
    WORD    Width;              /*  12h    Width of Bitmap in Pixels        */
    WORD    Height;             /*  14h    Height of Bitmap in Pixels       */
    WORD    Planes;             /*  16h    Number of Planes                 */
    WORD    BitCount;           /*  18h    Color Bits Per Pixel             */
    DWORD   Compression;        /*  1Ah    Compression Scheme (0=none)      */
    DWORD   SizeImage;          /*  1Eh    Size of bitmap in bytes          */
    DWORD   XPelsPerMeter;      /*  22h    Horz. Resolution in Pixels/Meter */
    DWORD   YPelsPerMeter;      /*  26h    Vert. Resolution in Pixels/Meter */
    DWORD   ClrUsed;            /*  2Ah    Number of Colors in Color Table  */
    DWORD   ClrImportant;       /*  2Eh    Number of Important Colors       */
    WORD    Units;              /*  32h    Resolution Mesaurement Used      */
    WORD    Reserved;           /*  34h    Reserved FIelds (always 0)       */
    WORD    Recording;          /*  36h    Orientation of Bitmap            */
    WORD    Rendering;          /*  38h    Halftone Algorithm Used on Image */
    DWORD   Size1;              /*  3Ah    Halftone Algorithm Data          */
    DWORD   Size2;              /*  3Eh    Halftone Algorithm Data          */
    DWORD   ColorEncoding;      /*  42h    Color Table Format (always 0)    */
    DWORD   Identifier;         /*  46h    Misc. Field for Application Use  */
} PM2INFOHEAD;

/*
**  Presentation Manager (OS/2) RGB Color Triple.
*/
typedef struct _PmRgbTriple
{
    BYTE   rgbBlue;             /* Blue Intensity Value  */
    BYTE   rgbGreen;            /* Green Intensity Value */
    BYTE   rgbRed;              /* Red Intensity Value   */
} PMRGBTRIPLE;

/*
**  Windows 3.x RGB Color Quadruple.
*/
typedef struct _WinRgbQuad
{
    BYTE   rgbBlue;             /* Blue Intensity Value   */
    BYTE   rgbGreen;            /* Green Intensity Value  */
    BYTE   rgbRed;              /* Red Intensity Value    */
    BYTE   rgbReserved;         /* Reserved (should be 0) */
} WINRGBQUAD;

/*
**  OS/2 2.0 RGB Color Quadruple.
*/
typedef struct _Pm2RgbQuad
{
    BYTE   rgbBlue;             /* Blue Intensity Value   */
    BYTE   rgbGreen;            /* Green Intensity Value  */
    BYTE   rgbRed;              /* Red Intensity Value    */
    BYTE   rgbReserved;         /* Reserved (should be 0) */
} PM2RGBQUAD;


/*
** Composite structure of the BMP image file format.
**
** This structure holds information for all three flavors of the BMP format.
*/
typedef struct _BmpHeader
{
    BMPINFO      Header;        /* Bitmap Header                */
    PMINFOHEAD   PmInfo;        /* OS/2 1.x Information Header  */
    PMRGBTRIPLE *PmColorTable;  /* OS/2 1.x Color Table         */
    WININFOHEAD  WinInfo;       /* Windows 3 Information Header */
    WINRGBQUAD  *WinColorTable; /* Windows 3 Color Table        */
    PM2INFOHEAD  Pm2Info;       /* OS/2 2.0 Information Header  */
    PM2RGBQUAD  *Pm2ColorTable; /* OS/2 2.0 Color Table         */
} BMPHEADER2;

/*
**  Function prototypes
*/
//SHORT ReadBmpHeader(BMPHEADER *, FILE *);
//VOID  WriteBmpHeader(BMPHEADER *, FILE *);
//SHORT BmpEncodeScanLine(BYTE *, WORD, WORD, DWORD, FILE *);
//SHORT BmpDecodeScanLine(BYTE *, WORD, WORD, DWORD, FILE *);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* BMP2_H */

