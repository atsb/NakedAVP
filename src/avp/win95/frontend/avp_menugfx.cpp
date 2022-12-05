#include "3dc.h"
#include "inline.h"

#include "tallfont.hpp"
#include "strtab.hpp"

#include "awtexld.h"

#include "chnktexi.h"
#include "hud_layout.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "ffstdio.h"


extern void D3D_RenderHUDString(char *stringPtr,int x,int y,int colour);

extern "C"
{
#include "avp_menus.h"
extern unsigned char *ScreenBuffer;
extern long BackBufferPitch;
/* extern DDPIXELFORMAT DisplayPixelFormat; */
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

char AAFontWidths[256];

AVPMENUGFX AvPMenuGfxStorage[MAX_NO_OF_AVPMENUGFXS] =
{
	{"Menus\\fractal.rim"},
	{"Common\\aa_font.rim"},// Warning! Texture from common used

	{"Menus\\copyright.rim"},

	{"Menus\\FIandRD.rim"},
	{"Menus\\presents.rim"},
	{"Menus\\AliensVPredator.rim"},
	
	{"Menus\\sliderbar.rim"},//AVPMENUGFX_SLIDERBAR,
	{"Menus\\slider.rim"},//AVPMENUGFX_SLIDER,

	{"Menus\\starfield.rim"},
	{"Menus\\aliens.rim"},
	{"Menus\\Alien.rim"},
	{"Menus\\Marine.rim"},
	{"Menus\\Predator.rim"},

	{"Menus\\glowy_left.rim"},
	{"Menus\\glowy_middle.rim"},
	{"Menus\\glowy_right.rim"},
	
	// Marine level
	{"Menus\\MarineEpisode1.rim"},
	{"Menus\\MarineEpisode2.rim"},
	{"Menus\\MarineEpisode3.rim"},
	{"Menus\\MarineEpisode4.rim"},
	{"Menus\\MarineEpisode5.rim"},
	{"Menus\\MarineEpisode6.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	
	// Predator level
	{"Menus\\PredatorEpisode1.rim"},
	{"Menus\\PredatorEpisode2.rim"},
	{"Menus\\PredatorEpisode3.rim"},
	{"Menus\\PredatorEpisode4.rim"},
	{"Menus\\PredatorEpisode5.rim"},
	{"Menus\\PredatorEpisode5.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},

	// Alien level
	{"Menus\\AlienEpisode2.rim"},
	{"Menus\\AlienEpisode4.rim"},
	{"Menus\\AlienEpisode1.rim"},
	{"Menus\\AlienEpisode3.rim"},
	{"Menus\\AlienEpisode5.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},
	{"Menus\\bonus.rim"},

	// Splash screens
	#if MARINE_DEMO
	{"MarineSplash\\splash00.rim"},
	{"MarineSplash\\splash01.rim"},
	{"MarineSplash\\splash02.rim"},
	{"MarineSplash\\splash03.rim"},
	{"MarineSplash\\splash04.rim"},
	{"MarineSplash\\splash05.rim"},
	#elif ALIEN_DEMO
	{"AlienSplash\\splash00.rim"},
	{"AlienSplash\\splash01.rim"},
	{"AlienSplash\\splash02.rim"},
	{"AlienSplash\\splash03.rim"},
	{"AlienSplash\\splash04.rim"},
	{"AlienSplash\\splash05.rim"},
	#else
	{"PredatorSplash\\splash00.rim"},
	{"PredatorSplash\\splash01.rim"},
	{"PredatorSplash\\splash02.rim"},
	{"PredatorSplash\\splash03.rim"},
	{"PredatorSplash\\splash04.rim"},
	{"PredatorSplash\\splash05.rim"},
	#endif
};

static void LoadMenuFont(void);
static void UnloadMenuFont(void);
static int RenderSmallFontString(char *textPtr,int sx,int sy,int alpha, int red, int green, int blue);
static void CalculateWidthsOfAAFont(void);
extern void DrawAvPMenuGlowyBar(int topleftX, int topleftY, int alpha, int length);
extern void DrawAvPMenuGlowyBar_Clipped(int topleftX, int topleftY, int alpha, int length, int topY, int bottomY);
static void LoadMenuFont(void)
{
	char buffer[100];
	IndexedFont_Kerned_Column :: Create
	(
		IntroFont_Light, // FontIndex I_Font_New,
		CL_GetImageFileName(buffer, 100, "Menus\\IntroFont.rim", LIO_RELATIVEPATH),
		33,//21, // int HeightPerChar_New,
		5, // int SpaceWidth_New,
		32 // ASCIICodeForInitialCharacter
	);
}

static void UnloadMenuFont(void)
{
	IndexedFont :: UnloadFont( IntroFont_Light );
}
extern int LengthOfMenuText(char *textPtr)
{
	IndexedFont* pFont = IndexedFont :: GetFont(IntroFont_Light);

	return (pFont->CalcSize(textPtr).w);
}

extern int RenderMenuText(const char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format) 
{
	IndexedFont* pFont = IndexedFont :: GetFont(IntroFont_Light);
	r2pos R2Pos_StartOfRow;
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			x -= (pFont->CalcSize(textPtr).w);
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			x -= (pFont->CalcSize(textPtr).w)/2;
			break;
		}
	}

	LOCALASSERT(x>0);
	
	if (alpha >BRIGHTNESS_OF_DARKENED_ELEMENT)
	{
		int size = pFont->CalcSize(textPtr).w - 18;
		if (size<18) size = 18;
		DrawAvPMenuGfx(AVPMENUGFX_GLOWY_LEFT,x+18,y-8,alpha,AVPMENUFORMAT_RIGHTJUSTIFIED);
		DrawAvPMenuGlowyBar(x+18,y-8,alpha,size-18);
//		for (int i=18; i<size; i++)
//		DrawAvPMenuGfx(AVPMENUGFX_GLOWY_MIDDLE,x+i,y-8,alpha,AVPMENUFORMAT_LEFTJUSTIFIED);
		DrawAvPMenuGfx(AVPMENUGFX_GLOWY_RIGHT,x+size,y-8,alpha,AVPMENUFORMAT_LEFTJUSTIFIED);
	}
	R2Pos_StartOfRow = r2pos(x,y);
	{
		{
		   	SCString* pSCString_Name = new SCString(textPtr);

			pFont -> RenderString_Unclipped
			(
				R2Pos_StartOfRow, // struct r2pos& R2Pos_Cursor,
				alpha, // int FixP_Alpha,
				*pSCString_Name // const SCString& SCStr
			);

			pSCString_Name -> R_Release();
		}

	}
	return R2Pos_StartOfRow.x;
}
extern int RenderMenuText_Clipped(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int topY, int bottomY) 
{
	IndexedFont* pFont = IndexedFont :: GetFont(IntroFont_Light);
	r2pos R2Pos_StartOfRow;
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			x -= (pFont->CalcSize(textPtr).w);
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			x -= (pFont->CalcSize(textPtr).w)/2;
			break;
		}
	}

	LOCALASSERT(x>0);

	if (alpha > BRIGHTNESS_OF_DARKENED_ELEMENT)
	{
		int size = pFont->CalcSize(textPtr).w - 18;
		if (size<18) size = 18;
		DrawAvPMenuGfx_Clipped(AVPMENUGFX_GLOWY_LEFT,x+18,y-8,alpha,AVPMENUFORMAT_RIGHTJUSTIFIED,topY,bottomY);
		DrawAvPMenuGlowyBar_Clipped(x+18,y-8,alpha,size-18,topY,bottomY);
		DrawAvPMenuGfx_Clipped(AVPMENUGFX_GLOWY_RIGHT,x+size,y-8,alpha,AVPMENUFORMAT_LEFTJUSTIFIED,topY,bottomY);
	}

	R2Pos_StartOfRow = r2pos(x,y);
	const struct r2rect R2Rect_Clip = r2rect(0,topY,0,bottomY);
	{
		{
		   	SCString* pSCString_Name = new SCString(textPtr);

			pFont -> RenderString_Clipped
			(
				R2Pos_StartOfRow, // struct r2pos& R2Pos_Cursor,
				R2Rect_Clip,				 //const struct r2rect& R2Rect_Clip
				alpha, // int FixP_Alpha,
				*pSCString_Name // const SCString& SCStr
			);

			pSCString_Name -> R_Release();
		}

	}
	return R2Pos_StartOfRow.x;
}


extern int RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format) 
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}
	}

	LOCALASSERT(x>0);

	x = RenderSmallFontString(textPtr,x,y,alpha,ONE_FIXED,ONE_FIXED,ONE_FIXED);
	return x;
}
extern int RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue) 
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}
	}

	LOCALASSERT(x>0);

	x = RenderSmallFontString(textPtr,x,y,alpha,red,green,blue);
	return x;
}

extern int Hardware_RenderSmallMenuText(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format) 
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}	
	}

	LOCALASSERT(x>0);

	{
		unsigned int colour = alpha>>8;
		if (colour>255) colour = 255;
		colour = (colour<<24)+0xffffff;
		D3D_RenderHUDString(textPtr,x,y,colour);
	}
	return x;
}

extern int Hardware_RenderSmallMenuText_Coloured(char *textPtr, int x, int y, int alpha, enum AVPMENUFORMAT_ID format, int red, int green, int blue)
{
	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			int length = 0;
			char *ptr = textPtr;

			while(*ptr)
			{
				length+=AAFontWidths[*ptr++];
			}

			x -= length/2;
			break;
		}	
	}

	LOCALASSERT(x>0);

	{
		unsigned int colour = alpha>>8;
		if (colour>255) colour = 255;
		colour = (colour<<24);
		colour += MUL_FIXED(red,255)<<16;
		colour += MUL_FIXED(green,255)<<8;
		colour += MUL_FIXED(blue,255);
		D3D_RenderHUDString(textPtr,x,y,colour);
	}
	return x;
}

extern void Hardware_RenderKeyConfigRectangle(int alpha)
{
	extern void D3D_DrawRectangle(int x, int y, int w, int h, int alpha);
	D3D_DrawRectangle(10,ScreenDescriptorBlock.SDB_Height/2+25-115,ScreenDescriptorBlock.SDB_Width-20,250,alpha);
}
extern void RenderKeyConfigRectangle(int alpha)
{
	int x1 = 10;
	int x2 = ScreenDescriptorBlock.SDB_Width-10;
	int y1 = ScreenDescriptorBlock.SDB_Height/2+25-115;
	int y2 = ScreenDescriptorBlock.SDB_Height/2+25-115+250;
	int x,y;
	int c;

	c = MUL_FIXED(DisplayPixelFormat.dwRBitMask,alpha) & DisplayPixelFormat.dwRBitMask;
 	c |= MUL_FIXED(DisplayPixelFormat.dwGBitMask,alpha) & DisplayPixelFormat.dwGBitMask;
	c |= MUL_FIXED(DisplayPixelFormat.dwBBitMask,alpha) & DisplayPixelFormat.dwBBitMask;

	y=y1;
	{
		unsigned short *destPtr = (unsigned short *)(ScreenBuffer + x1*2 + y*BackBufferPitch);

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}
	y=y2;
	{
		unsigned short *destPtr = (unsigned short *)(ScreenBuffer + x1*2 + y*BackBufferPitch);

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}
	{

		for (y=y1+1; y<y2; y++)
		{
			unsigned short *destPtr = (unsigned short *)(ScreenBuffer + x1*2 + y*BackBufferPitch);
			*destPtr |= c;
		}
	}
	{

		for (y=y1+1; y<y2; y++)
		{
			unsigned short *destPtr = (unsigned short *)(ScreenBuffer + x2*2 + y*BackBufferPitch);
			*destPtr |= c;
		}
	}
}
extern void Hardware_RenderHighlightRectangle(int x1,int y1,int x2,int y2,int r, int g, int b)
{
	r2rect rectangle
	(
		x1,
		y1,
		x2,
		y2
		
	);
	rectangle . AlphaFill
	(
		r, // unsigned char R,
		g,// unsigned char G,
		b,// unsigned char B,
	   	255 // unsigned char translucency
	);

}
extern void RenderHighlightRectangle(int x1,int y1,int x2,int y2, int r, int g, int b)
{
	int x,y;
	short c;
	
	c = ((DisplayPixelFormat.dwRBitMask*r)/256)&(DisplayPixelFormat.dwRBitMask);
	c |= ((DisplayPixelFormat.dwGBitMask*g)/256)&(DisplayPixelFormat.dwGBitMask);
	c |= ((DisplayPixelFormat.dwBBitMask*b)/256)&(DisplayPixelFormat.dwBBitMask);
	for (y=y1; y<=y2; y++)
	{
		unsigned short *destPtr = (unsigned short *)(ScreenBuffer + x1*2 + y*BackBufferPitch);

		for (x=x1; x<=x2; x++)
		{
			*destPtr |= c;
			destPtr++;
		}
	}
}

static int RenderSmallFontString(char *textPtr,int sx,int sy,int alpha, int red, int green, int blue)
{
	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	int extra = 0;
	int alphaR = MUL_FIXED(alpha,red);
	int alphaG = MUL_FIXED(alpha,green);
	int alphaB = MUL_FIXED(alpha,blue);

	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	surface = gfxPtr->ImagePtr;

	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);


	while( *textPtr )
	{
		char c = *textPtr++;

		if (c>=' ')
		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;

			srcPtr = (unsigned short *)(ddsdimage.lpSurface) + (topLeftU)+(topLeftV*ddsdimage.lPitch/2);
			
			int x,y;

			for (y=sy; y<HUD_FONT_HEIGHT+sy; y++)
			{
				destPtr = (unsigned short *)(ScreenBuffer + sx*2 + y*BackBufferPitch);

				for (x=0; x<HUD_FONT_WIDTH; x++)
				{
					if (*srcPtr)
					{
						unsigned int srcR,srcG,srcB;
						unsigned int destR,destG,destB;

						destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
						destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
						destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

						srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
						srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
						srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

						destR += MUL_FIXED(alphaR,srcR);
						if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
						else destR &= DisplayPixelFormat.dwRBitMask;

						destG += MUL_FIXED(alphaG,srcG);
						if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
						else destG &= DisplayPixelFormat.dwGBitMask;

						destB += MUL_FIXED(alphaB,srcB);
						if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
						else destB &= DisplayPixelFormat.dwBBitMask;

						*destPtr = (short)(destR|destG|destB);
					}
					destPtr++;
					srcPtr++;
				}
				srcPtr += (ddsdimage.lPitch/2) - HUD_FONT_WIDTH; 
			}
			sx += AAFontWidths[c];
			#if 0
			if(c!=32)
			{
				extra += 8-AAFontWidths[c];
			}
			else
			{
				sx+=extra+8-AAFontWidths[c];
				extra=0;
			}
			#endif
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);
	return sx;
}


extern void RenderSmallFontString_Wrapped(const char *textPtr,RECT* area,int alpha,int* output_x,int* output_y)
{
	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	int extra = 0;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;
	int wordWidth;
		

	int sx=area->left;
	int sy=area->top;

	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	surface = gfxPtr->ImagePtr;

	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	
	/*
	Determine area used by text , so we can draw it centrally
	*/
	{
		char* textPtr2=textPtr;
		while( *textPtr2)
		{
			//find the width of the next word
			int widthFromSpaces=0;
			int widthFromChars=0;

			// get width used by spaces	before this word
			while(*textPtr2 && *textPtr2==' ')
			{
				widthFromSpaces+=AAFontWidths[*textPtr2++];
			}
			
			//get width used by word
			while(*textPtr2 && *textPtr2!=' ')
			{
				widthFromChars+=AAFontWidths[*textPtr2++];
			}
			wordWidth=widthFromSpaces+widthFromChars;

			if(wordWidth> area->right-sx)
			{
				if(wordWidth >area->right-area->left)
				{
					int extraLinesNeeded=0;
					//word is too long too fit on one line , so it will have to be split
					wordWidth-=(area->right-sx);
					
					//advance to the beginning of the next line
					sy+=HUD_FONT_HEIGHT;
					sx=area->left;

					//see how many extra whole lines are equired
					extraLinesNeeded=wordWidth/(area->right-area->left);

					sy+=HUD_FONT_HEIGHT*extraLinesNeeded;
					wordWidth %= (area->right-area->left);

					//make sure we haven't gone off the bottom
					if(sy+HUD_FONT_HEIGHT> area->bottom) break;
					

				}
				else
				{
					//word to long to fit on this line , so go to next line
					sy+=HUD_FONT_HEIGHT;
					sx=area->left;

					//make sure we haven't gone off the bottom
					if(sy+HUD_FONT_HEIGHT> area->bottom) break;

					//make sure the word will fit on a line anyway
					if(wordWidth> area->right-sx) break;

					//don't bother drawing spaces at the start of the new line
					wordWidth-=widthFromSpaces;
				}

			}
			sx+=wordWidth;
		}

		//if the string fits on one line , centre it horizontally
		if(sy==area->top)
		{
			sx=area->left+ (area->right-sx)/2;
		}
		else
		{
			sx=area->left;
		}

		//centre string vertically
		sy+=HUD_FONT_HEIGHT;
		if(sy<area->bottom)
		{
			sy=area->top + (area->bottom-sy)/2;
		}
		else
		{
			sy=area->top;
		}

	}
	


	while( *textPtr )
	{
		//find the width of the next word
		char* textPtr2=textPtr;
		wordWidth=0;

		// get width used by spaces	before this word
		while(*textPtr2 && *textPtr2==' ')
		{
			wordWidth+=AAFontWidths[*textPtr2++];
		}
		
		//get width used by word
		while(*textPtr2 && *textPtr2!=' ')
		{
			wordWidth+=AAFontWidths[*textPtr2++];
		}

		if(wordWidth> area->right-sx)
		{
			if(wordWidth>area->right - area->left)
			{
				//word is too long too fit on one line , so we'll just have to allow it to be split
			}
			else
			{
				//word to long to fit on this line , so go to next line
				sy+=HUD_FONT_HEIGHT;
				sx=area->left;

				//make sure we haven't gone off the bottom
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;

				//make sure the word will fit on a line anyway
				if(wordWidth> area->right-sx) break;

				//don't bother drawing spaces at the start of the new line
				while(*textPtr && *textPtr==' ')
				{
					*textPtr++;
				}
			}

		}

		//'render' the spaces
		while(*textPtr && *textPtr==' ')
		{
			sx+=AAFontWidths[*textPtr++];
		}

		if(sx>area->right)
		{
			//gone of the end of the line doing spaces
			while(sx>area->right)
			{
				sx-=(area->right-area->left);
				sy+=HUD_FONT_HEIGHT;
			}
			//make sure we haven't gone off the bottom
			if(sy+HUD_FONT_HEIGHT> area->bottom) break;

		}

		//render this word
		while(*textPtr && *textPtr!=' ')
		{
			char c = *textPtr++;
			int letterWidth = AAFontWidths[c];
			
			if(sx+letterWidth>area->right)
			{
				//need to go on to the next line
				sx=area->left;
				sy+=HUD_FONT_HEIGHT;

				//make sure we haven't gone off the bottom
				if(sy+HUD_FONT_HEIGHT> area->bottom) break;

			}

			if (c>=' ' || c<='z')
			{
				int topLeftU = 1+((c-32)&15)*16;
				int topLeftV = 1+((c-32)>>4)*16;

				srcPtr = (unsigned short *)(ddsdimage.lpSurface) + (topLeftU)+(topLeftV*ddsdimage.lPitch/2);
				
				int x,y;

				for (y=sy; y<HUD_FONT_HEIGHT+sy; y++)
				{
					destPtr = (unsigned short *)(ScreenBuffer + sx*2 + y*BackBufferPitch);

					for (x=0; x<HUD_FONT_WIDTH; x++)
					{
						if (*srcPtr)
						{
							unsigned int srcR,srcG,srcB;
							unsigned int destR,destG,destB;

							destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
							destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
							destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

							srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
							srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
							srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

							destR += MUL_FIXED(alpha,srcR);
							if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
							else destR &= DisplayPixelFormat.dwRBitMask;

							destG += MUL_FIXED(alpha,srcG);
							if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
							else destG &= DisplayPixelFormat.dwGBitMask;

							destB += MUL_FIXED(alpha,srcB);
							if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
							else destB &= DisplayPixelFormat.dwBBitMask;

							*destPtr = (short)(destR|destG|destB);
						}
						destPtr++;
						srcPtr++;
					}
					srcPtr += (ddsdimage.lPitch/2) - HUD_FONT_WIDTH; 
				}
				sx += AAFontWidths[c];
				#if 0
				if(c!=32)
				{
					extra += 8-AAFontWidths[c];
				}
				else
				{
					sx+=extra+8-AAFontWidths[c];
					extra=0;
				}
				#endif
			}
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);

	if(output_x) *output_x=sx;
	if(output_y) *output_y=sy;
}



extern void LoadAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	AVPMENUGFX *gfxPtr;
	char buffer[100];
	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
		
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	
	CL_GetImageFileName(buffer, 100, gfxPtr->FilenamePtr, LIO_RELATIVEPATH);
	
	//see if graphic can be found in fast file
	unsigned int fastFileLength;
	void const * pFastFileData = ffreadbuf(buffer,&fastFileLength);

	if(pFastFileData)
	{
		//load from fast file
		#if 0
		gfxPtr->ImagePtr = AwCreateSurface
							(
								"pxfXYB",
								pFastFileData,
								fastFileLength,
								AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
								&(gfxPtr->Width),
								&(gfxPtr->Height),
								&(gfxPtr->hBackup)
							);
		#else
		gfxPtr->ImagePtr = AwCreateSurface
							(
								"pxfXY",
								pFastFileData,
								fastFileLength,
								AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
								&(gfxPtr->Width),
								&(gfxPtr->Height)
							);
		#endif
	}
	else
	{
		//load graphic from rim file
		gfxPtr->ImagePtr = AwCreateSurface
							(
								"sfXYB",
								buffer,
								AW_TLF_TRANSP|AW_TLF_CHROMAKEY,
								&(gfxPtr->Width),
								&(gfxPtr->Height),
								&(gfxPtr->hBackup)
							);
	}
	GLOBALASSERT(gfxPtr->ImagePtr);
//	GLOBALASSERT(gfxPtr->hBackup);
	GLOBALASSERT(gfxPtr->Width>0);
	GLOBALASSERT(gfxPtr->Height>0);
	gfxPtr->hBackup=0;
//	ATIncludeSurface(gfxPtr->ImagePtr,gfxPtr->hBackup);
}

static void ReleaseAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	AVPMENUGFX *gfxPtr;
	
	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
		
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	
	GLOBALASSERT(gfxPtr);
	GLOBALASSERT(gfxPtr->ImagePtr);

  //	ATRemoveSurface(gfxPtr->ImagePtr);
	ReleaseDDSurface(gfxPtr->ImagePtr);
	
	gfxPtr->ImagePtr = NULL;
	#if 0
	if (gfxPtr->hBackup)
	{
		AwDestroyBackupTexture(gfxPtr->hBackup);
		gfxPtr->hBackup = NULL;
	}
	#endif
}

extern void LoadAllAvPMenuGfx(void)
{
	int i = 0;
	while(i<AVPMENUGFX_WINNER_SCREEN)
	{
		LoadAvPMenuGfx((enum AVPMENUGFX_ID)i++);
	}

	LoadMenuFont();	
	{	
		DDSURFACEDESC ddsdimage;
		unsigned short *srcPtr;
		   
		AVPMENUGFX *gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_CLOUDY];
		LPDIRECTDRAWSURFACE surface;

		surface = gfxPtr->ImagePtr;
	  		
		memset(&ddsdimage, 0, sizeof(ddsdimage));
		ddsdimage.dwSize = sizeof(ddsdimage);

		/* lock the image */
		while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

		srcPtr = (unsigned short *)ddsdimage.lpSurface;

		{
			int x,y;

			for (y=0; y<gfxPtr->Height; y++)
			{
				for (x=0; x<gfxPtr->Width; x++)
				{
					extern int CloudTable[128][128];
					int r = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
//					int g = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
//					int b = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;
					r = DIV_FIXED(r,DisplayPixelFormat.dwRBitMask);
//					g = DIV_FIXED(g,DisplayPixelFormat.dwGBitMask);
//					b = DIV_FIXED(b,DisplayPixelFormat.dwBBitMask);
					CloudTable[x][y]=r;
//					CloudTable[x][y]=g;
//					CloudTable[x][y]=b;
					srcPtr++;
				}
				srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
			}
		}
   	
	   	surface->Unlock((LPVOID)ddsdimage.lpSurface);
	}
	CalculateWidthsOfAAFont();
}
extern void LoadAllSplashScreenGfx(void)
{
	int i = AVPMENUGFX_SPLASH_SCREEN1;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		LoadAvPMenuGfx((enum AVPMENUGFX_ID)i++);
	}
}

extern void InitialiseMenuGfx(void)
{
	int i=0;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		AvPMenuGfxStorage[i++].ImagePtr = 0;
	}
}
extern void ReleaseAllAvPMenuGfx(void)
{
	int i=0;
	while(i<MAX_NO_OF_AVPMENUGFXS)
	{
		if (AvPMenuGfxStorage[i].ImagePtr)
		{
			ReleaseAvPMenuGfx((enum AVPMENUGFX_ID)i);
		}
		i++;
	}	
	UnloadMenuFont();
}

extern void DrawAvPMenuGfx(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{																	
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}


	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned short *)ddsdimage.lpSurface;
	int length = gfxPtr->Width;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}
	if (length<=0) return;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<length; x++)
			{
				*destPtr = *srcPtr;
				destPtr++;
				srcPtr++;
			}
			srcPtr += (ddsdimage.lPitch/2) - length; 
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<length; x++)
			{
				if (*srcPtr)
				{
					unsigned int srcR,srcG,srcB;
					unsigned int destR,destG,destB;

					destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
					destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
					destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

					srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
					srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
					srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

					destR += MUL_FIXED(alpha,srcR);
					if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
					else destR &= DisplayPixelFormat.dwRBitMask;

					destG += MUL_FIXED(alpha,srcG);
					if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
					else destG &= DisplayPixelFormat.dwGBitMask;

					destB += MUL_FIXED(alpha,srcB);
					if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
					else destB &= DisplayPixelFormat.dwBBitMask;

					*destPtr = (short)(destR|destG|destB);
				}
				destPtr++;
				srcPtr++;
			}
			srcPtr += (ddsdimage.lPitch/2) - length; 
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);
	
	UnlockSurface();
}
extern void DrawAvPMenuGlowyBar(int topleftX, int topleftY, int alpha, int length)
{							
	enum AVPMENUGFX_ID menuGfxID = AVPMENUGFX_GLOWY_MIDDLE;										
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;

	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned short *)ddsdimage.lpSurface;

	if (ScreenDescriptorBlock.SDB_Width - topleftX < length)
	{
		length = ScreenDescriptorBlock.SDB_Width - topleftX;
	}

	if (length<0) length = 0;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<length; x++)
			{
				*destPtr = *srcPtr;
				destPtr++;
			}
			srcPtr += (ddsdimage.lPitch/2); 
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<length; x++)
			{
				if (*srcPtr)
				{
					unsigned int srcR,srcG,srcB;
					unsigned int destR,destG,destB;

					destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
					destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
					destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

					srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
					srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
					srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

					destR += MUL_FIXED(alpha,srcR);
					if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
					else destR &= DisplayPixelFormat.dwRBitMask;

					destG += MUL_FIXED(alpha,srcG);
					if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
					else destG &= DisplayPixelFormat.dwGBitMask;

					destB += MUL_FIXED(alpha,srcB);
					if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
					else destB &= DisplayPixelFormat.dwBBitMask;

					*destPtr = (short)(destR|destG|destB);
				}
				destPtr++;
			}
			srcPtr += (ddsdimage.lPitch/2); 
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);
	
	UnlockSurface();
}
extern void DrawAvPMenuGlowyBar_Clipped(int topleftX, int topleftY, int alpha, int length, int topY, int bottomY)
{							
	enum AVPMENUGFX_ID menuGfxID = AVPMENUGFX_GLOWY_MIDDLE;										
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;

	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned short *)ddsdimage.lpSurface;

	if (length<0) length = 0;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			if(y>=topY && y<=bottomY)
			{
				destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
				for (x=0; x<length; x++)
				{
					*destPtr = *srcPtr;
					destPtr++;
				}
				srcPtr += (ddsdimage.lPitch/2); 
			}
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			if(y>=topY && y<=bottomY)
			{
				destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

				for (x=0; x<length; x++)
				{
					if (*srcPtr)
					{
						unsigned int srcR,srcG,srcB;
						unsigned int destR,destG,destB;

						destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
						destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
						destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

						srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
						srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
						srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

						destR += MUL_FIXED(alpha,srcR);
						if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
						else destR &= DisplayPixelFormat.dwRBitMask;

						destG += MUL_FIXED(alpha,srcG);
						if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
						else destG &= DisplayPixelFormat.dwGBitMask;

						destB += MUL_FIXED(alpha,srcB);
						if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
						else destB &= DisplayPixelFormat.dwBBitMask;

						*destPtr = (short)(destR|destG|destB);
					}
					destPtr++;
				}
			}
			srcPtr += (ddsdimage.lPitch/2); 
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);
	
	UnlockSurface();
}
extern void DrawAvPMenuGfx_CrossFade(enum AVPMENUGFX_ID menuGfxID,enum AVPMENUGFX_ID menuGfxID2,int alpha)
{																	
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage,ddsdimage2;
   	unsigned int *destPtr;
	unsigned int  *srcPtr;
	unsigned int  *srcPtr2;
	AVPMENUGFX *gfxPtr;
	AVPMENUGFX *gfxPtr2;
	LPDIRECTDRAWSURFACE surface;
	LPDIRECTDRAWSURFACE surface2;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;
	GLOBALASSERT(menuGfxID2 < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr2 = &AvPMenuGfxStorage[menuGfxID2];
	surface2 = gfxPtr2->ImagePtr;

  	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned int *)ddsdimage.lpSurface;
										   
	memset(&ddsdimage2, 0, sizeof(ddsdimage2));
	ddsdimage2.dwSize = sizeof(ddsdimage2);

	/* lock the image */
	while (surface2->Lock(NULL, &ddsdimage2, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr2 = (unsigned int *)ddsdimage2.lpSurface;

	if (alpha==ONE_FIXED)
	{
		int x,y;

		for (y=0; y<480; y++)
		{
			destPtr = (unsigned int *)(ScreenBuffer + y*BackBufferPitch);

			for (x=0; x<320; x++)
			{
				*destPtr = *srcPtr;
				destPtr++;
				srcPtr++;
			}
			srcPtr += (ddsdimage.lPitch/4) - 320; 
		}
	}
	else
	{
		int x,y;

		for (y=0; y<480; y++)
		{
			destPtr = (unsigned int *)(ScreenBuffer + y*BackBufferPitch);

			for (x=0; x<320; x++)
			{
				int destination;
				int src1=*srcPtr,src2=*srcPtr2;
				
				{
					unsigned int srcR,srcG,srcB;
					unsigned int srcR2,srcG2,srcB2;

					srcR2 = (src2) & DisplayPixelFormat.dwRBitMask;
					srcG2 = (src2) & DisplayPixelFormat.dwGBitMask;
					srcB2 = (src2) & DisplayPixelFormat.dwBBitMask;

					srcR = (src1) & DisplayPixelFormat.dwRBitMask;
					srcG = (src1) & DisplayPixelFormat.dwGBitMask;
					srcB = (src1) & DisplayPixelFormat.dwBBitMask;

					srcR2 = MUL_FIXED(ONE_FIXED-alpha,srcR2)+MUL_FIXED(alpha,srcR);
					if (srcR2>DisplayPixelFormat.dwRBitMask) srcR2 = DisplayPixelFormat.dwRBitMask;
					else srcR2 &= DisplayPixelFormat.dwRBitMask;

					srcG2 = MUL_FIXED(ONE_FIXED-alpha,srcG2)+MUL_FIXED(alpha,srcG);
					if (srcG2>DisplayPixelFormat.dwGBitMask) srcG2 = DisplayPixelFormat.dwGBitMask;
					else srcG2 &= DisplayPixelFormat.dwGBitMask;

					srcB2 = MUL_FIXED(ONE_FIXED-alpha,srcB2)+MUL_FIXED(alpha,srcB);
					if (srcB2>DisplayPixelFormat.dwBBitMask) srcB2 = DisplayPixelFormat.dwBBitMask;
					else srcB2 &= DisplayPixelFormat.dwBBitMask;

					destination = (srcR2|srcG2|srcB2);
				}
				src1>>=16;
				src2>>=16;
				{
					unsigned int srcR,srcG,srcB;
					unsigned int srcR2,srcG2,srcB2;

					srcR2 = (src2) & DisplayPixelFormat.dwRBitMask;
					srcG2 = (src2) & DisplayPixelFormat.dwGBitMask;
					srcB2 = (src2) & DisplayPixelFormat.dwBBitMask;
						   
					srcR = (src1) & DisplayPixelFormat.dwRBitMask;
					srcG = (src1) & DisplayPixelFormat.dwGBitMask;
					srcB = (src1) & DisplayPixelFormat.dwBBitMask;

					srcR2 = MUL_FIXED(ONE_FIXED-alpha,srcR2)+MUL_FIXED(alpha,srcR);
					if (srcR2>DisplayPixelFormat.dwRBitMask) srcR2 = DisplayPixelFormat.dwRBitMask;
					else srcR2 &= DisplayPixelFormat.dwRBitMask;

					srcG2 = MUL_FIXED(ONE_FIXED-alpha,srcG2)+MUL_FIXED(alpha,srcG);
					if (srcG2>DisplayPixelFormat.dwGBitMask) srcG2 = DisplayPixelFormat.dwGBitMask;
					else srcG2 &= DisplayPixelFormat.dwGBitMask;

					srcB2 = MUL_FIXED(ONE_FIXED-alpha,srcB2)+MUL_FIXED(alpha,srcB);
					if (srcB2>DisplayPixelFormat.dwBBitMask) srcB2 = DisplayPixelFormat.dwBBitMask;
					else srcB2 &= DisplayPixelFormat.dwBBitMask;

					destination |= (srcR2|srcG2|srcB2)<<16;

				}
				*destPtr++=destination;
				srcPtr++;
				srcPtr2++;
			}
			srcPtr += (ddsdimage.lPitch/4) - 320; 
			srcPtr2 += (ddsdimage2.lPitch/4) - 320; 
		}
	}
   	surface2->Unlock((LPVOID)ddsdimage2.lpSurface);
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);

	UnlockSurface();
}

extern void DrawAvPMenuGfx_Faded(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format)
{																	
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}


	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned short *)ddsdimage.lpSurface;

	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			for (x=0; x<gfxPtr->Width; x++)
			{
				if (*srcPtr)
				{
					unsigned int srcR,srcG,srcB;

					srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
					srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
					srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;
					srcR = MUL_FIXED(alpha,srcR);
					srcR &= DisplayPixelFormat.dwRBitMask;

					srcG = MUL_FIXED(alpha,srcG);
					srcG &= DisplayPixelFormat.dwGBitMask;
					
					srcB = MUL_FIXED(alpha,srcB);
					srcB &= DisplayPixelFormat.dwBBitMask;

					*destPtr = (short)(srcR|srcG|srcB);
				}
				else
				{
					*destPtr = 0;
				}

				destPtr++;
				srcPtr++;
			}
			srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);

	UnlockSurface();
}
extern void DrawAvPMenuGfx_Clipped(enum AVPMENUGFX_ID menuGfxID, int topleftX, int topleftY, int alpha,enum AVPMENUFORMAT_ID format, int topY, int bottomY)
{																	
	LockSurfaceAndGetBufferPointer();

	DDSURFACEDESC ddsdimage;
   	unsigned short *destPtr;
	unsigned short *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;

	GLOBALASSERT(menuGfxID < MAX_NO_OF_AVPMENUGFXS);
	gfxPtr = &AvPMenuGfxStorage[menuGfxID];
	surface = gfxPtr->ImagePtr;

	switch(format)
	{
		default:
		GLOBALASSERT("UNKNOWN TEXT FORMAT"==0);
		case AVPMENUFORMAT_LEFTJUSTIFIED:
		{
			// supplied x is correct
			break;
		}
		case AVPMENUFORMAT_RIGHTJUSTIFIED:
		{
			topleftX -= gfxPtr->Width;
			break;
		}
		case AVPMENUFORMAT_CENTREJUSTIFIED:
		{
			topleftX -= gfxPtr->Width/2;
			break;
		}
	}


	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned short *)ddsdimage.lpSurface;

	if (alpha>ONE_FIXED)
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);
			if(y>=topY && y<=bottomY)
			{
				for (x=0; x<gfxPtr->Width; x++)
				{
					*destPtr = *srcPtr;
					destPtr++;
					srcPtr++;
				}
				srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
			}
			else
			{
				srcPtr+=(ddsdimage.lPitch/2);
			}
		}
	}
	else
	{
		int x,y;

		for (y=topleftY; y<gfxPtr->Height+topleftY; y++)
		{
			destPtr = (unsigned short *)(ScreenBuffer + topleftX*2 + y*BackBufferPitch);

			if(y>=topY && y<=bottomY)
			{
				for (x=0; x<gfxPtr->Width; x++)
				{
					if (*srcPtr)
					{
						unsigned int srcR,srcG,srcB;
						unsigned int destR,destG,destB;

						destR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
						destG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
						destB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;

						srcR = (int)(*srcPtr) & DisplayPixelFormat.dwRBitMask;
						srcG = (int)(*srcPtr) & DisplayPixelFormat.dwGBitMask;
						srcB = (int)(*srcPtr) & DisplayPixelFormat.dwBBitMask;

						destR += MUL_FIXED(alpha,srcR);
						if (destR>DisplayPixelFormat.dwRBitMask) destR = DisplayPixelFormat.dwRBitMask;
						else destR &= DisplayPixelFormat.dwRBitMask;

						destG += MUL_FIXED(alpha,srcG);
						if (destG>DisplayPixelFormat.dwGBitMask) destG = DisplayPixelFormat.dwGBitMask;
						else destG &= DisplayPixelFormat.dwGBitMask;

						destB += MUL_FIXED(alpha,srcB);
						if (destB>DisplayPixelFormat.dwBBitMask) destB = DisplayPixelFormat.dwBBitMask;
						else destB &= DisplayPixelFormat.dwBBitMask;

						*destPtr = (short)(destR|destG|destB);
					}
					destPtr++;
					srcPtr++;
				}
				srcPtr += (ddsdimage.lPitch/2) - gfxPtr->Width; 
			}
			else
			{
				srcPtr += (ddsdimage.lPitch/2);
			}
		}
	}
   	
   	surface->Unlock((LPVOID)ddsdimage.lpSurface);

	UnlockSurface();
}

extern int HeightOfMenuGfx(enum AVPMENUGFX_ID menuGfxID)
{
	return AvPMenuGfxStorage[menuGfxID].Height; 
}

extern void ClearScreenToBlack(void)
{ 
	LockSurfaceAndGetBufferPointer();
	{
		int x,y;

		for (y=0; y<480; y++)
		{
			unsigned int *destPtr = (unsigned int *)(ScreenBuffer + y*BackBufferPitch);

			for (x=0; x<320; x++)
			{
 				*destPtr++=0;
			}
		}
	}
	UnlockSurface();
}

extern void FadedScreen(int alpha)
{																	
	LockSurfaceAndGetBufferPointer();

	{
		int x,y;

		for (y=60; y<ScreenDescriptorBlock.SDB_Height-60; y++)
		{
			unsigned short *destPtr = (unsigned short *)(ScreenBuffer + y*BackBufferPitch);

			for (x=0; x<ScreenDescriptorBlock.SDB_Width; x++)
			{
				if (*destPtr)
				{
					unsigned int srcR,srcG,srcB;

					srcR = (int)(*destPtr) & DisplayPixelFormat.dwRBitMask;
					srcG = (int)(*destPtr) & DisplayPixelFormat.dwGBitMask;
					srcB = (int)(*destPtr) & DisplayPixelFormat.dwBBitMask;
					srcR = MUL_FIXED(alpha,srcR);
					srcR &= DisplayPixelFormat.dwRBitMask;

					srcG = MUL_FIXED(alpha,srcG);
					srcG &= DisplayPixelFormat.dwGBitMask;
					
					srcB = MUL_FIXED(alpha,srcB);
					srcB &= DisplayPixelFormat.dwBBitMask;

					*destPtr = (short)(srcR|srcG|srcB);
				}

				destPtr++;
			}
		}
	}
	UnlockSurface();
}


static void CalculateWidthsOfAAFont(void)
{
	DDSURFACEDESC ddsdimage;
	unsigned char *srcPtr;
	AVPMENUGFX *gfxPtr;
	LPDIRECTDRAWSURFACE surface;
	int c;

	gfxPtr = &AvPMenuGfxStorage[AVPMENUGFX_SMALL_FONT];
	surface = gfxPtr->ImagePtr;
	
	memset(&ddsdimage, 0, sizeof(ddsdimage));
	ddsdimage.dwSize = sizeof(ddsdimage);

	/* lock the image */
	while (surface->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

	srcPtr = (unsigned char *)ddsdimage.lpSurface;

	// special case for space
	AAFontWidths[32]=3;

	for (c=33; c<255; c++)
	{
		int x,y;
		int x1 = 1+((c-32)&15)*16;
		int y1 = 1+((c-32)>>4)*16;
		AAFontWidths[c]=17;

		for (x=x1+HUD_FONT_WIDTH; x>x1; x--)
		{
			int blank = 1;
			for (y=y1; y<y1+HUD_FONT_HEIGHT; y++)
			{
				unsigned short s = *(unsigned short *)(srcPtr + x*2 + y*ddsdimage.lPitch);
				if (s&DisplayPixelFormat.dwBBitMask == DisplayPixelFormat.dwBBitMask)
				{
					blank=0;
					break;
				}
			}
			if(blank)
			{
				AAFontWidths[c]--;
			}
			else
			{
				break;
			}
			
		}

	}

   	surface->Unlock((LPVOID)ddsdimage.lpSurface);

}
	 
};
