#include "3dc.h"
#include "inline.h"

/*
 externs for commonly used global variables and arrays
*/

extern MORPHDISPLAY MorphDisplay;
extern int NormalFrameTime;

/*
 Global Variables
*/

/*
 Update Morphing Animation Control Block
*/

void UpdateMorphing(MORPHCTRL *mcptr)
{
	MORPHHEADER *mhdr = mcptr->ObMorphHeader;
	int UpdateRate;


	/*textprint("UpdateMorphing\n");*/


	if(mcptr->ObMorphFlags & mph_flag_play) {

		/* How fast? */

		if(mcptr->ObMorphSpeed == ONE_FIXED) {

			UpdateRate = NormalFrameTime;

		}

		else {

			UpdateRate = MUL_FIXED(NormalFrameTime, mcptr->ObMorphSpeed);

		}


		/* Update the current frame */

		if(mcptr->ObMorphFlags & mph_flag_reverse) {

			mcptr->ObMorphCurrFrame -= UpdateRate;

			if(mcptr->ObMorphCurrFrame < 0) {

				if(mcptr->ObMorphFlags & mph_flag_noloop) {

					mcptr->ObMorphCurrFrame = 0;

					/* The sequence has finished and we are at the start */

					mcptr->ObMorphFlags |= (mph_flag_finished | mph_flag_start);

				}

				else {

					mcptr->ObMorphCurrFrame += mhdr->mph_maxframes;

					/* The sequence has looped and we are back at the end */

					mcptr->ObMorphFlags |= (mph_flag_looped | mph_flag_end);

				}

			}

		}

		else {

			mcptr->ObMorphCurrFrame += UpdateRate;

			if(mcptr->ObMorphCurrFrame >= mhdr->mph_maxframes) {

				if(mcptr->ObMorphFlags & mph_flag_noloop) {

					/* The sequence has finished and we are at the end */

					mcptr->ObMorphFlags |= (mph_flag_finished | mph_flag_end);

					mcptr->ObMorphCurrFrame = mhdr->mph_maxframes - 1;

				}

				else {

					mcptr->ObMorphCurrFrame -= mhdr->mph_maxframes;

					/* The sequence has looped and we are back at the start */

					mcptr->ObMorphFlags |= (mph_flag_looped | mph_flag_start);

				}

			}

		}

	}

}



/*

 Update the Morphing Animation for this object

*/

void UpdateMorphingDptr(DISPLAYBLOCK *dptr)
{
	SHAPEHEADER *sptr1;
	SHAPEHEADER *sptr2;

	/* Update object radius and extents */

	GetMorphDisplay(&MorphDisplay, dptr);

	sptr1 = MorphDisplay.md_sptr1;
	sptr2 = MorphDisplay.md_sptr2;

	#if 0
	textprint("sptr1->shaperadius = %d\n", sptr1->shaperadius);
	textprint("sptr2->shaperadius = %d\n", sptr2->shaperadius);
	#endif


	/* Radius */

	if(sptr1->shaperadius == sptr2->shaperadius) {

		dptr->ObRadius = sptr1->shaperadius;

	}

	else {

		dptr->ObRadius = WideMul2NarrowDiv(sptr1->shaperadius,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shaperadius,
													MorphDisplay.md_lerp, ONE_FIXED);

	}


	/* X Extent */

	if(sptr1->shapemaxx == sptr2->shapemaxx) {

		dptr->ObMaxX = sptr1->shapemaxx;

	}

	else {

		dptr->ObMaxX = WideMul2NarrowDiv(sptr1->shapemaxx,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapemaxx,
													MorphDisplay.md_lerp, ONE_FIXED);

	}

	if(sptr1->shapeminx == sptr2->shapeminx) {

		dptr->ObMinX = sptr1->shapeminx;

	}

	else {

		dptr->ObMinX = WideMul2NarrowDiv(sptr1->shapeminx,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapeminx,
													MorphDisplay.md_lerp, ONE_FIXED);

	}


	/* Y Extent */

	if(sptr1->shapemaxy == sptr2->shapemaxy) {

		dptr->ObMaxY = sptr1->shapemaxy;

	}

	else {

		dptr->ObMaxY = WideMul2NarrowDiv(sptr1->shapemaxy,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapemaxy,
													MorphDisplay.md_lerp, ONE_FIXED);

	}

	if(sptr1->shapeminy == sptr2->shapeminy) {

		dptr->ObMinY = sptr1->shapeminy;

	}

	else {

		dptr->ObMinY = WideMul2NarrowDiv(sptr1->shapeminy,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapeminy,
													MorphDisplay.md_lerp, ONE_FIXED);

	}


	/* Z Extent */

 	if(sptr1->shapemaxz == sptr2->shapemaxz) {

		dptr->ObMaxZ = sptr1->shapemaxz;

	}

	else {

		dptr->ObMaxZ = WideMul2NarrowDiv(sptr1->shapemaxz,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapemaxz,
													MorphDisplay.md_lerp, ONE_FIXED);

	}

	if(sptr1->shapeminz == sptr2->shapeminz) {

		dptr->ObMinZ = sptr1->shapeminz;

	}

	else {

		dptr->ObMinZ = WideMul2NarrowDiv(sptr1->shapeminz,
													MorphDisplay.md_one_minus_lerp,
													sptr2->shapeminz,
													MorphDisplay.md_lerp, ONE_FIXED);

	}

	#if 0
	textprint("dptr->ObRadius = %d\n", dptr->ObRadius);
	#endif

}



/*

 Using the current frame, calculate the lerp values and find out which two
 shapes to interpolate between.

 Write this information back to a MORPHDISPLAY structure.

*/

void GetMorphDisplay(MORPHDISPLAY *md, DISPLAYBLOCK *dptr)
{
	MORPHFRAME *mdata;
	MORPHCTRL *mc = dptr->ObMorphCtrl;
	MORPHHEADER *mhdr = mc->ObMorphHeader;


	md->md_lerp = mc->ObMorphCurrFrame & 0xffff;
	md->md_one_minus_lerp = ONE_FIXED - md->md_lerp;

	mdata = mhdr->mph_frames;
	mdata = &mdata[mc->ObMorphCurrFrame >> 16];

	md->md_shape1 = mdata->mf_shape1;
	md->md_shape2 = mdata->mf_shape2;

	md->md_sptr1 = GetShapeData(md->md_shape1);
	md->md_sptr2 = GetShapeData(md->md_shape2);
}


void CopyMorphCtrl(MORPHCTRL *src, MORPHCTRL *dst)
{
	dst->ObMorphCurrFrame = src->ObMorphCurrFrame;
	dst->ObMorphFlags     = src->ObMorphFlags;
	dst->ObMorphSpeed     = src->ObMorphSpeed;
	dst->ObMorphHeader    = src->ObMorphHeader;
}
