#if 1


#include "3dc.h"

#include <sys/stat.h>

#include "inline.h"

#ifdef RIFF_SYSTEM
#include "chnktexi.h"
#endif

#define UseLocalAssert 0
#include "ourasert.h"

#else

#include <stdio.h>
#include <sys/stat.h>

#include "system.h"
#include "equates.h"
#include "platform.h"
#include "shape.h"
#include "prototyp.h"
#include "inline.h"

#ifdef RIFF_SYSTEM
#include "chnktexi.h"
#endif


#endif


#include "awtexld.h"

#if 0 /* SBF - commented out */
#include "alt_tab.h"
#endif

/*
	#define for experimental purposes 
	ONLY!!!
*/

#define DefinedTextureType TextureTypePPM


/*

 externs for commonly used global variables and arrays

*/

	extern SHAPEHEADER **mainshapelist;
	extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
	extern unsigned char *ScreenBuffer;
	extern char projectsubdirectory[];
    extern int ScanDrawMode;
	extern int VideoModeType;

/*

 Global Variables for PC Functions

*/

	TEXTURE *ImageBuffer;							/* Memory Resident Image Data */

	#ifdef MaxImageGroups
	#if MaxImageGroups < 2 /* optimize if this multiple groups are not required */
	#undef MaxImageGroups
	#endif /* MaxImageGroups < 2 */
	#endif /* MaxImageGroups */

	#ifdef MaxImageGroups

	#include "txioctrl.h"
	
	/*
	basically, I want there to be more than one image header array
	so that I can load some images once only, then load shapes, call
	InitializeTextures, DeallocateAllImages, etc. and only the images associated
	with the shapes load are deallocated.
	I might want to load shapes in two blocks, calling InitializeTextures
	for each load.

	There will need to be as many slots for ImageHeaderArrays as MaxImageGroups

	I want this to be completely invisible to anyone except programmers on
	PC projects that require this feature

	Jake.
	*/

	/* these three globals must behave the same */
	int NumImages = 0;								/* # current images */
	IMAGEHEADER *ImageHeaderPtrs[MaxImageGroups*MaxImages];	/* Ptrs to Image Header Blocks */
	IMAGEHEADER ImageHeaderArray[MaxImageGroups*MaxImages];	/* Array of Image Headers */
	
	int NumImagesArray[MaxImageGroups]; /* must be static to ensure initialization to zero */
	static int CurrentImageGroup = 0;
	static IMAGEHEADER *NextFreeImageHeaderPtr[MaxImageGroups];
	
	#else /* ! MaxImageGroups */
	
	int NumImages = 0;								/* # current images */
	IMAGEHEADER *ImageHeaderPtrs[MaxImages];	/* Ptrs to Image Header Blocks */
	IMAGEHEADER ImageHeaderArray[MaxImages];	/* Array of Image Headers */
	
	static IMAGEHEADER *NextFreeImageHeaderPtr;

	#endif /* ! MaxImageGroups */


/*

 Initialise General Texture Data Structures and Variables

*/


#if LoadingMapsShapesAndTexturesEtc


void InitialiseImageHeaders(void)

{
	#ifdef MaxImageGroups

	NumImages = CurrentImageGroup * MaxImages;
	NextFreeImageHeaderPtr[CurrentImageGroup] = &ImageHeaderArray[CurrentImageGroup*MaxImages];

	#else

	NumImages = 0;
	NextFreeImageHeaderPtr = ImageHeaderArray;

	#endif
}

#else


#define InitTexPrnt No

int InitialiseTextures(void)

{

	SHAPEHEADER **shlistptr;
	SHAPEHEADER *shptr;
	char **txfiles;
	int TxIndex;
	int LTxIndex;

	/*

	Free up any currently loaded images

	The caller is responsible for any other structures which might refer
	to the currently loaded images

	*/

	#ifdef MaxImageGroups

    DeallocateCurrentImages();

	NumImages = CurrentImageGroup * MaxImages;
	NextFreeImageHeaderPtr[CurrentImageGroup] = &ImageHeaderArray[CurrentImageGroup*MaxImages];

	#else

    DeallocateAllImages();

	/* Initialise Image Header Variables */

	NumImages = 0;
	NextFreeImageHeaderPtr = ImageHeaderArray;

	#endif

	/* Added 23/3/98 by DHM so that this can be called without loading any
	shapes (to get textprint working in the menus):
	*/
	if ( NULL == mainshapelist )
	{
		return Yes;
			// early exit
	}

	/* Build the Texture List */

	shlistptr = &mainshapelist[0];

	while(*shlistptr) {

		shptr = *shlistptr++;

		/* If the shape has textures */

		if(shptr->sh_localtextures) {

			#if InitTexPrnt
			textprint("This shape has textures\n");
			#endif

			txfiles = shptr->sh_localtextures;

			LTxIndex = 0;

			while(*txfiles) {

				/* The RIFF Image loaders have changed to support not loading the same image twice - JH 17-2-96 */

				char *src;
				char *dst;
				char fname[ImageNameSize];
				char *txfilesptr;
				#ifndef RIFF_SYSTEM
				int i, j, NewImage;
				char *iname;
				IMAGEHEADER *ihptr;
				IMAGEHEADER *new_ihptr;
				void* im;
				#endif
				
				txfilesptr = *txfiles++;
				
				/*

				"txfilesptr" is in the form "textures\<fname>". We need to
				prefix that text with the name of the current textures path.

				Soon this path may be varied but for now it is just the name of
				the current project subdirectory.

				*/

				src = projectsubdirectory;
				dst = fname;

				while(*src)
					*dst++ = *src++;

				src = txfilesptr;

				while(*src)
					*dst++ = *src++;

				*dst = 0;


				#if InitTexPrnt
				textprint(" A Texture\n");
				#endif

				
				#ifdef RIFF_SYSTEM

				/* This function calls GetExistingImageHeader to figure out if the image is already loaded */
				TxIndex = CL_LoadImageOnce(fname,(ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)|LIO_TRANSPARENT|LIO_RELATIVEPATH|LIO_RESTORABLE);
				GLOBALASSERT(GEI_NOTLOADED != TxIndex);
				
				#else
				
				/* If there are already images, try and find this one */

				NewImage = Yes;

				#ifdef MaxImageGroups

				TxIndex = CurrentImageGroup * MaxImages;			/* Assume image 0 */
				
				if(NumImagesArray[CurrentImageGroup]) {

					for(i=NumImagesArray[CurrentImageGroup]; i!=0 && NewImage!=No; i--) {

				#else

				TxIndex = 0;			/* Assume image 0 */
				
				if(NumImages) {

					for(i=NumImages; i!=0 && NewImage!=No; i--) {

				#endif

						ihptr = ImageHeaderPtrs[TxIndex];

						iname = &ihptr->ImageName[0];

						j = CompareFilenameCH(txfilesptr, iname);

						if(j) NewImage = No;

						else TxIndex++;

					}

				}


				/* If this is a new image, add it */

				if(NewImage) {

					#if InitTexPrnt
					textprint("New Image\n");
					WaitForReturn();
					#endif

					/* Get an Image Header */

					new_ihptr = GetImageHeader();

					if(new_ihptr) {

			            if (ScanDrawMode == ScanDrawDirectDraw)
						  im = (void*) LoadImageCH(&fname[0], new_ihptr);
						else
						  im = LoadImageIntoD3DImmediateSurface
						     (&fname[0], new_ihptr, DefinedTextureType);

						if(im) {

							#if InitTexPrnt
							textprint("Load OK, NumImages = %d\n", NumImages);
							WaitForReturn();
							#endif

						}

					}

				}


				/* test */

				#if InitTexPrnt
				else textprint("Image Already Exists\n");
				#endif

				#endif

				/*

					The local index for this image in this shape is
					"LTxIndex".

					The global index for the image is "TxIndex".

					We must go through the shape's items and change all the
					local references to global.

				*/

				#if InitTexPrnt
				textprint("\nLocal to Global for Shape\n");
				#endif

				MakeShapeTexturesGlobal(shptr, TxIndex, LTxIndex);

				LTxIndex++;			/* Next Local Texture */

			}

			/* Is this shape a sprite that requires resizing? */

			if((shptr->shapeflags & ShapeFlag_Sprite) &&
				(shptr->shapeflags & ShapeFlag_SpriteResizing)) {

				SpriteResizing(shptr);

			}

		}

	}

	#if InitTexPrnt
	textprint("\nFinished PP for textures\n");

	WaitForReturn();

	#endif

	return Yes;
}


#endif


/*

 This function accepts a shape header, a global texture index and a local
 index. It searches through all the items that use textures and converts all
 local references to a texture to global references.

 The index to a texture that refers to the IMAGEHEADER pointer array is in
 the low word of the colour int.

 Bit #15 is set if this index refers to a local texture.

 Here is an example of a textured item:


int CUBE_item3[]={

	I_2dTexturedPolygon,3*vsize,0,

	(3<<TxDefn) + TxLocal + 0,

	7*vsize,5*vsize,1*vsize,3*vsize,
	Term

};


 To test for a local texture use:

	if(ColourInt & TxLocal)



 NOTE

 The procedure is NOT reversible!

 If one wishes to reconstruct the table, all the shapes and textures must be
 reloaded and the initialisation function recalled.

 Actually a function could be written that relates global filenames back to
 local filenames, so in a sense that is not true. This function will only be
 written if it is needed.

*/

void MakeShapeTexturesGlobal(SHAPEHEADER *shptr, int TxIndex, int LTxIndex)

{

	int **ShapeItemArrayPtr;
	POLYHEADER *ShapeItemPtr;

	#if SupportBSP
	SHAPEDATA_BSP_BLOCK *ShapeBSPPtr;
	int num_bsp_blocks;
	int j, k;
	#endif

	int i, txi;


	/* Are the items in a pointer array? */

	if(shptr->items) {

		#if InitTexPrnt
		textprint("Item Array\n");
		#endif

		ShapeItemArrayPtr = shptr->items;

		for(i = shptr->numitems; i!=0; i--) {

			ShapeItemPtr = (POLYHEADER *) *ShapeItemArrayPtr++;
			
			#if SupportZBuffering

			if(ShapeItemPtr->PolyItemType == I_2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ZB_2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_Gouraud2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ZB_Gouraud2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_Gouraud3dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ZB_Gouraud3dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ScaledSprite
				|| ShapeItemPtr->PolyItemType == I_3dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ZB_3dTexturedPolygon) {
			#else
			if(ShapeItemPtr->PolyItemType == I_2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_Gouraud2dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_Gouraud3dTexturedPolygon
				|| ShapeItemPtr->PolyItemType == I_ScaledSprite
				|| ShapeItemPtr->PolyItemType == I_3dTexturedPolygon){
			#endif /* SupportZBuffering */

				if(ShapeItemPtr->PolyFlags & iflag_txanim) {

					MakeTxAnimFrameTexturesGlobal(shptr, ShapeItemPtr,
															LTxIndex, TxIndex);

				}

				if(ShapeItemPtr->PolyColour & TxLocal) {

					txi = ShapeItemPtr->PolyColour;
					txi &= ~TxLocal;							/* Clear Flag */
					txi &= ClrTxDefn;							/* Clear UV array index */

					/* Is this the local index? */

					if(txi == LTxIndex) {

						/* Clear low word, OR in global index */

						ShapeItemPtr->PolyColour &= ClrTxIndex;
						ShapeItemPtr->PolyColour |= TxIndex;

					}

				}

			}

		}

	}

	#if SupportBSP

	/* Or are they in a BSP block array? */

	else if(shptr->sh_bsp_blocks) {

		#if InitTexPrnt
		textprint("BSP Block Array\n");
		#endif

		#if 0
		/* Find the BSP Instruction */
		ShInstrPtr = shptr->sh_instruction;
		num_bsp_blocks = 0;
		while(ShInstrPtr->sh_instr != I_ShapeEnd) {
			if(ShInstrPtr->sh_instr == I_ShapeBSPTree) {
				num_bsp_blocks = ShInstrPtr->sh_numitems;
			}
			ShInstrPtr++;
		}
		#endif

		num_bsp_blocks = FindNumBSPNodes(shptr);


		#if InitTexPrnt
		textprint("Number of BSP blocks = %d\n", num_bsp_blocks);
		#endif

		if(num_bsp_blocks) {

			ShapeBSPPtr = shptr->sh_bsp_blocks;

			for(k=num_bsp_blocks; k!=0; k--) {

				ShapeItemArrayPtr = ShapeBSPPtr->bsp_block_data;

				for(j=ShapeBSPPtr->bsp_numitems; j!=0; j--) {

					ShapeItemPtr = (POLYHEADER *) *ShapeItemArrayPtr++;

					#if InitTexPrnt
					textprint("shape item\n");
					#endif

					if(ShapeItemPtr->PolyItemType == I_2dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_ZB_2dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_Gouraud2dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_ZB_Gouraud2dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_Gouraud3dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_ZB_Gouraud3dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_ScaledSprite
						|| ShapeItemPtr->PolyItemType == I_3dTexturedPolygon
						|| ShapeItemPtr->PolyItemType == I_ZB_3dTexturedPolygon) {

						if(ShapeItemPtr->PolyFlags & iflag_txanim) {

							MakeTxAnimFrameTexturesGlobal(shptr, ShapeItemPtr,
																	LTxIndex, TxIndex);

						}

						#if InitTexPrnt
						textprint(" - textured\n");
						#endif

						if(ShapeItemPtr->PolyColour & TxLocal) {

							#if InitTexPrnt
							textprint("  - local index\n");
							#endif

							txi = ShapeItemPtr->PolyColour;
							txi &= ~(TxLocal);					/* Clear Flag */
							txi &= ClrTxDefn;						/* Clear Defn */

							#if InitTexPrnt
							textprint("  - is %d\n", txi);
							textprint("  - LTxIndex is %d\n", LTxIndex);
							#endif

							/* Is this the local index? */

							if(txi == LTxIndex) {

								/* Clear low word, OR in global index */

								ShapeItemPtr->PolyColour &= ClrTxIndex;
								ShapeItemPtr->PolyColour |= TxIndex;

								#if InitTexPrnt
								textprint("Local %d, Global %d\n", LTxIndex, TxIndex);
								#endif

							}

						}

					}

				}

				ShapeBSPPtr++;

			}

		}

	}

	#endif	/* SupportBSP */

	/* Otherwise the shape has no item data */


}


/*

 The animated texture frames each have a local image index in their frame
 header structure. Convert these to global texture list indices in the same
 way that the item function does.

*/

void MakeTxAnimFrameTexturesGlobal(SHAPEHEADER *sptr,
														POLYHEADER *pheader,
														int LTxIndex, int TxIndex)

{

	TXANIMHEADER **txah_ptr;
	TXANIMHEADER *txah;
	TXANIMFRAME *txaf;
	int **shape_textures;
	int *txf_imageptr;
	int texture_defn_index;
	int i, txi, image;


	#if 0
	textprint("LTxIndex = %d, TxIndex = %d\n", LTxIndex, TxIndex);
	WaitForReturn();
	#endif


	/* Get the animation sequence header */

	shape_textures = sptr->sh_textures;
	texture_defn_index = (pheader->PolyColour >> TxDefn);
	txah_ptr = (TXANIMHEADER **) shape_textures[texture_defn_index];


	/* The first array element is the sequence shadow, which we skip here */

	txah_ptr++;


	/* Process the animation sequences */

	while(*txah_ptr) {

		/* Get the animation header */

		txah = *txah_ptr++;

		/* Process the animation frames */

		if(txah && txah->txa_numframes) {

			txaf = txah->txa_framedata;

			for(i = txah->txa_numframes; i!=0; i--) {

				/* Multi-View Sprite? */

				if(sptr->shapeflags & ShapeFlag_MultiViewSprite) {

					txf_imageptr = (int *) txaf->txf_image;

					for(image = txah->txa_num_mvs_images; image!=0; image--) {

						if(*txf_imageptr & TxLocal) {

							txi = *txf_imageptr;
							txi &= ~TxLocal;					/* Clear Flag */

							if(txi == LTxIndex) {

								*txf_imageptr = TxIndex;

							}

						}

						txf_imageptr++;

					}

				}

				else {

					if(txaf->txf_image & TxLocal) {

						txi = txaf->txf_image;
						txi &= ~TxLocal;					/* Clear Flag */

						if(txi == LTxIndex) {

							txaf->txf_image = TxIndex;

						}

					}

				}

				txaf++;

			}

		}

	}

}



/*

 Sprite Resizing

 or

 An Optimisation For Sprite Images

 This shape is a sprite which has requested the UV rescaling optimisation.
 The UV array is resized to fit the sprite image bounding rectangle, and
 after the UV array, in the space provided (don't forget that!), is a new
 shape local space XY array of points which overwrite the standard values
 when the shape is displayed.

*/

#define sr_print No

void SpriteResizing(SHAPEHEADER *sptr)

{

	TXANIMHEADER **txah_ptr;
	TXANIMHEADER *txah;
	TXANIMFRAME *txaf;
	int **shape_textures;
	IMAGEHEADER *ihdr;
	IMAGEEXTENTS e;
	IMAGEEXTENTS e_curr;
	IMAGEPOLYEXTENTS e_poly;
	int *uvptr;
	int texture_defn_index;
	int **item_array_ptr;
	int *item_ptr;
	POLYHEADER *pheader;
	int i, f;
	int polypts[4 * vsize];
	int *iptr;
	int *iptr2;
	int *mypolystart;
	int *ShapePoints = *(sptr->points);
	VECTOR2D cen_poly;
	VECTOR2D size_poly;
	VECTOR2D cen_uv_curr;
	VECTOR2D size_uv_curr;
	VECTOR2D cen_uv;
	VECTOR2D size_uv;
	VECTOR2D tv;
	int *txf_imageptr;
	int **txf_uvarrayptr;
	int *txf_uvarray;
	int image;
	int num_images;


	#if sr_print
	textprint("\nSprite Resize Shape\n\n");
	#endif


	/* Get the animation sequence header */

	shape_textures = sptr->sh_textures;

	item_array_ptr = sptr->items;		/* Assume item array */
	item_ptr = item_array_ptr[0];		/* Assume only one polygon */
	pheader = (POLYHEADER *) item_ptr;


	/* Get the polygon points, and at the same time the extents, assuming an XY plane polygon */

	e_poly.x_low = bigint;
	e_poly.y_low = bigint;

	e_poly.x_high = smallint;
	e_poly.y_high = smallint;

	iptr = polypts;
	mypolystart = &pheader->Poly1stPt;

	for(i = 4; i!=0; i--) {

		iptr[ix] = ((VECTORCH*)ShapePoints)[*mypolystart].vx;
		iptr[iy] = ((VECTORCH*)ShapePoints)[*mypolystart].vy;
		iptr[iz] = ((VECTORCH*)ShapePoints)[*mypolystart].vz;

		if(iptr[ix] < e_poly.x_low) e_poly.x_low = iptr[ix];
		if(iptr[iy] < e_poly.y_low) e_poly.y_low = iptr[iy];
		if(iptr[ix] > e_poly.x_high) e_poly.x_high = iptr[ix];
		if(iptr[iy] > e_poly.y_high) e_poly.y_high = iptr[iy];

		iptr += vsize;
		mypolystart++;

	}


	texture_defn_index = (pheader->PolyColour >> TxDefn);
	txah_ptr = (TXANIMHEADER **) shape_textures[texture_defn_index];


	/* The first array element is the sequence shadow, which we skip here */

	txah_ptr++;


	/* Process the animation sequences */

	while(*txah_ptr) {

		/* Get the animation header */

		txah = *txah_ptr++;

		/* Process the animation frames */

		if(txah && txah->txa_numframes) {

			txaf = txah->txa_framedata;

			for(f = txah->txa_numframes; f!=0; f--) {


				/* Multi-View Sprite? */

				if(sptr->shapeflags & ShapeFlag_MultiViewSprite) {

					txf_imageptr = (int *) txaf->txf_image;
					num_images = txah->txa_num_mvs_images;

					txf_uvarrayptr = (int **) txaf->txf_uvdata;

				}

				/* A standard "Single View" Sprite has just one image */

				else {

					txf_imageptr = &txaf->txf_image;
					num_images = 1;

					txf_uvarrayptr = &txaf->txf_uvdata;

				}


				for(image = 0; image < num_images; image++) {


					#if sr_print
					textprint("image %d of %d   \n", (image + 1), num_images);
					#endif


					/* Get the image */

					ihdr = ImageHeaderPtrs[txf_imageptr[image]];

					/* Get the uv array ptr */

					txf_uvarray = txf_uvarrayptr[image];

					/* Find the extents of the image, assuming transparency */

					#if 0
					FindImageExtents(ihdr, txaf->txf_numuvs, txaf->txf_uvdata, &e, &e_curr);
					#else
					FindImageExtents(ihdr, txaf->txf_numuvs, txf_uvarray, &e, &e_curr);
					#endif

					/* Convert the image extents to fixed point */

					#if sr_print
					textprint("extents = %d, %d\n", e.u_low, e.v_low);
					textprint("          %d, %d\n", e.u_high, e.v_high);
					WaitForReturn();
					#endif

					e.u_low  <<= 16;
					e.v_low  <<= 16;
					e.u_high <<= 16;
					e.v_high <<= 16;

					/*

					We now have all the information needed to create a NEW UV array and a NEW
					polygon points XY array.

					*/

					/* Centre of the polygon */

					cen_poly.vx = (e_poly.x_low + e_poly.x_high) / 2;
					cen_poly.vy = (e_poly.y_low + e_poly.y_high) / 2;

					/* Size of the polygon */

					size_poly.vx = e_poly.x_high - e_poly.x_low;
					size_poly.vy = e_poly.y_high - e_poly.y_low;


					/* Centre of the current cookie */

					cen_uv_curr.vx = (e_curr.u_low + e_curr.u_high) / 2;
					cen_uv_curr.vy = (e_curr.v_low + e_curr.v_high) / 2;

					/* Size of the current cookie */

					size_uv_curr.vx = e_curr.u_high - e_curr.u_low;
					size_uv_curr.vy = e_curr.v_high - e_curr.v_low;


					/* Centre of the new cookie */

					cen_uv.vx = (e.u_low + e.u_high) / 2;
					cen_uv.vy = (e.v_low + e.v_high) / 2;

					/* Size of the new cookie */

					size_uv.vx = e.u_high - e.u_low;
					size_uv.vy = e.v_high - e.v_low;


					/* Write out the new UV data */

					#if 0
					uvptr = txaf->txf_uvdata;
					#else
					uvptr = txf_uvarray;
					#endif


					/*

					Convert the duplicate UV array to this new scale
					ASSUME that the format is "TL, BL, BR, TR"

					*/

					uvptr[0] = e.u_low;
					uvptr[1] = e.v_low;

					uvptr[2] = e.u_low;
					uvptr[3] = e.v_high;

					uvptr[4] = e.u_high;
					uvptr[5] = e.v_high;

					uvptr[6] = e.u_high;
					uvptr[7] = e.v_low;


					/*

					Create the new polygon XY array

					*/

					uvptr += (txaf->txf_numuvs * 2);	/* Advance the pointer past the UV array */


					/* Copy the polygon points (XY only) to the UV array space */

					iptr  = polypts;
					iptr2 = uvptr;

					for(i = 4; i!=0; i--) {

						iptr2[0] = iptr[ix];
						iptr2[1] = iptr[iy];

						iptr  += vsize;
						iptr2 += 2;

					}


					/* Scale the polygon points */

					iptr = uvptr;

					for(i = 4; i!=0; i--) {

						iptr[0] = WideMulNarrowDiv(iptr[0], size_uv.vx, size_uv_curr.vx);
						iptr[1] = WideMulNarrowDiv(iptr[1], size_uv.vy, size_uv_curr.vy);

						iptr += 2;

					}


					/* The translation vector in UV space */

					tv.vx = cen_uv.vx - cen_uv_curr.vx;
					tv.vy = cen_uv.vy - cen_uv_curr.vy;


					/* And now in world space */

					tv.vx = WideMulNarrowDiv(tv.vx, size_poly.vx, size_uv_curr.vx);
					tv.vy = WideMulNarrowDiv(tv.vy, size_poly.vy, size_uv_curr.vy);


					/* Translate the polygon points */

					iptr = uvptr;

					for(i = 4; i!=0; i--) {

						iptr[0] += tv.vx;
						iptr[1] += tv.vy;

						iptr += 2;

					}


					#if sr_print
					textprint(" (image done)\n");
					#endif


				}


				#if sr_print
				textprint("\n");
				#endif


				/* Next Texture Animation Frame */

				txaf++;

			}

		}

	}


	#if sr_print
	textprint("\nResize done\n\n");
	#endif

}


/*

 This function is for animated sprites, although it has been given an interface that would
 allow anyone to call it for their own purposes. It assumes that colour 0 is transparent and
 then calculates the actual maximum extents of the image.

 NOTE:

 The image scanned is that given by the CURRENT UV ARRAY and may therefore be smaller than
 the actual image. This allows for animation sequences with frames on the same image.

*/

void FindImageExtents(IMAGEHEADER *ihdr, int numuvs, int *uvdata, IMAGEEXTENTS *e, IMAGEEXTENTS *e_curr)

{

	int i;
	int *uvptr;
	int u, v;
	int startu, endu;
	int startv, endv;


	/* Find the current UV extents */

	e_curr->u_low = bigint;
	e_curr->v_low = bigint;

	e_curr->u_high = smallint;
	e_curr->v_high = smallint;

	uvptr = uvdata;

	for(i = numuvs; i!=0; i--) {

		if(uvptr[0] < e_curr->u_low) e_curr->u_low = uvptr[0];
		if(uvptr[1] < e_curr->v_low) e_curr->v_low = uvptr[1];

		if(uvptr[0] > e_curr->u_high) e_curr->u_high = uvptr[0];
		if(uvptr[1] > e_curr->v_high) e_curr->v_high = uvptr[1];

		uvptr += 2;

	}


	/* Look for the actual UV extents, assuming that colour 0 is transparent */

	switch(VideoModeType) {

		case VideoModeType_8:

			{

				TEXTURE *tptr = ihdr->ImagePtr;
				TEXTURE texel;


				/* Search for u_low and v_low */

				e->u_low = bigint;
				e->v_low = bigint;

				startv = e_curr->v_low  >> 16;
				endv   = e_curr->v_high >> 16;
				startu = e_curr->u_low  >> 16;
				endu   = e_curr->u_high >> 16;

				for(v = startv; v <= endv; v++) {

					for(u = startu; u <= endu; u++) {

						texel = tptr[(v * ihdr->ImageWidth) + u];

						if(texel) {

							if(u < e->u_low) e->u_low = u;
							if(v < e->v_low) e->v_low = v;

						}

					}

				}

				if(e->u_low == bigint) e->u_low = e_curr->u_low;
				if(e->v_low == bigint) e->v_low = e_curr->v_low;


				/* Search for u_high and v_high */

				e->u_high = smallint;
				e->v_high = smallint;

				for(v = endv; v >= startv; v--) {

					for(u = endu; u >= startu;  u--) {

						texel = tptr[(v * ihdr->ImageWidth) + u];

						if(texel) {

							if(u > e->u_high) e->u_high = u;
							if(v > e->v_high) e->v_high = v;

						}

					}

				}

				if(e->u_high == smallint) e->u_high = e_curr->u_high;
				if(e->v_high == smallint) e->v_high = e_curr->v_high;

			}

			break;

		case VideoModeType_15:
			break;

		case VideoModeType_24:
			break;

		case VideoModeType_8T:
			break;

	}

}



/*

 Return a pointer to the next image header

*/


IMAGEHEADER* GetImageHeader(void)
{

	IMAGEHEADER *iheader;

	#ifdef MaxImageGroups

	/* NumImages always points to the correct point in the array */
	do
	{
		iheader = NextFreeImageHeaderPtr[CurrentImageGroup]++;
	}
	while (IsImageInUse(CurrentImageGroup,NumImagesArray[CurrentImageGroup]++) ? ++NumImages : 0);
	
	GLOBALASSERT(NumImagesArray[CurrentImageGroup] < MaxImages);

	#else

	iheader = NextFreeImageHeaderPtr++;
	GLOBALASSERT(NumImages < MaxImages);

	#endif
	
	/* ensure flags are zero */
	memset(iheader,0,sizeof(IMAGEHEADER));

	ImageHeaderPtrs[NumImages] = iheader;
	NumImages++;
	
	return iheader;

}


/*

 Return the address of a texture image in memory, else null

*/

#if 0
void* GetTexture(int texindex)

{

    /* Ahem... */

	return No;

}
#endif

/*

 Allocate memory for a Texture Image

*/

#if 0
TEXTURE* GetTextureMemory(int txsize)

{

    /* Err... */
	return No;

}
#endif

/*

 Deallocate memory for a Texture Image

*/

#if 0
void ReturnTextureMemory(TEXTURE *txptr)

{

}
#endif

static void DeallocateImageHeader(IMAGEHEADER * ihptr)
{
	if (ihptr->hBackup)
	{
		if (ihptr->DDSurface)
		{
			GLOBALASSERT(!ihptr->D3DTexture);
			ATRemoveSurface(ihptr->DDSurface);
		}
		else if (ihptr->D3DTexture)
		{
			GLOBALASSERT(!ihptr->DDSurface);
			ATRemoveTexture(ihptr->D3DTexture);
		}
		
		AwDestroyBackupTexture(ihptr->hBackup);
		ihptr->hBackup = 0;
	}
	
	if (ihptr->ImagePtr) 
	{
		DeallocateMem(ihptr->ImagePtr);
		ihptr->ImagePtr = 0;
	}

	if (ihptr->DDSurface)
	{
		ReleaseDDSurface(ihptr->DDSurface);
		ihptr->DDSurface = (void*) 0;
	}

	if (ihptr->D3DTexture)
	{
		ReleaseD3DTexture(ihptr->D3DTexture);
		ihptr->D3DTexture = (void*) 0;
		ihptr->D3DHandle = /* (void*) */ 0;
	}
}

static void MinimizeImageHeader(IMAGEHEADER * ihptr)
{
	if (ihptr->DDSurface)
	{
		ReleaseDDSurface(ihptr->DDSurface);
		ihptr->DDSurface = (void*) 0;
	}

	if (ihptr->D3DTexture)
	{
		ReleaseD3DTexture(ihptr->D3DTexture);
		ihptr->D3DTexture = (void*) 0;
		ihptr->D3DHandle = /* (void*) */ 0;
	}
}

static void RestoreImageHeader(IMAGEHEADER * ihptr)
{
	if (ScanDrawDirectDraw != ScanDrawMode)
		ReloadImageIntoD3DImmediateSurface(ihptr);
}

#ifdef MaxImageGroups

void SetCurrentImageGroup(unsigned int group)
{
	GLOBALASSERT(group < MaxImageGroups);
	CurrentImageGroup = group;
	NumImages = group*MaxImages + NumImagesArray[group];
}

int DeallocateCurrentImages(void)
{
	int i;
	IMAGEHEADER *ihptr;

	if (NumImagesArray[CurrentImageGroup])
	{
		ihptr = &ImageHeaderArray[CurrentImageGroup*MaxImages];
		for (i = 0; i < NumImagesArray[CurrentImageGroup]; ++i)
		{
			if (CanDeleteImage(CurrentImageGroup,i))
				DeallocateImageHeader(ihptr);
			++ihptr;
		}		
		NumImagesArray[CurrentImageGroup] = 0;
		NumImages = CurrentImageGroup * MaxImages;
		NextFreeImageHeaderPtr[CurrentImageGroup] = &ImageHeaderArray[CurrentImageGroup*MaxImages];
		ImageGroupFreed(CurrentImageGroup);
	}

	return Yes; /* ok for the moment */
}

void NowDeleteImage(int img_group, int img_num_offset)
{
	DeallocateImageHeader(&ImageHeaderArray[img_group*MaxImages+img_num_offset]);
}

int DeallocateAllImages(void)
{
	int i, j;
	IMAGEHEADER *ihptr;

	for (j=0; j<MaxImageGroups; ++j)
	{
		if (NumImagesArray[j])
		{
			ihptr = &ImageHeaderArray[j*MaxImages];
			for (i = 0; i<NumImagesArray[j]; ++i)
			{
				if (CanDeleteImage(j,i))
					DeallocateImageHeader(ihptr);
				++ihptr;
			}		
			NumImagesArray[j] = 0;
		}
		ImageGroupFreed(j);
	}
	NumImages = CurrentImageGroup * MaxImages;
	NextFreeImageHeaderPtr[CurrentImageGroup] = &ImageHeaderArray[CurrentImageGroup*MaxImages];

	return Yes; /* ok for the moment */
}

static void MinimizeImageCallback(int i, void * gP)
{
	int g = *(int *)gP;
	MinimizeImageHeader(ImageHeaderPtrs[g*MaxImages+i]);
}

int MinimizeAllImages(void)
{
	int i, j;
	IMAGEHEADER *ihptr;

	for (j=0; j<MaxImageGroups; ++j)
	{
		if (NumImagesArray[j])
		{
			ihptr = &ImageHeaderArray[j*MaxImages];
			for (i = 0; i<NumImagesArray[j]; ++i)
			{
				MinimizeImageHeader(ihptr);
				++ihptr;
			}		
		}
		EnumLeftoverImages(j,NumImagesArray[j],MinimizeImageCallback,&j);
	}

	return Yes; /* ok for the moment */
}

static void RestoreImageCallback(int i, void * gP)
{
	int g = *(int *)gP;
	RestoreImageHeader(ImageHeaderPtrs[g*MaxImages+i]);
}

int RestoreAllImages(void)
{
	int i, j;
	IMAGEHEADER *ihptr;

	for (j=0; j<MaxImageGroups; ++j)
	{
		if (NumImagesArray[j])
		{
			ihptr = &ImageHeaderArray[j*MaxImages];
			for (i = 0; i<NumImagesArray[j]; ++i)
			{
				RestoreImageHeader(ihptr);
				++ihptr;
			}		
		}
		EnumLeftoverImages(j,NumImagesArray[j],RestoreImageCallback,&j);
	}

	return Yes; /* ok for the moment */
}

#if debug

struct ImageGroupDebugInfo
{
	int num_texels;
	int num_images;
	int num_shared;
	int num_leftover;
};

static struct ImageGroupDebugInfo db_gp_info[MaxImageGroups];

static void DbShareImgCallback(int imgnum, void * user)
{
	int g = *(int *)user;
	
	++db_gp_info[g].num_shared;
}

static void DbLeftoverImgCallback(int i, void * user)
{
	int g = *(int *)user;
	
	++db_gp_info[g].num_leftover;

	db_gp_info[g].num_texels += ImageHeaderPtrs[g*MaxImages+i]->ImageWidth * ImageHeaderPtrs[g*MaxImages+i]->ImageHeight;
}

void ImageGroupsDebugPrintInit(void)
{
	int g;
	for (g=0; g<MaxImageGroups; g++)
	{
		int i;
		
		db_gp_info[g].num_texels = 0;
		db_gp_info[g].num_images = NumImagesArray[g];
		db_gp_info[g].num_shared = 0;
		db_gp_info[g].num_leftover = 0;

		EnumSharedImages(g,NumImagesArray[g],DbShareImgCallback,&g);
		EnumLeftoverImages(g,NumImagesArray[g],DbLeftoverImgCallback,&g);

		for (i=0; i<NumImagesArray[g]; ++i)
		{
			db_gp_info[g].num_texels += ImageHeaderPtrs[g*MaxImages+i]->ImageWidth * ImageHeaderPtrs[g*MaxImages+i]->ImageHeight;
		}
	}
}

void ImageGroupsDebugPrint(void)
{
	int g;
	textprint("IMAGE GROUP DEBUG INFO\nGP  N_IMG  N_SHR  N_LFT  N_TEXELS\n");
	for (g=0; g<MaxImageGroups; ++g)
	{
		textprint("%2d  %5d  %5d  %5d  %8d\n",g,db_gp_info[g].num_images,db_gp_info[g].num_shared,db_gp_info[g].num_leftover,db_gp_info[g].num_texels);
	}
}

#endif
	
#else

int DeallocateAllImages(void)
{
	int i;
	IMAGEHEADER *ihptr;

	if (NumImages)
	{
		ihptr = ImageHeaderArray;
		for (i = NumImages; i!=0; i--)
		{
			DeallocateImageHeader(ihptr++);
		}		
		NumImages = 0;
		NextFreeImageHeaderPtr = ImageHeaderArray;
	}

	return Yes; /* ok for the moment */
}

int MinimizeAllImages(void)
{
	int i;
	IMAGEHEADER *ihptr;

	if (NumImages)
	{
		ihptr = ImageHeaderArray;
		for (i = NumImages; i!=0; i--)
		{
			MinimizeImageHeader(ihptr++);
		}		
	}

	return Yes; /* ok for the moment */
}

int RestoreAllImages(void)
{
	int i;
	IMAGEHEADER *ihptr;

	if (NumImages)
	{
		ihptr = ImageHeaderArray;
		for (i = NumImages; i!=0; i--)
		{
			RestoreImageHeader(ihptr++);
		}		
	}

	return Yes; /* ok for the moment */
}

#endif


#ifdef RIFF_SYSTEM

/*
The RIFF_SYSTEM uses this function to return an image number
for an image which might be already loaded. The argument
passed points to the full pathname of the image that the
system wants to load, so an explicit stricmp on the
image names of already loaded images will suffice. To
avoid loading images more than once, it ensures that the
path generated for two identical images will always be
the same. It also fills in the ImageName field with the
fill path of images it loads.

Currently I am assuming that users of ImageGroups will
no longer need images in group n+1 when images in group
n are deleted, so I can check in all groups from 0...Current
to see if the image is already loaded.

Jake.
*/

int GetExistingImageNum(char const * fname)
{
	int i;
	IMAGEHEADER * iharrayptr;
	
	#ifdef MaxImageGroups

	int g;

	for (g=0; g<MaxImageGroups; ++g)
	{
		for (i=0, iharrayptr = &ImageHeaderArray[g*MaxImages]; i<NumImagesArray[g]; ++i, ++iharrayptr)
		{
			if (!stricmp(iharrayptr->ImageName,fname))
			{
				if (g!=CurrentImageGroup)
					MarkImageInUseByGroup(g,i,CurrentImageGroup);
				return i+g*MaxImages;
			}
		}
	}

	#else
	
	for (i=0, iharrayptr = ImageHeaderArray; i<NumImages; ++i, ++iharrayptr)
	{
		if (!stricmp(iharrayptr->ImageName,fname)) return i;
	}

	#endif

	return GEI_NOTLOADED;
}

#endif

