#ifndef _font_h_included
#define _font_h_included 1

	#ifdef __cplusplus
		// Necessary header files for the C++ stuff:
		#ifndef _projtext
		#include "projtext.h"
		#endif
	#endif

#include "aw.h"

/************* fonts. not set up yet************/

typedef enum fonts
{
	MENU_FONT_1,
	DATABASE_FONT_DARK,
	DATABASE_FONT_LITE,
	DATABASE_MESSAGE_FONT,

	IntroFont_Dark,
	IntroFont_Light,

	NUM_FONTS,

}AVP_FONTS;

#define MAXNUM_CHARS_IN_FONT (0x7e)


/* platform dependent character descriptor

we dont need give any info apart from the size and src of the
thing to BLT for W95. The textprint routines will handle the
position of the surface on the screen

*/

typedef enum font_types
{
	I_FONT_NUMERIC,
	I_FONT_UC_NUMERIC,
	I_FONT_UCLC_NUMERIC,
}FONT_TYPE;

/* NUMERIC - COUNTS FROM ASCII 0x30 to 0x39 inc*/
/* UC NUMERIC 0x20 t0 0x5A				*/
/* UCLC_NUMERIC 0cx20 to 0x7E inc */


// menu strings - printed to the creen with a certain
// justification

typedef enum font_justification
{
	FJ_LEFT_JUST,
	FJ_CENTRED,
	FJ_RIGHT_JUST,
}FONT_JUST;


// bitfield of flags

typedef struct 
{
	unsigned int loaded  : 1;
	
}FONT_FLAGS;


typedef struct pffont
{
	LPDIRECTDRAWSURFACE data;	 	/*LPDIRECTDRAWSURFACE, etc - fill out*/
	char filename[100];			/*compile in -filename */
	int fontHeight;				/* max height of chars */
	int num_chars_in_font;			/*number of chars in this font */
	FONT_TYPE font_type;
	FONT_FLAGS flags;
	RECT srcRect[MAXNUM_CHARS_IN_FONT];  /*source rectangles*/

	int fttexWidth;                       /* filled in during loading */
	int fttexHeight;
	int fttexBitDepth;
	unsigned hotSpotValue;
	AW_BACKUPTEXTUREHANDLE hBackup;

	#ifdef __cplusplus
	// C++ methods: ////////////////////////////////
	int GetHeight(void) const;
	int GetWidth(const ProjChar ProjCh) const;
	int bPrintable(const ProjChar ProjCh) const;
	int ProjCharToOffset(const ProjChar ProjCh) const;

	int GetOffset(void) const;
		// DHM 25/11/97:
		// Returns offset for subtracting from ASCII code to get
		// to index into the font
	int GetMinChar(void) const;
	int GetMaxChar(void) const;
		// returns extent within the ASCII set that can be printed (inclusive of boundary values)
	
	#endif // __cplusplus

}PFFONT;

#ifdef __cplusplus

	// Inline method implementations:
	inline int pffont::GetHeight(void) const
	{
		return fontHeight;
	}
	inline int pffont::GetWidth(const ProjChar ProjCh) const
	{
		if ( bPrintable(ProjCh) )
		{
			const RECT& charRect = srcRect[ ProjCharToOffset(ProjCh) ] ;

			return (charRect . right - charRect . left);
		}
		else
		{
			return 0;
		}
	}
	inline int pffont::bPrintable(const ProjChar ProjCh) const
	{
		const int Offset = GetOffset();

		if ( (int)ProjCh < Offset )
		{
			return No;
		}
		if ( (int)ProjCh >= Offset + num_chars_in_font )
		{
			return No;
		}
		return Yes;
	}
	inline int pffont::ProjCharToOffset(const ProjChar ProjCh) const
	{
		return ((int)ProjCh - GetOffset());
	}
	inline int pffont::GetOffset(void) const
	{
		// DHM 25/11/97:
		// Returns offset for subtracting from ASCII code to get
		// to index into the font
		if (font_type == I_FONT_NUMERIC)
		{
			return 0x30;
		}
		else
		{
			return 0x20;
		}
	}
	inline int pffont::GetMinChar(void) const
	{
		return GetOffset();
	}
	inline int pffont::GetMaxChar(void) const
	{
		return (GetOffset() + num_chars_in_font -1);
	}


	extern "C" {
#endif

// platform independent externs
extern void LoadAllFonts();
extern void LoadPFFont(int fontnum);

 // platform dependent callbacks for loading
extern void LoadFont(PFFONT *pffont);
extern void UnloadFont(PFFONT *pffont);

extern void FillCharacterSlot(int u, int v, int width, int height,
								int charnum, PFFONT *font);

extern int BLTFontOffsetToHUD(PFFONT* font , int xdest, int ydest, int offset);


/* to lock a font and get the raw data - pPitch receives the pitch of the surface */
extern void * FontLock(PFFONT const * pFont, unsigned * pPitch);
extern void FontUnlock(PFFONT const * pFont);


// the array of all the Fonts int the game
extern PFFONT AvpFonts[];

#ifdef __cplusplus
	};
#endif

#endif /* _font_h_included */

