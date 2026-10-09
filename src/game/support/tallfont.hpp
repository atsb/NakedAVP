/*
	
	tallfont.hpp

*/

#ifndef _tallfont
#define _tallfont 1

	#if defined( _MSC_VER )
		#pragma once
	#endif


	#ifdef __cplusplus
		#ifndef _indexfnt
			#include "indexfnt.hpp"
		#endif
	#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
#ifdef __cplusplus
	// A new way of handling proportionally-spaced fonts:  artists prepare
	// a tall thin bitmap containing all the characters, with a constant
	// height.  Loader needs to know this height, and extracts character
	// widths using transparency information.
	class IndexedFont_Proportional_Column : public IndexedFont_Proportional
	{
		enum { MAX_CHARS_IN_TALLFONT = 100 };

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

		OurBool bCanRender( ProjChar ProjCh_In ) const
		{
			unsigned int offsetTemp;
			return GetOffset
			(
				offsetTemp,
				ProjCh_In
			);
		}

		int GetMaxWidth(void) const;
		int GetWidth
		(
			ProjChar ProjCh
		) const;
		int GetHeight(void) const
		{
			return HeightPerChar_Val;
		}

		static IndexedFont_Proportional_Column* Create
		(
			FontIndex I_Font_New,
			char* Filename,
			int HeightPerChar_New,
			int SpaceWidth_New,
			int ASCIICodeForInitialCharacter
		);

		~IndexedFont_Proportional_Column();

	private:
		IndexedFont_Proportional_Column
		(
			FontIndex I_Font_New,
			char* Filename,
			int HeightPerChar_New,
			int SpaceWidth_New,
			int ASCIICodeForInitialCharacter
		);

		int HeightPerChar_Val;
		int SpaceWidth_Val;

		LPDIRECTDRAWSURFACE image_ptr;

		AW_BACKUPTEXTUREHANDLE hBackup;

		r2size R2Size_OverallImage;

		int ASCIICodeForOffset0;
		int NumChars;
		RECT WindowsRectForOffset[ MAX_CHARS_IN_TALLFONT ];
			// coordinates of characters in the image
		int WidthForOffset[ MAX_CHARS_IN_TALLFONT ];

	private:
		int SpaceWidth(void) const
		{
			return SpaceWidth_Val;
		}

		void UpdateWidths(void);
			// called by constructor
		void SetWidth(unsigned int Offset, int newWidth)
		{
			WindowsRectForOffset[ Offset ] . right = newWidth;
			WidthForOffset[ Offset ] = newWidth;
		}

		static OurBool bAnyNonTransparentPixelsInColumn
		(
			r2pos R2Pos_TopOfColumn,
			int HeightOfColumn,
			LPDDSURFACEDESC lpDDSurfaceDesc
				// assumes you have a read lock
		);

		OurBool GetOffset
		(
			unsigned int& outputOffset,
			ProjChar inProjCh
		) const
		{
			if (inProjCh < ASCIICodeForOffset0 )
			{
				return No;
			}

			outputOffset = (inProjCh - ASCIICodeForOffset0);

			if (outputOffset >= NumChars)
			{
				return No;
			}

			return Yes;

		}
	};

	// 3/4/98 DHM: A new implementation, supporting character kerning
	class IndexedFont_Kerned_Column : public IndexedFont_Kerned
	{
		enum { MAX_CHARS_IN_TALLFONT = 255 };
			// note that we define an array of ints sized this squared
			// so for 100, this works out as 100*100*4bytes
			// which is 40,000 bytes, or about 39k

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

		OurBool bCanRender( ProjChar ProjCh_In ) const
		{
			unsigned int offsetTemp;
			return GetOffset
			(
				offsetTemp,
				ProjCh_In
			);
		}

		int GetMaxWidth(void) const;
		int GetWidth
		(
			ProjChar ProjCh
		) const;
		int GetHeight(void) const
		{
			return HeightPerChar_Val;
		}

		int GetXInc
		(
			ProjChar currentProjCh,
			ProjChar nextProjCh
		) const;

		static IndexedFont_Kerned_Column* Create
		(
			FontIndex I_Font_New,
			char* Filename,
			int HeightPerChar_New,
			int SpaceWidth_New,
			int ASCIICodeForInitialCharacter
		);

		~IndexedFont_Kerned_Column();

		int FullWidthForOffset[ MAX_CHARS_IN_TALLFONT ];
		
		LPDIRECTDRAWSURFACE GetImagePtr(void) const
		{
			return (LPDIRECTDRAWSURFACE) image_ptr;
		}
		LPDIRECTDRAWSURFACE image_ptr;
	private:
		IndexedFont_Kerned_Column
		(
			FontIndex I_Font_New,
			char* Filename,
			int HeightPerChar_New,
			int SpaceWidth_New,
			int ASCIICodeForInitialCharacter
		);

		int HeightPerChar_Val;
		int SpaceWidth_Val;

		AW_BACKUPTEXTUREHANDLE hBackup;

		r2size R2Size_OverallImage;

		int ASCIICodeForOffset0;
		int NumChars;
		RECT WindowsRectForOffset[ MAX_CHARS_IN_TALLFONT ];
			// coordinates of characters in the image
		int XIncForOffset[ MAX_CHARS_IN_TALLFONT ][ MAX_CHARS_IN_TALLFONT ];

	private:
		int SpaceWidth(void) const
		{
			return SpaceWidth_Val;
		}

		void UpdateWidths(void);
		void UpdateXIncs(void);
			// called by constructor

		void SetWidth(unsigned int Offset, int newWidth)
		{
			WindowsRectForOffset[ Offset ] . right = newWidth;
			FullWidthForOffset[ Offset ] = newWidth;
		}

		static OurBool bAnyNonTransparentPixelsInColumn
		(
			r2pos R2Pos_TopOfColumn,
			int HeightOfColumn,
			LPDDSURFACEDESC lpDDSurfaceDesc
				// assumes you have a read lock
		);

		OurBool GetOffset
		(
			unsigned int& outputOffset,
			ProjChar inProjCh
		) const
		{
			if (inProjCh < ASCIICodeForOffset0 )
			{
				return No;
			}

			outputOffset = (inProjCh - ASCIICodeForOffset0);

			if (outputOffset >= NumChars)
			{
				return No;
			}

			return Yes;

		}

		int CalcXInc
		(
			unsigned int currentOffset,
			unsigned int nextOffset,
			int* minOpaqueX,
			int* maxOpaqueX
		);

		OurBool OverlapOnRow
		(
			unsigned int currentOffset,
			unsigned int nextOffset,
			int Row,
			int ProposedXInc
		);

		int GetSmallestXIncForRow
		(
			unsigned int currentOffset,
			unsigned int nextOffset,
			int* minOpaqueX,
			int* maxOpaqueX,
			unsigned int Row
		);		

		OurBool bOpaque
		(
			LPDDSURFACEDESC lpDDSurfaceDesc,
				// assumes you have a read lock
			int x,
			int y
				// must be in range
		);

	};

#endif

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef __cplusplus
	};
#endif

#endif
