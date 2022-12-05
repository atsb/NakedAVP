/*******************************************************************
 *
 *    DESCRIPTION: 	tallfont.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 18/3/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"

	#include "db.h"
	#include "dxlog.h"

	#include "tallfont.hpp"

	#include "awTexLd.h"
	#include "alt_tab.h"

	#include "ffstdio.h"
	
	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/
	#define UseSoftwareAlphaRendering Yes
		// an option which assumes you're in a 16-bit graphic mode...

		#if UseSoftwareAlphaRendering
			#include "inline.h"
		#endif

/* Constants *******************************************************/

/* Macros **********************************************************/

/* Imported function prototypes ************************************/

/* Imported data ***************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif

		extern unsigned char *ScreenBuffer;
		extern long BackBufferPitch;
		extern LPDIRECTDRAWSURFACE lpDDSBack;
		extern DDPIXELFORMAT DisplayPixelFormat;
		extern int CloudTable[128][128];
		extern int CloakingPhase;

		#if 0
		extern OurBool			DaveDebugOn;
		extern FDIEXTENSIONTAG	FDIET_Dummy;
		extern IFEXTENSIONTAG	IFET_Dummy;
		extern FDIQUAD			FDIQuad_WholeScreen;
		extern FDIPOS			FDIPos_Origin;
		extern FDIPOS			FDIPos_ScreenCentre;
		extern IFOBJECTLOCATION IFObjLoc_Origin;
		extern UncompressedGlobalPlotAtomID UGPAID_StandardNull;
		extern IFCOLOUR			IFColour_Dummy;
 		extern IFVECTOR			IFVec_Zero;
		#endif
#ifdef __cplusplus
	};
#endif



/* Exported globals ************************************************/

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class IndexedFont_Proportional_Column : public IndexedFont_Proportional
// public:
void
IndexedFont_Proportional_Column :: RenderChar_Clipped
(
	struct r2pos& R2Pos_Cursor,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha,
	ProjChar ProjCh
) const
{
	// Easy first attempt: pass on to unclipped routine, but only if no clipping
	// required.  Otherwise ignore...
	if
	(
		r2rect
		(
			R2Pos_Cursor,
			GetWidth(ProjCh),
			GetHeight()
		) . bFitsIn( R2Rect_Clip )
	)
	{
		RenderChar_Unclipped
		(
			R2Pos_Cursor,
			FixP_Alpha,
			ProjCh
		);
	}
	else
	{
		R2Pos_Cursor . x += GetWidth(ProjCh);
	}
}

void
IndexedFont_Proportional_Column :: RenderChar_Unclipped
(
	struct r2pos& R2Pos_Cursor,
	int FixP_Alpha,
	ProjChar ProjCh
) const
{
	#if 1
	unsigned int theOffset;

	if (ProjCh == ' ')
	{
		// Space is a special case:
		R2Pos_Cursor . x += SpaceWidth();
		return;
	}

	if
	(
		GetOffset
		(
			theOffset,
			ProjCh
		)
	)
	{
		if
		(
			GetWidth(ProjCh)>0
		)
		{
			RECT destRect;

			destRect.left = R2Pos_Cursor . x;
			destRect.top = R2Pos_Cursor . y;

			R2Pos_Cursor . x += GetWidth(ProjCh);

			destRect.right = R2Pos_Cursor . x++;
			destRect.bottom = R2Pos_Cursor . y + GetHeight();

			RECT tempnonConstRECTSoThatItCanWorkWithMicrosoft = WindowsRectForOffset[ theOffset ];

			#if 0
			DDBLTFX tempDDBltFx;

			memset(&tempDDBltFx,0,sizeof(DDBLTFX));
			tempDDBltFx . dwSize = sizeof(DDBLTFX);

			tempDDBltFx . ddckSrcColorkey . dwColorSpaceLowValue = 0;
			tempDDBltFx . ddckSrcColorkey . dwColorSpaceHighValue = 0;
			#endif

			HRESULT ddrval = lpDDSBack->Blt
			(
				&destRect,
				image_ptr,
				&tempnonConstRECTSoThatItCanWorkWithMicrosoft,
				(
					DDBLT_WAIT
					#if 1
					| DDBLT_KEYSRC
					#else
					| DDBLT_KEYSRCOVERRIDE
					#endif

					#if 0
					| DDBLT_ALPHADEST
					#endif
				),
				NULL
				// &tempDDBltFx // LPDDBLTFX lpDDBltFx 
			);


			#if 0
				// or even:
				ddrval = lpDDSBack->BltFast(x, y,
						 lpDDDbgFont, &source, 
						 DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
			#endif

			#if 0
			if(ddrval != DD_OK)
			{
				ReleaseDirect3D();
				exit(0x666009);
			}
			#endif
		}
	}
	#else
	textprintXY
	(
		R2Pos_Cursor . x,
		R2Pos_Cursor . y,
		"%c",
		ProjCh
	);
	#endif
}

int
IndexedFont_Proportional_Column :: GetMaxWidth(void) const
{
	return R2Size_OverallImage . w;
}

int
IndexedFont_Proportional_Column :: GetWidth
(
	ProjChar ProjCh_In
) const
{
	unsigned int offsetTemp;

	if (ProjCh_In == ' ')
	{
		return SpaceWidth();
	}

	if
	(
		GetOffset
		(
			offsetTemp,
			ProjCh_In
		)
	)
	{
		return WidthForOffset[ offsetTemp ];
	}
	else
	{
		return 0;		
	}
}

// static
IndexedFont_Proportional_Column* IndexedFont_Proportional_Column :: Create
(
	FontIndex I_Font_New,
	char* Filename,
	int HeightPerChar_New,
	int SpaceWidth_New,
	int ASCIICodeForInitialCharacter
)
{
	IndexedFont_Proportional_Column* pFont = new IndexedFont_Proportional_Column
	(
		I_Font_New,
		Filename,
		HeightPerChar_New,
		SpaceWidth_New,
		ASCIICodeForInitialCharacter
	);

	SCString :: UpdateAfterFontChange( I_Font_New );

	return pFont;
}


IndexedFont_Proportional_Column :: ~IndexedFont_Proportional_Column()
{
	GLOBALASSERT(image_ptr);
	ATRemoveSurface(image_ptr);
	ReleaseDDSurface(image_ptr);
	image_ptr = NULL;
	
	if (hBackup)
	{
		AwDestroyBackupTexture(hBackup);
	}
	
	hBackup = NULL;
}

IndexedFont_Proportional_Column :: IndexedFont_Proportional_Column
(
	FontIndex I_Font_New,
	char* Filename,
	int HeightPerChar_New,
	int SpaceWidth_New,
	int ASCIICodeForInitialCharacter
) : IndexedFont_Proportional
	(
		I_Font_New
	),
	ASCIICodeForOffset0
	(
		ASCIICodeForInitialCharacter
	),
	HeightPerChar_Val(HeightPerChar_New),
	SpaceWidth_Val(SpaceWidth_New),
	NumChars(0)
{
	{
		unsigned nWidth,nHeight;
	
		//see if graphic can be found in fast file
		unsigned int fastFileLength;
		void const * pFastFileData = ffreadbuf(Filename,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			image_ptr = AwCreateSurface
			(
				"pxfXYB",
				pFastFileData,
				fastFileLength,
				(
					#if 1
					0
					#else
					AW_TLF_TRANSP
					#endif
					#if 0
					| AW_TLF_CHROMAKEY
					#endif
				),
				&nWidth,
				&nHeight,
				&hBackup
			);
		}
		else
		{
		//load graphic from rim file
			image_ptr = AwCreateSurface
			(
				"sfXYB",
				Filename,
				(
					#if 1
					0
					#else
					AW_TLF_TRANSP
					#endif
					#if 0
					| AW_TLF_CHROMAKEY
					#endif
				),
				&nWidth,
				&nHeight,
				&hBackup
			);
		}
		R2Size_OverallImage . w = nWidth;
		R2Size_OverallImage . h = nHeight;
	}

	GLOBALASSERT(image_ptr);
	GLOBALASSERT(hBackup);

	GLOBALASSERT(R2Size_OverallImage . w>0);
	GLOBALASSERT(R2Size_OverallImage . h>0);
	ATIncludeSurface(image_ptr,hBackup);

	DDCOLORKEY tempDDColorKey;

    tempDDColorKey . dwColorSpaceLowValue = 0;
    tempDDColorKey . dwColorSpaceHighValue = 0;


	HRESULT hrSetColorKey = image_ptr -> SetColorKey
	(
		(
			DDCKEY_SRCBLT
		), // DWORD dwFlags,
		&tempDDColorKey // LPDDCOLORKEY lpDDColorKey  
	);

	if ( hrSetColorKey != DD_OK )
	{
		LOGDXERR(hrSetColorKey);
	}
	 
#if 0

typedef struct _DDCOLORKEY{ 
    DWORD dwColorSpaceLowValue; 
    DWORD dwColorSpaceHighValue; 
} DDCOLORKEY,FAR* LPDDCOLORKEY; 

	Parameters

	dwFlags

	Determines which color key is requested. 

	DDCKEY_COLORSPACE 
	Set if the structure contains a color space. Not set if the structure contains a single color key. 
	DDCKEY_DESTBLT 
	Set if the structure specifies a color key or color space to be used as a destination color key for blit operations. 
	DDCKEY_DESTOVERLAY
	Set if the structure specifies a color key or color space to be used as a destination color key for overlay operations. 
	DDCKEY_SRCBLT
	Set if the structure specifies a color key or color space to be used as a source color key for blit operations. 
	DDCKEY_SRCOVERLAY
	Set if the structure specifies a color key or color space to be used as a source color key for overlay operations. 
	lpDDColorKey

	Address of the DDCOLORKEY structure that contains the new color key values for the DirectDrawSurface object.
#endif



	
	NumChars = (R2Size_OverallImage . h)/HeightPerChar_Val;

	GLOBALASSERT( NumChars < MAX_CHARS_IN_TALLFONT );

	for (int i=0;i<NumChars;i++)
	{
		WindowsRectForOffset[ i ] . top = (i*HeightPerChar_Val);
		WindowsRectForOffset[ i ] . bottom = ((i+1)*HeightPerChar_Val);
		WindowsRectForOffset[ i ] . left = 0;
		WindowsRectForOffset[ i ] . right = R2Size_OverallImage . w;

		WidthForOffset[ i ] = R2Size_OverallImage . w;
	}

	UpdateWidths();

}

void
IndexedFont_Proportional_Column :: UpdateWidths(void)
{
	// called by constructor

	// Test: read the surface:
	{
		// LPDIRECTDRAWSURFACE image_ptr;

		DDSURFACEDESC tempDDSurfaceDesc;

		tempDDSurfaceDesc . dwSize = sizeof(DDSURFACEDESC);

		HRESULT hrLock = image_ptr -> Lock
		(
			NULL, // LPRECT lpDestRect,                
			&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc,
			(
				DDLOCK_READONLY
				| DDLOCK_SURFACEMEMORYPTR
				#if 0
				| DDLOCK_WAIT
				| DDLOCK_NOSYSLOCK
				#endif
			), // DWORD dwFlags,
			NULL // HANDLE hEvent
		);

		if ( hrLock != DD_OK )
		{
			// ought really to throw an exception

			LOGDXERR(hrLock);
			return;
		}

		// Read the data...
		{
			for (int iOffset=0;iOffset<NumChars;iOffset++)
			{
				int y = iOffset * HeightPerChar_Val;
				int x = ( R2Size_OverallImage . w - 1);

				#if 0
				db_logf1(("Character offset %i",iOffset));
				#endif

				while (x>0)
				{
					if
					(
						bAnyNonTransparentPixelsInColumn
						(
							r2pos(x,y), // r2pos R2Pos_TopOfColumn,
							HeightPerChar_Val, // int HeightOfColumn
							&tempDDSurfaceDesc // LPDDSURFACEDESC lpDDSurfaceDesc
						)
					)
					{
						break;
							// and the current value of (x+1) is the width to use
					}

					x--;
				}

				SetWidth(iOffset,x+1);
			}
		}


		HRESULT hrUnlock = image_ptr -> Unlock
		(
			NULL // LPVOID lpSurfaceData  
		);


		if ( hrUnlock != DD_OK )
		{
			// ought really to throw an exception

			LOGDXERR(hrUnlock);
			return;
		}
	}
}

// static
OurBool
IndexedFont_Proportional_Column :: bAnyNonTransparentPixelsInColumn
(
	r2pos R2Pos_TopOfColumn,
	int HeightOfColumn,
	LPDDSURFACEDESC lpDDSurfaceDesc
		// assumes you have a read lock
)
{
	GLOBALASSERT( lpDDSurfaceDesc );

	void* pSurface = lpDDSurfaceDesc -> lpSurface;

	int BytesPerPixel = lpDDSurfaceDesc -> ddpfPixelFormat . dwRGBBitCount /8;

	int BytesPerRow =
	(
		#if 0
		(BytesPerPixel * lpDDSurfaceDesc -> dwWidth)
		+
		#endif
		lpDDSurfaceDesc -> lPitch
	);

	int y = R2Pos_TopOfColumn . y;

	#if 0
	db_logf1(("x=%i",R2Pos_TopOfColumn . x));
	#endif

	while ( y < R2Pos_TopOfColumn . y + HeightOfColumn )
	{
		int Pixel =
		(
			*(int*)
			(
				((char*)pSurface)
				+
				( y * BytesPerRow )
				+
				(R2Pos_TopOfColumn . x * BytesPerPixel)
			)
		);

		int R = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwRBitMask;
		int G = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwGBitMask;
		int B = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwBBitMask;

		#if 0
		db_logf1(("y=%i",y));
		db_logf1(("Pixel=0x%x",Pixel));
		db_logf1(("R=0x%x",R));
		db_logf1(("G=0x%x",G));
		db_logf1(("B=0x%x",B));
		#endif

		#if 1
		if (Pixel > 0 )
		{
			return Yes;
		}
		#else
		if
		(
			(R > 32)
			||
			(G > 32)
			||
			(B > 32)
		)
		{
			// nasty hack to get it working...
			return Yes;
		}
		#endif

		y++;
	}

	return No;
}






/////////////////////////////////////////////////////////////////
// 3/4/98 DHM: A new implementation, supporting character kerning
/////////////////////////////////////////////////////////////////

// class IndexedFont_Kerned_Column : public IndexedFont_Kerned
// public:
void
IndexedFont_Kerned_Column :: RenderChar_Clipped
(
	struct r2pos& R2Pos_Cursor,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha,
	ProjChar ProjCh
) const
{
	unsigned int theOffset;

	if (ProjCh == ' ')
	{
		// Space is a special case:
		return;
	}

	if
	(
		GetOffset
		(
			theOffset,
			ProjCh
		)
	)
	{
		if
		(
			GetWidth(ProjCh)>0
		)
		{
			{
				// This code adapted from DrawGraphicWithAlphaChannel();
				// it assumes you're in a 16-bit mode...
				DDSURFACEDESC ddsdimage;
				
				memset(&ddsdimage, 0, sizeof(ddsdimage));
				ddsdimage.dwSize = sizeof(ddsdimage);

				/* lock the image */
				while (image_ptr->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

				// okay, now we have the surfaces, we can copy from one to the other,
				// darkening pixels as we go
				{
					long fontimagePitchInShorts = (ddsdimage.lPitch/2); 
					long backbufferPitchInShorts = (BackBufferPitch/2); 

					unsigned short* fontimageRowStartPtr =
					(
						((unsigned short *)ddsdimage.lpSurface)
						+
						(GetHeight()*theOffset*fontimagePitchInShorts)
					);

					unsigned short* backbufferRowStartPtr =
					(
						((unsigned short *)ScreenBuffer)
						+
						(R2Pos_Cursor.y*backbufferPitchInShorts)
						+
						(R2Pos_Cursor.x)
					);
					int screenY = R2Pos_Cursor.y;

					for (int yCount=GetHeight(); yCount>0; yCount--)
					{
						unsigned short* fontimagePtr = fontimageRowStartPtr;
						unsigned short* backbufferPtr = backbufferRowStartPtr;

						if (screenY >= R2Rect_Clip.y0 && screenY <= R2Rect_Clip.y1)
						for (int xCount=FullWidthForOffset[theOffset]; xCount>0;xCount--)
						{
							int r = CloudTable[(xCount+R2Pos_Cursor.x+CloakingPhase/64)&127][(screenY+CloakingPhase/128)&127];
//							b += CloudTable[((xCount+R2Pos_Cursor.x)/2-CloakingPhase/96)&127][((yCount+R2Pos_Cursor.y)/4+CloakingPhase/64)&127]/4;
//							b += CloudTable[((xCount+R2Pos_Cursor.x+10)/4-CloakingPhase/64)&127][((yCount+R2Pos_Cursor.y-50)/8+CloakingPhase/32)&127]/8;
//							if (b>ONE_FIXED) b = ONE_FIXED;
							r = MUL_FIXED(FixP_Alpha,r);
							if (*fontimagePtr)
							{
								unsigned int backR = (int)(*backbufferPtr) & DisplayPixelFormat.dwRBitMask;
								unsigned int backG = (int)(*backbufferPtr) & DisplayPixelFormat.dwGBitMask;
								unsigned int backB = (int)(*backbufferPtr) & DisplayPixelFormat.dwBBitMask;

								unsigned int fontR = (int)(*fontimagePtr) & DisplayPixelFormat.dwRBitMask;
								unsigned int fontG = (int)(*fontimagePtr) & DisplayPixelFormat.dwGBitMask;
								unsigned int fontB = (int)(*fontimagePtr) & DisplayPixelFormat.dwBBitMask;

								backR += MUL_FIXED(r,fontR);
								if (backR>DisplayPixelFormat.dwRBitMask) backR = DisplayPixelFormat.dwRBitMask;
								else backR &= DisplayPixelFormat.dwRBitMask;

								backG += MUL_FIXED(r,fontG);
								if (backG>DisplayPixelFormat.dwGBitMask) backG = DisplayPixelFormat.dwGBitMask;
								else backG &= DisplayPixelFormat.dwGBitMask;

								backB += MUL_FIXED(r,fontB);
								if (backB>DisplayPixelFormat.dwBBitMask) backB = DisplayPixelFormat.dwBBitMask;
								else backB &= DisplayPixelFormat.dwBBitMask;

								*backbufferPtr = (short)(backR|backG|backB);
							}
							fontimagePtr++;
							backbufferPtr++;
						}
						screenY++;
						fontimageRowStartPtr += fontimagePitchInShorts;
						backbufferRowStartPtr += backbufferPitchInShorts;
					}
				}
			   	
			   	image_ptr->Unlock((LPVOID)ddsdimage.lpSurface);
			}
		}
	}
}

void
IndexedFont_Kerned_Column :: RenderChar_Unclipped
(
	struct r2pos& R2Pos_Cursor,
	int FixP_Alpha,
	ProjChar ProjCh
) const
{
	unsigned int theOffset;

	if (ProjCh == ' ')
	{
		// Space is a special case:
		return;
	}

	if
	(
		GetOffset
		(
			theOffset,
			ProjCh
		)
	)
	{
		if
		(
			GetWidth(ProjCh)>0
		)
		{
			{
				// This code adapted from DrawGraphicWithAlphaChannel();
				// it assumes you're in a 16-bit mode...
				DDSURFACEDESC ddsdback, ddsdimage;
				
				memset(&ddsdback, 0, sizeof(ddsdback));
				memset(&ddsdimage, 0, sizeof(ddsdimage));
				ddsdback.dwSize = sizeof(ddsdback);
				ddsdimage.dwSize = sizeof(ddsdimage);

				/* lock the image */
				while (image_ptr->Lock(NULL, &ddsdimage, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

				/* lock the backbuffer */
				while (lpDDSBack->Lock(NULL, &ddsdback, DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING);

				// okay, now we have the surfaces, we can copy from one to the other,
				// darkening pixels as we go
				{
					long fontimagePitchInShorts = (ddsdimage.lPitch/2); 
					long backbufferPitchInShorts = (ddsdback.lPitch/2); 

					unsigned short* fontimageRowStartPtr =
					(
						((unsigned short *)ddsdimage.lpSurface)
						+
						(GetHeight()*theOffset*fontimagePitchInShorts)
					);

					unsigned short* backbufferRowStartPtr =
					(
						((unsigned short *)ddsdback.lpSurface)
						+
						(R2Pos_Cursor.y*backbufferPitchInShorts)
						+
						(R2Pos_Cursor.x)
					);

					for (int yCount=GetHeight(); yCount>0; yCount--)
					{
						unsigned short* fontimagePtr = fontimageRowStartPtr;
						unsigned short* backbufferPtr = backbufferRowStartPtr;
						int yIndex = (yCount+R2Pos_Cursor.y+CloakingPhase/128)&127;
						int xIndex = R2Pos_Cursor.x+CloakingPhase/64;

						for (int xCount=FullWidthForOffset[theOffset]; xCount>0;xCount--)
						{
							int r = CloudTable[(xCount+xIndex)&127][yIndex];
//							b += CloudTable[((xCount+R2Pos_Cursor.x)/2-CloakingPhase/96)&127][((yCount+R2Pos_Cursor.y)/4+CloakingPhase/64)&127]/4;
//							b += CloudTable[((xCount+R2Pos_Cursor.x+10)/4-CloakingPhase/64)&127][((yCount+R2Pos_Cursor.y-50)/8+CloakingPhase/32)&127]/8;
//							if (b>ONE_FIXED) b = ONE_FIXED;
							r = MUL_FIXED(FixP_Alpha,r);
							if (*fontimagePtr)
							{
								unsigned int backR = (int)(*backbufferPtr) & DisplayPixelFormat.dwRBitMask;
								unsigned int backG = (int)(*backbufferPtr) & DisplayPixelFormat.dwGBitMask;
								unsigned int backB = (int)(*backbufferPtr) & DisplayPixelFormat.dwBBitMask;

								unsigned int fontR = (int)(*fontimagePtr) & DisplayPixelFormat.dwRBitMask;
								unsigned int fontG = (int)(*fontimagePtr) & DisplayPixelFormat.dwGBitMask;
								unsigned int fontB = (int)(*fontimagePtr) & DisplayPixelFormat.dwBBitMask;

								backR += MUL_FIXED(r,fontR);
								if (backR>DisplayPixelFormat.dwRBitMask) backR = DisplayPixelFormat.dwRBitMask;
								else backR &= DisplayPixelFormat.dwRBitMask;

								backG += MUL_FIXED(r,fontG);
								if (backG>DisplayPixelFormat.dwGBitMask) backG = DisplayPixelFormat.dwGBitMask;
								else backG &= DisplayPixelFormat.dwGBitMask;

								backB += MUL_FIXED(r,fontB);
								if (backB>DisplayPixelFormat.dwBBitMask) backB = DisplayPixelFormat.dwBBitMask;
								else backB &= DisplayPixelFormat.dwBBitMask;

								*backbufferPtr = (short)(backR|backG|backB);
							}
							fontimagePtr++;
							backbufferPtr++;
						}

						fontimageRowStartPtr += fontimagePitchInShorts;
						backbufferRowStartPtr += backbufferPitchInShorts;
					}
				}
			   	
			   	lpDDSBack->Unlock((LPVOID)ddsdback.lpSurface);
			   	image_ptr->Unlock((LPVOID)ddsdimage.lpSurface);
			}
		}
	}
}

int
IndexedFont_Kerned_Column :: GetMaxWidth(void) const
{
	return R2Size_OverallImage . w;
}

int
IndexedFont_Kerned_Column :: GetWidth
(
	ProjChar ProjCh_In
) const
{
	unsigned int offsetTemp;

	if (ProjCh_In == ' ')
	{
		return SpaceWidth();
	}

	if
	(
		GetOffset
		(
			offsetTemp,
			ProjCh_In
		)
	)
	{
		return FullWidthForOffset[ offsetTemp ];
	}
	else
	{
		return 0;		
	}
}

int
IndexedFont_Kerned_Column :: GetXInc
(
	ProjChar currentProjCh,
	ProjChar nextProjCh
) const
{
	GLOBALASSERT(currentProjCh!='\0');
		// LOCALISEME

	if (currentProjCh == ' ')
	{
		return SpaceWidth();
	}

	if (!((nextProjCh>='0' && nextProjCh<=']') || (nextProjCh>='a' && nextProjCh<='}')))
	{
		return GetWidth(currentProjCh);
	}

	unsigned int currentOffset;

	if
	(
		GetOffset
		(
			currentOffset,
			currentProjCh
		)
	)
	{
		unsigned int nextOffset;

		if
		(
			GetOffset
			(
				nextOffset,
				nextProjCh
			)
		)
		{
			return XIncForOffset[currentOffset][nextOffset];
		}
		else
		{
			return FullWidthForOffset[currentOffset];
		}
	}
	else
	{
		return 0;
	}		

}


// static
IndexedFont_Kerned_Column* IndexedFont_Kerned_Column :: Create
(
	FontIndex I_Font_New,
	char* Filename,
	int HeightPerChar_New,
	int SpaceWidth_New,
	int ASCIICodeForInitialCharacter
)
{
	IndexedFont_Kerned_Column* pFont = new IndexedFont_Kerned_Column
	(
		I_Font_New,
		Filename,
		HeightPerChar_New,
		SpaceWidth_New,
		ASCIICodeForInitialCharacter
	);

	SCString :: UpdateAfterFontChange( I_Font_New );

	return pFont;
}


IndexedFont_Kerned_Column :: ~IndexedFont_Kerned_Column()
{
	GLOBALASSERT(image_ptr);
	ATRemoveSurface(image_ptr);
	ReleaseDDSurface(image_ptr);
	image_ptr = NULL;
	
	if (hBackup)
	{
		AwDestroyBackupTexture(hBackup);
	}
	
	hBackup = NULL;
}

IndexedFont_Kerned_Column :: IndexedFont_Kerned_Column
(
	FontIndex I_Font_New,
	char* Filename,
	int HeightPerChar_New,
	int SpaceWidth_New,
	int ASCIICodeForInitialCharacter
) : IndexedFont_Kerned
	(
		I_Font_New
	),
	ASCIICodeForOffset0
	(
		ASCIICodeForInitialCharacter
	),
	HeightPerChar_Val(HeightPerChar_New),
	SpaceWidth_Val(SpaceWidth_New),
	NumChars(0)
{
	{
		unsigned nWidth,nHeight;
		
		//see if graphic can be found in fast file
		unsigned int fastFileLength;
		void const * pFastFileData = ffreadbuf(Filename,&fastFileLength);
		
		if(pFastFileData)
		{
			//load from fast file
			image_ptr = AwCreateSurface
			(
				"pxfXYB",
				pFastFileData,
				fastFileLength,
				(
					#if 1
					0
					#else
					AW_TLF_TRANSP
					#endif
					#if 0
					| AW_TLF_CHROMAKEY
					#endif
				),
				&nWidth,
				&nHeight,
				&hBackup
			);
		}
		else
		{
			//load graphic from rim file
			image_ptr = AwCreateSurface
			(
				"sfXYB",
				Filename,
				(
					#if 1
					0
					#else
					AW_TLF_TRANSP
					#endif
					#if 0
					| AW_TLF_CHROMAKEY
					#endif
				),
				&nWidth,
				&nHeight,
				&hBackup
			);
		}
		
		R2Size_OverallImage . w = nWidth;
		R2Size_OverallImage . h = nHeight;
	}

	GLOBALASSERT(image_ptr);
	GLOBALASSERT(hBackup);

	GLOBALASSERT(R2Size_OverallImage . w>0);
	GLOBALASSERT(R2Size_OverallImage . h>0);
	ATIncludeSurface(image_ptr,hBackup);

	DDCOLORKEY tempDDColorKey;

    tempDDColorKey . dwColorSpaceLowValue = 0;
    tempDDColorKey . dwColorSpaceHighValue = 0;


	HRESULT hrSetColorKey = image_ptr -> SetColorKey
	(
		(
			DDCKEY_SRCBLT
		), // DWORD dwFlags,
		&tempDDColorKey // LPDDCOLORKEY lpDDColorKey  
	);

	if ( hrSetColorKey != DD_OK )
	{
		LOGDXERR(hrSetColorKey);
	}
	 
#if 0

typedef struct _DDCOLORKEY{ 
    DWORD dwColorSpaceLowValue; 
    DWORD dwColorSpaceHighValue; 
} DDCOLORKEY,FAR* LPDDCOLORKEY; 

	Parameters

	dwFlags

	Determines which color key is requested. 

	DDCKEY_COLORSPACE 
	Set if the structure contains a color space. Not set if the structure contains a single color key. 
	DDCKEY_DESTBLT 
	Set if the structure specifies a color key or color space to be used as a destination color key for blit operations. 
	DDCKEY_DESTOVERLAY
	Set if the structure specifies a color key or color space to be used as a destination color key for overlay operations. 
	DDCKEY_SRCBLT
	Set if the structure specifies a color key or color space to be used as a source color key for blit operations. 
	DDCKEY_SRCOVERLAY
	Set if the structure specifies a color key or color space to be used as a source color key for overlay operations. 
	lpDDColorKey

	Address of the DDCOLORKEY structure that contains the new color key values for the DirectDrawSurface object.
#endif



	
	NumChars = (R2Size_OverallImage . h)/HeightPerChar_Val;

	GLOBALASSERT( NumChars < MAX_CHARS_IN_TALLFONT );

	for (int i=0;i<NumChars;i++)
	{
		WindowsRectForOffset[ i ] . top = (i*HeightPerChar_Val);
		WindowsRectForOffset[ i ] . bottom = ((i+1)*HeightPerChar_Val);
		WindowsRectForOffset[ i ] . left = 0;
		WindowsRectForOffset[ i ] . right = R2Size_OverallImage . w;

		FullWidthForOffset[ i ] = R2Size_OverallImage . w;
	}

	UpdateWidths();
	UpdateXIncs();
}

void
IndexedFont_Kerned_Column :: UpdateWidths(void)
{
	// called by constructor

	// Test: read the surface:
	{
		// LPDIRECTDRAWSURFACE image_ptr;

		DDSURFACEDESC tempDDSurfaceDesc;

		tempDDSurfaceDesc . dwSize = sizeof(DDSURFACEDESC);

		HRESULT hrLock = image_ptr -> Lock
		(
			NULL, // LPRECT lpDestRect,                
			&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc,
			(
				DDLOCK_READONLY
				| DDLOCK_SURFACEMEMORYPTR
				#if 0
				| DDLOCK_WAIT
				| DDLOCK_NOSYSLOCK
				#endif
			), // DWORD dwFlags,
			NULL // HANDLE hEvent
		);

		if ( hrLock != DD_OK )
		{
			// ought really to throw an exception

			LOGDXERR(hrLock);
			return;
		}

		// Read the data...
		{
			for (int iOffset=0;iOffset<NumChars;iOffset++)
			{
				int y = iOffset * HeightPerChar_Val;
				int x = ( R2Size_OverallImage . w - 1);

				#if 0
				db_logf1(("Character offset %i",iOffset));
				#endif

				while (x>0)
				{
					if
					(
						bAnyNonTransparentPixelsInColumn
						(
							r2pos(x,y), // r2pos R2Pos_TopOfColumn,
							HeightPerChar_Val, // int HeightOfColumn
							&tempDDSurfaceDesc // LPDDSURFACEDESC lpDDSurfaceDesc
						)
					)
					{
						break;
							// and the current value of (x+1) is the width to use
					}

					x--;
				}

				SetWidth(iOffset,x+1);
			}
		}


		HRESULT hrUnlock = image_ptr -> Unlock
		(
			NULL // LPVOID lpSurfaceData  
		);


		if ( hrUnlock != DD_OK )
		{
			// ought really to throw an exception

			LOGDXERR(hrUnlock);
			return;
		}
	}
}

void
IndexedFont_Kerned_Column :: UpdateXIncs(void)
{
	DDSURFACEDESC tempDDSurfaceDesc;

	tempDDSurfaceDesc . dwSize = sizeof(DDSURFACEDESC);

	HRESULT hrLock = image_ptr -> Lock
	(
		NULL, // LPRECT lpDestRect,                
		&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc,
		(
			DDLOCK_READONLY
			| DDLOCK_SURFACEMEMORYPTR
			#if 0
			| DDLOCK_WAIT
			| DDLOCK_NOSYSLOCK
			#endif
		), // DWORD dwFlags,
		NULL // HANDLE hEvent
	);

	if ( hrLock != DD_OK )
	{
		// ought really to throw an exception
		LOGDXERR(hrLock);

		// fill table up with sensible values:
		for (int i=0;i<NumChars;i++)
		{
			for (int j=0;j<NumChars;j++)
			{
				XIncForOffset[i][j] = FullWidthForOffset[i];
			}		
		}
		return;
	}

	int RowsToProcess = NumChars*GetHeight();
	int* minOpaqueX = new int[RowsToProcess];
	int* maxOpaqueX = new int[RowsToProcess];

	// Build tables of min/max opaque pixels in each row of the image:
	{
		for (int Row=0;Row<RowsToProcess;Row++)
		{
			// Find right-most pixel in row
			{
				int rightmostX=GetMaxWidth()-1;
				while (rightmostX>0)
				{
					if
					(
						bOpaque
						(
							&tempDDSurfaceDesc,
							rightmostX, // int x,
							Row
						)
					)
					{
						break;
					}
					else
					{
						rightmostX--;
					}
				}
				
				maxOpaqueX[Row]=rightmostX;
			}
			
			// Find left-most pixel in row of second character
			{
				int leftmostX=0;
				while(leftmostX<GetMaxWidth())
				{
					if
					(
						bOpaque
						(
							&tempDDSurfaceDesc,
							leftmostX, // int x,
							Row // int y
						)
					)
					{
						break;
					}
					else
					{
						leftmostX++;
					}
				}
				minOpaqueX[Row]=leftmostX;
			}
		}		
	}

	HRESULT hrUnlock = image_ptr -> Unlock
	(
		NULL // LPVOID lpSurfaceData  
	);


	if ( hrUnlock != DD_OK )
	{
		// ought really to throw an exception
		LOGDXERR(hrUnlock);

		// fill table up with sensible values:
		for (int i=0;i<NumChars;i++)
		{
			for (int j=0;j<NumChars;j++)
			{
				XIncForOffset[i][j] = FullWidthForOffset[i];
			}		
		}
		return;
	}
	else
	{
		// Use the table of opaque extents:

		for (int i=0;i<NumChars;i++)
		{
			for (int j=0;j<NumChars;j++)
			{
				int XInc = CalcXInc
				(
					i, // unsigned int currentOffset,
					j, // unsigned int nextOffset,
					minOpaqueX,
					maxOpaqueX
				);
				
				GLOBALASSERT(XInc>=0);
				#if 0
				GLOBALASSERT(XInc<=GetMaxWidth());
				#endif

				XIncForOffset[i][j] =XInc;
			}		
		}
	}

	// Destroy the table of opaque extents:
	{
		delete[] maxOpaqueX;
		delete[] minOpaqueX;
	}

}


// static
OurBool
IndexedFont_Kerned_Column :: bAnyNonTransparentPixelsInColumn
(
	r2pos R2Pos_TopOfColumn,
	int HeightOfColumn,
	LPDDSURFACEDESC lpDDSurfaceDesc
		// assumes you have a read lock
)
{
	GLOBALASSERT( lpDDSurfaceDesc );

	void* pSurface = lpDDSurfaceDesc -> lpSurface;

	int BytesPerPixel = lpDDSurfaceDesc -> ddpfPixelFormat . dwRGBBitCount /8;

	int BytesPerRow =
	(
		lpDDSurfaceDesc -> lPitch
	);

	int y = R2Pos_TopOfColumn . y;

	#if 0
	db_logf1(("x=%i",R2Pos_TopOfColumn . x));
	#endif

	while ( y < R2Pos_TopOfColumn . y + HeightOfColumn )
	{
		int Pixel =
		(
			*(int*)
			(
				((char*)pSurface)
				+
				( y * BytesPerRow )
				+
				(R2Pos_TopOfColumn . x * BytesPerPixel)
			)
		);

		int R = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwRBitMask;
		int G = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwGBitMask;
		int B = Pixel & lpDDSurfaceDesc -> ddpfPixelFormat . dwBBitMask;

		#if 0
		db_logf1(("y=%i",y));
		db_logf1(("Pixel=0x%x",Pixel));
		db_logf1(("R=0x%x",R));
		db_logf1(("G=0x%x",G));
		db_logf1(("B=0x%x",B));
		#endif

		#if 1
		if (Pixel > 0 )
		{
			return Yes;
		}
		#else
		if
		(
			(R > 32)
			||
			(G > 32)
			||
			(B > 32)
		)
		{
			// nasty hack to get it working...
			return Yes;
		}
		#endif

		y++;
	}

	return No;
}

int
IndexedFont_Kerned_Column :: CalcXInc
(
	unsigned int currentOffset,
	unsigned int nextOffset,
	int* minOpaqueX,
	int* maxOpaqueX
)
{
	GLOBALASSERT(currentOffset<NumChars);
	GLOBALASSERT(nextOffset<NumChars);
	GLOBALASSERT( minOpaqueX );
	GLOBALASSERT( maxOpaqueX );

	// Compare rows in the pair of images (they have the same height)
	// First idea:
	// Iterate through X-inc values, finding the smallest
	// one which is "valid" for all rows.
	// "Valid" means there's no overlapping of non-transparent pixels
	// when the second image is displaced by the X-inc.
	// Tried this, but it took too long.

	// Second attempt:
	// Iterate through all rows, finding smallest X-inc which is valid
	// for each row, and maintaining what is the biggest "smallest X-inc"
	// you have so far.  This will be the return value.

	#if 1
	{
		int Biggest_MinXInc = 0;
		for (int Row=0;Row<GetHeight();Row++)
		{
			int MinXIncForRow = GetSmallestXIncForRow
			(
				currentOffset,
				nextOffset,
				minOpaqueX,
				maxOpaqueX,
				Row
			);		

			if (MinXIncForRow > Biggest_MinXInc)
			{
				Biggest_MinXInc = MinXIncForRow;
			}
		}

		return Biggest_MinXInc;
	}
	#else
	return FullWidthForOffset[currentOffset];
	#endif
}

OurBool
IndexedFont_Kerned_Column :: OverlapOnRow
(
	unsigned int currentOffset,
	unsigned int nextOffset,
	int Row,
	int ProposedXInc
)
{
	GLOBALASSERT(currentOffset<NumChars);
	GLOBALASSERT(nextOffset<NumChars);
	
	#if 1
	{
		DDSURFACEDESC tempDDSurfaceDesc;

		tempDDSurfaceDesc . dwSize = sizeof(DDSURFACEDESC);

		HRESULT hrLock = image_ptr -> Lock
		(
			NULL, // LPRECT lpDestRect,                
			&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc,
			(
				DDLOCK_READONLY
				| DDLOCK_SURFACEMEMORYPTR
				#if 0
				| DDLOCK_WAIT
				| DDLOCK_NOSYSLOCK
				#endif
			), // DWORD dwFlags,
			NULL // HANDLE hEvent
		);

		if ( hrLock != DD_OK )
		{
			// ought really to throw an exception

			LOGDXERR(hrLock);
			return Yes;
		}

		// Find right-most pixel in row of first character
		int rightmostX;
		int firstoffsetY = Row+(currentOffset*GetHeight());
		for (rightmostX=GetMaxWidth()-1;rightmostX>0;rightmostX--)
		{
			if
			(
				bOpaque
				(
					&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc
					rightmostX, // int x,
					firstoffsetY // int y
				)
			)
			{
				break;
			}
		}
		
		// Find left-most pixel in row of second character
		int leftmostX;
		int nextoffsetY = Row+(nextOffset*GetHeight());
		for (leftmostX=0;leftmostX<GetMaxWidth();leftmostX++)
		{
			if
			(
				bOpaque
				(
					&tempDDSurfaceDesc, // LPDDSURFACEDESC lpDDSurfaceDesc
					leftmostX, // int x,
					nextoffsetY // int y
				)
			)
			{
				break;
			}
		}

		// Is there an overlap when displaced by the proposed XInc?
		{
			return
			(
				(rightmostX)
				>=
				(leftmostX+ProposedXInc)
			);
		}
	}
	#else
	return Yes;
		// for now
	#endif
}

int
IndexedFont_Kerned_Column :: GetSmallestXIncForRow
(
	unsigned int currentOffset,
	unsigned int nextOffset,
	int* minOpaqueX,
	int* maxOpaqueX,
	unsigned int Row
)
{
	GLOBALASSERT( currentOffset < NumChars );
	GLOBALASSERT( nextOffset < NumChars );
	GLOBALASSERT( minOpaqueX );
	GLOBALASSERT( maxOpaqueX );
	GLOBALASSERT( Row < GetHeight() );

	{
		int Difference =
		(
			maxOpaqueX[Row+(currentOffset*GetHeight())] - minOpaqueX[Row+(nextOffset*GetHeight())]
			+1
		);

		if (Difference>0)
		{
			return Difference;
		}
		else
		{
			return 0;
		}
	}	
}


OurBool
IndexedFont_Kerned_Column :: bOpaque
(
	LPDDSURFACEDESC lpDDSurfaceDesc,
		// assumes you have a read lock
	int x,
	int y
		// must be in range
)
{
	GLOBALASSERT(lpDDSurfaceDesc);

	void* pSurface = lpDDSurfaceDesc -> lpSurface;

	GLOBALASSERT(pSurface);

	int BytesPerPixel = lpDDSurfaceDesc -> ddpfPixelFormat . dwRGBBitCount /8;

	int BytesPerRow =
	(
		lpDDSurfaceDesc -> lPitch
	);

	int Pixel =
	(
		*(int*)
		(
			((char*)pSurface)
			+
			( y * BytesPerRow )
			+
			( x * BytesPerPixel)
		)
	);

	return (Pixel != 0 );
}

















/* Internal function definitions ***********************************/
