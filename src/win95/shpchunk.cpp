#include <math.h>
#include "unaligned.h"
#include "chunk.hpp"
#include "chnktype.hpp"
#include "shpchunk.hpp"
#include "obchunk.hpp"


//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(shpchunk)

RIF_IMPLEMENT_DYNCREATE("REBSHAPE",Shape_Chunk)

int Shape_Chunk::max_id = 0;
BOOL Shape_External_File_Chunk::UpdatingExternalShape=FALSE;

// Class Shape_Chunk functions

/*
Children for Shape_Chunk :

"SHPRAWVT"		Shape_Vertex_Chunk
"SHPPOLYS"		Shape_Polygon_Chunk
"SHPHEAD1"		Shape_Header_Chunk
"SHPVNORM"		Shape_Vertex_Normal_Chunk
"SHPPNORM"		Shape_Polygon_Normal_Chunk
"SHPTEXFN"		Shape_Texture_Filenames_Chunk
"SHPUVCRD"		Shape_UV_Coord_Chunk
"SHPMRGDT"		Shape_Merge_Data_Chunk
"SHPCENTR"		Shape_Centre_Chunk
"SHPMORPH"		Shape_Morphing_Data_Chunk
"SHPEXTFL"		Shape_External_File_Chunk
"SHPPCINF"		Shape_Poly_Change_Info_Chunk
"TEXTANIM"		Animation_Chunk
"SHPFRAGS"		Shape_Fragments_Chunk
"ANIMSEQU"		Anim_Shape_Sequence_Chunk
"PNOTINBB"		Poly_Not_In_Bounding_Box_Chunk
"ANSHCEN2"		Anim_Shape_Centre_Chunk
"ASALTTEX"		Anim_Shape_Alternate_Texturing_Chunk
"SHPPRPRO"		Shape_Preprocessed_Data_Chunk	
*/

Shape_Chunk::Shape_Chunk(Chunk_With_Children * parent, const char *data, size_t size)
: Lockable_Chunk_With_Children (parent, "REBSHAPE"), shape_data ()
{
	const char * buffer_ptr = data;

	shape_data_store = (ChunkShape *) &shape_data;
	
	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

/*--------------------------------------------------------------------**
** N.B. all changes to shape formats should be made to the sub shapes **
**--------------------------------------------------------------------*/

/*--------------------------------------------------------------------------**
** And of course the external file post input processing function should be **
** changed so that any new chunks will be copied over                       **
**--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------**
** Oy READ THE NOTES, also destroy_auxiliary_chunks should be changed as well **
**----------------------------------------------------------------------------*/

		DynCreate(data);
		data += *(int *)(data + 8);

/*--------------------------------------------------------------------**
** N.B. all changes to shape formats should be made to the sub shapes **
**--------------------------------------------------------------------*/

/*--------------------------------------------------------------------------**
** And of course the external file post input processing function should be **
** changed so that any new chunks will be copied over                       **
**--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------**
** Oy READ THE NOTES, also destroy_auxiliary_chunks should be changed as well **
**----------------------------------------------------------------------------*/

	}
}

Shape_Chunk::Shape_Chunk (Chunk_With_Children * parent, ChunkShape &shp_dat)
: Lockable_Chunk_With_Children (parent, "REBSHAPE"), shape_data (shp_dat)
{
	shape_data_store = (ChunkShape *) &shape_data;
	
	new Shape_Header_Chunk (this);

	if (shape_data.v_list) new Shape_Vertex_Chunk (this, shape_data.num_verts);
	if (shape_data.v_normal_list) new Shape_Vertex_Normal_Chunk (this, shape_data.num_verts);
	if (shape_data.p_normal_list) new Shape_Polygon_Normal_Chunk (this, shape_data.num_polys);
	if (shape_data.poly_list) new Shape_Polygon_Chunk (this, shape_data.num_polys);
	if (shape_data.uv_list) new Shape_UV_Coord_Chunk (this, shape_data.num_uvs);
	if (shape_data.texture_fns) new Shape_Texture_Filenames_Chunk (this, shape_data.num_texfiles);



	//calculate the shape's centre and radius_about_centre
	shape_data_store->centre=(shape_data_store->min+shape_data_store->max)/2;
	shape_data_store->radius_about_centre=0;
	for(int i=0;i<shape_data_store->num_verts;i++)
	{
		float length = (float) mod(shape_data_store->v_list[i]-shape_data_store->centre);
		if(length>shape_data_store->radius_about_centre)
		{
			shape_data_store->radius_about_centre=length;
		}
	}

	//if the shape hasn't got a Shape_Centre_Chunk , create one.

	if(!lookup_single_child("SHPCENTR"))
	{
		new Shape_Centre_Chunk(this);
	}	
}

Shape_Chunk::~Shape_Chunk ()
{
}

Shape_Chunk* Shape_Chunk::make_copy_of_chunk()
{
	char* Data=this->make_data_block_from_chunk();
	Shape_Chunk* NewShape=new Shape_Chunk(parent,Data+12,this->size_chunk()-12);
	delete [] Data;
	delete NewShape->get_header();
	new Shape_Header_Chunk(NewShape);
	//need to call post_input_processing in order to copy morphing data correctly
	NewShape->post_input_processing();
	NewShape->updated=TRUE;
	return NewShape;
}

Shape_Header_Chunk * Shape_Chunk::get_header()
{
	
	return (Shape_Header_Chunk *) this->lookup_single_child ("SHPHEAD1");

}

List<Object_Chunk *> const & Shape_Chunk::list_assoc_objs()
{
	if (!get_header())
	{
		static List<Object_Chunk *> empty_list;
		return empty_list;
	}

	return get_header()->associated_objects_store;
}

BOOL Shape_Chunk::assoc_with_object (Object_Chunk *obch)
{
	return obch->assoc_with_shape(this);
}

BOOL Shape_Chunk::deassoc_with_object (Object_Chunk *obch)
{
	return obch->deassoc_with_shape(this);
}


void Shape_Chunk::post_input_processing()
{
	if (get_header())
		if (get_header()->flags & GENERAL_FLAG_LOCKED)
			external_lock = TRUE;

#if 0 //shouldn't need to recalculate extents each time shape is loaded
	// recalculate the shape extents	
	
	ChunkVector max, min;
	
	max.x = -1000000000;
	max.y = -1000000000;
	max.z = -1000000000;
	
	min.x = 1000000000;
	min.y = 1000000000;
	min.z = 1000000000;
	
	float radius = 0;
	
	for (int i=0; i<shape_data_store->num_verts; i++)
	{
		max.x = max(max.x, shape_data_store->v_list[i].x);
		max.y = max(max.y, shape_data_store->v_list[i].y);
		max.z = max(max.z, shape_data_store->v_list[i].z);

		min.x = min(min.x, shape_data_store->v_list[i].x);
		min.y = min(min.y, shape_data_store->v_list[i].y);
		min.z = min(min.z, shape_data_store->v_list[i].z);
		
		float temp_rad =(float) mod(shape_data_store->v_list[i]);
		
		radius = max (radius, temp_rad);
	}

	shape_data_store->max = max;
	shape_data_store->min = min;
	shape_data_store->radius = radius;
#endif	


	Chunk_With_Children::post_input_processing();

}

void Shape_Chunk::destroy_auxiliary_chunks()
{
	//split up into different blocks to stop compiler crashing when optimizations are turned on
	
	List<Chunk *> chlst;

	lookup_child("SHPZSPDT",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	lookup_child("SHPMRGDT",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	
	
	lookup_child("SHPMORPH",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	
	lookup_child("TEXTANIM",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	
	
	lookup_child("SHPPCINF",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	
	lookup_child("SHPFRAGS",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	
	lookup_child("ANIMSEQU",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	lookup_child("PNOTINBB",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	lookup_child("ANSHCEN2",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	lookup_child("CONSHAPE",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
	
	lookup_child("ASALTTEX",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	lookup_child("SHPPRPRO",chlst);
	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}
	
}	



BOOL Shape_Chunk::file_equals(HANDLE &rif_file)
{
	unsigned long bytes_read;
	int id;
	Shape_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (FALSE);

	// get header list
	List<int> obhead;
	list_chunks_in_file (&obhead, rif_file, "SHPHEAD1");

	if (obhead.size() != 1) return FALSE;

	// get object identifier
	SetFilePointer(rif_file,obhead.first_entry() + 32,0,FILE_BEGIN);
	ReadFile (rif_file, (long *) &(id), 4, &bytes_read, 0);

	if (hdptr->file_id_num == id) return TRUE;

	return (FALSE);
}	

const char * Shape_Chunk::get_head_id()
{
	Shape_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (0);

	return(hdptr->identifier);
}	

void Shape_Chunk::set_lock_user (char * user)
{
	Shape_Header_Chunk * hdptr = get_header();
	
	if (!hdptr) return;

	strncpy (hdptr->lock_user, user,16);

	hdptr->lock_user[16] = 0;
}	

BOOL Shape_Chunk::inc_v_no ()
{
	Shape_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (FALSE);

	hdptr->version_no++;

	return (TRUE);
}

BOOL Shape_Chunk::same_and_updated(Shape_Chunk & shp)
{
	
	Shape_Header_Chunk * hd1ptr = get_header();

	if (!hd1ptr) return (0);

	Shape_Header_Chunk * hd2ptr = shp.get_header();

	if (!hd2ptr) return (0);

	return (hd1ptr->version_no < hd2ptr->version_no && hd1ptr->file_id_num == hd2ptr->file_id_num);

}

BOOL Shape_Chunk::assoc_with_object_list(File_Chunk *fc)
{
	Shape_Header_Chunk * hdptr = get_header();
	Object_Chunk * ob = NULL;
	
	if (!hdptr) return (FALSE);

	List<Chunk *> chlst;
	fc->lookup_child("RBOBJECT",chlst);
	
	for (LIF<char *> n(&(hdptr->object_names_store)); !n.done(); n.next())
	{
		LIF<Chunk *> l(&chlst);
		
		for (; !l.done(); l.next())
		{
			ob = (Object_Chunk *)l();
			if ( !strcmp(ob->object_data.o_name, n()) )
				break;
		}
		if (!l.done())
			assoc_with_object(ob);
		else
		{
			return(FALSE);
		}	
	}
	return(TRUE);
	
	
}

BOOL Shape_Chunk::update_my_chunkshape (ChunkShape & cshp)
{
	// Firstly lose all the chunks that were with
	// the old chunk shape
	List <Chunk *> chlst;

	lookup_child ("SHPRAWVT",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	lookup_child ("SHPVNORM",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPPNORM",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPPOLYS",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPUVCRD",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPTEXFN",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	*shape_data_store = cshp;

	if (shape_data.v_list) new Shape_Vertex_Chunk (this, shape_data.num_verts);
	if (shape_data.v_normal_list) new Shape_Vertex_Normal_Chunk (this, shape_data.num_verts);
	if (shape_data.p_normal_list) new Shape_Polygon_Normal_Chunk (this, shape_data.num_polys);
	if (shape_data.poly_list) new Shape_Polygon_Chunk (this, shape_data.num_polys);
	if (shape_data.uv_list) new Shape_UV_Coord_Chunk (this, shape_data.num_uvs);
	if (shape_data.texture_fns) new Shape_Texture_Filenames_Chunk (this, shape_data.num_texfiles);

	
	//calculate the shape's centre and radius_about_centre
	shape_data_store->centre=(shape_data_store->min+shape_data_store->max)/2;
	shape_data_store->radius_about_centre=0;
	for(int i=0;i<shape_data_store->num_verts;i++)
	{
		float length = (float) mod(shape_data_store->v_list[i]-shape_data_store->centre);
		if(length>shape_data_store->radius_about_centre)
		{
			shape_data_store->radius_about_centre=length;
		}
	}

	//if the shape hasn't got a Shape_Centre_Chunk , create one.

	if(!lookup_single_child("SHPCENTR"))
	{
		new Shape_Centre_Chunk(this);
	}	
	

	return TRUE;
}

Console_Shape_Chunk* Shape_Chunk::get_console_shape_data(Console_Type ct)
{
	List<Chunk*> chlist;
	lookup_child("CONSHAPE",chlist);
	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Console_Shape_Chunk* csc=(Console_Shape_Chunk*)chlif();
		List<Chunk*> chlist2;
		csc->lookup_child("CONSTYPE",chlist2);
		if(chlist2.size())
		{
			if(((Console_Type_Chunk*)chlist2.first_entry())->console==ct)
				return csc;
		}
	}
	
	return 0;//no console specific shape data
}

/////////////////////////////////////////

// Class Shape_Vertex_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created

// from buffer

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPRAWVT",Shape_Vertex_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPRAWVT",Shape_Vertex_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPRAWVT",Shape_Vertex_Chunk,"ANIMFRAM",Anim_Shape_Frame_Chunk)

Shape_Vertex_Chunk::Shape_Vertex_Chunk(Shape_Chunk * parent, const char * vtdata , size_t vtsize)
: Chunk (parent, "SHPRAWVT"), vert_data (NULL),
	num_verts (vtsize / 12) // 12 bytes per vertex
{
	int i;

	parent->shape_data_store->v_list = new ChunkVectorInt[num_verts];
	parent->shape_data_store->num_verts = num_verts;

	*((ChunkVectorInt**) &vert_data) = parent->shape_data_store->v_list;

	for (i=0;i<num_verts;i++)
	{
		parent->shape_data_store->v_list[i] = *((ChunkVectorInt *) vtdata );
		vtdata+=sizeof(ChunkVectorInt);
	}
	
}


Shape_Vertex_Chunk::Shape_Vertex_Chunk(Shape_Sub_Shape_Chunk * parent, const char * vtdata , size_t vtsize)
: Chunk (parent, "SHPRAWVT"), vert_data (NULL),
	num_verts (vtsize / 12) // 12 bytes per vertex
{
	int i;

	parent->shape_data_store->v_list = new ChunkVectorInt[num_verts];
	parent->shape_data_store->num_verts = num_verts;

	*((ChunkVectorInt**) &vert_data) = parent->shape_data_store->v_list;

	for (i=0;i<num_verts;i++)
	{
		parent->shape_data_store->v_list[i] = *((ChunkVectorInt *) vtdata );
		vtdata+=sizeof(ChunkVectorInt);
	}
	
}


Shape_Vertex_Chunk::Shape_Vertex_Chunk(Anim_Shape_Frame_Chunk * parent, const char * vtdata , size_t vtsize)
: Chunk (parent, "SHPRAWVT"), vert_data (NULL),
	num_verts (vtsize / 12) // 12 bytes per vertex
{
	int i;
	
	ChunkVectorInt* v_list = new ChunkVectorInt[num_verts];
	*(ChunkVectorInt**)&vert_data=v_list;

	for (i=0;i<num_verts;i++)
	{
		v_list[i] = *((ChunkVectorInt *) vtdata );
		vtdata+=sizeof(ChunkVectorInt);
	}
	
}

BOOL Shape_Vertex_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
		
}

void Shape_Vertex_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i=0;i<num_verts;i++)
	{
		*(ChunkVectorInt *) data_start  = vert_data[i];
		data_start+=sizeof(ChunkVectorInt);
		
	}
}

/////////////////////////////////////////

// Class Shape_Vertex_Normal_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPVNORM",Shape_Vertex_Normal_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPVNORM",Shape_Vertex_Normal_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)

// from buffer
Shape_Vertex_Normal_Chunk::Shape_Vertex_Normal_Chunk(Shape_Chunk * parent, const char * vtdata , size_t vtsize)
: Chunk (parent, "SHPVNORM"), vert_norm_data (NULL),
	num_verts (vtsize / sizeof(ChunkVectorInt)) 
{
	int i;

	parent->shape_data_store->v_normal_list = new ChunkVectorFloat[num_verts];
	*((ChunkVectorFloat**) &vert_norm_data) = parent->shape_data_store->v_normal_list;
	
	for (i=0;i<num_verts;i++)
	{
		parent->shape_data_store->v_normal_list[i] = *((ChunkVectorFloat *) vtdata);
		vtdata+=sizeof(ChunkVectorFloat);
	}
	
}

Shape_Vertex_Normal_Chunk::Shape_Vertex_Normal_Chunk(Shape_Sub_Shape_Chunk * parent, const char * vtdata , size_t vtsize)
: Chunk (parent, "SHPVNORM"), vert_norm_data (NULL),
	num_verts (vtsize / sizeof(ChunkVectorInt)) 
{
	int i;

	parent->shape_data_store->v_normal_list = new ChunkVectorFloat[num_verts];
	*((ChunkVectorFloat**) &vert_norm_data) = parent->shape_data_store->v_normal_list;
	
	for (i=0;i<num_verts;i++)
	{
		parent->shape_data_store->v_normal_list[i] = *((ChunkVectorFloat *) vtdata );
		vtdata+=sizeof(ChunkVectorFloat);
	}
	
}

BOOL Shape_Vertex_Normal_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
		
}

void Shape_Vertex_Normal_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i=0;i<num_verts;i++)
	{
		*((ChunkVectorFloat *) data_start ) = vert_norm_data[i];
		data_start+=sizeof(ChunkVectorFloat);
	}

}
/////////////////////////////////////////

// Class Shape_Polygon_Normal_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPNORM",Shape_Polygon_Normal_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPNORM",Shape_Polygon_Normal_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPNORM",Shape_Polygon_Normal_Chunk,"ANIMFRAM",Anim_Shape_Frame_Chunk)

// from buffer
Shape_Polygon_Normal_Chunk::Shape_Polygon_Normal_Chunk(Shape_Chunk * parent, const char * pndata , size_t pnsize)
: Chunk (parent, "SHPPNORM"), poly_norm_data (NULL),
	num_polys (pnsize / sizeof(ChunkVectorFloat)) 
{
	int i;

	parent->shape_data_store->p_normal_list = new ChunkVectorFloat[num_polys];
	*((ChunkVectorFloat**) &poly_norm_data) = parent->shape_data_store->p_normal_list;
	
	for (i=0;i<num_polys;i++)
	{
		parent->shape_data_store->p_normal_list[i] = *((ChunkVectorFloat *) pndata  );
		pndata+=sizeof(ChunkVectorFloat);
	}
	
}

Shape_Polygon_Normal_Chunk::Shape_Polygon_Normal_Chunk(Shape_Sub_Shape_Chunk * parent, const char * pndata , size_t pnsize)
: Chunk (parent, "SHPPNORM"), poly_norm_data (NULL),
	num_polys (pnsize / sizeof(ChunkVectorFloat)) 
{
	int i;

	parent->shape_data_store->p_normal_list = new ChunkVectorFloat[num_polys];
	*((ChunkVectorFloat**) &poly_norm_data) = parent->shape_data_store->p_normal_list;
	
	for (i=0;i<num_polys;i++)
	{
		parent->shape_data_store->p_normal_list[i] = *((ChunkVectorFloat *) pndata  );
		pndata+=sizeof(ChunkVectorFloat);
	}

}

Shape_Polygon_Normal_Chunk::Shape_Polygon_Normal_Chunk(Anim_Shape_Frame_Chunk * parent, const char * pndata , size_t pnsize)
: Chunk (parent, "SHPPNORM"), poly_norm_data (NULL),
	num_polys (pnsize / sizeof(ChunkVectorFloat)) 
{
	int i;
	
	ChunkVectorFloat* p_normal_list = new ChunkVectorFloat[num_polys];
	*((ChunkVectorFloat**) &poly_norm_data) = p_normal_list;
	
	for (i=0;i<num_polys;i++)
	{
		p_normal_list[i] = *((ChunkVectorFloat *) pndata  );
		pndata+=sizeof(ChunkVectorFloat);
	}
	
}

BOOL Shape_Polygon_Normal_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
		
}

void Shape_Polygon_Normal_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i=0;i<num_polys;i++)
	{
		*(ChunkVectorFloat *) data_start  = poly_norm_data[i];
		data_start+=sizeof(ChunkVectorFloat);
	}

}

/////////////////////////////////////////

// Class Shape_Polygon_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPOLYS",Shape_Polygon_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPOLYS",Shape_Polygon_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPPOLYS",Shape_Polygon_Chunk,"CONSHAPE",Console_Shape_Chunk)

// from buffer
Shape_Polygon_Chunk::Shape_Polygon_Chunk (Shape_Chunk * parent, const char * pdata, size_t psize)
: Chunk (parent, "SHPPOLYS"),
	poly_data (parent->shape_data_store->poly_list), num_polys (psize / 36) // 9 * 4 bytes per polygon
{
	int i, j;

	parent->shape_data_store->poly_list = new ChunkPoly [num_polys];
	parent->shape_data_store->num_polys = num_polys;

	*((ChunkPoly **) &poly_data) = parent->shape_data_store->poly_list;

	for (i=0; i<num_polys; i++)
	{
		parent->shape_data_store->poly_list[i].engine_type = *((int *) (pdata + (36*i)));
		parent->shape_data_store->poly_list[i].normal_index = *((int *) (pdata + (36*i) + 4));
		parent->shape_data_store->poly_list[i].flags = *((int *) (pdata + (36*i) + 8));
		parent->shape_data_store->poly_list[i].colour = *((int *) (pdata + (36*i) + 12));

		parent->shape_data_store->poly_list[i].num_verts = 0;

		for (j=0; *((int *) (pdata + (36*i) + 16 + (j*4))) != -1; j++)
		{
			parent->shape_data_store->poly_list[i].vert_ind[j] = *((int *) (pdata + (36*i) + 16 + (j*4)));
			parent->shape_data_store->poly_list[i].num_verts++;
		}

	}

}

Shape_Polygon_Chunk::Shape_Polygon_Chunk (Shape_Sub_Shape_Chunk * parent, const char * pdata, size_t psize)
: Chunk (parent, "SHPPOLYS"),
	poly_data (parent->shape_data_store->poly_list), num_polys (psize / 36) // 9 * 4 bytes per polygon
{
	int i, j;

	parent->shape_data_store->poly_list = new ChunkPoly [num_polys];
	parent->shape_data_store->num_polys = num_polys;

	*((ChunkPoly **) &poly_data) = parent->shape_data_store->poly_list;

	for (i=0; i<num_polys; i++)
	{
		parent->shape_data_store->poly_list[i].engine_type = *((int *) (pdata + (36*i)));
		parent->shape_data_store->poly_list[i].normal_index = *((int *) (pdata + (36*i) + 4));
		parent->shape_data_store->poly_list[i].flags = *((int *) (pdata + (36*i) + 8));
		parent->shape_data_store->poly_list[i].colour = *((int *) (pdata + (36*i) + 12));

		parent->shape_data_store->poly_list[i].num_verts = 0;

		for (j=0; *((int *) (pdata + (36*i) + 16 + (j*4))) != -1; j++)
		{
			parent->shape_data_store->poly_list[i].vert_ind[j] = *((int *) (pdata + (36*i) + 16 + (j*4)));
			parent->shape_data_store->poly_list[i].num_verts++;
		}

	}

}

Shape_Polygon_Chunk::Shape_Polygon_Chunk (Console_Shape_Chunk * parent, const char * pdata, size_t psize)
: Chunk (parent, "SHPPOLYS"),
	poly_data(0), num_polys (psize / 36) // 9 * 4 bytes per polygon
{
	int i, j;

	ChunkPoly* poly_list = new ChunkPoly [num_polys];

	*((ChunkPoly **) &poly_data) =poly_list;

	for (i=0; i<num_polys; i++)
	{
		poly_list[i].engine_type = *((int *) (pdata + (36*i)));
		poly_list[i].normal_index = *((int *) (pdata + (36*i) + 4));
		poly_list[i].flags = *((int *) (pdata + (36*i) + 8));
		poly_list[i].colour = *((int *) (pdata + (36*i) + 12));

		poly_list[i].num_verts = 0;

		for (j=0; *((int *) (pdata + (36*i) + 16 + (j*4))) != -1; j++)
		{
			poly_list[i].vert_ind[j] = *((int *) (pdata + (36*i) + 16 + (j*4)));
			poly_list[i].num_verts++;
		}

	}

}

BOOL Shape_Polygon_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
		
}

void Shape_Polygon_Chunk::fill_data_block(char * data_start)
{
	int i, j;
	
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (i=0;i<num_polys;i++) {
		*((int *) (data_start + i*36)) = poly_data[i].engine_type;
		*((int *) (data_start + i*36 + 4)) = poly_data[i].normal_index;
		*((int *) (data_start + i*36 + 8)) = poly_data[i].flags;
		*((int *) (data_start + i*36 + 12)) = poly_data[i].colour;
		for (j = 0; j<poly_data[i].num_verts; j++)
			*((int *) (data_start + i*36 + 16 + j*4)) = poly_data[i].vert_ind[j];
		for (; j<5; j++)
			*((int *) (data_start + i*36 + 16 + j*4)) = -1;
		
	}

}
/////////////////////////////////////////
//Class Shape_Centre_Chunk :

RIF_IMPLEMENT_DYNCREATE("SHPCENTR",Shape_Centre_Chunk)

Shape_Centre_Chunk::Shape_Centre_Chunk(Chunk_With_Children* parent,const char* data, size_t datasize)
:Chunk (parent,"SHPCENTR")
{
	assert(datasize==16);

	//find parent's chunkshape
	ChunkShape* cs=0;
	if(!strcmp(parent->identifier,"REBSHAPE"))
	{
		cs=((Shape_Chunk*)parent)->shape_data_store;
	}
	else if(!strcmp(parent->identifier,"SUBSHAPE"))
	{
		cs=((Shape_Sub_Shape_Chunk*)parent)->shape_data_store;
	}
	assert(cs);

	//fill in the appropriate entries
	cs->centre=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);
	
	cs->radius_about_centre=*(float*)data;
}

void Shape_Centre_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	//find parent's chunkshape
	ChunkShape* cs=0;
	if(!strcmp(parent->identifier,"REBSHAPE"))
	{
		cs=((Shape_Chunk*)parent)->shape_data_store;
	}
	else if(!strcmp(parent->identifier,"SUBSHAPE"))
	{
		cs=((Shape_Sub_Shape_Chunk*)parent)->shape_data_store;
	}
	assert(cs);


	*(ChunkVectorInt*)data_start=cs->centre;
	data_start+=sizeof(ChunkVectorInt);

	*(float*)data_start=cs->radius;
}


/////////////////////////////////////////

// Class Shape_UV_Coord_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPUVCRD",Shape_UV_Coord_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPUVCRD",Shape_UV_Coord_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPUVCRD",Shape_UV_Coord_Chunk,"CONSHAPE",Console_Shape_Chunk)

// from buffer
Shape_UV_Coord_Chunk::Shape_UV_Coord_Chunk (Shape_Chunk * parent, const char * uvdata, size_t /*uvsize*/)
: Chunk (parent, "SHPUVCRD"),
uv_data (NULL), num_uvs (*((int *) uvdata))
{
	int i,j;

	if (num_uvs)
	{
		parent->shape_data_store->uv_list = new ChunkUV_List[num_uvs];
	}
	else
	{
		parent->shape_data_store->uv_list = 0;
	}
	*((ChunkUV_List**) &uv_data) = parent->shape_data_store->uv_list;

	parent->shape_data_store->num_uvs = num_uvs;
	
	uvdata += 4;

	for (i=0;i<num_uvs;i++)
	{
		parent->shape_data_store->uv_list[i].num_verts = *((int *) uvdata);
		uvdata += 4;

		for (j=0; j<parent->shape_data_store->uv_list[i].num_verts; j++)
		{
			parent->shape_data_store->uv_list[i].vert[j] = *((ChunkUV *)uvdata);
			uvdata += sizeof(ChunkUV);
		}
	}
	
}

Shape_UV_Coord_Chunk::Shape_UV_Coord_Chunk (Shape_Sub_Shape_Chunk * parent, const char * uvdata, size_t /*uvsize*/)
: Chunk (parent, "SHPUVCRD"),
uv_data (NULL), num_uvs (*((int *) uvdata))
{
	int i,j;

	parent->shape_data_store->uv_list = new ChunkUV_List[num_uvs];
	*((ChunkUV_List**) &uv_data) = parent->shape_data_store->uv_list;

	parent->shape_data_store->num_uvs = num_uvs;
	
	uvdata += 4;

	for (i=0;i<num_uvs;i++)
	{
		parent->shape_data_store->uv_list[i].num_verts = *((int *) uvdata);
		uvdata += 4;

		for (j=0; j<parent->shape_data_store->uv_list[i].num_verts; j++) 
		{
			parent->shape_data_store->uv_list[i].vert[j] = *((ChunkUV *)uvdata);
			uvdata += sizeof(ChunkUV);
		}
	}
	
}

Shape_UV_Coord_Chunk::Shape_UV_Coord_Chunk (Console_Shape_Chunk * parent, const char * uvdata, size_t /*uvsize*/)
: Chunk (parent, "SHPUVCRD"),
uv_data (NULL), num_uvs (*((int *) uvdata))
{
	int i,j;

	ChunkUV_List* uv_list= new ChunkUV_List[num_uvs];
	*((ChunkUV_List**) &uv_data) = uv_list;

	uvdata += 4;

	for (i=0;i<num_uvs;i++)
	{
		uv_list[i].num_verts = *((int *) uvdata);
		uvdata += 4;

		for (j=0; j<uv_list[i].num_verts; j++)
		{
			uv_list[i].vert[j] = *((ChunkUV *)uvdata);
			uvdata += sizeof(ChunkUV);
		}
	}
	
}

BOOL Shape_UV_Coord_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
		
}


void Shape_UV_Coord_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = num_uvs;

	data_start += 4;

	for (int i=0;i<num_uvs;i++)
	{
		*((int *)data_start) = uv_data[i].num_verts;
		data_start += 4;
		for (int j = 0; j< uv_data[i].num_verts; j++)
		{
			*((ChunkUV *) data_start) = uv_data[i].vert[j];
			data_start += sizeof(ChunkUV);
		}
	}

}

size_t Shape_UV_Coord_Chunk::size_chunk ()
{
	chunk_size = 12 + 4;
	for (int i=0; i<num_uvs; i++) 
		chunk_size += (4+(sizeof(ChunkUV)*uv_data[i].num_verts));

	return chunk_size;
	
}

/////////////////////////////////////////

// Class Shape_Texture_Filenames_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPTEXFN",Shape_Texture_Filenames_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPTEXFN",Shape_Texture_Filenames_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)
// from buffer
Shape_Texture_Filenames_Chunk::Shape_Texture_Filenames_Chunk (Shape_Chunk * parent, const char * tfndata, size_t /*tfnsize*/)
: Chunk (parent, "SHPTEXFN"),
tex_fns (), num_tex_fns (*((int *) tfndata))
{
	int i;

	parent->shape_data_store->texture_fns = new char * [num_tex_fns];
	*((char***) &tex_fns) = parent->shape_data_store->texture_fns;

	parent->shape_data_store->num_texfiles = num_tex_fns;

	tfndata += 4;

	for (i=0; i<num_tex_fns; i++) {
		parent->shape_data_store->texture_fns[i] = new char [strlen(tfndata)+1];
		strcpy (parent->shape_data_store->texture_fns[i], tfndata);
		tfndata += (strlen(tfndata)+1);
	}
		
}

Shape_Texture_Filenames_Chunk::Shape_Texture_Filenames_Chunk (Shape_Sub_Shape_Chunk * parent, const char * tfndata, size_t /*tfnsize*/)
: Chunk (parent, "SHPTEXFN"),
tex_fns (), num_tex_fns (*((int *) tfndata))
{
	int i;

	parent->shape_data_store->texture_fns = new char * [num_tex_fns];
	*((char***) &tex_fns) = parent->shape_data_store->texture_fns;

	parent->shape_data_store->num_texfiles = num_tex_fns;

	tfndata += 4;

	for (i=0; i<num_tex_fns; i++) {
		parent->shape_data_store->texture_fns[i] = new char [strlen(tfndata)+1];
		strcpy (parent->shape_data_store->texture_fns[i], tfndata);
		tfndata += (strlen(tfndata)+1);
	}
		
}

BOOL Shape_Texture_Filenames_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;		
}


void Shape_Texture_Filenames_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = num_tex_fns;

	data_start += 4;

	for (int i=0;i<num_tex_fns;i++) {
		sprintf(data_start, "%s", tex_fns[i]);
		data_start += (strlen (tex_fns[i]) + 1);
	}

}

size_t Shape_Texture_Filenames_Chunk::size_chunk()
{
	chunk_size = 16;

	for (int i=0;i<num_tex_fns;i++)
		chunk_size += (strlen (tex_fns[i]) + 1);

	chunk_size += (4-chunk_size%4)%4;

	return chunk_size;

}


/////////////////////////////////////////

// Class Shape_Header_Chunk functions

// These can only be children of Shape_Chunks
// the shape chunks data will automatically be updated by it is created
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPHEAD1",Shape_Header_Chunk,"REBSHAPE",Shape_Chunk)


Shape_Header_Chunk::Shape_Header_Chunk (Shape_Chunk * parent, const char * hdata, size_t /*hsize*/)
	: Chunk (parent, "SHPHEAD1"),
	shape_data (parent->shape_data_store)
{
	int num_as_obj;

	flags = *((int *) hdata);

	strncpy (lock_user, (hdata + 4), 16);
	lock_user[16] = '\0';
	hdata+=20;

	file_id_num = *((int *) hdata );
	hdata+=4;

	
	parent->shape_data_store->num_verts = *((int *) hdata );
	hdata+=4;
	parent->shape_data_store->num_polys = *((int *) hdata );
	hdata+=4;

	parent->shape_data_store->radius = *((float *) hdata);
	hdata+=4;
	
	parent->shape_data_store->max.x = *((int *) hdata);
	hdata+=4;
	parent->shape_data_store->min.x = *((int *) hdata);
	hdata+=4;

	parent->shape_data_store->max.y = *((int *) hdata);
	hdata+=4;
	parent->shape_data_store->min.y = *((int *) hdata);
	hdata+=4;

	parent->shape_data_store->max.z = *((int *) hdata);
	hdata+=4;
	parent->shape_data_store->min.z = *((int *) hdata);
	hdata+=4;

	version_no = *((int *) hdata);
	hdata+=4;

	num_as_obj = *((int *) hdata);
	hdata+=4;

	char * obj_store;

	for (int i = 0; i< num_as_obj; i++) 
	{
		obj_store = new char [strlen (hdata) +1];
		strcpy (obj_store, (hdata));
		object_names_store.add_entry(obj_store);
		hdata += (strlen (hdata)+1);
	}

}

Shape_Header_Chunk::~Shape_Header_Chunk()
{
	for (LIF<char *> aon(&object_names_store);
			!aon.done(); aon.next() )
		delete [] aon();
}

size_t Shape_Header_Chunk::size_chunk()
{
	int length = 80;

	for (LIF<char *> aon(&object_names_store);
			!aon.done(); aon.next() )
		length += (strlen (aon()) + 1);

	length += (4-length%4)%4;

	chunk_size = length;

	return length;
}

BOOL Shape_Header_Chunk::output_chunk(HANDLE & hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}

void Shape_Header_Chunk::fill_data_block(char * data_start)
{

	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	strncpy ((data_start + 4), lock_user, 16);
	*((int *) (data_start+20)) = file_id_num;
	data_start+=24;

	*((int *) data_start) = shape_data->num_verts;
	data_start+=4;
	*((int *) data_start) = shape_data->num_polys;
	data_start+=4;

	*((float *) data_start) = shape_data->radius;
	data_start+=4;

	*((int *) data_start) = shape_data->max.x;
	data_start+=4;
	*((int *) data_start) = shape_data->min.x;
	data_start+=4;

	*((int *) data_start) = shape_data->max.y;
	data_start+=4;
	*((int *) data_start) = shape_data->min.y;
	data_start+=4;

	*((int *) data_start) = shape_data->max.z;
	data_start+=4;
	*((int *) data_start) = shape_data->min.z;
	data_start+=4;

	*((int *) data_start) = version_no;
	data_start+=4;

	*((int *) data_start) = object_names_store.size();
	data_start+=4;


	for (LIF<char *> ons(&object_names_store); !ons.done(); ons.next())
	{
		strcpy (data_start, ons());
		data_start += (strlen(ons())+1);
	}

}


void Shape_Header_Chunk::prepare_for_output()
{
// this will also set the object chunks numbers as well,
// so that it is done in the right order

	char * str;

	if (file_id_num == -1) file_id_num = ++(Shape_Chunk::max_id);

	while (object_names_store.size()) {
		delete [] object_names_store.first_entry();
		object_names_store.delete_first_entry();
	}

	for (LIF<Object_Chunk *> aosl(&associated_objects_store);
		!aosl.done(); aosl.next() ) {

		if (aosl()->get_header())
			aosl()->get_header()->shape_id_no = file_id_num;

		str = new char [strlen(aosl()->object_data.o_name) + 1];
		strcpy(str, aosl()->object_data.o_name);
		object_names_store.add_entry(str);

	}

	Shape_Chunk::max_id = max(Shape_Chunk::max_id, file_id_num);

// this should always be the last thing

	version_no ++;
}



/////////////////////////////////////////

// Class Shape_Merge_Data_Chunk functions

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPMRGDT",Shape_Merge_Data_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPMRGDT",Shape_Merge_Data_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)

Shape_Merge_Data_Chunk::Shape_Merge_Data_Chunk(Shape_Chunk * parent, int * m_dt, int n_ps)
: Chunk (parent, "SHPMRGDT"), num_polys (n_ps)
{
	merge_data = new int [n_ps];
	for (int i=0; i<n_ps; i++)
	{
		merge_data[i] = m_dt[i];
	}
	
}	

Shape_Merge_Data_Chunk::Shape_Merge_Data_Chunk(Shape_Sub_Shape_Chunk * parent, int * m_dt, int n_ps)
: Chunk (parent, "SHPMRGDT"), num_polys (n_ps)
{
	merge_data = new int [n_ps];
	for (int i=0; i<n_ps; i++)
	{
		merge_data[i] = m_dt[i];
	}
	
}	


Shape_Merge_Data_Chunk::Shape_Merge_Data_Chunk (Shape_Chunk * parent, const char *md, size_t ms)
: Chunk (parent, "SHPMRGDT"), num_polys (ms/4)
{
	
	merge_data = new int [num_polys];
	for (int i=0; i<num_polys; i++)
	{
		merge_data[i] = *((int *)(md+4*i));
	}
}

Shape_Merge_Data_Chunk::Shape_Merge_Data_Chunk (Shape_Sub_Shape_Chunk * parent, const char *md, size_t ms)
: Chunk (parent, "SHPMRGDT"), num_polys (ms/4)
{
	
	merge_data = new int [num_polys];
	for (int i=0; i<num_polys; i++)
	{
		merge_data[i] = *((int *)(md+4*i));
	}
}


Shape_Merge_Data_Chunk::~Shape_Merge_Data_Chunk()
{
	if (num_polys) delete [] merge_data;
}

void Shape_Merge_Data_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	for (int i=0; i<num_polys; i++)
	{
		*((int *) (data_start+i*4) ) = merge_data[i];
	}
	
}


/////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SHPEXTFL",Shape_External_File_Chunk)

/*
Children for Shape_External_File_Chunk :

"SHPEXTFN"		Shape_External_Filename_Chunk)
"BMPLSTST"		Bitmap_List_Store_Chunk)
"BMNAMVER"		BMP_Names_Version_Chunk)
"BMNAMEXT"		BMP_Names_ExtraData_Chunk)
"RIFFNAME"		RIF_Name_Chunk)
"BMPMD5ID"		Bitmap_MD5_Chunk)
"EXTOBJNM"		Shape_External_Object_Name_Chunk)
*/

CHUNK_WITH_CHILDREN_LOADER("SHPEXTFL",Shape_External_File_Chunk)

	

Shape_External_File_Chunk::Shape_External_File_Chunk (Chunk_With_Children * parent, const char * fname)
:Chunk_With_Children (parent, "SHPEXTFL")
{
	new Shape_External_Filename_Chunk (this, fname);
	post_input_processing();
}

void Shape_External_File_Chunk::post_input_processing()
{
	Chunk_With_Children::post_input_processing();
}

const char* Shape_External_File_Chunk::get_shape_name()
{
	Shape_External_Object_Name_Chunk* seonc=(Shape_External_Object_Name_Chunk*)lookup_single_child("EXTOBJNM");
	if(seonc)
	{
		return seonc->shape_name;
	}

	RIF_Name_Chunk* rnc=(RIF_Name_Chunk*)lookup_single_child("RIFFNAME");
	if(rnc)
	{
		return rnc->rif_name;
	}

	return 0;

}

/////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SHPEXTFN",Shape_External_Filename_Chunk)

Shape_External_Filename_Chunk::Shape_External_Filename_Chunk(Chunk_With_Children * parent, const char * fname)
: Chunk (parent, "SHPEXTFN")
{
	file_name = new char [strlen(fname)+1];
	strcpy (file_name, fname);
	
	rescale = 1;
	version_no = -1;
	
}

Shape_External_Filename_Chunk::Shape_External_Filename_Chunk (Chunk_With_Children * parent, const char *fdata, size_t /*fsize*/)
: Chunk (parent, "SHPEXTFN")
{
	rescale = *((unaligned_f64 *) fdata);
	fdata += 8;
	version_no = *((unaligned_s32 *) fdata);
	fdata += 4;
	file_name = new char [strlen(fdata)+1];
	strcpy (file_name, fdata);
}

Shape_External_Filename_Chunk::~Shape_External_Filename_Chunk()
{
	if (file_name)
		delete [] file_name;
}

void Shape_External_Filename_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*((double *) data_start) = rescale;
	
	data_start += 8;

	*((int *) data_start) = version_no;
	
	data_start += 4;

	strcpy (data_start, file_name);
		
}

///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("EXTOBJNM",Shape_External_Object_Name_Chunk)

Shape_External_Object_Name_Chunk::Shape_External_Object_Name_Chunk(Chunk_With_Children * parent, const char * oname)
: Chunk (parent, "EXTOBJNM")
{
	object_name = new char [strlen(oname)+1];
	shape_name=0;
	strcpy (object_name, oname);
	pad=0;
	
	RIF_Name_Chunk* rnc=(RIF_Name_Chunk*)parent->lookup_single_child("RIFFNAME");
	if(rnc)
	{
		shape_name=new char[strlen(object_name)+strlen(rnc->rif_name)+2];
		sprintf(shape_name,"%s@%s",object_name,rnc->rif_name);
	}
	else
	{
		shape_name=new char[strlen(object_name)+10+2];
		sprintf(shape_name,"%s@*NotFound*",object_name);
	}
}

Shape_External_Object_Name_Chunk::Shape_External_Object_Name_Chunk (Chunk_With_Children * parent, const char *data, size_t /*fsize*/)
: Chunk (parent, "EXTOBJNM")
{
	pad=*(int*)data;
	data+=4;
	object_name = new char [strlen(data)+1];
	strcpy (object_name, data);
	shape_name=0;
}

Shape_External_Object_Name_Chunk::~Shape_External_Object_Name_Chunk()
{
	if (object_name)
		delete [] object_name;
	if(shape_name)
		delete [] shape_name;
}

void  Shape_External_Object_Name_Chunk::post_input_processing()
{
	RIF_Name_Chunk* rnc=(RIF_Name_Chunk*)parent->lookup_single_child("RIFFNAME");
	if(rnc)
	{
		shape_name=new char[strlen(object_name)+strlen(rnc->rif_name)+2];
		sprintf(shape_name,"%s@%s",object_name,rnc->rif_name);
	}
	else
	{
		shape_name=new char[strlen(object_name)+10+2];
		sprintf(shape_name,"%s@*NotFound*",object_name);
	}
}

void Shape_External_Object_Name_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start =pad;

	data_start+=4;

	strcpy (data_start, object_name);
		
}

///////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPMORPH",Shape_Morphing_Data_Chunk,"REBSHAPE",Shape_Chunk)
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SHPMORPH",Shape_Morphing_Data_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)

/*
Children for Shape_Morphing_Data_Chunk :

"SUBSHAPE"		Shape_Sub_Shape_Chunk
"FRMMORPH"		Shape_Morphing_Frame_Data_Chunk
*/


Shape_Morphing_Data_Chunk::Shape_Morphing_Data_Chunk (Shape_Chunk * parent, const char *data, size_t size)
: Chunk_With_Children (parent, "SHPMORPH"), parent_shape (parent), parent_sub_shape (0)
{
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(data);
		data += *(int *)(data + 8);

	}
	
}	
Shape_Morphing_Data_Chunk::Shape_Morphing_Data_Chunk (Shape_Sub_Shape_Chunk * parent, const char *data, size_t size)
: Chunk_With_Children (parent, "SHPMORPH"), parent_shape (0), parent_sub_shape (parent) 
{
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}
		DynCreate(data);
		data += *(int *)(data + 8);


	}
	
}	


void Shape_Morphing_Data_Chunk::prepare_for_output()
{
	int max_id = 0;
	
	List<Chunk *> cl;
	lookup_child("SUBSHAPE",cl);
	
	LIF<Chunk *> cli(&cl);
	
	for (; !cli.done(); cli.next())
	{
		max_id = max (max_id, ((Shape_Sub_Shape_Chunk *)cli())->get_header()->file_id_num);
	}

	for (cli.restart(); !cli.done(); cli.next())
	{
		if (((Shape_Sub_Shape_Chunk *)cli())->get_header()->file_id_num == -1)
		{
			((Shape_Sub_Shape_Chunk *)cli())->get_header()->file_id_num = ++max_id;
		}
	}
	Chunk_With_Children::prepare_for_output();
	
}

///////////////////////////////////////

#ifdef new
#pragma message("new defined")
#endif

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("FRMMORPH",Shape_Morphing_Frame_Data_Chunk,"SHPMORPH",Shape_Morphing_Data_Chunk)

Shape_Morphing_Frame_Data_Chunk::Shape_Morphing_Frame_Data_Chunk (Shape_Morphing_Data_Chunk * parent,const char *data, size_t /*size*/)
: Chunk (parent, "FRMMORPH")
{
	a_flags = *((int *)data);
	data +=4;
	a_speed = *((int *)data);
	data +=4;
	num_frames = *((int *)data);
	data +=4;

	if (num_frames)
		frame_store = new int [num_frames * 3];
	else 
		frame_store=0;
	for (int i=0; i<num_frames; i++)
	{
		frame_store[i*3] = *((int *)data);
		data +=4;
		frame_store[i*3+1] = *((int *)data);
		data +=4;
		frame_store[i*3+2] = *((int *)data);
		data +=4;
	}
}	

void Shape_Morphing_Frame_Data_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;
	*((int *) data_start) = a_flags;
	data_start += 4;
	*((int *) data_start) = a_speed;
	data_start += 4;
	*((int *) data_start) = num_frames;
	data_start += 4;
	for (int i=0; i<num_frames; i++)
	{
		*((int *) data_start) = frame_store[i*3];
		data_start += 4;
		*((int *) data_start) = frame_store[i*3 + 1];
		data_start += 4;
		*((int *) data_start) = frame_store[i*3 + 2];
		data_start += 4;
	}
}

Shape_Morphing_Frame_Data_Chunk::~Shape_Morphing_Frame_Data_Chunk()
{
	if (frame_store) delete [] frame_store;
	while (anim_frames.size())
	{
		delete anim_frames.first_entry();
		anim_frames.delete_first_entry();
	}
}

void Shape_Morphing_Frame_Data_Chunk::prepare_for_output()
{
	// get the number of each shape in each frame
	
	// N.B. this relies on the fact that the shapes are numbered
	// but they will be by the parent class which will have it's prepare
	// for output called first
	
	if (frame_store)
		delete [] frame_store;

	if (anim_frames.size())
	{
		frame_store = new int [ anim_frames.size() * 3 ];
	}
	else
	{
		frame_store = 0;
	}
	
	int num_f = 0;
	
	for (LIF<a_frame *> afi(&anim_frames); !afi.done(); afi.next())
	{
		int s1no, s2no;
		if (afi()->shape1a)
			s1no = afi()->shape1a->get_header()->file_id_num;
		else
			s1no = -1;
		if (afi()->shape2a)
			s2no = afi()->shape2a->get_header()->file_id_num;
		else
			s2no = -1;	
			
		frame_store[num_f] = s1no;
		frame_store[num_f+1] = s2no;
		frame_store[num_f+2] = afi()->spare;
		num_f ++;
	}
	num_frames = num_f;	
}


void Shape_Morphing_Frame_Data_Chunk::post_input_processing()
{
	List<Shape_Sub_Shape_Chunk *> shplist;
	List<Chunk *> child_lists;

	Shape_Morphing_Data_Chunk * pchnk = (Shape_Morphing_Data_Chunk *) parent;
	
	pchnk->lookup_child("SUBSHAPE",child_lists);

	while (child_lists.size()) {
		shplist.add_entry((Shape_Sub_Shape_Chunk *)child_lists.first_entry());
		child_lists.delete_first_entry();
	}
	
	LIF<Shape_Sub_Shape_Chunk *> sl(&shplist);
	
	for (int i = 0; i<num_frames; i++)
	{
		Shape_Chunk * sh1b = 0, *sh2b = 0;
		Shape_Sub_Shape_Chunk * sh1a = 0, *sh2a = 0;
		a_frame * fr;
		
		if (frame_store[i*2] == -1)
		{
			sh1b = pchnk->parent_shape;
		}
		else
		{
			for (; !sl.done(); sl.next()) {
				if (sl()->get_header())
					if (sl()->get_header()->file_id_num == frame_store[i])
						break;
			}
			if (!sl.done())
			{
				sh1a = sl();
			}
		}

		if (frame_store[i*2+1] == -1)
		{
			sh2b = pchnk->parent_shape;
		}
		else
		{
			for (sl.restart(); !sl.done(); sl.next()) {
				if (sl()->get_header())
					if (sl()->get_header()->file_id_num == frame_store[i+1])
						break;
			}
			if (!sl.done())
			{
				sh2a = sl();
			}
		}
		if ((sh1a || sh1b) && (sh2a || sh2b))
		{
		 	fr = new a_frame;
			if (sh1a)
				fr->shape1a = sh1a;
			else if (sh1b)
				fr->shape1b = sh1b;

			if (sh2a)
				fr->shape2a = sh2a;
			else if (sh2b)
				fr->shape2b = sh2b;

			fr->spare = frame_store[i+2];
			anim_frames.add_entry(fr);
		}
			
	}
}

/////////////////////////////////////////

// Class Shape_Sub_Shape_Chunk functions


RIF_IMPLEMENT_DYNCREATE("SUBSHAPE",Shape_Sub_Shape_Chunk)
/*
Children for Shape_Sub_Shape_Chunk :

"SHPRAWVT"		Shape_Vertex_Chunk
"SHPPOLYS"		Shape_Polygon_Chunk
"SUBSHPHD"		Shape_Sub_Shape_Header_Chunk
"SHPVNORM"		Shape_Vertex_Normal_Chunk
"SHPPNORM"		Shape_Polygon_Normal_Chunk
"SHPTEXFN"		Shape_Texture_Filenames_Chunk
"SHPUVCRD"		Shape_UV_Coord_Chunk
"SHPMRGDT"		Shape_Merge_Data_Chunk
"SHPCENTR"		Shape_Centre_Chunk
"SHPMORPH"		Shape_Morphing_Data_Chunk
"SHPEXTFL"		Shape_External_File_Chunk
"SHPPCINF"		Shape_Poly_Change_Info_Chunk
"TEXTANIM"		Animation_Chunk
"SHPFRAGS"		Shape_Fragments_Chunk
"ANIMSEQU"		Anim_Shape_Sequence_Chunk
"PNOTINBB"		Poly_Not_In_Bounding_Box_Chunk
"ANSHCEN2"		Anim_Shape_Centre_Chunk
"ASALTTEX"		Anim_Shape_Alternate_Texturing_Chunk
"SHPPRPRO"		Shape_Preprocessed_Data_Chunk	


"SHPFNAME"		Shape_Name_Chunk
"FRAGDATA"		Shape_Fragments_Data_Chunk
"FRAGLOCN"		Shape_Fragment_Location_Chunk
*/

Shape_Sub_Shape_Chunk::Shape_Sub_Shape_Chunk(Chunk_With_Children * parent, const char *data, size_t size)
: Chunk_With_Children (parent, "SUBSHAPE"), shape_data ()
{
	const char * buffer_ptr = data;

	shape_data_store = (ChunkShape *) &shape_data;
	
	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(data);
		data += *(int *)(data + 8);

	}
}

Shape_Sub_Shape_Chunk::Shape_Sub_Shape_Chunk (Chunk_With_Children * parent, ChunkShape &shp_dat)
: Chunk_With_Children (parent, "SUBSHAPE"), shape_data (shp_dat)
{
	shape_data_store = (ChunkShape *) &shape_data;
	
	new Shape_Sub_Shape_Header_Chunk (this);

	if (shape_data.v_list) new Shape_Vertex_Chunk (this, shape_data.num_verts);
	if (shape_data.v_normal_list) new Shape_Vertex_Normal_Chunk (this, shape_data.num_verts);
	if (shape_data.p_normal_list) new Shape_Polygon_Normal_Chunk (this, shape_data.num_polys);
	if (shape_data.poly_list) new Shape_Polygon_Chunk (this, shape_data.num_polys);
	if (shape_data.uv_list) new Shape_UV_Coord_Chunk (this, shape_data.num_uvs);
	if (shape_data.texture_fns) new Shape_Texture_Filenames_Chunk (this, shape_data.num_texfiles);
	
	//calculate the shape's centre and radius_about_centre
	shape_data_store->centre=(shape_data_store->min+shape_data_store->max)/2;
	shape_data_store->radius_about_centre=0;
	for(int i=0;i<shape_data_store->num_verts;i++)
	{
		float length = (float) mod(shape_data_store->v_list[i]-shape_data_store->centre);
		if(length>shape_data_store->radius_about_centre)
		{
			shape_data_store->radius_about_centre=length;
		}
	}

	//if the shape hasn't got a Shape_Centre_Chunk , create one.

	if(!lookup_single_child("SHPCENTR"))
	{
		new Shape_Centre_Chunk(this);
	}	
}

Shape_Sub_Shape_Chunk::~Shape_Sub_Shape_Chunk ()
{
}

Shape_Sub_Shape_Chunk* Shape_Sub_Shape_Chunk::make_copy_of_chunk()
{
	char* Data=this->make_data_block_from_chunk();
	Shape_Sub_Shape_Chunk* NewShape=new Shape_Sub_Shape_Chunk(parent,Data+12,this->size_chunk()-12);
	delete [] Data;
	delete NewShape->get_header();
	new Shape_Sub_Shape_Header_Chunk(NewShape);
	return NewShape;
}

Shape_Sub_Shape_Header_Chunk * Shape_Sub_Shape_Chunk::get_header()
{
	
	return (Shape_Sub_Shape_Header_Chunk *) this->lookup_single_child ("SUBSHPHD");

}

BOOL Shape_Sub_Shape_Chunk::update_my_chunkshape (ChunkShape & cshp)
{
	// Firstly lose all the chunks that were with
	// the old chunk shape
	List <Chunk *> chlst;

	lookup_child ("SHPRAWVT",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	lookup_child ("SHPVNORM",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPPNORM",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPPOLYS",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPUVCRD",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}


	lookup_child ("SHPTEXFN",chlst);

	while (chlst.size())
	{
		delete chlst.first_entry();
		chlst.delete_first_entry();
	}

	*shape_data_store = cshp;

	if (shape_data.v_list) new Shape_Vertex_Chunk (this, shape_data.num_verts);
	if (shape_data.v_normal_list) new Shape_Vertex_Normal_Chunk (this, shape_data.num_verts);
	if (shape_data.p_normal_list) new Shape_Polygon_Normal_Chunk (this, shape_data.num_polys);
	if (shape_data.poly_list) new Shape_Polygon_Chunk (this, shape_data.num_polys);
	if (shape_data.uv_list) new Shape_UV_Coord_Chunk (this, shape_data.num_uvs);
	if (shape_data.texture_fns) new Shape_Texture_Filenames_Chunk (this, shape_data.num_texfiles);

	//calculate the shape's centre and radius_about_centre
	shape_data_store->centre=(shape_data_store->min+shape_data_store->max)/2;
	shape_data_store->radius_about_centre=0;
	for(int i=0;i<shape_data_store->num_verts;i++)
	{
		float length=(float)mod(shape_data_store->v_list[i]-shape_data_store->centre);
		if(length>shape_data_store->radius_about_centre)
		{
			shape_data_store->radius_about_centre=length;
		}
	}

	//if the shape hasn't got a Shape_Centre_Chunk , create one.

	if(!lookup_single_child("SHPCENTR"))
	{
		new Shape_Centre_Chunk(this);
	}	
	
	
	return TRUE;
}


const char * Shape_Sub_Shape_Chunk::get_shape_name()
{
	Shape_Name_Chunk* snc=(Shape_Name_Chunk*)lookup_single_child("SHPFNAME");
	if (snc)
	{
		return snc->shape_name;
	}
	else
	{
		return(0);
	}
}

Console_Shape_Chunk* Shape_Sub_Shape_Chunk::get_console_shape_data(Console_Type ct)
{
	List<Chunk*> chlist;
	lookup_child("CONSHAPE",chlist);
	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Console_Shape_Chunk* csc=(Console_Shape_Chunk*)chlif();
		List<Chunk*> chlist2;
		csc->lookup_child("CONSTYPE",chlist2);
		if(chlist2.size())
		{
			if(((Console_Type_Chunk*)chlist2.first_entry())->console==ct)
				return csc;
		}
	}
	
	return 0;//no console specific shape data
}

/////////////////////////////////////////

// Class Shape_Sub_Shape_Header_Chunk functions

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SUBSHPHD",Shape_Sub_Shape_Header_Chunk,"SUBSHAPE",Shape_Sub_Shape_Chunk)

// These can only be children of Shape_Sub_Shape_Chunks
// the shape chunks data will automatically be updated by it is created

Shape_Sub_Shape_Header_Chunk::Shape_Sub_Shape_Header_Chunk (Shape_Sub_Shape_Chunk * parent, const char * hdata, size_t /*hsize*/)
	: Chunk (parent, "SUBSHPHD"),
	shape_data (parent->shape_data_store)
{
	flags = *((int *) hdata);
	hdata += 4;
	
	file_id_num = *((int *) (hdata));
	hdata += 4;

	parent->shape_data_store->num_verts = *((int *) (hdata));
	hdata += 4;
	parent->shape_data_store->num_polys = *((int *) (hdata));
	hdata += 4;

	parent->shape_data_store->radius = *((float *) (hdata));
	hdata += 4;
	
	parent->shape_data_store->max.x = *((int *) (hdata));
	hdata += 4;
	parent->shape_data_store->min.x = *((int *) (hdata));
	hdata += 4;

	parent->shape_data_store->max.y = *((int *) (hdata));
	hdata += 4;
	parent->shape_data_store->min.y = *((int *) (hdata));
	hdata += 4;

	parent->shape_data_store->max.z = *((int *) (hdata));
	hdata += 4;
	parent->shape_data_store->min.z = *((int *) (hdata));
}

Shape_Sub_Shape_Header_Chunk::~Shape_Sub_Shape_Header_Chunk()
{
}

size_t Shape_Sub_Shape_Header_Chunk::size_chunk()
{
	return chunk_size = 44 + 12;
}


void Shape_Sub_Shape_Header_Chunk::fill_data_block(char * data_start)
{

	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	data_start += 4;

	*((int *) (data_start)) = file_id_num;
	data_start += 4;

	*((int *) (data_start)) = shape_data->num_verts;
	data_start += 4;
	*((int *) (data_start)) = shape_data->num_polys;
	data_start += 4;

	*((float *) (data_start)) = shape_data->radius;
	data_start += 4;

	*((int *) (data_start)) = shape_data->max.x;
	data_start += 4;
	*((int *) (data_start)) = shape_data->min.x;
	data_start += 4;

	*((int *) (data_start)) = shape_data->max.y;
	data_start += 4;
	*((int *) (data_start)) = shape_data->min.y;
	data_start += 4;

	*((int *) (data_start)) = shape_data->max.z;
	data_start += 4;
	*((int *) (data_start)) = shape_data->min.z;

}

/////////////////////////////////////////


// Class Shape_Poly_Change_Info_Chunk functions

RIF_IMPLEMENT_DYNCREATE("SHPPCINF",Shape_Poly_Change_Info_Chunk)

void Shape_Poly_Change_Info_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = original_num_verts;

	data_start += 4;

	*((int *) data_start) = change_list.size();

	data_start += 4;

	for (LIF<poly_change_info> pcii(&change_list); !pcii.done(); pcii.next())
	{
		*((int *) data_start) = pcii().poly_num;

		data_start += 4;
		
		*((int *) data_start) = pcii().vert_num_before;

		data_start += 4;
	
		*((int *) data_start) = pcii().vert_num_after;

		data_start += 4;
	}
	
		
}

Shape_Poly_Change_Info_Chunk::Shape_Poly_Change_Info_Chunk (Chunk_With_Children * parent,const char * data, size_t /*size*/)
: Chunk (parent, "SHPPCINF")
{
	original_num_verts = *((int *) data);

	data += 4;

	int n_entries = *((int *) data);

	data += 4;

	for (int i=0; i<n_entries; i++)
	{
		poly_change_info pci;
		
		pci.poly_num = *((int *) data);
		data += 4;
		pci.vert_num_before = *((int *) data);
		data += 4;
		pci.vert_num_after = *((int *) data);
		data += 4;
	
		change_list.add_entry(pci);
	}
	
}	


/////////////////////////////////////////

// Class Shape_Name_Chunk functions

RIF_IMPLEMENT_DYNCREATE("SHPFNAME",Shape_Name_Chunk)

Shape_Name_Chunk::Shape_Name_Chunk (Chunk_With_Children * parent, const char * sname)
: Chunk (parent, "SHPFNAME")
{
	shape_name = new char [strlen(sname)+1];
	strcpy (shape_name, sname);
}

Shape_Name_Chunk::Shape_Name_Chunk (Chunk_With_Children * parent, const char * sndata, size_t /*rsize*/)
: Chunk (parent, "SHPFNAME")
{
	shape_name = new char [strlen(sndata)+1];
	strcpy (shape_name, sndata);
}

Shape_Name_Chunk::~Shape_Name_Chunk ()
{
	if (shape_name)
		delete [] shape_name;
}


void Shape_Name_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	strcpy (data_start, shape_name);

}

/////////////////////////////////////////

// Class Shape_Fragments_Chunk functions

RIF_IMPLEMENT_DYNCREATE("SHPFRAGS",Shape_Fragments_Chunk)

CHUNK_WITH_CHILDREN_LOADER("SHPFRAGS",Shape_Fragments_Chunk)
/*
Children for Shape_Fragments_Chunk :

"SUBSHAPE"		Shape_Sub_Shape_Chunk
"SHPFRGTP"		Shape_Fragment_Type_Chunk
"FRGSOUND"		Fragment_Type_Sound_Chunk
*/


/////////////////////////////////////////

// Class Shape_Fragments_Data_Chunk functions
RIF_IMPLEMENT_DYNCREATE("FRAGDATA",Shape_Fragments_Data_Chunk)

void Shape_Fragments_Data_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*((int *) data_start) = num_fragments;

	data_start += 4;
	
	*((int *) data_start) = pad1;

	data_start += 4;
	
	*((int *) data_start) = pad2;

	data_start += 4;
	
	*((int *) data_start) = pad3;

}

Shape_Fragments_Data_Chunk::Shape_Fragments_Data_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
: Chunk (parent, "FRAGDATA")
{
	num_fragments = *((int *) sdata);

	sdata += 4;
	
	pad1 = *((int *) sdata);

	sdata += 4;
	
	pad2 = *((int *) sdata);

	sdata += 4;
	
	pad3 = *((int *) sdata);

	sdata += 4;
	
}


/////////////////////////////////////////

// Class Shape_Fragments_Location_Chunk functions
RIF_IMPLEMENT_DYNCREATE("FRAGLOCN",Shape_Fragment_Location_Chunk)

void Shape_Fragment_Location_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*((ChunkVectorInt *) data_start) = frag_loc;

	data_start += sizeof(ChunkVectorInt);
	
	
	*((int *) data_start) = pad1;

	data_start += 4;
	
	*((int *) data_start) = pad2;

	data_start += 4;
	
	*((int *) data_start) = pad3;

	data_start += 4;
	
	*((int *) data_start) = pad4;

}

Shape_Fragment_Location_Chunk::Shape_Fragment_Location_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
: Chunk (parent, "FRAGLOCN")
{
	frag_loc = *((ChunkVectorInt *) sdata);

	sdata += sizeof(ChunkVectorInt);
	
	
	pad1 = *((int *) sdata);

	sdata += 4;
	
	pad2 = *((int *) sdata);

	sdata += 4;
	
	pad3 = *((int *) sdata);

	sdata += 4;
	
	pad4 = *((int *) sdata);
	
}

/////////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("SHPFRGTP",Shape_Fragment_Type_Chunk)
Shape_Fragment_Type_Chunk::Shape_Fragment_Type_Chunk(Chunk_With_Children* parent,const char* name)
:Chunk(parent,"SHPFRGTP")
{
	frag_type_name=new char[strlen(name)+1];
	strcpy(frag_type_name,name);
	pad1=pad2-0;
}

Shape_Fragment_Type_Chunk::Shape_Fragment_Type_Chunk(Chunk_With_Children* const parent,const char* data,size_t const )
:Chunk(parent,"SHPFRGTP")
{
	int length=strlen(data)+1;
	frag_type_name=new char[length];
	strcpy(frag_type_name,data);
	data+=(length+3)&~3;

	pad1=*(int*)data;
	data+=4;
	pad2=*(int*)data;
	data+=4;
}

Shape_Fragment_Type_Chunk::~Shape_Fragment_Type_Chunk()
{
	if(frag_type_name) delete [] frag_type_name;
}											 

void Shape_Fragment_Type_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,frag_type_name);
	data_start+=(strlen(frag_type_name)+4)&~3;	

	*(int*)data_start=pad1;
	data_start+=4;
	*(int*)data_start=pad2;
	data_start+=4;

}

size_t Shape_Fragment_Type_Chunk::size_chunk()
{
	chunk_size=12+8;
	chunk_size+=(strlen(frag_type_name)+4)&~3;
	return chunk_size;
}


/////////////////////////////////////////

// Class Anim_Shape_Sequence_Chunk functions

RIF_IMPLEMENT_DYNCREATE("ANIMSEQU",Anim_Shape_Sequence_Chunk)

/*
Children for Anim_Shape_Sequence_Chunk :
"ANISEQDT"		Anim_Shape_Sequence_Data_Chunk
"ANIMFRAM"		Anim_Shape_Frame_Chunk
*/

Anim_Shape_Sequence_Chunk::Anim_Shape_Sequence_Chunk(Chunk_With_Children * const parent, const char *data, size_t const size)
: Chunk_With_Children (parent, "ANIMSEQU")
{
	
	sequence_data_store=(ChunkAnimSequence*)&sequence_data;
	
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(data);
		data += *(int *)(data + 8);
		
	}
	if(!ConstructSequenceDataFromChildren())
	{
		//this sequence is no longer valid
		delete this;
	}
}
Anim_Shape_Sequence_Chunk::Anim_Shape_Sequence_Chunk(Chunk_With_Children * const parent, int sequencenum,const char* name)
: Chunk_With_Children (parent, "ANIMSEQU")
{
	
	sequence_data_store=(ChunkAnimSequence*)&sequence_data;
	
	sequence_data_store->SequenceNum=sequencenum;
	sequence_data_store->name=new char[strlen(name)+1];
	strcpy(sequence_data_store->name,name);
	RegenerateChildChunks();
}

Anim_Shape_Sequence_Chunk::Anim_Shape_Sequence_Chunk(Chunk_With_Children * const parent, ChunkAnimSequence const* cas )
: Chunk_With_Children (parent, "ANIMSEQU")
{
	
	sequence_data_store=(ChunkAnimSequence*)&sequence_data;
	*(ChunkAnimSequence*)&sequence_data=*cas;
	RegenerateChildChunks();
}


int Anim_Shape_Sequence_Chunk::ConstructSequenceDataFromChildren()
{
	Anim_Shape_Sequence_Data_Chunk* assdc=(Anim_Shape_Sequence_Data_Chunk*)lookup_single_child("ANISEQDT");
	if(!assdc)
	{
		return 0;
	}		
	sequence_data_store->name=assdc->name;
	sequence_data_store->SequenceNum=assdc->SequenceNum;
	sequence_data_store->flags=assdc->flags;
	sequence_data_store->pad2=assdc->pad2;
	sequence_data_store->pad3=assdc->pad3;
	sequence_data_store->pad4=assdc->pad4;

	List<Chunk*> chlist;
	lookup_child("ANIMFRAM",chlist);
	sequence_data_store->NumFrames=chlist.size();
	if(sequence_data_store->NumFrames)
	{
		sequence_data_store->Frames=new ChunkAnimFrame*[sequence_data_store->NumFrames];
		for(int i=0;i<sequence_data_store->NumFrames;i++)
		{
			sequence_data_store->Frames[i]=0;
		}
	}
	for(LIF<Chunk*> chlif(& chlist);!chlif.done();chlif.next())
	{
		Anim_Shape_Frame_Chunk* asfc=(Anim_Shape_Frame_Chunk*)chlif();

		Anim_Shape_Frame_Data_Chunk* asfdc=(Anim_Shape_Frame_Data_Chunk*)asfc->lookup_single_child("ANIFRADT");
		if(!asfdc) return 0;
		if(asfdc->FrameNum>=sequence_data_store->NumFrames) return 0;
		
		if(sequence_data_store->Frames[asfdc->FrameNum])return 0;
		sequence_data_store->Frames[asfdc->FrameNum]=new ChunkAnimFrame;
		ChunkAnimFrame* caf=sequence_data_store->Frames[asfdc->FrameNum];
		
		Shape_Vertex_Chunk* svc=(Shape_Vertex_Chunk*) asfc->lookup_single_child("SHPRAWVT");
		if(!svc)return 0;

		Shape_Polygon_Normal_Chunk* spnc=(Shape_Polygon_Normal_Chunk*)asfc->lookup_single_child("SHPPNORM");
		if(!spnc) return 0;

		caf->name=asfdc->name;
		caf->flags=asfdc->flags;
		caf->num_interp_frames=asfdc->num_interp_frames;
		caf->pad3=asfdc->pad3;
		caf->pad4=asfdc->pad4;

		caf->num_verts=svc->num_verts;
		caf->v_list=(ChunkVectorInt*)svc->vert_data;

		caf->num_polys=spnc->num_polys;
		caf->p_normal_list=(ChunkVectorFloat*)spnc->poly_norm_data;
	
	}
	return 1;	
}
void Anim_Shape_Sequence_Chunk::post_input_processing()
{
	
	List<int>* poly_not_in_bb=0;
	List<Chunk*> chlist;
	parent->lookup_child("PNOTINBB",chlist);
	if(chlist.size())
		poly_not_in_bb=&((Poly_Not_In_Bounding_Box_Chunk*)chlist.first_entry())->poly_no;
	if(!strcmp(parent->identifier,"REBSHAPE"))
	{
		sequence_data_store->UpdateNormalsAndExtents(&((Shape_Chunk*)parent)->shape_data,poly_not_in_bb);
	}
	else if(!strcmp(parent->identifier,"SUBSHAPE"))
	{
		sequence_data_store->UpdateNormalsAndExtents(&((Shape_Sub_Shape_Chunk*)parent)->shape_data,poly_not_in_bb);
	}
	
	Chunk_With_Children::post_input_processing();
}


void Anim_Shape_Sequence_Chunk::update_my_sequencedata(ChunkAnimSequence & seq)
{
	*sequence_data_store=seq;
	RegenerateChildChunks();	
}

void Anim_Shape_Sequence_Chunk::set_sequence_flags(int flags)
{
	sequence_data_store->flags=flags;
}
void Anim_Shape_Sequence_Chunk::set_frame_flags(int frameno,int flags)
{
	if(frameno< sequence_data_store->NumFrames)
	{
		if(sequence_data_store->Frames[frameno])
		{
			sequence_data_store->Frames[frameno]->flags=flags;
		}
	}
}

void Anim_Shape_Sequence_Chunk::RegenerateChildChunks()
{
	List<Chunk*> chlist;
	lookup_child("ANISEQDT",chlist);
	while(chlist.size())
	{
		delete chlist.first_entry();
		chlist.delete_first_entry();	
	}
	lookup_child("ANIMFRAM",chlist);
	while(chlist.size())
	{
		delete chlist.first_entry();
		chlist.delete_first_entry();	
	}

	new Anim_Shape_Sequence_Data_Chunk(this,sequence_data_store);
	for(int i=0;i<sequence_data_store->NumFrames;i++)
	{
	 	if(sequence_data_store->Frames[i]->flags & animframeflag_interpolated_frame)continue;
	 	new Anim_Shape_Frame_Chunk(this,sequence_data_store->Frames[i],i);
	}
}

void Anim_Shape_Sequence_Chunk::GenerateInterpolatedFrames()
{

	if(!strcmp(parent->identifier,"REBSHAPE"))
	{
		sequence_data_store->GenerateInterpolatedFrames(&((Shape_Chunk*)parent)->shape_data);
	}
	else if(!strcmp(parent->identifier,"SUBSHAPE"))
	{
		sequence_data_store->GenerateInterpolatedFrames(&((Shape_Sub_Shape_Chunk*)parent)->shape_data);
	}
}
/////////////////////////////////////////

// Class Anim_Shape_Frame_Chunk functions
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("ANIMFRAM",Anim_Shape_Frame_Chunk,"ANIMSEQU",Anim_Shape_Sequence_Chunk)

/*
Children for Anim_Shape_Frame_Chunk :

"ANIFRADT"		Anim_Shape_Frame_Data_Chunk
"SHPPNORM"		Shape_Polygon_Normal_Chunk
"SHPRAWVT"		Shape_Vertex_Chunk
*/

Anim_Shape_Frame_Chunk::Anim_Shape_Frame_Chunk(Anim_Shape_Sequence_Chunk *const parent, const char *data, size_t const size)
: Chunk_With_Children (parent, "ANIMFRAM")
{
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}
		DynCreate(data);
		data += *(int *)(data + 8);
	}
}


Anim_Shape_Frame_Chunk::Anim_Shape_Frame_Chunk(Anim_Shape_Sequence_Chunk * const parent,ChunkAnimFrame* caf,int frameno)
: Chunk_With_Children (parent, "ANIMFRAM")
{
	new Anim_Shape_Frame_Data_Chunk(this,caf,frameno);
	new Shape_Vertex_Chunk(this,caf->v_list,caf->num_verts);
	new Shape_Polygon_Normal_Chunk(this,caf->p_normal_list,caf->num_polys);

}
/////////////////////////////////////////

// Class Anim_Shape_Sequence_Data_Chunk functions

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("ANISEQDT",Anim_Shape_Sequence_Data_Chunk,"ANIMSEQU",Anim_Shape_Sequence_Chunk)

Anim_Shape_Sequence_Data_Chunk::Anim_Shape_Sequence_Data_Chunk(Anim_Shape_Sequence_Chunk* const parent,const char* data,size_t const/*datasize*/)
: Chunk(parent, "ANISEQDT")
{
	SequenceNum=*(int*)data;
	data+=4;
	flags=*(int*)data;
	data+=4;
	pad2=*(int*)data;
	data+=4;
	pad3=*(int*)data;
	data+=4;
	pad4=*(int*)data;
	data+=4;
	name=new char[strlen(data)+1];
	strcpy(name,data);
					
}					  

Anim_Shape_Sequence_Data_Chunk::Anim_Shape_Sequence_Data_Chunk(Anim_Shape_Sequence_Chunk* const parent,ChunkAnimSequence* cas)
: Chunk(parent, "ANISEQDT")
{
	SequenceNum=cas->SequenceNum;
	name=cas->name;
	flags=cas->flags;
	pad2=cas->pad2;
	pad3=cas->pad3;
	pad4=cas->pad4;

}

void Anim_Shape_Sequence_Data_Chunk::fill_data_block(char* datastart)
{
	strncpy (datastart, identifier, 8);
	datastart += 8;
	*((int *) datastart) = chunk_size;
	datastart += 4;

	*(int*)datastart=SequenceNum;
	datastart+=4;
	*(int*)datastart=flags;
	datastart+=4;
	*(int*)datastart=pad2;
	datastart+=4;
	*(int*)datastart=pad3;
	datastart+=4;
	*(int*)datastart=pad4;
	datastart+=4;
	
	strcpy(datastart,name ? name : "");

}

size_t Anim_Shape_Sequence_Data_Chunk::size_chunk()
{
	return chunk_size =	12+20
		+(name ? strlen(name) : 0)
		+3 +3&~3;
}


/////////////////////////////////////////

// Class Anim_Shape_Frame_Data_Chunk functions
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("ANIFRADT",Anim_Shape_Frame_Data_Chunk,"ANIMFRAM",Anim_Shape_Frame_Chunk)

Anim_Shape_Frame_Data_Chunk::Anim_Shape_Frame_Data_Chunk(Anim_Shape_Frame_Chunk* const parent,const char* data,size_t const /*datasize*/)
: Chunk(parent, "ANIFRADT")
{
	FrameNum=*(int*)data;
	data+=4;
	flags=*(int*)data;
	data+=4;
	num_interp_frames=*(int*)data;
	data+=4;
	pad3=*(int*)data;
	data+=4;
	pad4=*(int*)data;
	data+=4;
	name=new char[strlen(data)+1];
	strcpy(name,data);
					
}					  

Anim_Shape_Frame_Data_Chunk::Anim_Shape_Frame_Data_Chunk(Anim_Shape_Frame_Chunk* const parent,ChunkAnimFrame* caf,int frameno)
: Chunk(parent, "ANIFRADT")
{
	FrameNum=frameno;
	name=caf->name;
	flags=caf->flags;
	num_interp_frames=caf->num_interp_frames;
	pad3=caf->pad3;
	pad4=caf->pad4;

}


void Anim_Shape_Frame_Data_Chunk::fill_data_block(char* datastart)
{
	strncpy (datastart, identifier, 8);
	datastart += 8;
	*((int *) datastart) = chunk_size;
	datastart += 4;

	*(int*)datastart=FrameNum;
	datastart+=4;
	*(int*)datastart=flags;
	datastart+=4;
	*(int*)datastart=num_interp_frames;
	datastart+=4;
	*(int*)datastart=pad3;
	datastart+=4;
	*(int*)datastart=pad4;
	datastart+=4;
	
	strcpy(datastart,name ? name : "");

}

size_t Anim_Shape_Frame_Data_Chunk::size_chunk()
{
	return chunk_size =	12+20
		+(name ? strlen(name) : 0)
		+3 +3&~3;
}

/////////////////////////////////////////

// Class Anim_Shape_Alternate_Texturing_Chunk functions
RIF_IMPLEMENT_DYNCREATE("ASALTTEX",Anim_Shape_Alternate_Texturing_Chunk)

/*
Children for Anim_Shape_Alternate_Texturing_Chunk :
"SUBSHAPE"		Shape_Sub_Shape_Chunk
*/

Anim_Shape_Alternate_Texturing_Chunk::Anim_Shape_Alternate_Texturing_Chunk(Chunk_With_Children *const parent, const char *data, size_t const size)
: Chunk_With_Children (parent, "ASALTTEX")
{
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}
		DynCreate(data);
		data += *(int *)(data + 8);
	}
}

Shape_Sub_Shape_Chunk* Anim_Shape_Alternate_Texturing_Chunk::CreateNewSubShape(const char* name)
{
	if(!name) return 0;
	//check to make sure that name isn't already in use
	List<Chunk*> chlist;
	lookup_child("SUBSHAPE",chlist);
	for(LIF<Chunk*> chlif(& chlist);!chlif.done();chlif.next())
	{
		Shape_Sub_Shape_Chunk* sssc=(Shape_Sub_Shape_Chunk*) chlif();
		if(!_stricmp(name,sssc->get_shape_name()))return 0;
	}
	ChunkShape cs;
	if(!_stricmp(parent->identifier,"REBSHAPE"))
		cs=((Shape_Chunk*)parent)->shape_data;
	else if(!_stricmp(parent->identifier,"SUBSHAPE"))
		cs=((Shape_Sub_Shape_Chunk*)parent)->shape_data;
	else
		return 0;
	Shape_Sub_Shape_Chunk* sssc=new Shape_Sub_Shape_Chunk(this,cs);
	new Shape_Name_Chunk(sssc,name);

	
	//copy the texture animation data
	Chunk_With_Children* anim_chunk=(Chunk_With_Children*)parent->lookup_single_child("TEXTANIM");
	if(anim_chunk)
	{
		char * tempbuffer = anim_chunk->make_data_block_from_chunk();
		sssc->DynCreate(tempbuffer);
		delete [] tempbuffer;

	}

	Shape_Merge_Data_Chunk* smdc=(Shape_Merge_Data_Chunk*)parent->lookup_single_child("SHPMRGDT");
	if(smdc)
	{
		new Shape_Merge_Data_Chunk(sssc,smdc->merge_data,smdc->num_polys);	
	}
	return sssc;
	
}

/////////////////////////////////////////

// Class Poly_Not_In_Bounding_Box_Chunk functions
RIF_IMPLEMENT_DYNCREATE("PNOTINBB",Poly_Not_In_Bounding_Box_Chunk)

Poly_Not_In_Bounding_Box_Chunk::Poly_Not_In_Bounding_Box_Chunk(Chunk_With_Children* const parent,const char* data,size_t const datasize)
:Chunk(parent,"PNOTINBB")
{
	for(int i=0;i<( ( (signed)datasize ) /4)-2;i++)
	{
		poly_no.add_entry(*(int*)data);
		data+=4;
	}
	pad1=*(int*)data;
	data+=4;
	pad2=*(int*)data;

}

Poly_Not_In_Bounding_Box_Chunk::Poly_Not_In_Bounding_Box_Chunk(Chunk_With_Children* const parent)
:Chunk(parent,"PNOTINBB")
{
	pad1=pad2=0;
}

size_t Poly_Not_In_Bounding_Box_Chunk::size_chunk()
{
	return chunk_size=(20+4*poly_no.size());
}

void Poly_Not_In_Bounding_Box_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	for(LIF<int> plif(&poly_no);!plif.done();plif.next())
	{
		*(int*)data_start=plif();
		data_start+=4;
	}
	*(int*)data_start=pad1;
	data_start+=4;
	*(int*)data_start=pad2;
}


/////////////////////////////////////////

// Class Anim_Shape_Centre_Chunk functions

RIF_IMPLEMENT_DYNCREATE("ANSHCEN2",Anim_Shape_Centre_Chunk)

Anim_Shape_Centre_Chunk::Anim_Shape_Centre_Chunk(Chunk_With_Children* const parent,const char* data,size_t const /*datasize*/)
:Chunk(parent,"ANSHCEN2")
{
	Centre=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);
	flags=*(int*)data;
	data+=4;
	pad2=*(int*)data;

}

Anim_Shape_Centre_Chunk::Anim_Shape_Centre_Chunk(Chunk_With_Children* const parent)
:Chunk(parent,"ANSHCEN2")
{
	Centre.x=0;
	Centre.y=0;
	Centre.z=0;
	flags=pad2=0;
}

size_t Anim_Shape_Centre_Chunk::size_chunk()
{
	return chunk_size=(20+sizeof(ChunkVectorInt));
}

void Anim_Shape_Centre_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(ChunkVectorInt*)data_start=Centre;
	data_start+=sizeof(ChunkVectorInt);
	*(int*)data_start=flags;
	data_start+=4;
	*(int*)data_start=pad2;

}



/////////////////////////////////////////

// Class Console_Shape_Chunk functions

/*
Children for Console_Shape_Chunk :

"SHPPOLYS"		Shape_Polygon_Chunk
"SHPUVCRD"		Shape_UV_Coord_Chunk
"TEXTANIM"		Animation_Chunk

"CONSTYPE"		Console_Type_Chunk
*/


Console_Shape_Chunk::Console_Shape_Chunk(Chunk_With_Children * const parent, const char *data, size_t  size)
: Chunk_With_Children (parent, "CONSHAPE")
{
	
	shape_data_store=(ChunkShape*)&shape_data;
	
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed)size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed)size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
	
		DynCreate(data);
		data += *(int *)(data + 8);
		}
	}
	
}

Console_Shape_Chunk::Console_Shape_Chunk(Chunk_With_Children* const parent,Console_Type ct)
:Chunk_With_Children(parent,"CONSHAPE")
{
	shape_data_store=(ChunkShape*)&shape_data;
	new Console_Type_Chunk(this,ct);
}
void Console_Shape_Chunk::generate_console_chunkshape()
{
	//sort out the chunkshape
	if(!strcmp(parent->identifier,"REBSHAPE"))
	{
		*shape_data_store=((Shape_Chunk*)parent)->shape_data;
	}
	else if(!strcmp(parent->identifier,"SUBSHAPE"))
	{
		*shape_data_store=((Shape_Sub_Shape_Chunk*)parent)->shape_data;
	}
	else
	{
		delete this;
		return;
	}
	
	Shape_Polygon_Chunk* spc=(Shape_Polygon_Chunk*)lookup_single_child("SHPPOLYS");
	if(spc)
	{
		if(spc->num_polys!=shape_data_store->num_polys)
		{
			//console shape is no longer valid.kill it.
			delete this;
			return;
		}
		
		ChunkPoly* poly_list=shape_data_store->poly_list;
		shape_data_store->poly_list=(ChunkPoly*)spc->poly_data;
		for(int i=0;i<shape_data_store->num_polys;i++)
		{
			for(int j=0;j<4;j++)
			{
				shape_data_store->poly_list[i].vert_ind[j]=poly_list[i].vert_ind[j];
			}
		}
		delete [] poly_list;
	}
	Shape_UV_Coord_Chunk* succ=(Shape_UV_Coord_Chunk*)lookup_single_child("SHPUVCRD");
	if(succ)
	{
		delete [] shape_data_store->uv_list;
		shape_data_store->uv_list=(ChunkUV_List*)succ->uv_data;
		shape_data_store->num_uvs=succ->num_uvs;
	}
}

void Console_Shape_Chunk::update_my_chunkshape(ChunkShape & cshp)
{
	List<Chunk*> chlist;
	lookup_child("SHPUVCRD",chlist);
	while(chlist.size())
	{
		delete chlist.first_entry();
		chlist.delete_first_entry();
	}
	lookup_child("SHPPOLYS",chlist);
	while(chlist.size())
	{
		delete chlist.first_entry();
		chlist.delete_first_entry();
	}
	*shape_data_store=cshp;
	if (shape_data.poly_list) new Shape_Polygon_Chunk (this, shape_data.num_polys);
	if (shape_data.uv_list) new Shape_UV_Coord_Chunk (this, shape_data.num_uvs);

}


/////////////////////////////////////////

// Class Console_Type_Chunk functions
RIF_IMPLEMENT_DYNCREATE("CONSTYPE",Console_Type_Chunk)

void Console_Type_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(int*)data_start=(int)console;
}


/////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SHPPRPRO",Shape_Preprocessed_Data_Chunk)

Shape_Preprocessed_Data_Chunk::Shape_Preprocessed_Data_Chunk (Chunk_With_Children * parent, const char * data, size_t )
:Chunk(parent,"SHPPRPRO")
{
	block_size=*(int*)data;
	data+=4;
	first_pointer=*(int*)data;
	data+=4;
		
	if(block_size)
	{
		memory_block=new unsigned int[block_size];
		memcpy(memory_block,data,block_size*4);	
		data+=block_size*4;
	}
	else
	{
		memory_block=0;
	}
	
	num_extra_data=*(int*)data;
	data+=4;
	
	if(num_extra_data)
	{
		extra_data=new int[num_extra_data];
		memcpy(extra_data,data,num_extra_data*4);	
	}	
	else
	{
		extra_data=0;
	}		
}

Shape_Preprocessed_Data_Chunk::Shape_Preprocessed_Data_Chunk (Chunk_With_Children * parent,int _block_size,int _first_pointer,unsigned int* _memory_block)
:Chunk(parent,"SHPPRPRO")
{
	num_extra_data=0;
	extra_data=0;
	block_size=_block_size;
	first_pointer=_first_pointer;
	
	if(block_size)
	{
		memory_block=new unsigned int[block_size];
		memcpy(memory_block,_memory_block,block_size*4);	
	}
	else
	{
		memory_block=0;
	}

}

void Shape_Preprocessed_Data_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(int*)data_start=block_size;
	data_start+=4;
	*(int*)data_start=first_pointer;
	data_start+=4;

	memcpy(data_start,memory_block,4*block_size);
	data_start+=4*block_size;

	*(int*)data_start=num_extra_data;
	data_start+=4;
	memcpy(data_start,extra_data,4*num_extra_data);
}

void* Shape_Preprocessed_Data_Chunk::GetMemoryBlock()
{
	void* retval=memory_block;

// 64HACK
#pragma message ("64HACK")
#if 0
	unsigned int* current=(unsigned int*)&first_pointer;
	unsigned int* next;
	while((*current>>16)!=0xffff)
	{
		next=&memory_block[(*current)>>16];
		*current=(unsigned int)&memory_block[(*current)&0xffff];
		current=next;
	}
	*current=(unsigned int)&memory_block[(*current)&0xffff];
#endif
	memory_block=0;
	block_size=0;
	return retval;
}

