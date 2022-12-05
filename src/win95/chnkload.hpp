#ifndef _chnkload_hpp
#define _chnkload_hpp 1

#include "chnkload.h"

#ifdef __cplusplus

#include "chunk.hpp"
#include "shpchunk.hpp"
#include "obchunk.hpp"
#include "bmpnames.hpp"
#include "projload.hpp"

class Sprite_Header_Chunk;

#if 0
extern BOOL copy_to_mainshpl (Shape_Chunk *shape, int list_pos);
extern BOOL copy_to_mainshpl (Shape_Sub_Shape_Chunk *shape, int list_pos);
#endif

extern void copy_to_module (Object_Chunk * ob, int mod_pos, int shplst_pos);

extern BOOL copy_to_shapeheader (
							RIFFHANDLE, 
							ChunkShape const & cshp, 
							SHAPEHEADER *& shphd, 
							Chunk_With_Children * shape, 
							int flags,
							int local_max_index,
							int * local_tex_index_nos,
							int listpos = GLS_NOTINLIST,
							const ChunkObject* object=0	  //object used so that conversion from float to int can be done in world coordinates
							);
extern BOOL copy_preprocessed_to_shapeheader (
							RIFFHANDLE, 
							Shape_Preprocessed_Data_Chunk*, 
							SHAPEHEADER *& shphd, 
							Chunk_With_Children * shape, 
							int flags,
							int local_max_index,
							int * local_tex_index_nos,
							int listpos = GLS_NOTINLIST,
							const ChunkObject* object=0	  //object used so that conversion from float to int can be done in world coordinates
							);
							
extern BOOL copy_sprite_to_shapeheader (RIFFHANDLE, SHAPEHEADER *& shphd,Sprite_Header_Chunk* shc, int listpos = GLS_NOTINLIST);


extern BOOL copy_to_map6(Object_Chunk *,MAPBLOCK6*, int shplst_pos);

extern void merge_polygons_in_chunkshape (ChunkShape & shp, Shape_Merge_Data_Chunk * smdc);

extern File_Chunk * Env_Chunk;

extern double local_scale;

// copies shape to msl
#if SupportMorphing && LOAD_MORPH_SHAPES
typedef struct
{
	int start_list_pos;
	int main_list_pos;
	MORPHCTRL * mc;

} CTM_ReturnType;
#else
typedef int CTM_ReturnType;
#endif

CTM_ReturnType copy_to_mainshapelist(RIFFHANDLE,  Shape_Chunk *, int flags,const ChunkObject* object=0);
#define CopyToMainshapelist(h,r,p,f) copy_to_mainshapelist(h,r,p,f)

// copies sprite to msl
int copy_sprite_to_mainshapelist(RIFFHANDLE, Sprite_Header_Chunk *, int flags);
#define CopySpriteToMainshapelist(h,p,f) copy_sprite_to_mainshapelist(h,p,f)

// hook to load a bitmap - so you can load them from test directories, etc. should return tex index
extern int load_rif_bitmap (char const * fname, BMPN_Flags flags);
#define LoadRIFBitmap(s,f) load_rif_bitmap(s,f)

// project specific shape pre processing - usually merge polys
extern void pre_process_shape (RIFFHANDLE, ChunkShape &, Chunk_With_Children * shape_chunk, int flags);
#define PreProcessShape(h,r,p,f) pre_process_shape(h,r,p,f)


struct _RifHandle : Project_RifHandle
{
	File_Chunk * fc;
	Environment_Data_Chunk * envd;
	Chunk_With_Children * palparent;
	List<int> shape_nums;
	int max_index;
	int * tex_index_nos;

	~_RifHandle()
		{}
	_RifHandle()
		: fc(0)
		, envd(0)
		, palparent(0)
		, max_index(0)
		, tex_index_nos(0)
		{}
};

#endif

#endif
