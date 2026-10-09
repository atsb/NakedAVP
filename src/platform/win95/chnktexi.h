#ifndef _INCLUDED_CHNKTEXI_H_
#define _INCLUDED_CHNKTEXI_H_

#ifdef __cplusplus
	extern "C" {
#endif /* __cplusplus */

/* image number for already loaded image - really an internal function */
#define GEI_NOTLOADED (-1)
extern int GetExistingImageNum(char const * pszActualFileName);

enum /* flags, etc */
{
	/* destination surface type */
	LIO_CHIMAGE      = 0x00000000U, /* Chris Humphries texture */
	LIO_DDSURFACE    = 0x00000001U, /* Direct Draw Surface */
	LIO_D3DTEXTURE   = 0x00000002U, /* Direct 3D Texture */
	_LIO_SURFTYPEMASK= 0x00000003U,
	/* target memory type for DDSURFACE only - D3DTextures dest depends on driver */
	LIO_SYSMEM       = 0x00000000U, /* system memory */
	LIO_VIDMEM       = 0x00000004U, /* video memory */
	/* transparency flags - unless specified in the file */
	LIO_NONTRANSP    = 0x00000000U, /* no transparency */
	LIO_TRANSPARENT  = 0x00000008U, /* has transparency */
	/* alpha or chroma key? */
	LIO_USEALPHA     = 0x00000000U, /* use alpha mask if available instead of chroma keying */
	LIO_CHROMAKEY    = 0x00000010U, /* use chroma key even if surface has alpha channel */
	/* path flags */
	LIO_ABSOLUTEPATH = 0x00000000U, /* path is correct */
	LIO_RELATIVEPATH = 0x00000020U, /* path is relative to a textures directory */
	LIO_RIFFPATH     = 0x00000040U, /* current RIF file used to build path and extension */
	_LIO_PATHTYPEMASK= 0x00000060U,
	/* mip maps? */
	LIO_NOMIPMAPS    = 0x00000000U, /* no mip maps */
	LIO_LOADMIPMAPS  = 0x00000080U, /* load mip maps if available */
	/* restorable ? */
	LIO_NORESTORE    = 0x00000000U, /* not going to be restorable */
	LIO_RESTORABLE   = 0x00000100U, /* put something in imageheader to allow restoring */
};

/* CL_LoadImageOnce relies on this value to be 1 greater
   than the index of the last loaded image */
extern int NumImages;

/* directories used with the LIO_RIFFPATH flag */
extern char const * GameTex_Directory;
extern char const * SubShps_Directory;
extern char const * GenTex_Directory;
extern char const * FixTex_Directory;
extern char const * ToolsTex_Directory;

/* game mode for use with the above */
extern char const * cl_pszGameMode;

/* directories used with the LIO_RELATIVEPATH flag
   these are searched in order*/
extern char const * FirstTex_Directory;
extern char const * SecondTex_Directory;

/* returns GEI_NOTLOADED on failure */
extern int CL_LoadImageOnce(char const * pszFileName, unsigned fFlagsEtc);

/* returns NULL on failure, or pointer to pszDestBuf on success, nBufSize includes nul terminator */
extern char * CL_GetImageFileName(char * pszDestBuf, unsigned nBufSize, char const * pszFileName, unsigned fFlagsEtc);

#ifdef __cplusplus
	}
#endif /* __cplusplus */

#endif /* !_INCLUDED_CHNKTEXI_H_ */
