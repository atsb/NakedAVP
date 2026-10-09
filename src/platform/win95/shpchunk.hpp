#ifndef _shpchunk_hpp
#define _shpchunk_hpp 1

#include "chunk.hpp"

#include "chnktype.hpp"
#include "mishchnk.hpp"

// shape flags

#define SHAPE_FLAG_PALETTISED		0x0000100
#define SHAPE_FLAG_USEZSP		0x0000200
#define SHAPE_FLAG_USEAUGZS		0x0000400
#define SHAPE_FLAG_USEAUGZSL		0x0000800
#define SHAPE_FLAG_EXTERNALFILE		0x0001000
#define SHAPE_FLAG_RECENTRED		0x0002000


#define SHAPE_FLAG_UNSTABLEBOUND_ZPOS 0x00004000
#define SHAPE_FLAG_UNSTABLEBOUND_ZNEG 0x00008000
#define SHAPE_FLAG_UNSTABLEBOUND_YPOS 0x00010000
#define SHAPE_FLAG_UNSTABLEBOUND_YNEG 0x00020000
#define SHAPE_FLAG_UNSTABLEBOUND_XPOS 0x00040000
#define SHAPE_FLAG_UNSTABLEBOUND_XNEG 0x00080000


//flags that need to be removed before being copied into the shapeheaders
#define ChunkInternalItemFlags 0x00000000

class Object_Chunk;
class Shape_Header_Chunk;
class Anim_Shape_Frame_Chunk;
class Console_Shape_Chunk;
// flags structure


enum Console_Type
{
	CT_PSX=0,
	CT_Saturn=1,
	
	// IMPORTANT
	// since enums are not guaranteed to assume any particular
	// storage class, code compiled on different compilers or
	// with different settings may result in enums to be written
	// to the data block as a char and read back in as an int,
	// with the three most significant bytes containing junk.
	// THIS MASK MUST BE KEPT UP TO DATE AS THE ENUM IS EXTENDED;
	// ALSO ENSURE THAT NEW FILES LOADED INTO OLD SOFTWARE WILL
	// NOT HAVE THEIR ENUM VALUE OVER-MASKED; THE MASK IS ONLY
	// HERE TO ATTEMPT TO REMOVE PROBLEMS FROM FILES MADE
	// PRIOR TO ITS INTRODUCTION
	CT_MASK=0xff
};

// The shape chunk contains and handles all the interface data for the
// child chunks that it recognises in ChunkShape

/*--------------------------------------------------------------------**
** N.B. all changes to shape formats should be made to the sub shapes **
**--------------------------------------------------------------------*/

class Shape_Chunk : public Lockable_Chunk_With_Children
{
public:

	// constructor from buffer
	Shape_Chunk (Chunk_With_Children * parent, const char *, size_t);

	// constructor from data (public so that other shape tools can call)
	Shape_Chunk (Chunk_With_Children * parent, ChunkShape &shp_dat);

	// I want the external file load to be as transparent as possible
	// i.e. we have a shape which is loaded as usaul - it should then
	// load the shape which is its main copy and copy itself over !!!
	
	// Problems will be encountered with the texturing data though - 
	// but this stuff should onyl be worked with with a hardware
	// accelerator - so that should solve many of the problems
	
	// destructor
	virtual ~Shape_Chunk();

	const ChunkShape shape_data;

	Shape_Header_Chunk * get_header();

	Shape_Chunk* make_copy_of_chunk();

	List<Object_Chunk *> const & list_assoc_objs();

	BOOL assoc_with_object(Object_Chunk *);

	BOOL deassoc_with_object(Object_Chunk *);

	// destroy all auxiliary chunks
	// that is all except the header and the chunkshape
	void destroy_auxiliary_chunks();
	
	// functions for the locking functionality

	BOOL file_equals (HANDLE &);
	const char * get_head_id();
	void set_lock_user(char *);

	virtual void post_input_processing();

	Console_Shape_Chunk* get_console_shape_data(Console_Type);
	
	BOOL inc_v_no();

	BOOL same_and_updated(Shape_Chunk &);
	
	BOOL assoc_with_object_list(File_Chunk *);
	
	BOOL update_my_chunkshape (ChunkShape & cshp);

	// THIS IS A HACK, DO NOT USE AFTER AVP AUGUST DEMO
	
	BOOL has_door;
	
private:

	friend class Object_Chunk;
	friend class Shape_Vertex_Chunk;
	friend class Shape_Vertex_Normal_Chunk;
	friend class Shape_Polygon_Normal_Chunk;
	friend class Shape_Polygon_Chunk;
	friend class Shape_Texture_Filenames_Chunk;
	friend class Shape_UV_Coord_Chunk;
	friend class Shape_Header_Chunk;
	friend class Shape_External_File_Chunk;
	friend class RIF_File_Chunk;
	friend class File_Chunk;
	friend class Shape_Centre_Chunk;

	ChunkShape * shape_data_store;

	static int max_id; // for id checking

};

///////////////////////////////////////////////

class Shape_Sub_Shape_Header_Chunk;

class Shape_Sub_Shape_Chunk : public Chunk_With_Children
{
public:

	// constructor from buffer
	Shape_Sub_Shape_Chunk (Chunk_With_Children * parent, const char *, size_t);

	// constructor from data (public so that other shape tools can call)
	Shape_Sub_Shape_Chunk (Chunk_With_Children * parent, ChunkShape &shp_dat);

	// destructor
	virtual ~Shape_Sub_Shape_Chunk();

	const ChunkShape shape_data;

	Shape_Sub_Shape_Header_Chunk * get_header();

	Shape_Sub_Shape_Chunk* make_copy_of_chunk();

	BOOL update_my_chunkshape (ChunkShape & cshp);

	const char * get_shape_name();

	Console_Shape_Chunk* get_console_shape_data(Console_Type);
	
	// Number stored for list_pos when loading
	
	int list_pos_number;
	
private:

	friend class Shape_Vertex_Chunk;
	friend class Shape_Vertex_Normal_Chunk;
	friend class Shape_Polygon_Normal_Chunk;
	friend class Shape_Polygon_Chunk;
	friend class Shape_Texture_Filenames_Chunk;
	friend class Shape_UV_Coord_Chunk;
	friend class Shape_Sub_Shape_Header_Chunk;
	friend class Shape_Centre_Chunk;

	ChunkShape * shape_data_store;

};

///////////////////////////////////////////////

class Shape_Sub_Shape_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Shape_Sub_Shape_Header_Chunk (Shape_Sub_Shape_Chunk * parent, const char * pdata, size_t psize);
	
	~Shape_Sub_Shape_Header_Chunk();

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	const ChunkShape * const shape_data;

	int file_id_num;

	int flags;

private:

	friend class Shape_Sub_Shape_Chunk;
	friend class Shape_Morphing_Frame_Data_Chunk;



	// constructor from data
	Shape_Sub_Shape_Header_Chunk (Shape_Sub_Shape_Chunk * parent)
	: Chunk (parent, "SUBSHPHD"),
	shape_data(parent->shape_data_store),
	file_id_num(-1), flags(0)
	{}

};


///////////////////////////////////////////////
#define AnimCentreFlag_CentreSetByUser		0x00000001  //centre should not be recalculated from the extents of the sequences

class Anim_Shape_Centre_Chunk : public Chunk
{
	public:
	Anim_Shape_Centre_Chunk(Chunk_With_Children* const parent,const char*,size_t const);
	Anim_Shape_Centre_Chunk(Chunk_With_Children* const parent);

	ChunkVectorInt Centre;
	int flags;
	int pad2;

	virtual void fill_data_block(char* data_start);
	virtual size_t size_chunk();
};
///////////////////////////////////////////////
class Anim_Shape_Sequence_Chunk :public Chunk_With_Children
{
	public:
//if the first constructor is used then the vertex normals and extents won't be calculated
//unless the post_input_processing function is called
	Anim_Shape_Sequence_Chunk(Chunk_With_Children* const parent,const char*,size_t const);
	Anim_Shape_Sequence_Chunk(Chunk_With_Children* const parent,int sequencenum,const char* name);
	Anim_Shape_Sequence_Chunk(Chunk_With_Children* const parent,ChunkAnimSequence const* cas);
	
	virtual void post_input_processing();

	const ChunkAnimSequence sequence_data;
	
	void update_my_sequencedata (ChunkAnimSequence &);

	void set_sequence_flags(int flags);
	void set_frame_flags(int frameno,int flags);

	void GenerateInterpolatedFrames();

	private :
	ChunkAnimSequence *sequence_data_store;

	int ConstructSequenceDataFromChildren();//copies everyting into the sequence_data
	void RegenerateChildChunks();
};

///////////////////////////////////////////////
class Anim_Shape_Sequence_Data_Chunk:public Chunk
{
	public:
	Anim_Shape_Sequence_Data_Chunk(Anim_Shape_Sequence_Chunk* const parent,const char*,size_t const);
	Anim_Shape_Sequence_Data_Chunk(Anim_Shape_Sequence_Chunk* const parent,ChunkAnimSequence* cas);

	private:
	friend class Anim_Shape_Sequence_Chunk;

	int SequenceNum;
	int flags,pad2,pad3,pad4;
	char* name;

	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);
};

///////////////////////////////////////////////
class Anim_Shape_Frame_Chunk : public Chunk_With_Children
{
	public:
	Anim_Shape_Frame_Chunk(Anim_Shape_Sequence_Chunk* const parent,const char*,size_t const);
	Anim_Shape_Frame_Chunk(Anim_Shape_Sequence_Chunk * const parent,ChunkAnimFrame* caf,int frameno);
};

///////////////////////////////////////////////
class Anim_Shape_Frame_Data_Chunk : public Chunk
{
	public:
	Anim_Shape_Frame_Data_Chunk(Anim_Shape_Frame_Chunk* const parent,const char*,size_t const);
	Anim_Shape_Frame_Data_Chunk(Anim_Shape_Frame_Chunk* const parent,ChunkAnimFrame* caf,int frameno);
	
	private:
	friend class Anim_Shape_Frame_Chunk;
	friend class Anim_Shape_Sequence_Chunk;
	
	int FrameNum;
	int flags,num_interp_frames,pad3,pad4;
	char* name;//name of asc file for frame

	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);
};
///////////////////////////////////////////////
class Anim_Shape_Alternate_Texturing_Chunk : public Chunk_With_Children
{
	public :
	Anim_Shape_Alternate_Texturing_Chunk(Chunk_With_Children* const parent,const char*,size_t const);
	Anim_Shape_Alternate_Texturing_Chunk(Chunk_With_Children* const parent)
	:Chunk_With_Children(parent,"ASALTTEX")
	{}
	
	Shape_Sub_Shape_Chunk* CreateNewSubShape(const char* name);
};
///////////////////////////////////////////////
class Poly_Not_In_Bounding_Box_Chunk : public Chunk
{
	public :
	Poly_Not_In_Bounding_Box_Chunk(Chunk_With_Children* const parent,const char*,size_t const);
	Poly_Not_In_Bounding_Box_Chunk(Chunk_With_Children* const parent);

	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);
	
	
	List<int> poly_no;
	int pad1,pad2;
};
///////////////////////////////////////////////

class Console_Type_Chunk: public Chunk
{
	public :
	Console_Type_Chunk(Chunk_With_Children* const parent,const char* data,size_t)
	:Chunk(parent,"CONSTYPE"),console((Console_Type)(*(int*)data & CT_MASK))
	{}
	Console_Type_Chunk(Chunk_With_Children* const parent,Console_Type ct)
	:Chunk(parent,"CONSTYPE"),console(ct)
	{}	
	
	Console_Type console;
	
	virtual size_t size_chunk()
	{return chunk_size=16;}
	
	virtual void fill_data_block(char * data_start);
};
///////////////////////////////////////////////
class Console_Shape_Chunk : public Chunk_With_Children
{
	public :
	Console_Shape_Chunk(Chunk_With_Children* const  parent,const char*,size_t);
	Console_Shape_Chunk(Chunk_With_Children* const  parent,Console_Type ct);
	
	const ChunkShape shape_data;

	void generate_console_chunkshape();//needs parent's chunkshape to be set up first

	void update_my_chunkshape(ChunkShape&);
	private:
	friend class Shape_Polygon_Chunk;
	friend class Shape_UV_Coord_Chunk;
	ChunkShape * shape_data_store;
};
///////////////////////////////////////////////

class Shape_Vertex_Chunk : public Chunk
{
public:


	virtual size_t size_chunk ()
	{
		return (chunk_size = (12 + (num_verts*sizeof(ChunkVectorInt))));
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkVectorInt * const vert_data;

	const int num_verts;

	// constructor from buffer
	Shape_Vertex_Chunk (Shape_Sub_Shape_Chunk * parent, const char * vtdata, size_t vtsize);
	Shape_Vertex_Chunk (Shape_Chunk * parent, const char * vtdata, size_t vtsize);
	Shape_Vertex_Chunk (Anim_Shape_Frame_Chunk * parent, const char * vtdata, size_t vtsize);
private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	friend class Anim_Shape_Frame_Chunk;

	
	// constructor from data
	Shape_Vertex_Chunk (Shape_Sub_Shape_Chunk * parent, int n_verts)
	: Chunk (parent, "SHPRAWVT"),
	vert_data (parent->shape_data_store->v_list), num_verts (n_verts)
	{}

	Shape_Vertex_Chunk (Shape_Chunk * parent, int n_verts)
	: Chunk (parent, "SHPRAWVT"),
	vert_data (parent->shape_data_store->v_list), num_verts (n_verts)
	{}
	
	Shape_Vertex_Chunk (Anim_Shape_Frame_Chunk * parent,ChunkVectorInt* v_list ,int n_verts)
	: Chunk (parent, "SHPRAWVT"),
	vert_data (v_list), num_verts (n_verts)
	{}

};


///////////////////////////////////////////////

class Shape_Vertex_Normal_Chunk: public Chunk
{
public:
	// constructor from buffer
	Shape_Vertex_Normal_Chunk (Shape_Sub_Shape_Chunk * parent, const char * vtdata, size_t vtsize);
	Shape_Vertex_Normal_Chunk (Shape_Chunk * parent, const char * vtdata, size_t vtsize);

	virtual size_t size_chunk ()
	{
		return (chunk_size = (12 + (num_verts*sizeof(ChunkVectorFloat))));
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkVectorFloat * const vert_norm_data;

	const int num_verts;

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;


	// constructor from data
	Shape_Vertex_Normal_Chunk (Shape_Sub_Shape_Chunk * parent, int n_verts)
	: Chunk (parent, "SHPVNORM"),
	vert_norm_data (parent->shape_data_store->v_normal_list), num_verts (n_verts)
	{}

	Shape_Vertex_Normal_Chunk (Shape_Chunk * parent, int n_verts)
	: Chunk (parent, "SHPVNORM"),
	vert_norm_data (parent->shape_data_store->v_normal_list), num_verts (n_verts)
	{}

};


///////////////////////////////////////////////

class Shape_Polygon_Normal_Chunk: public Chunk
{
public:
	// constructor from buffer
	Shape_Polygon_Normal_Chunk (Shape_Sub_Shape_Chunk * parent, const char * pndata, size_t pnsize);
	Shape_Polygon_Normal_Chunk (Shape_Chunk * parent, const char * pndata, size_t pnsize);
	Shape_Polygon_Normal_Chunk (Anim_Shape_Frame_Chunk * parent, const char * pndata, size_t pnsize);

	virtual size_t size_chunk ()
	{
		return (chunk_size = (12 + (num_polys*sizeof(ChunkVectorFloat))));
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkVectorFloat * const poly_norm_data;

	const int num_polys;

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	friend class Anim_Shape_Frame_Chunk;



	// constructor from data
	Shape_Polygon_Normal_Chunk (Shape_Sub_Shape_Chunk * parent, int n_polys)
	: Chunk (parent, "SHPPNORM"),
	poly_norm_data (parent->shape_data_store->p_normal_list), num_polys (n_polys)
	{}

	Shape_Polygon_Normal_Chunk (Shape_Chunk * parent, int n_polys)
	: Chunk (parent, "SHPPNORM"),
	poly_norm_data (parent->shape_data_store->p_normal_list), num_polys (n_polys)
	{}

	Shape_Polygon_Normal_Chunk (Anim_Shape_Frame_Chunk * parent,ChunkVectorFloat* p_normal_list ,int n_polys)
	: Chunk (parent, "SHPPNORM"),
	poly_norm_data (p_normal_list), num_polys (n_polys)
	{}

};


///////////////////////////////////////////////

class Shape_Polygon_Chunk : public Chunk
{
public:
	// constructor from buffer
	Shape_Polygon_Chunk (Shape_Sub_Shape_Chunk * parent, const char * pdata, size_t psize);
	Shape_Polygon_Chunk (Shape_Chunk * parent, const char * pdata, size_t psize);
	Shape_Polygon_Chunk (Console_Shape_Chunk * parent, const char * pdata, size_t psize);
	
	virtual size_t size_chunk ()
	{
		return (chunk_size = (12 + (num_polys*36)));// 36 bytes per vertex
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkPoly * const poly_data;

	const int num_polys;

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	friend class Console_Shape_Chunk;



	// constructor from data
	Shape_Polygon_Chunk (Shape_Sub_Shape_Chunk * parent, int n_polys)
	: Chunk (parent, "SHPPOLYS"),
	poly_data (parent->shape_data_store->poly_list), num_polys (n_polys)
	{}

	Shape_Polygon_Chunk (Shape_Chunk * parent, int n_polys)
	: Chunk (parent, "SHPPOLYS"),
	poly_data (parent->shape_data_store->poly_list), num_polys (n_polys)
	{}
	
	Shape_Polygon_Chunk (Console_Shape_Chunk * parent, int n_polys)
	: Chunk (parent, "SHPPOLYS"),
	poly_data (parent->shape_data_store->poly_list), num_polys (n_polys)
	{}

};

///////////////////////////////////////////////

class Shape_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Shape_Header_Chunk (Shape_Chunk * parent, const char * pdata, size_t psize);
	
	~Shape_Header_Chunk();

	virtual size_t size_chunk ();

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkShape * const shape_data;

	virtual void prepare_for_output();

	char lock_user[17];

	List<char *> object_names_store;

	int flags;

private:

	friend class Shape_Chunk;
	friend class Object_Chunk;
	friend class RIF_File_Chunk;
	friend class File_Chunk;
	friend class Environment;
	friend class Shape_Morphing_Frame_Data_Chunk;
	friend class Shape_External_File_Chunk;

	int version_no;

	int file_id_num;

	List<Object_Chunk *> associated_objects_store;



	// constructor from data
	Shape_Header_Chunk (Shape_Chunk * parent)
	: Chunk (parent, "SHPHEAD1"),
	shape_data(parent->shape_data_store),
	flags(0), version_no(0), file_id_num(-1)
	{}

};
///////////////////////////////////////////////
class Shape_Centre_Chunk : public Chunk
{
public :
	// constructor from buffer
	Shape_Centre_Chunk (Chunk_With_Children * parent, const char * data, size_t datasize);

	virtual size_t size_chunk ()
	{return chunk_size=4*4+12;}

	virtual void fill_data_block (char * data_start);

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	

	// constructor from data
	Shape_Centre_Chunk (Chunk_With_Children * parent)
	: Chunk (parent, "SHPCENTR")
	{}

	//the data for this chunk is stored in its parent's ChunkShape
};
///////////////////////////////////////////////

class Shape_UV_Coord_Chunk : public Chunk
{
public:
	// constructor from buffer
	Shape_UV_Coord_Chunk (Shape_Sub_Shape_Chunk * parent, const char * uvdata, size_t uvsize);
	Shape_UV_Coord_Chunk (Shape_Chunk * parent, const char * uvdata, size_t uvsize);
	Shape_UV_Coord_Chunk (Console_Shape_Chunk * parent, const char * uvdata, size_t uvsize);

	virtual size_t size_chunk ();

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkUV_List * const uv_data;

	const int num_uvs;

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	friend class Console_Shape_Chunk;
	

	// constructor from data
	Shape_UV_Coord_Chunk (Shape_Sub_Shape_Chunk * parent, int n_uvs)
	: Chunk (parent, "SHPUVCRD"),
	uv_data (parent->shape_data_store->uv_list), num_uvs (n_uvs)
	{}

	Shape_UV_Coord_Chunk (Shape_Chunk * parent, int n_uvs)
	: Chunk (parent, "SHPUVCRD"),
	uv_data (parent->shape_data_store->uv_list), num_uvs (n_uvs)
	{}

	Shape_UV_Coord_Chunk (Console_Shape_Chunk * parent, int n_uvs)
	: Chunk (parent, "SHPUVCRD"),
	uv_data (parent->shape_data_store->uv_list), num_uvs (n_uvs)
	{}

};

///////////////////////////////////////////////

class Shape_Texture_Filenames_Chunk : public Chunk
{
public:
	// constructor from buffer
	Shape_Texture_Filenames_Chunk (Shape_Sub_Shape_Chunk * parent, const char * tfndata, size_t tfnsize);
	Shape_Texture_Filenames_Chunk (Shape_Chunk * parent, const char * tfndata, size_t tfnsize);

	virtual size_t size_chunk ();

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	char ** tex_fns;

	const int num_tex_fns;

private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;


	// constructor from data
	Shape_Texture_Filenames_Chunk (Shape_Sub_Shape_Chunk * parent, int n_tfns)
	: Chunk (parent, "SHPTEXFN"),
	tex_fns (parent->shape_data_store->texture_fns), num_tex_fns (n_tfns)
	{}

	Shape_Texture_Filenames_Chunk (Shape_Chunk * parent, int n_tfns)
	: Chunk (parent, "SHPTEXFN"),
	tex_fns (parent->shape_data_store->texture_fns), num_tex_fns (n_tfns)
	{}


};



///////////////////////////////////////////////

class Shape_Merge_Data_Chunk : public Chunk
{
public:

	Shape_Merge_Data_Chunk (Shape_Sub_Shape_Chunk * parent, int *, int);
	Shape_Merge_Data_Chunk (Shape_Chunk * parent, int *, int);
	
	~Shape_Merge_Data_Chunk ();

	int * merge_data;
	int num_polys;
	
	size_t size_chunk()
	{
		return chunk_size = 12 + num_polys*4;
	}
	
	void fill_data_block (char *);

	Shape_Merge_Data_Chunk (Shape_Sub_Shape_Chunk * parent, const char *, size_t);
	Shape_Merge_Data_Chunk (Shape_Chunk * parent, const char *, size_t);

};


///////////////////////////////////////////////

class Shape_External_File_Chunk : public Chunk_With_Children
{
public:

	Shape_External_File_Chunk (Chunk_With_Children * parent, const char * fname);
	Shape_External_File_Chunk (Chunk_With_Children * const parent, const char *, size_t const);
	
	void post_input_processing();
	
	//gets name from shape_external_object_name_chunk if it has one
	//otherwise takes name from rif_name_chunk
	const char * get_shape_name();
private:

	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;

	//Used to avoid updating external shapes in files that have been loaded 
	//by Shape_External_File_Chunk::post_input_processing
	static BOOL UpdatingExternalShape;

};


///////////////////////////////////////////////


class Shape_External_Filename_Chunk : public Chunk
{
public:

	Shape_External_Filename_Chunk (Chunk_With_Children * parent, const char * fname);
	Shape_External_Filename_Chunk (Chunk_With_Children * parent, const char *, size_t);
	~Shape_External_Filename_Chunk ();

	// Here is stored the shape name, rescale value and version number.
	
	char * file_name;

	double rescale;
	
	int version_no;

	size_t size_chunk()
	{
		return chunk_size = 12 + 8 + 4 + strlen(file_name) + (4-strlen(file_name)%4);
	}
	
	void fill_data_block (char *);

	
private:
	
	friend class Shape_External_File_Chunk;
	friend class Sprite_Header_Chunk;


};



///////////////////////////////////////////////

//This is needed if more than one shape is being imported from a rif file
class Shape_External_Object_Name_Chunk : public Chunk
{
public :
	Shape_External_Object_Name_Chunk(Chunk_With_Children * parent, const char * fname);
	Shape_External_Object_Name_Chunk (Chunk_With_Children * parent, const char *, size_t);
	~Shape_External_Object_Name_Chunk();

	int pad;
	char* object_name;
	char* shape_name; //a combination of object and file name
	size_t size_chunk()
	{
		return chunk_size = 12 +4+ strlen(object_name) + (4-strlen(object_name)%4);
	}
	
	void post_input_processing();
	
	void fill_data_block (char *);

private:
	
	friend class Shape_External_File_Chunk;

};

///////////////////////////////////////////////

class Shape_Morphing_Data_Chunk : public Chunk_With_Children
{
public:

	Shape_Morphing_Data_Chunk (Shape_Chunk * parent)
	: Chunk_With_Children (parent, "SHPMORPH"), parent_shape (parent) {}
	
	Shape_Chunk * parent_shape;
	Shape_Sub_Shape_Chunk * parent_sub_shape;
	
	virtual void prepare_for_output();

	Shape_Morphing_Data_Chunk (Shape_Chunk * parent, const char *, size_t);
	Shape_Morphing_Data_Chunk (Shape_Sub_Shape_Chunk * parent, const char *, size_t);
	
};


///////////////////////////////////////////////

class Shape_Morphing_Frame_Data_Chunk : public Chunk
{
public:

	// constructor from wherever
	Shape_Morphing_Frame_Data_Chunk (Shape_Morphing_Data_Chunk * parent)
	: Chunk (parent, "FRMMORPH"), num_frames(0), frame_store(0)
	{}
	
	// constructor from buffer
	Shape_Morphing_Frame_Data_Chunk (Shape_Morphing_Data_Chunk * parent,const char *, size_t);

	~Shape_Morphing_Frame_Data_Chunk();

	int a_flags;
	int a_speed;

	List<a_frame *> anim_frames;	

	virtual void fill_data_block (char *);
	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + 12 + num_frames * 12);
	}

	virtual void prepare_for_output();
	virtual void post_input_processing();
	
private:

	int num_frames;
	int * frame_store;

	friend class Shape_Morphing_Data_Chunk;


};

///////////////////////////////////////////////

class Shape_Poly_Change_Info_Chunk : public Chunk
{
public:
	
	Shape_Poly_Change_Info_Chunk (Shape_Chunk * parent, List<poly_change_info> & pci, int orig_num_v)
	: Chunk (parent, "SHPPCINF"), original_num_verts (orig_num_v), change_list (pci)
	{}

	int original_num_verts;
	List<poly_change_info> change_list;
	
	virtual void fill_data_block (char *);
	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + 4 + 4 + change_list.size() * 12);
	}

	Shape_Poly_Change_Info_Chunk (Chunk_With_Children * parent,const char *, size_t);
};

///////////////////////////////////////////////

class Shape_Name_Chunk : public Chunk
{
public:
	Shape_Name_Chunk (Chunk_With_Children * parent, const char * rname);
	// constructor from buffer
	Shape_Name_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);
	~Shape_Name_Chunk();

	char * shape_name;

	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + strlen (shape_name) + 4 - strlen (shape_name)%4);
	}

	virtual void fill_data_block (char * data_start);
	
private:

	friend class Shape_Sub_Shape_Chunk;

};

///////////////////////////////////////////////

class Shape_Fragments_Chunk : public Chunk_With_Children
{
public:

	Shape_Fragments_Chunk (Chunk_With_Children * parent)
	: Chunk_With_Children (parent, "SHPFRAGS")
	{}

	Shape_Fragments_Chunk (Chunk_With_Children * const parent, char const *, const size_t);

};

///////////////////////////////////////////////
class Shape_Fragment_Type_Chunk : public Chunk
{
public :
	Shape_Fragment_Type_Chunk(Chunk_With_Children* parent,const char* name);
	Shape_Fragment_Type_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	~Shape_Fragment_Type_Chunk();

	size_t size_chunk ();
	void fill_data_block (char * data_start);

	char* frag_type_name;
	int pad1,pad2;

	
	
};
///////////////////////////////////////////////

class Shape_Fragments_Data_Chunk : public Chunk
{
public:

	Shape_Fragments_Data_Chunk (Chunk_With_Children * parent, int num_frags)
	: Chunk (parent, "FRAGDATA"), num_fragments (num_frags),
		pad1(0), pad2(0), pad3(0)
	{}
	Shape_Fragments_Data_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);
	
	int num_fragments;
	
	int pad1, pad2, pad3;
	
	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + 16);
	}
	
	virtual void fill_data_block (char *);
	
private:

	friend class Shape_Sub_Shape_Chunk;
	
	
};
	
///////////////////////////////////////////////

class Shape_Fragment_Location_Chunk : public Chunk
{
public:

	Shape_Fragment_Location_Chunk (Chunk_With_Children * parent, ChunkVectorInt & location)
	: Chunk (parent, "FRAGLOCN"), frag_loc (location),
		pad1(0), pad2(0), pad3(0), pad4(0)
	{}
	Shape_Fragment_Location_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);
	
	ChunkVectorInt frag_loc;
	
	int pad1, pad2, pad3, pad4;
	
	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + 28);
	}
	
	virtual void fill_data_block (char *);
	
private:

	friend class Shape_Sub_Shape_Chunk;
	
	
};

///////////////////////////////////////////////
class Shape_Preprocessed_Data_Chunk : public Chunk
{
public:

	Shape_Preprocessed_Data_Chunk (Chunk_With_Children * parent,int _blocksize,int _first_pointer,unsigned int* _memory_block);
	Shape_Preprocessed_Data_Chunk (Chunk_With_Children * parent, const char * data, size_t ssize);
	~Shape_Preprocessed_Data_Chunk ()
	{
		if(extra_data) delete extra_data;
		if(memory_block) delete memory_block;
	}
	
	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + 12+block_size*4+num_extra_data*4);
	}
	
	virtual void fill_data_block (char *);
	

	int num_extra_data;
	int* extra_data;

	void* GetMemoryBlock();

private:

	int block_size;
	int first_pointer;
	unsigned int* memory_block;
	
	friend class Shape_Chunk;
	friend class Shape_Sub_Shape_Chunk;
	
};
#endif
