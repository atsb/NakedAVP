/*
JH - 18/02/98
Deal with lost surfaces and textures - restore them when the application is re-activated
*/

#ifndef _INCLUDED_ALT_TAB_H_
#define _INCLUDED_ALT_TAB_H_

#include "aw.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef void (* AT_PFN_RESTORETEXTURE) (D3DTexture * pTexture, void * pUser);
typedef void (* AT_PFN_RESTORESURFACE) (DDSurface * pSurface, void * pUser);

#ifdef NDEBUG
	extern void ATIncludeTexture(D3DTexture * pTexture, AW_BACKUPTEXTUREHANDLE hBackup);
	extern void ATIncludeTextureEx(D3DTexture * pTexture, AT_PFN_RESTORETEXTURE pfnRestore, void * pUser);
	extern void ATIncludeSurface(DDSurface * pSurface, AW_BACKUPTEXTUREHANDLE hBackup);
	extern void ATIncludeSurfaceEx(DDSurface * pSurface, AT_PFN_RESTORESURFACE pfnRestore, void * pUser);
#else
	extern void _ATIncludeTexture(D3DTexture * pTexture, AW_BACKUPTEXTUREHANDLE hBackup, char const * pszFile, unsigned nLine, char const * pszDebugString);
	extern void _ATIncludeTextureEx(D3DTexture * pTexture, AT_PFN_RESTORETEXTURE pfnRestore, void * pUser, char const * pszFile, unsigned nLine, char const * pszFuncName, char const * pszDebugString);
	extern void _ATIncludeSurface(DDSurface * pSurface, AW_BACKUPTEXTUREHANDLE hBackup, char const * pszFile, unsigned nLine, char const * pszDebugString);
	extern void _ATIncludeSurfaceEx(DDSurface * pSurface, AT_PFN_RESTORESURFACE pfnRestore, void * pUser, char const * pszFile, unsigned nLine, char const * pszFuncName, char const * pszDebugString);
	#define ATIncludeTexture(p,h) _ATIncludeTexture(p,h,__FILE__,__LINE__,NULL)
	#define ATIncludeTextureEx(p,f,u) _ATIncludeTextureEx(p,f,u,__FILE__,__LINE__,#f ,NULL)
	#define ATIncludeSurface(p,h) _ATIncludeSurface(p,h,__FILE__,__LINE__,NULL)
	#define ATIncludeSurfaceEx(p,f,u) _ATIncludeSurfaceEx(p,f,u,__FILE__,__LINE__,#f ,NULL)
	#define ATIncludeTextureDb(p,h,d) _ATIncludeTexture(p,h,__FILE__,__LINE__,d)
	#define ATIncludeTextureExDb(p,f,u,d) _ATIncludeTextureEx(p,f,u,__FILE__,__LINE__,#f ,d)
	#define ATIncludeSurfaceDb(p,h,d) _ATIncludeSurface(p,h,__FILE__,__LINE__,d)
	#define ATIncludeSurfaceExDb(p,f,u,d) _ATIncludeSurfaceEx(p,f,u,__FILE__,__LINE__,#f ,d)
#endif

extern void ATRemoveTexture(D3DTexture * pTexture);
extern void ATRemoveSurface(DDSurface * pSurface);

extern void ATOnAppReactivate();

#ifdef __cplusplus
	}
#endif

#endif /* ! _INCLUDED_ALT_TAB_H_ */
