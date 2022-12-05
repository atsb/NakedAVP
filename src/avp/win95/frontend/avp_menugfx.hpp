#ifndef __AVP_WIN95_FRONTEND_AVP_MENUGFX_HPP__
#define __AVP_WIN95_FRONTEND_AVP_MENUGFX_HPP__

/* KJL 12:27:18 26/06/98 - AvP_MenuGfx.hpp */

enum AVPMENUGFX_ID
{	
	AVPMENUGFX_CLOUDY,		   
	AVPMENUGFX_SMALL_FONT,
	AVPMENUGFX_COPYRIGHT_SCREEN,

	AVPMENUGFX_PRESENTS,
	AVPMENUGFX_AREBELLIONGAME,
	AVPMENUGFX_ALIENSVPREDATOR,

	AVPMENUGFX_SLIDERBAR,
	AVPMENUGFX_SLIDER,

	AVPMENUGFX_BACKDROP,
	AVPMENUGFX_ALIENS_LOGO,
	AVPMENUGFX_ALIEN_LOGO,		
	AVPMENUGFX_MARINE_LOGO,		
	AVPMENUGFX_PREDATOR_LOGO,

	AVPMENUGFX_GLOWY_LEFT,
	AVPMENUGFX_GLOWY_MIDDLE,
	AVPMENUGFX_GLOWY_RIGHT,

	AVPMENUGFX_MARINE_EPISODE1,
	AVPMENUGFX_MARINE_EPISODE2,
	AVPMENUGFX_MARINE_EPISODE3,
	AVPMENUGFX_MARINE_EPISODE4,
	AVPMENUGFX_MARINE_EPISODE5,
	AVPMENUGFX_MARINE_EPISODE6,

	AVPMENUGFX_MARINE_EPISODE7,
	AVPMENUGFX_MARINE_EPISODE8,
	AVPMENUGFX_MARINE_EPISODE9,
	AVPMENUGFX_MARINE_EPISODE10,
	AVPMENUGFX_MARINE_EPISODE11,

	AVPMENUGFX_PREDATOR_EPISODE1,
	AVPMENUGFX_PREDATOR_EPISODE2,
	AVPMENUGFX_PREDATOR_EPISODE3,
	AVPMENUGFX_PREDATOR_EPISODE4,
	AVPMENUGFX_PREDATOR_EPISODE5,
	AVPMENUGFX_PREDATOR_EPISODE6,

	AVPMENUGFX_PREDATOR_EPISODE7,
	AVPMENUGFX_PREDATOR_EPISODE8,
	AVPMENUGFX_PREDATOR_EPISODE9,
	AVPMENUGFX_PREDATOR_EPISODE10,
	AVPMENUGFX_PREDATOR_EPISODE11,

	AVPMENUGFX_ALIEN_EPISODE1,
	AVPMENUGFX_ALIEN_EPISODE2,
	AVPMENUGFX_ALIEN_EPISODE3,
	AVPMENUGFX_ALIEN_EPISODE4,
	AVPMENUGFX_ALIEN_EPISODE5,
	AVPMENUGFX_ALIEN_EPISODE6,
	AVPMENUGFX_ALIEN_EPISODE7,
	AVPMENUGFX_ALIEN_EPISODE8,
	AVPMENUGFX_ALIEN_EPISODE9,
	AVPMENUGFX_ALIEN_EPISODE10,

	AVPMENUGFX_WINNER_SCREEN,

	AVPMENUGFX_SPLASH_SCREEN1,
	AVPMENUGFX_SPLASH_SCREEN2,
	AVPMENUGFX_SPLASH_SCREEN3,
	AVPMENUGFX_SPLASH_SCREEN4,
	AVPMENUGFX_SPLASH_SCREEN5,

	MAX_NO_OF_AVPMENUGFXS,
};

typedef struct
{
	char *FilenamePtr;
	void *ImagePtr;

	AW_BACKUPTEXTUREHANDLE hBackup;
	int Width;
	int Height;

} AVPMENUGFX;

enum AVPMENUFORMAT_ID
{
	AVPMENUFORMAT_LEFTJUSTIFIED,
	AVPMENUFORMAT_RIGHTJUSTIFIED,
	AVPMENUFORMAT_CENTREJUSTIFIED,
};

extern void LoadAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID);
extern void LoadAllAvPMenuGfx(void);
extern void LoadAllSplashScreenGfx(void);
extern void ReleaseAllAvPMenuGfx(void);

extern int RenderMenuText(const char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format);

extern int RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format);
extern int RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue); 

extern int Hardware_RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format);
extern int Hardware_RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue);

extern int RenderMenuText_Clipped(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int topY, int bottomY);
extern void RenderSmallFontString_Wrapped(const char *textPtr,RECT* area,int alpha,int* output_x,int* output_y);
extern void Hardware_RenderKeyConfigRectangle(int alpha);
extern void RenderKeyConfigRectangle(int alpha);
extern void Hardware_RenderHighlightRectangle(int x1,int y1,int x2,int y2,int r, int g, int b);
extern void RenderHighlightRectangle(int x1,int y1,int x2,int y2, int r, int g, int b);


extern void DrawAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format);
extern void DrawAvPMenuGfx_Clipped(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format, int topY, int bottomY);
extern void DrawAvPMenuGfx_CrossFade(enum AVPMENUGFX_ID menuGfxID,enum AVPMENUGFX_ID menuGfxID2,int alpha);
extern void DrawAvPMenuGfx_Faded(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format);
extern int HeightOfMenuGfx(enum AVPMENUGFX_ID menuGfxID);


extern void ClearScreenToBlack(void);
extern void InitialiseMenuGfx(void);

#endif
