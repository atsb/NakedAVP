/*
	
	indexfnt.hpp

	Created 19/11/97 by DHM : An "indexed font" type; it is assumed that
	there is a maximum number of possible fonts that the program can have
	installed, with an enum for accessing the different fonts (as opposed
	to a system allowing arbitrary numbers of fonts to be loaded at run-time).

	Expects an enum "FontIndex" for the font to be defined in PROJFONT.H


	(haven't worked out what happens for colour information/selection)
*/

#ifndef _indexfnt
#define _indexfnt 1

	#ifndef _projfont
		#include "projfont.h"
	#endif

	#ifndef _r2base
		#include "r2base.h"
	#endif

#ifdef __cplusplus
	
	#ifndef _scstring
	#include "scstring.hpp"
	#endif

#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/
#include "hud_layout.h"
/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	#ifdef __cplusplus
	class IndexedFont
	{
	public:
		static IndexedFont* GetFont( FontIndex I_Font_ToGet );
		// can return NULL if no font loaded in that slot

		static void UnloadFont( FontIndex I_Font_ToGet );
			// there must be a font loaded in that slot

		virtual ~IndexedFont();

		virtual void RenderString_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			const SCString& SCStr
		) const = 0;
			// unsupported characters come out invisible, they may or may not affect
			// the spacing/size depending on implementation
			// the cursor pos is used for both input and output; the function returns
			// the next position after the string for rendering further text

		virtual void RenderString_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			const SCString& SCStr
		) const = 0;
			// unsupported characters come out invisible, they may or may not affect
			// the spacing/size depending on implementation
			// the cursor pos is used for both input and output; the function returns
			// the next position after the string for rendering further text

		// These functions are as above but for single characters:
		virtual void RenderChar_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			ProjChar ProjCh
		) const = 0;

		virtual void RenderChar_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			ProjChar ProjCh
		) const = 0;

		virtual r2size CalcSize
		(
			ProjChar* pProjCh
		) const = 0;
			// calculates how big the string would occupy as a single line
			// without carriage returns etc
			// unsupported characters come out invisible, they may or may not affect
			// the spacing/size depending on implementation
			// The SCString code assumes that for any strings pCh1, pCh2:
			//
			// 	height(pCh1) == height (pCh2) == height (pCh1+pCh2) 
			// 	width(pCh1 + pCh2) = width(pCh1) + width(pCh2)
			//
			// where "+" on strings used to mean concatenation

		virtual r2size CalcSize
		(
			ProjChar* pProjCh,
			int MaxChars
		) const = 0;
			// as above, but will only use at most MaxChars characters, ignoring
			// any that follow (used by the word wrap code)

		virtual r2size CalcSize
		(
			ProjChar ProjCh
		) const = 0;
			// calc size of individual character
			// however size of (Ch0+Ch1+...ChN) doesn't need to equal
			// size(Ch0) + size(Ch1) + ... size(ChN) because the string
			// sizing/display fns are allowed to add spacing pixels
			// in their own ways

		virtual OurBool bCanRender( ProjChar ProjCh_In ) const = 0;
			// many fonts don't support all the characters within the ProjChar
			// character set; you can check with this

		virtual int GetMaxWidth(void) const = 0;
		virtual int GetWidth
		(
			ProjChar ProjCh
		) const = 0;
		virtual int GetHeight(void) const = 0;

		OurBool bCanRenderFully( ProjChar* pProjCh );
			// returns true iff all characters in the string are renderable by the font

		#if debug
		void Render_Clipped_Report
		(
			const struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			const SCString& SCStr
		) const;
		#endif

	protected:
		// Protected constructor since abstract class
		IndexedFont
		(
			FontIndex I_Font_New
		);
		
	private:
		FontIndex I_Font_Val;

		static IndexedFont* pIndexedFont[ IndexedFonts_MAX_NUMBER_OF_FONTS ];

	};
	// Inline methods:
		inline /* static */ IndexedFont* IndexedFont::GetFont( FontIndex I_Font_ToGet )
		{
			return pIndexedFont[ I_Font_ToGet ];
		}

	/*
	   KJL 17:20:10 15/04/98 - May God have mercy on my soul
	   fixed space HUD font
	*/
	class IndexedFont_HUD : public IndexedFont
	{
	public:
		void RenderString_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			const SCString& SCStr
		) const;

		void RenderString_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			const SCString& SCStr
		) const;

		void RenderChar_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			ProjChar ProjCh
		) const;

		void RenderChar_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			ProjChar ProjCh
		) const;

		OurBool bCanRender( ProjChar ProjCh_In ) const
		{
			return Yes;
		}

		inline int GetMaxWidth(void) const
		{
			return HUD_FONT_WIDTH;
		}

		inline int GetWidth
		(
			ProjChar ProjCh
		) const
		{
			return AAFontWidths[(unsigned char)ProjCh];
		}
		
		inline int GetHeight(void) const
		{
			// +2 for line spacing 
			return HUD_FONT_HEIGHT+2;
		}

		r2size CalcSize
		(
			ProjChar* pProjCh
		) const;
		r2size CalcSize
		(
			ProjChar* pProjCh,
			int MaxChars
		) const;

		r2size CalcSize
		(
			ProjChar ProjCh
		) const;

//	protected:
		IndexedFont_HUD(FontIndex I_Font_New)
		 : IndexedFont
			(
				I_Font_New
			) 
		{
		}
	};
		inline r2size IndexedFont_HUD::CalcSize
		(
			ProjChar ProjCh
		) const
		{
			return r2size
			(
				GetWidth( ProjCh ),
				GetHeight()
			);
		}
		#if 1
		inline r2size IndexedFont_HUD::CalcSize
		(
			ProjChar* pProjCh,
			int MaxChars
		) const
		{
			int width=0;
			for (int i=0; i<MaxChars && *pProjCh!=0; i++)
			{
				width+=GetWidth(*pProjCh++);
			}
			return r2size
			(
				width,
				GetHeight()
			);
		}
		#endif
	class IndexedFont_Proportional : public IndexedFont
	{
	public:
		void RenderString_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			const SCString& SCStr
		) const;

		void RenderString_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			const SCString& SCStr
		) const;

		// The string CalcSize() functions are implemented at this level
		// (by repeatedly calling the virtual character level CalcSize())
		r2size CalcSize
		(
			ProjChar* pProjCh
		) const;
		r2size CalcSize
		(
			ProjChar* pProjCh,
			int MaxChars
		) const;

		r2size CalcSize
		(
			ProjChar ProjCh
		) const;

		#if 0
		OurBool bCanRender( ProjChar ProjCh_In ) const;
		#endif

	protected:
		IndexedFont_Proportional
		(
			FontIndex I_Font_New
		) : IndexedFont
			(
				I_Font_New
			)
		{
		}
	};
	
	// Inline methods:
		inline r2size IndexedFont_Proportional::CalcSize
		(
			ProjChar ProjCh
		) const
		{
			return r2size
			(
				GetWidth( ProjCh ),
				GetHeight()
			);
		}

	// Created 3/4/98 by DHM: a new type of font in which every character
	// has a width with respect to the character following it, so that
	// e.g. "A" and "W" nuzzle together, with the top of the "W" above the
	// bottom of the "A".
	class IndexedFont_Kerned : public IndexedFont
	{
	public:
		void RenderString_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			const SCString& SCStr
		) const;

		void RenderString_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			const SCString& SCStr
		) const;
			// the RenderChar functions should not modify the cursor
			// position; this is done by the RenderString functions

		// The string CalcSize() functions are implemented at this level
		// For this class, it's not just done by adding together the sizes for
		// the characters:
		r2size CalcSize
		(
			ProjChar* pProjCh
		) const;
		r2size CalcSize
		(
			ProjChar* pProjCh,
			int MaxChars
		) const;

		r2size CalcSize
		(
			ProjChar ProjCh
		) const;


		// A new pure virtual function for this class:
		// Calculate the x increment after rendering a character, assuming
		// a particular character is following.  (First character must be
		// non-null; must be able to survive having the second character be
		// null, in which case the full width of the first charater is to be
		// returned).
		virtual int GetXInc
		(
			ProjChar currentProjCh,
			ProjChar nextProjCh
		) const = 0;
		
//		virtual LPDIRECTDRAWSURFACE GetImagePtr(void) const = 0;

	protected:
		IndexedFont_Kerned
		(
			FontIndex I_Font_New
		) : IndexedFont
			(
				I_Font_New
			)
		{
		}
	};
	
	// Inline methods:
		inline r2size IndexedFont_Kerned::CalcSize
		(
			ProjChar ProjCh
		) const
		{
			return r2size
			(
				GetWidth( ProjCh ),
				GetHeight()
			);
		}

	// Interface to the old PFFONT system written by Roxby and loathed
	// by all programmers and artists who've since worked with it...
	class IndexedFont_Proportional_PF : public IndexedFont_Proportional
	{
	public:
		void RenderChar_Clipped
		(
			struct r2pos& R2Pos_Cursor,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha,
			ProjChar ProjCh
		) const;

		void RenderChar_Unclipped
		(
			struct r2pos& R2Pos_Cursor,
			int FixP_Alpha,
			ProjChar ProjCh
		) const;

		OurBool bCanRender( ProjChar ProjCh_In ) const;

		int GetMaxWidth(void) const;
		int GetWidth
		(
			ProjChar ProjCh
		) const;
		int GetHeight(void) const;

		static void PFLoadHook
		(
			FontIndex I_Font_New,
			PFFONT *pffont_New
		);
		static void PFUnLoadHook
		(
			FontIndex I_Font_Old
		);

		const PFFONT& GetPFFont(void) const;

	private:
		IndexedFont_Proportional_PF
		(
			FontIndex I_Font_New,
			PFFONT *pffont_New
		);

		PFFONT *pffont_Val;
		int MaxWidth_Val;

	};

	// Inline methods:
		inline const PFFONT& IndexedFont_Proportional_PF::GetPFFont(void) const
		{
			return *pffont_Val;
		}

		// both inline and virtual...
		inline OurBool IndexedFont_Proportional_PF::bCanRender( ProjChar ProjCh_In ) const
		{
			return pffont_Val -> bPrintable(ProjCh_In);
		}

		inline int IndexedFont_Proportional_PF::GetWidth
		(
			ProjChar ProjCh
		) const
		{
			return pffont_Val -> GetWidth( ProjCh );
		}
		inline int IndexedFont_Proportional_PF::GetHeight(void) const { return pffont_Val -> GetHeight(); }

		inline int IndexedFont_Proportional_PF::GetMaxWidth(void) const
		{
			return MaxWidth_Val;
		}



	#endif // __cplusplus
/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
#ifdef __cplusplus
	extern "C" {
#endif
		extern void INDEXFNT_PFLoadHook
		(
			FontIndex I_Font_New,
			PFFONT *pffont_New
		);
#ifdef __cplusplus
	};
#endif

/* End of the header ****************************************************/



#endif
