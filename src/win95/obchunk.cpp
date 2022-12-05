#include "chunk.hpp"
#include "chnktype.hpp"
#include "mishchnk.hpp"
#include "obchunk.hpp"
#include "shpchunk.hpp"
#include "envchunk.hpp"
#include "md5.h"
// Class Object_Chunk functions

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(obchunk)

RIF_IMPLEMENT_DYNCREATE("RBOBJECT",Object_Chunk)

/*
Children for Object_Chunk :

"OBJHEAD1"		Object_Header_Chunk
"OBINTDT"		Object_Interface_Data_Chunk
"OBJPRJDT"		Object_Project_Data_Chunk
"MODULEDT"		Object_Module_Data_Chunk
"SHPVTINT"		Shape_Vertex_Intensities_Chunk
"OBJTRAK2"		Object_Track_Chunk2
"TRAKSOUN"		Object_Track_Sound_Chunk
"OBANSEQS"		Object_Animation_Sequences_Chunk
"PLOBJLIT"		Placed_Object_Light_Chunk
"ALTLOCAT"		Object_Alternate_Locations_Chunk
*/





// from buffer
Object_Chunk::Object_Chunk(Chunk_With_Children * parent, const char *data, size_t size)
: Lockable_Chunk_With_Children (parent, "RBOBJECT"),
 object_data ()
{
	const char * buffer_ptr = data;

	object_data_store = (ChunkObject *) &object_data;
	
	while ((data-buffer_ptr)< (signed) size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(data);
		data += *(int *)(data + 8);

	}

	program_object_index=-1;

}

// from data
Object_Chunk::Object_Chunk (Chunk_With_Children * parent, ChunkObject &obj)
: Lockable_Chunk_With_Children (parent, "RBOBJECT"), 
	object_data (obj)
{
	object_data_store = (ChunkObject *) &object_data;
	object_data_store->index_num=-1;

	new Object_Header_Chunk(this);
	new Object_Interface_Data_Chunk(this);

	//if the parent is a file_chunk get an object index
	if(!strcmp(parent->identifier,"REBINFF2"))
	{
		((File_Chunk*)parent)->assign_index_to_object(this);
	}
	program_object_index=-1;

	CalculateID();
}

Object_Chunk::~Object_Chunk()
{
}

Object_Header_Chunk * Object_Chunk::get_header()
{
	
	return (Object_Header_Chunk *) this->lookup_single_child ("OBJHEAD1");


}

Shape_Chunk * Object_Chunk::get_assoc_shape()
{
	if (!get_header()) return 0;

	return get_header()->associated_shape;

}

BOOL Object_Chunk::assoc_with_shape (Shape_Chunk * shpch)
{
	Object_Header_Chunk * obhead = get_header();
	
	if (!obhead) return 0;

	// check that we are not already associated with this shape

	if (obhead->associated_shape == shpch) return 1;

	Shape_Header_Chunk * shphead = shpch->get_header();

	if (!shphead) return 0;

	// deassociate with the old shape

	if (obhead->associated_shape)
		deassoc_with_shape (obhead->associated_shape);

	// associate with the new

	obhead->associated_shape = shpch;

	if (!shphead->associated_objects_store.contains(this))
		shphead->associated_objects_store.add_entry(this);

	return 1;

}

BOOL Object_Chunk::deassoc_with_shape (Shape_Chunk * shpch)
{
	Shape_Header_Chunk * shphead = shpch->get_header();

	if (!shphead) return 0;

	if (shphead->associated_objects_store.contains(this))
		shphead->associated_objects_store.delete_entry(this);
	
 	Object_Header_Chunk * obhead = get_header();

	obhead->associated_shape = 0;
	
	return 1;

}

BOOL Object_Chunk::file_equals(HANDLE &rif_file)
{
	unsigned long bytes_read;
	char name[50];

	// get header list
	List<int> obhead;
	list_chunks_in_file (&obhead, rif_file, "OBJHEAD1");

	if (obhead.size() != 1) return FALSE;

	// get object identifier
	SetFilePointer(rif_file,obhead.first_entry() + 96,0,FILE_BEGIN);
	int i = 0; 
	do ReadFile (rif_file, (long *) (name + i), 1, &bytes_read, 0);
	while (name[i++] != 0);

	if (!strcmp(name, object_data.o_name) ) return (TRUE);

	return (FALSE);
}	

const char * Object_Chunk::get_head_id()
{
	Object_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (0);

	return(hdptr->identifier);
}	

void Object_Chunk::set_lock_user (char * user)
{
	Object_Header_Chunk * hdptr = get_header();
	
	if (!hdptr) return;

	strncpy (hdptr->lock_user, user,16);

	hdptr->lock_user[16] = 0;
}	


BOOL Object_Chunk::inc_v_no ()
{
	Object_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (FALSE);

	hdptr->version_no++;

	return (TRUE);
}

BOOL Object_Chunk::same_and_updated(Object_Chunk & obj)
{
	
	Object_Header_Chunk * hd1ptr = get_header();

	if (!hd1ptr) return (0);

	Object_Header_Chunk * hd2ptr = obj.get_header();

	if (!hd2ptr) return (0);

	return (hd1ptr->version_no < hd2ptr->version_no && 
		(!strcmp(obj.object_data.o_name, object_data.o_name)) );

}

BOOL Object_Chunk::assoc_with_shape_no(File_Chunk *fc)
{
	Object_Header_Chunk * hdptr = get_header();
	Shape_Chunk * shp = NULL;
	Shape_Header_Chunk * shphd;
	
	if (!hdptr) return (FALSE);

	List<Chunk *> chlst;
	fc->lookup_child("REBSHAPE",chlst);
	
	LIF<Chunk *> l(&chlst);
	for (; !l.done(); l.next())
	{
		shp = (Shape_Chunk *)l();
		shphd = shp->get_header();
		if (shphd)
		{
			if (hdptr->shape_id_no == shphd->file_id_num)
				break;
		}
	}
	if (!l.done())
	{
		assoc_with_shape(shp);
	}
	else
	{
		return(FALSE);
	}	

	return(TRUE);
	
	
}




void Object_Chunk::post_input_processing()
{
	CalculateID();	
	
	if (get_header())
		if (get_header()->flags & GENERAL_FLAG_LOCKED)
			external_lock = TRUE;

	Chunk_With_Children::post_input_processing();

}


VModule_Array_Chunk * Object_Chunk::get_vmod_chunk()
{
	Object_Module_Data_Chunk * omdc = 0;
	
 	omdc = (Object_Module_Data_Chunk *) lookup_single_child ("MODULEDT");
	
	
	if (omdc)
	{
		VModule_Array_Chunk * vmac = 0;
		vmac = (VModule_Array_Chunk *)omdc->lookup_single_child ("VMDARRAY") ;
		
		if (vmac) return(vmac);
	}
	
	return(0);
	
	
}

void Object_Chunk::destroy(File_Chunk * fc)
{
	List<Chunk *> cl;
	fc->lookup_child("RBOBJECT",cl);
	
	for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
	{
		Object_Chunk * ob = (Object_Chunk *)cli();
		if (ob == this) continue;
		
		VModule_Array_Chunk * vmac = ob->get_vmod_chunk();
		
		BOOL in_object_va = FALSE;
		
		if (vmac)
		{
			int pos=0;
			for(int i=0;i<vmac->num_array_items;i++)
			{
								
				if (vmac->vmod_array[i].object_index==object_data.index_num)
				{
					in_object_va=TRUE;
					//update branch indeces for branches that go beyond this point
					for(int j=0;j<vmac->num_array_items;j++)
					{
						if(vmac->vmod_array[j].branch_no> pos)
							vmac->vmod_array[j].branch_no--;
					}		
				}
				else
				{
					if(pos!=i)
					{
						vmac->vmod_array[pos]=vmac->vmod_array[i];
					}
					pos++;
				}					
			}

			if(in_object_va)
			{
				if(pos==0)
				{
					delete vmac;
				}
				else
				{
					//excess entries should be properly deleted when the array is deleted
					vmac->num_array_items=pos;
				}
					
			}
				
		}

		//now remove this object from any adjacency data

		Object_Module_Data_Chunk* omdc=(Object_Module_Data_Chunk*)ob->lookup_single_child("MODULEDT");
		if(omdc)
		{
			Adjacent_Module_Entry_Points_Chunk* amepc=(Adjacent_Module_Entry_Points_Chunk*)omdc->lookup_single_child("ADJMDLEP");
		
			if(amepc)
			{
				for(LIF<Adjacent_Module> ad_lif(&amepc->adjacent_modules_list);!ad_lif.done();)
				{
					if(ad_lif().object_index==object_data.index_num)
						ad_lif.delete_current();
					else
						ad_lif.next();
										
				}
			}
		}
		
		
	}
	
	Shape_Chunk * shp = get_assoc_shape();
	
	deassoc_with_shape (shp);

	lock_chunk(*fc);
	
	deleted = TRUE;

	unlock_chunk(*fc,TRUE);
	
}


void Object_Chunk::update_my_chunkobject(ChunkObject & cob)
{
	//store the object's index , so that it doesn't get lost
	int object_index=object_data_store->index_num;
	*object_data_store = cob;
	//replace the object index
	object_data_store->index_num=object_index;
	//recalculate the object's id in case the name has changed
	CalculateID();	
}

ObjectID Object_Chunk::CalculateID()
{
	ObjectID retval={0,0};
	List<Chunk*> chlist;
	parent->lookup_child("REBENVDT",chlist);
	if(!chlist.size()) return retval;
	((Environment_Data_Chunk*)chlist.first_entry())->lookup_child("RIFFNAME",chlist);
	if(!chlist.size()) return retval;
	char Name[100];

	strcpy(Name,((RIF_Name_Chunk*)chlist.first_entry())->rif_name);

	strcat(Name,object_data.o_name);
	char buffer[16];
	md5_buffer(Name,strlen(Name),&buffer[0]);
	buffer[7]=0;
	object_data_store->ID=*(ObjectID*)&buffer[0];
	return 	object_data_store->ID;
	
}

/////////////////////////////////////////

// Class Object_Header_Chunk functions
RIF_IMPLEMENT_DYNCREATE("OBJHEAD1",Object_Header_Chunk)

// from buffer
Object_Header_Chunk::Object_Header_Chunk(Chunk_With_Children * parent, const char * hdata, size_t /*hsize*/)
: Chunk (parent, "OBJHEAD1"), object_data (((Object_Chunk*)parent)->object_data_store),
flags(0), version_no (0), associated_shape (0)
{
	Object_Chunk* parent_object = (Object_Chunk*) parent;

	flags = *((int *) hdata);

	if (flags & OBJECT_FLAG_BASE_OBJECT)
		parent_object->object_data_store->is_base_object = TRUE;
	else
	{
		parent_object->object_data_store->is_base_object = FALSE;
	}

	strncpy (lock_user, (hdata + 4), 16);
	lock_user[16] = '\0';

	hdata+=20;

	parent_object->object_data_store->location = *(ChunkVectorInt*) hdata;
	hdata+=sizeof(ChunkVectorInt);

	parent_object->object_data_store->orientation = *((ChunkQuat *) hdata);
	hdata+=sizeof(ChunkQuat);

	parent_object->object_data_store->index_num = *(int *) hdata;
	hdata+=4;

	version_no = *((int *) hdata);
	hdata+=4;

	shape_id_no = *((int *) hdata);
	hdata+=4;

	strcpy (parent_object->object_data_store->o_name, hdata);
	parent_object->object_data_store->ID.id1=0;
	parent_object->object_data_store->ID.id2=0;
}

//from data

Object_Header_Chunk::Object_Header_Chunk (Object_Chunk * parent)
: Chunk (parent, "OBJHEAD1"),
object_data (parent->object_data_store), 
flags (0), version_no (0), associated_shape(0)
{
	if (object_data->is_base_object) {
		flags = OBJECT_FLAG_BASE_OBJECT;
	}
}


size_t Object_Header_Chunk::size_chunk()
{
	int length = 72;

	length += (strlen(object_data->o_name)+1);

	length += (4-length%4)%4;

	chunk_size = length;

	return length;
}

BOOL Object_Header_Chunk::output_chunk (HANDLE & hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}

void Object_Header_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	strncpy ((data_start + 4), lock_user, 16);
	data_start+=20;

	*((ChunkVectorInt *) (data_start)) = object_data->location;
	data_start+=sizeof(ChunkVectorInt);
		
	*((ChunkQuat *) (data_start)) = object_data->orientation;
	data_start+=sizeof(ChunkQuat);

	*(int*) data_start=object_data->index_num;
	data_start+=4;

	*((int *) data_start) = version_no;
	data_start+=4;
	
	*((int *) data_start) = shape_id_no;
	data_start+=4;

	strcpy (data_start, object_data->o_name);
}

void Object_Header_Chunk::prepare_for_output()
{
	version_no ++;
}

/////////////////////////////////////////

// Class Object_Interface_Data_Chunk functions
RIF_IMPLEMENT_DYNCREATE("OBINTDT",Object_Interface_Data_Chunk)

/*
Children for Object_Interface_Data_Chunk :

"OBJNOTES"		Object_Notes_Chunk
*/

// from buffer
Object_Interface_Data_Chunk::Object_Interface_Data_Chunk(Chunk_With_Children * parent, const char *data, size_t size)
: Chunk_With_Children (parent, "OBINTDT")
{
	const char * buffer_ptr = data;

	while ((data-buffer_ptr)< (signed) size) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(data);
		data += *(int *)(data + 8);
	}
}

Object_Interface_Data_Chunk::Object_Interface_Data_Chunk (Object_Chunk * parent)
: Chunk_With_Children (parent, "OBINTDT")
{
	new Object_Notes_Chunk (this, "Enter notes here", strlen("Enter notes here") + 1);
}


///////////////////////////////////////

// Class Object_Notes_Chunk functions
RIF_IMPLEMENT_DYNCREATE("OBJNOTES",Object_Notes_Chunk)

Object_Notes_Chunk::Object_Notes_Chunk (Chunk_With_Children * parent,
 const char * _data, size_t _data_size) 
: Chunk(parent, "OBJNOTES"), data_size(_data_size), data(NULL)
{
	data_store = new char [data_size];

	*((char **) &data) = data_store;

	for (int i = 0; i<(signed) data_size; i++)
		data_store[i] = _data[i];

}

void Object_Notes_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i = 0; i<((signed) chunk_size-12); i++)
		data_start[i] = data[i];
}

Object_Notes_Chunk::~Object_Notes_Chunk ()
{
	delete [] data_store;
}

BOOL Object_Notes_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}


///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("MODULEDT",Object_Module_Data_Chunk)

/*
Children For Object_Module_Data_Chunk :

"VMDARRAY"		VModule_Array_Chunk
"ADJMDLEP"		Adjacent_Module_Entry_Points_Chunk
"MODFLAGS"		Module_Flag_Chunk
"WAYPOINT"		Module_Waypoint_Chunk
"MODACOUS"		Module_Acoustics_Chunk
"AIMODMAS"		AI_Module_Master_Chunk
"AIMODSLA"		AI_Module_Slave_Chunk

*/

Object_Module_Data_Chunk::Object_Module_Data_Chunk (Chunk_With_Children * parent,const char * mdata, size_t msize)
: Chunk_With_Children (parent, "MODULEDT")
{
	const char * buffer_ptr = mdata;

	while ((mdata-buffer_ptr)< (signed) msize) {

		if ((*(int *)(mdata + 8)) + (mdata-buffer_ptr) > (signed) msize) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}
		DynCreate(mdata);
		mdata += *(int *)(mdata + 8);

	}
	
}

///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("VMDARRAY",VModule_Array_Chunk)

VModule_Array_Chunk::VModule_Array_Chunk(Object_Module_Data_Chunk * parent, VMod_Arr_Item * vma, int num_in_vma)
: Chunk (parent, "VMDARRAY")
{
	int i;
	
	num_array_items = num_in_vma;
	
	vmod_array = new VMod_Arr_Item [num_in_vma];
	
	for (i=0; i<num_in_vma; i++)
	{
		vmod_array[i] = vma[i];
	}
	
}


void VModule_Array_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = num_array_items;

	data_start += 4;
	
	for (int i=0; i<num_array_items; i++)
	{
		*((int *) data_start) = vmod_array[i].object_index;
		data_start += 4;
			
		*((int *) data_start) = vmod_array[i].branch_no;
		data_start += 4;
		
		*((int *) data_start) = vmod_array[i].flags;
		data_start += 4;

		*((int *) data_start) = vmod_array[i].spare;
		data_start += 4;
		
	}
	
}


size_t VModule_Array_Chunk::size_chunk()
{
	chunk_size =16+16*num_array_items;
	return(chunk_size);
		
}

VModule_Array_Chunk::VModule_Array_Chunk (Chunk_With_Children * parent, const char * vmdata, size_t /*vmsize*/)
: Chunk (parent, "VMDARRAY")
{
	num_array_items = *((int *) vmdata);

	vmdata += 4;
	
	vmod_array = new VMod_Arr_Item [num_array_items];

	for (int i=0; i<num_array_items; i++)
	{

		vmod_array[i].object_index = *((int *) vmdata);
		vmdata += 4;

		vmod_array[i].branch_no = *((int *) vmdata);
		vmdata += 4;
		
		vmod_array[i].flags = *((int *) vmdata);
		vmdata += 4;

		vmod_array[i].spare = *((int *) vmdata);
		vmdata += 4;
			
	}

}

VModule_Array_Chunk::~VModule_Array_Chunk ()
{
	delete [] vmod_array;
}

///////////////////////////////////////
#if 0
void Adjacent_Modules_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = adjacent_modules_list.size();

	data_start += 4;
	
	for (LIF<Adjacent_Module> ami(&adjacent_modules_list); !ami.done(); ami.next())
	{
	
		*((int *) data_start) = 0;
		data_start += 4;

		*((int *) data_start) = 0;
		data_start += 4;

		strcpy (data_start, ami().o_name);
		data_start += (strlen(ami().o_name) + 1) + (4-(strlen(ami().o_name) + 1)%4)%4;
		
	}
	
}

size_t Adjacent_Modules_Chunk::size_chunk()
{
	int size = 16;
	
	for (LIF<Adjacent_Module> ami(&adjacent_modules_list); !ami.done(); ami.next())
	{
		size += 8;
		size += (strlen(ami().o_name) + 1) + (4-(strlen(ami().o_name) + 1)%4)%4;
	}

	chunk_size = size;
	return(size);
		
}

Adjacent_Modules_Chunk::Adjacent_Modules_Chunk (Object_Module_Data_Chunk * parent, const char * data, size_t /*size*/)
: Chunk (parent, "ADJMDLST")
{
	int num_array_items = *((int *) data);

	data += 4;
	
	for (int i=0; i<num_array_items; i++)
	{
		Adjacent_Module am;

		data += 8;

		am.o_name = new char [strlen(data) + 1];
		strcpy (am.o_name, data);
		data += (strlen(am.o_name) + 1) + (4-(strlen(am.o_name) + 1)%4)%4;
		
		adjacent_modules_list.add_entry(am);	
			
	}
}
#endif
///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("ADJMDLEP",Adjacent_Module_Entry_Points_Chunk)

void Adjacent_Module_Entry_Points_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = adjacent_modules_list.size();

	data_start += 4;
	
	for (LIF<Adjacent_Module> ami(&adjacent_modules_list); !ami.done(); ami.next())
	{
	
		*((int *) data_start) = ami().object_index;
		data_start += 4;
		
		*((int *) data_start) = ami().flags;
		data_start += 4;

		*((ChunkVectorInt *) data_start) = ami().entry_point;
		data_start += sizeof(ChunkVectorInt);

		
	}
	
}


size_t Adjacent_Module_Entry_Points_Chunk::size_chunk()
{
	chunk_size = 16+adjacent_modules_list.size()*20;
	return(chunk_size);
}

Adjacent_Module_Entry_Points_Chunk::Adjacent_Module_Entry_Points_Chunk (Chunk_With_Children * parent, const char * data, size_t /*size*/)
: Chunk (parent, "ADJMDLEP")
{
	int num_array_items = *((int *) data);

	data += 4;
	
	for (int i=0; i<num_array_items; i++)
	{
		Adjacent_Module am;

		am.object_index = *((int *) data);
		data += 4;
		
		am.flags = *((int *) data);
		data += 4;

		am.entry_point = *((ChunkVectorInt *) data);
		data += sizeof(ChunkVectorInt);

		
		adjacent_modules_list.add_entry(am);	
			
	}

}

///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("MODFLAGS",Module_Flag_Chunk)

Module_Flag_Chunk::Module_Flag_Chunk(Object_Module_Data_Chunk* parent)
:Chunk(parent,"MODFLAGS")
{
	Flags=spare=0;
}

Module_Flag_Chunk::Module_Flag_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"MODFLAGS")
{
	Flags=*(int*)data;
	data+=4;
	spare=*(int*)data;
}

void Module_Flag_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*(int*)data_start=Flags;
	data_start+=4;
	*(int*)data_start=spare;
	
}
///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("MODZONE",Module_Zone_Chunk)

Module_Zone_Chunk::Module_Zone_Chunk(Object_Module_Data_Chunk* parent)
:Chunk(parent,"MODZONE")
{
	Zone=spare=0;
}

Module_Zone_Chunk::Module_Zone_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"MODZONE")
{
	Zone=*(int*)data;
	data+=4;
	spare=*(int*)data;
}

void Module_Zone_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*(int*)data_start=Zone;
	data_start+=4;
	*(int*)data_start=spare;
	
}

///////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("MODACOUS",Module_Acoustics_Chunk)

Module_Acoustics_Chunk::Module_Acoustics_Chunk(Object_Module_Data_Chunk* parent)
:Chunk(parent,"MODACOUS")
{
	env_index=-1; //negative means default environment acoustics
	reverb=-1.0;
	spare=0;
}

Module_Acoustics_Chunk::Module_Acoustics_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"MODACOUS")
{
	env_index=*(int*)data;
	data+=4;
	reverb=*(float*)data;
	data+=4;
	spare=*(int*)data;
}

void Module_Acoustics_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*(int*)data_start=env_index;
	data_start+=4;
	*(float*)data_start=reverb;
	data_start+=4;
	*(int*)data_start=spare;
	
}


///////////////////////////////////////


RIF_IMPLEMENT_DYNCREATE("OBJTRAK2",Object_Track_Chunk2)

Object_Track_Chunk2::Object_Track_Chunk2(Object_Chunk * parent)
: Chunk (parent, "OBJTRAK2")
{
	num_sections=0;
	sections=0;
	flags=timer_start=0;
}

Object_Track_Chunk2::~Object_Track_Chunk2()
{
	if(sections) delete [] sections;
}

void Object_Track_Chunk2::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start=num_sections;
	data_start+=4;

	for(int i=0;i<num_sections;i++)
	{
		*(ChunkTrackSection*)data_start=sections[i];
		data_start+=sizeof(ChunkTrackSection);
	}

	*(int*)data_start=flags;
	data_start+=4;
	
	*(int*)data_start=timer_start;

	
	
}

size_t Object_Track_Chunk2::size_chunk ()
{
	chunk_size = 12 + 12 +num_sections*sizeof(ChunkTrackSection) ;
	return(chunk_size);
}

Object_Track_Chunk2::Object_Track_Chunk2 (Chunk_With_Children * parent,const char * data, size_t /*size*/)
: Chunk (parent, "OBJTRAK2")
{
	int i;
	
	num_sections=*(int*)data;
	data+=4;

	if(num_sections)
		sections=new ChunkTrackSection[num_sections];
	else
		sections=0;

	for(i=0;i<num_sections;i++)
	{
		sections[i]=*(ChunkTrackSection*)data;
		data+=sizeof(ChunkTrackSection);
	}

	flags=*(int*)data;
	data+=4;
	timer_start=*(int*)data;

	if(!(flags & TrackFlag_QuatProblemSorted))
	{
		for(i=0;i<num_sections;i++)
		{
			ChunkQuat temp=sections[i].quat_start;
			sections[i].quat_start.x=-temp.w;
			sections[i].quat_start.y=temp.x;
			sections[i].quat_start.z=temp.y;
			sections[i].quat_start.w=-temp.z;

			temp=sections[i].quat_end;
			sections[i].quat_end.x=-temp.w;
			sections[i].quat_end.y=temp.x;
			sections[i].quat_end.z=temp.y;
			sections[i].quat_end.w=-temp.z;
		}
		flags|=TrackFlag_QuatProblemSorted;
	}
	
}

///////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("TRAKSOUN",Object_Track_Sound_Chunk)

Object_Track_Sound_Chunk::Object_Track_Sound_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"TRAKSOUN")
{
	wav_name=0;
	inner_range=5000;
	outer_range=40000;
	max_volume=127;
	pitch=0;
	flags=0;
	index=0;
}

Object_Track_Sound_Chunk::Object_Track_Sound_Chunk(Chunk_With_Children* const parent,const char* data,size_t const )
:Chunk(parent,"TRAKSOUN")
{
	inner_range=*(unsigned long*)data;
	data+=4;
	outer_range=*(unsigned long*)data;
	data+=4;

	max_volume=*(int*)data;
	data+=4;
	pitch=*(int*)data;
	data+=4;
	flags=*(int*)data;
	data+=4;
	index=*(int*)data;
	data+=4;

	wav_name=new char[strlen(data)+1];
	strcpy(wav_name,data);

}

Object_Track_Sound_Chunk::~Object_Track_Sound_Chunk()
{
	if(wav_name) delete [] wav_name;
}											 

void Object_Track_Sound_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;
	
	*(unsigned long*)data_start=inner_range;
	data_start+=4;
	*(unsigned long*)data_start=outer_range;
	data_start+=4;

	*(int*)data_start=max_volume;
	data_start+=4;
	*(int*)data_start=pitch;
	data_start+=4;
	*(int*)data_start=flags;
	data_start+=4;
	*(int*)data_start=index;
	data_start+=4;

	strcpy(data_start,wav_name);

}

size_t Object_Track_Sound_Chunk::size_chunk()
{
	chunk_size=12+24;
	chunk_size+=(strlen(wav_name)+4)&~3;
	return chunk_size;
}
///////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("ALTLOCAT",Object_Alternate_Locations_Chunk)

Object_Alternate_Locations_Chunk::Object_Alternate_Locations_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"ALTLOCAT")
{
	group=*(int*) data;
	data+=4;
	spare2=*(int*) data;
	data+=4;
	
	num_locations=*(int*) data;
	data+=4;

	if(num_locations)
		locations=new AltObjectLocation[num_locations];
	else
		locations=0;

	for(int i=0;i<num_locations;i++)
	{
		locations[i] = *(AltObjectLocation*) data;
		data+=sizeof(AltObjectLocation);
	}
}

Object_Alternate_Locations_Chunk::Object_Alternate_Locations_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"ALTLOCAT")
{
	num_locations=0;
	locations=0;
	group=spare2=0;
}

Object_Alternate_Locations_Chunk::~Object_Alternate_Locations_Chunk()
{
	if(locations) delete [] locations;
}

void Object_Alternate_Locations_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;

	*(int*)data=group;
	data+=4;
	
	*(int*)data=spare2;
	data+=4;
	
	
	*(int*) data = num_locations;
	data+=4;	
	
	for(int i=0;i<num_locations;i++)
	{
		*(AltObjectLocation*)data=locations[i];
		data+=sizeof(AltObjectLocation);
	}

}

size_t Object_Alternate_Locations_Chunk::size_chunk()
{
	chunk_size=24+num_locations*sizeof(AltObjectLocation);
	return chunk_size;
}


/////////////////////////////////////////
//class Object_Project_Data_Chunk

RIF_IMPLEMENT_DYNCREATE("OBJPRJDT",Object_Project_Data_Chunk)

CHUNK_WITH_CHILDREN_LOADER("OBJPRJDT",Object_Project_Data_Chunk)

/*
Children for Object_Project_Data_Chunk :

"MAPBLOCK"	Map_Block_Chunk
"STRATEGY"	Strategy_Chunk
"AVPSTRAT"	AVP_Strategy_Chunk
*/
