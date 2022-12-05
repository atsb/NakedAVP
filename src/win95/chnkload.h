#ifndef _chnkload_h_
#define _chnkload_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "system.h"
#include "equates.h"
#include "platform.h"
#include "shape.h"
#include "prototyp.h"
#include "module.h"

extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

#define GLS_NOTINLIST (-1)

///////////////////
// RIF Loading, etc
///////////////////

typedef struct _RifHandle * RIFFHANDLE;
#define INVALID_RIFFHANDLE 0

// flags - project specific ones start at lsb
// - generic ones to start from msb

#define CCF_NOMORPH 0x80000000

typedef enum UVCoordType
{
	UVC_SPRITE_U,
	UVC_SPRITE_V,
	UVC_POLY_U,
	UVC_POLY_V,
	
} UVCOORDTYPE;

// Note: for aesthetic reasons, macros enable one to have all one's fuctions in lower case or captialized, to suit one's style!

// For clarity, functions which are to be defined in project specific files
// are here declared with extern

/////////////////////////////////////////
// Functions which operate on RIFFHANDLEs
/////////////////////////////////////////

// load a rif file into memory
RIFFHANDLE load_rif (const char * fname);
RIFFHANDLE load_rif_non_env (const char * fname);
#define LoadRIF(s) load_rif(s)

// deallocate the shapes, unload the rif, close the handle
void undo_rif_load (RIFFHANDLE);
#define UndoRIFLoad(h) undo_rif_load(h)

// deallocate the shapes copied from the rif
void deallocate_loaded_shapes (RIFFHANDLE);
#define DeallocateLoadedShapes(h) deallocate_loaded_shapes(h)

// unloads the rif but keeps the handle and associated copied shapes
void unload_rif (RIFFHANDLE);
#define UnloadRIF(h) unload_rif(h);

// close the handle - performs tidying up and memory deallocation
void close_rif_handle (RIFFHANDLE);
#define CloseRIFHandle(h) close_rif_handle(h)

// load textures for environment
BOOL load_rif_bitmaps (RIFFHANDLE, int flags);
#define LoadRIFBitmaps(h,f) load_rif_bitmaps(h,f)

// set the quantization event depending on CL_RIFFImage::game_mode
BOOL set_quantization_event(RIFFHANDLE, int flags);
#define SetQuantizationEvent(h,f) set_quantization_event(h,f)

// copy palette
BOOL copy_rif_palette (RIFFHANDLE, int flags);
#define CopyRIFPalette(h,f) copy_rif_palette(h,f)

// copy texture lighting table
BOOL copy_rif_tlt (RIFFHANDLE, int flags);
#define CopyRIFTLT(h,f) copy_rif_tlt(h,f)

// copy palette remap table (15-bit) - post_process_shape may use it
BOOL get_rif_palette_remap_table (RIFFHANDLE, int flags);
#define GetRIFPaletteRemapTable(h,f) get_rif_palette_remap_table(h,f)

// copy one named shape or sprite; does not put in main shape list, needs deallocating
SHAPEHEADER * CopyNamedShapePtr (RIFFHANDLE, char const * shapename);
#define copy_named_shape_ptr(h,s) CopyNamedShapePtr(h,s)

// copy one named shape or sprite; put it in the main shape list
int CopyNamedShapeMSL (RIFFHANDLE, char const * shapename);
#define copy_named_shape_msl(h,s) CopyNamedShapeMSL(h,s)

////////////////////////////////////////////////////////////////////////
// Functions which do not operate on RIFFHANDLEs and may become obsolete
////////////////////////////////////////////////////////////////////////

// these functions work on the current rif; they only remain for historical reasons
extern RIFFHANDLE current_rif_handle;
// returns NULL on fail; does not put it in the mainshapelist
SHAPEHEADER * CopyNamedShape (char const * shapename);

/////////////////////////////////////////////
// Functions for handling the main shape list
/////////////////////////////////////////////

// reserves the next avaialbe position in the main shape list and returns it
extern int GetMSLPos(void);
#define get_msl_pos() GetMSLPos()

// frees a position in the main shape list
extern void FreeMSLPos(int);
#define free_msl_pos(i) FreeMSLPos(i)

////////////////////////////////////////////////
// Functions retrieving data about loaded shapes
////////////////////////////////////////////////

// gets the main shape list position of a shape loaded into the msl
int GetLoadedShapeMSL(char const * shapename);
#define get_loaded_shape_msl(s) GetLoadedShapeMSL(s)
// ditto, but returns a pointer; the shape need not be in the msl
SHAPEHEADER * GetLoadedShapePtr(char const * shapename);
#define get_loaded_shape_ptr(s) GetLoadedShapePtr(s)

// gets name of shape from msl pos
char const * GetMSLLoadedShapeName(int listpos);
#define get_msl_loaded_shape_name(i) GetMSLLoadedShapeName(i)
// gets name of shape from pointer; the shape need not be in msl
char const * GetPtrLoadedShapeName(SHAPEHEADER *);
#define get_ptr_loaded_shape_name(p) GetPtrLoadedShapeName(p)

// free a reference to a named shape if it exists - not necessary since these are all tidied up
void FreeShapeNameReference(SHAPEHEADER * shptr);
#define free_shape_name_reference(p) FreeShapeNameReference(p)

//////////////////////////////////////////////////////////////////////////////
// Initializing, deallocating of shapes, mainly hooks for project specific fns
//////////////////////////////////////////////////////////////////////////////

// perform initial post processing on shape just after loading
// note that the copy named shape functions will not call this
extern void post_process_shape (SHAPEHEADER *);
#define PostProcessShape(p) post_process_shape(p)

// hook to perhaps scale the uv coordinates - should return new value
extern int ProcessUVCoord(RIFFHANDLE,UVCOORDTYPE,int uv_value,int image_num);
#define process_uv_coord(h,t,u,i) ProcessUVCoord(h,t,u,i)

// delete a shape by the shapeheader
void DeallocateLoadedShapePtr(SHAPEHEADER *);
#define deallocate_loaded_shape_ptr(h,p) DeallocateLoadedShapePtr(h,p)

// delete a shape by the shape list number
void DeallocateLoadedShapeMSL(RIFFHANDLE, int);
#define deallocate_loaded_shape_msl(h,i) DeallocateLoadedShapeMSL(h,i)

// your function could perform any extra tidying up you need
extern void DeallocateLoadedShapeheader(SHAPEHEADER *);
#define deallocate_loaded_shapeheader(p) DeallocateLoadedShapeHeader(p)

// your function should call this function which undoes the allocation done when copied from rif data
void DeallocateRifLoadedShapeheader(SHAPEHEADER *);
#define deallocate_rif_loaded_shapeheader(p) DeallocateRifLoadedShapeHeader(p)

///////
// Misc
///////

// return TRUE if the poly item type corresponds to a textured polygon
BOOL is_textured(int);
#define IsTextured(i) is_textured(i)

/////////////////////
// Rif loader globals
/////////////////////

extern unsigned char const * PaletteMapTable;

/////////////////
// Engine globals
/////////////////

extern int start_of_loaded_shapes;

extern unsigned char *TextureLightingTable;

extern SHAPEHEADER ** mainshapelist;

extern MAPHEADER Map[];

extern MAPBLOCK8 Player_and_Camera_Type8[];
extern MAPBLOCK6 Empty_Landscape_Type6;
extern MAPBLOCK6 Empty_Object_Type6;
extern MAPBLOCK6 Term_Type6;

extern unsigned char TestPalette[];

extern unsigned char LPTestPalette[]; /* to cast to lp*/

#if 0
extern int NumImages;								/* # current images */
extern IMAGEHEADER *ImageHeaderPtrs[];	/* Ptrs to Image Header Blocks */
extern IMAGEHEADER ImageHeaderArray[];	/* Array of Image Headers */
extern IMAGEHEADER *NextFreeImageHeaderPtr;
#endif

#if SupportModules

extern SCENEMODULE MainScene;
extern MODULE Empty_Module;
extern MODULE Term_Module;

extern MODULEMAPBLOCK Empty_Module_Map;

#endif

#ifdef __cplusplus

}

#endif

#endif
