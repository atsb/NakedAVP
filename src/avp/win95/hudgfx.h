#ifndef _hudgfx_h
#define _hudgfx_h
/*KJL*********************************************************************************************
* HUDGFX.H                                                                                       *
*                                                                                                *
* Contains the enumerations and structures that are used in DDPLAT.CPP to draw the HUD graphics. *
*********************************************************************************************KJL*/


/*KJL****************************************************************************************
* 										D E F I N E S 										*
****************************************************************************************KJL*/

/* fonts used in HUD */
enum HUD_FONT
{
    MARINE_HUD_FONT_BLUE,
    MARINE_HUD_FONT_RED,
	MARINE_HUD_FONT_MT_SMALL,
	MARINE_HUD_FONT_MT_BIG,

	ALIEN_HUD_FONT,

	NUM_HUD_FONTS
};
/* DD Surfaces */
enum MARINE_HUD_GFX
{
    MARINE_HUD_GFX_MOTIONTRACKERBLIP,
    MARINE_HUD_GFX_NUMERALS,
    MARINE_HUD_GFX_GUNSIGHTS,
	MARINE_HUD_GFX_TRACKERFONT,
	MARINE_HUD_GFX_BLUEBAR,
    NO_OF_MARINE_HUD_GFX
};

enum PREDATOR_HUD_GFX
{
	PREDATOR_HUD_GFX_TOP,
	PREDATOR_HUD_GFX_BOTTOM,
	PREDATOR_HUD_GFX_NUMBERS,
	PREDATOR_HUD_GFX_SYMBOLS,
	NO_OF_PREDATOR_HUD_GFX
};

enum ALIEN_HUD_GFX
{
	ALIEN_HUD_GFX_BOTTOM,
	ALIEN_HUD_GFX_LEFT,
	ALIEN_HUD_GFX_RIGHT,
	ALIEN_HUD_GFX_TOP,
	ALIEN_HUD_GFX_NUMBERS,
	NO_OF_ALIEN_HUD_GFX
};

enum HUD_RES_ID
{
	HUD_RES_LO,
	HUD_RES_MED,
	HUD_RES_HI,
};
/*KJL****************************************************************************************
* 									 S T R U C T U R E S									*
****************************************************************************************KJL*/

/* description of a single DD surface used for HUD gfx */
struct DDGraphicTag 
{
	LPDIRECTDRAWSURFACE    LPDDS;

	AW_BACKUPTEXTUREHANDLE hBackup; // JH 12/2/98 changed for new gfx loading system
    RECT				   SrcRect;
};

/* description of a digit appearing on the HUD */
struct DigitPropertiesTag
{
	int	X;
    int	Y;
   	int	Font;
};	

/* info about a particular frame */
struct FrameInfoTag
{
	int	X;			   /* centre X coord of frame */
    int	Y;			   /* bottom Y of frame */
   	char *FilenamePtr; /* filename of graphic */
};	

struct HUDFontDescTag
{
	int XOffset;
	int Height;
	int Width;
};

struct LittleMDescTag
{
	int SourceTop;
	int SourceLeft;
	int Width;
	int Height;
	int X;
	int Y;
};

/*JH*****************************************************************************************
*                                     P R O T O T Y P E S                                   *
*****************************************************************************************JH*/

/* JH 3/6/97 - these functions are to 
   release hud graphics with any links to direct draw
   but still keep them in memory, so they can be restored */
extern void MinimizeAllDDGraphics(void);
extern void RestoreAllDDGraphics(void);

/*KJL****************************************************************************************
* 										 E X T E R N S 	  									*
****************************************************************************************KJL*/
//extern LPDIRECTDRAW lpDD;
//extern LPDIRECTDRAWSURFACE lpDDSBack;

#endif
