#ifndef _included_d3_func_h_
#define _included_d3_func_h_

#ifdef __cplusplus
extern "C" {
#endif


/* KJL 14:24:45 12/4/97 - render state information */
enum TRANSLUCENCY_TYPE
{
	TRANSLUCENCY_OFF,
	TRANSLUCENCY_NORMAL,
	TRANSLUCENCY_INVCOLOUR,
	TRANSLUCENCY_COLOUR,
	TRANSLUCENCY_GLOWING,
	TRANSLUCENCY_DARKENINGCOLOUR,
	TRANSLUCENCY_JUSTSETZ,
	TRANSLUCENCY_NOT_SET
};

enum FILTERING_MODE_ID
{
	FILTERING_BILINEAR_OFF,
	FILTERING_BILINEAR_ON,
	FILTERING_NOT_SET
};

typedef struct
{
	enum TRANSLUCENCY_TYPE TranslucencyMode;
	enum FILTERING_MODE_ID FilteringMode;
	int FogDistance;
	unsigned int FogIsOn :1;
	unsigned int WireFrameModeIsOn :1;

} RENDERSTATES;

typedef struct D3DTextureFormat {
//    DDSURFACEDESC ddsd; /* DDSURFACEDESC for the surface description */
    BOOL Palette;   /* is Palettized? */
    int RedBPP;         /* #red bits per pixel */
    int BlueBPP;        /* #blue bits per pixel */
    int GreenBPP;       /* #green bits per pixel */
    int IndexBPP;       /* number of bits in palette index */
} D3DTEXTUREFORMAT;

#if 0 // disabled direct3d stuff 
/*
  Direct3D globals
*/

/* 
  Maximum number of Direct3D drivers ever
  expected to be resident on the system.
*/
#define MAX_D3D_DRIVERS 5
/*
  Maximum number of texture formats ever
  expected to be reported by a Direct3D
  driver.
*/
#define MAX_TEXTURE_FORMATS 10

/*
  Description of a D3D driver.
*/

typedef struct D3DDriverInfo {
    char Name[30]; /* short name of driver */
	char About[50]; /* string about driver */
    D3DDEVICEDESC Desc; /* full driver description */
    GUID Guid; /* wacky universally unique id thingy */
	BOOL Hardware; /* accelerated driver? */
	BOOL Textures; /* Texture mapping available? */
	BOOL ZBuffer; /* Z Buffering available? */
} D3DDRIVERINFO;

/*
  Description of a D3D driver texture 
  format.
*/

typedef struct D3DTextureFormat {
    DDSURFACEDESC ddsd; /* DDSURFACEDESC for the surface description */
    BOOL Palette;   /* is Palettized? */
    int RedBPP;         /* #red bits per pixel */
    int BlueBPP;        /* #blue bits per pixel */
    int GreenBPP;       /* #green bits per pixel */
    int IndexBPP;       /* number of bits in palette index */
} D3DTEXTUREFORMAT;


typedef struct D3DInfo {
    LPDIRECT3D          lpD3D;
    LPDIRECT3DDEVICE    lpD3DDevice;
    LPDIRECT3DVIEWPORT  lpD3DViewport;
    int                 NumDrivers;
    int                 CurrentDriver;
    D3DDEVICEDESC       ThisDriver;
    D3DDRIVERINFO       Driver[MAX_D3D_DRIVERS];
    int                 CurrentTextureFormat;
    int                 NumTextureFormats;
    D3DTEXTUREFORMAT    TextureFormat[MAX_TEXTURE_FORMATS];
} D3DINFO;



/* KJL 14:24:45 12/4/97 - render state information */
enum TRANSLUCENCY_TYPE
{
	TRANSLUCENCY_OFF,
	TRANSLUCENCY_NORMAL,
	TRANSLUCENCY_INVCOLOUR,
	TRANSLUCENCY_COLOUR,
	TRANSLUCENCY_GLOWING,
	TRANSLUCENCY_DARKENINGCOLOUR,
	TRANSLUCENCY_JUSTSETZ,
	TRANSLUCENCY_NOT_SET
};

enum FILTERING_MODE_ID
{
	FILTERING_BILINEAR_OFF,
	FILTERING_BILINEAR_ON,
	FILTERING_NOT_SET
};

typedef struct
{
	enum TRANSLUCENCY_TYPE TranslucencyMode;
	enum FILTERING_MODE_ID FilteringMode;
	int FogDistance;
	unsigned int FogIsOn :1;
	unsigned int WireFrameModeIsOn :1;

} RENDERSTATES;

#endif




#ifdef __cplusplus
}
#endif

#endif /* ! _included_d3_func_h_ */
