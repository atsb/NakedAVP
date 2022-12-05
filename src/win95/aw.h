#ifndef _INCLUDED_AW_H_
#define _INCLUDED_AW_H_

struct AwBackupTexture;
typedef struct AwBackupTexture * AW_BACKUPTEXTUREHANDLE;

// fake type used by opengl.c
typedef struct DIRECTDRAWSURFACE
{
	unsigned char *buf;
	int id;

	unsigned int w;
	unsigned int h;

	unsigned int IsNpot;
	unsigned int TexWidth;
	unsigned int TexHeight;
	float RecipW;
	float RecipH;

	int hasAlpha;
	int hasChroma;

	int filter;
} DIRECTDRAWSURFACE;

typedef DIRECTDRAWSURFACE * LPDIRECTDRAWSURFACE;
typedef DIRECTDRAWSURFACE DDSurface;

typedef DIRECTDRAWSURFACE DIRECT3DTEXTURE;
typedef DIRECT3DTEXTURE * LPDIRECT3DTEXTURE;
typedef DIRECT3DTEXTURE D3DTexture;

typedef int D3DTEXTUREHANDLE;

#endif /* _INCLUDED_AW_H_ */
