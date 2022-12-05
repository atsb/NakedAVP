#include "chunk.hpp"
#include "envchunk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(envchunk)

// Class Environment_Data_Chunk functions
RIF_IMPLEMENT_DYNCREATE("REBENVDT",Environment_Data_Chunk)

// constructor from buffer
LOCKABLE_CHUNK_WITH_CHILDREN_LOADER("REBENVDT",Environment_Data_Chunk)
/*
Children for Enviornment_Data_Chunk :

"ENVSDSCL"		Environment_Scale_Chunk
"GAMEMODE"		Environment_Game_Mode_Chunk
"ENVPALET"		Environment_Palette_Chunk
"ENVTXLIT"		Environment_TLT_Chunk
"TLTCONFG"		TLT_Config_Chunk
"CLRLOOKP"		Coloured_Polygons_Lookup_Chunk
"MATCHIMG"		Matching_Images_Chunk
"BMPNAMES"		Global_BMP_Name_Chunk
"BMNAMVER"		BMP_Names_Version_Chunk
"BMNAMEXT"		BMP_Names_ExtraData_Chunk
"RIFFNAME"		RIF_Name_Chunk
"ENDTHEAD"		Environment_Data_Header_Chunk
"LIGHTSET"		Light_Set_Chunk
"PRSETPAL"		Preset_Palette_Chunk
"SPECLOBJ"		Special_Objects_Chunk
"AVPEXSTR"		AVP_External_Strategy_Chunk
"BMPMD5ID"		Bitmap_MD5_Chunk
"GLOGENDC"		Global_Generator_Data_Chunk
"FRAGTYPE"		Fragment_Type_Chunk
"ENVACOUS"		Environment_Acoustics_Chunk
"AVPENVIR"		AVP_Environment_Settings_Chunk
"SOUNDDIR"		Sound_Directory_Chunk
"RANTEXID"		Random_Texture_ID_Chunk
*/




// empty constructor
Environment_Data_Chunk::Environment_Data_Chunk (Chunk_With_Children  * parent)
:Lockable_Chunk_With_Children (parent, "REBENVDT")
{
	// as necessary, generated automatically
	new Environment_Data_Header_Chunk (this);
}


BOOL Environment_Data_Chunk::file_equals (HANDLE & /*rif_file*/)
{
	return(TRUE);
}	

Environment_Data_Header_Chunk * Environment_Data_Chunk::get_header()
{
	return (Environment_Data_Header_Chunk *) this->lookup_single_child ("ENDTHEAD");
}	

const char * Environment_Data_Chunk::get_head_id()
{
	Environment_Data_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (0);

	return(hdptr->identifier);
	
}	

void Environment_Data_Chunk::set_lock_user (char * user)
{
	Environment_Data_Header_Chunk * hdptr = get_header();

	if (!hdptr) return;

	strncpy (hdptr->lock_user, user,16);

	hdptr->lock_user[16] = 0;
}
	
void Environment_Data_Chunk::post_input_processing()
{
	if (get_header())
		if (get_header()->flags & GENERAL_FLAG_LOCKED)
			external_lock = TRUE;

	Chunk_With_Children::post_input_processing();

}

///////////////////////////////////////

// Class Environment_Data_Header_Chunk functions
RIF_IMPLEMENT_DYNCREATE("ENDTHEAD",Environment_Data_Header_Chunk)

// from buffer
Environment_Data_Header_Chunk::Environment_Data_Header_Chunk (Chunk_With_Children * parent, const char * hdata, size_t /*hsize*/)
	: Chunk (parent, "ENDTHEAD"),
	flags (0), version_no (0)
{
	flags = *((int *) hdata);

	strncpy (lock_user, (hdata + 4), 16);
	lock_user[16] = '\0';

	version_no = *((int *) (hdata + 20));
}	

BOOL Environment_Data_Header_Chunk::output_chunk (HANDLE & hand)
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

void Environment_Data_Header_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	strncpy ((data_start + 4), lock_user, 16);

	*((int *) (data_start+20)) = version_no;
}

void Environment_Data_Header_Chunk::prepare_for_output()
{
	version_no ++;
}

///////////////////////////////////////

// Class Environment_Scale_Chunk functions
RIF_IMPLEMENT_DYNCREATE("ENVSDSCL",Environment_Scale_Chunk)

void Environment_Scale_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((double *) data_start) = scale;
}

///////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("ENVACOUS",Environment_Acoustics_Chunk)

Environment_Acoustics_Chunk::Environment_Acoustics_Chunk(Environment_Data_Chunk* parent)
:Chunk(parent,"ENVACOUS")
{
	env_index=0; 
	reverb=2;
	spare=0;
}

Environment_Acoustics_Chunk::Environment_Acoustics_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"ENVACOUS")
{
	env_index=*(int*)data;
	data+=4;
	reverb=*(float*)data;
	data+=4;
	spare=*(int*)data;
}

void Environment_Acoustics_Chunk::fill_data_block(char* data_start)
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

RIF_IMPLEMENT_DYNCREATE("SOUNDDIR",Sound_Directory_Chunk)

Sound_Directory_Chunk::Sound_Directory_Chunk(Environment_Data_Chunk* parent,const char* dir)
:Chunk(parent,"SOUNDDIR")
{
	directory=new char[strlen(dir)+1];
	strcpy(directory,dir);
}

Sound_Directory_Chunk::Sound_Directory_Chunk(Chunk_With_Children * const parent, const char* data, size_t const )
:Chunk(parent,"SOUNDDIR")
{
	directory=new char[strlen(data)+1];
	strcpy(directory,data);
}

Sound_Directory_Chunk::~Sound_Directory_Chunk()
{
	delete [] directory;
}

void Sound_Directory_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,directory);
}


///////////////////////////////////////
/////////////////////Available shape set collections////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("RANTEXID",Random_Texture_ID_Chunk)

Random_Texture_ID_Chunk::Random_Texture_ID_Chunk(Chunk_With_Children* parent,const char* _name)
:Chunk(parent,"RANTEXID")
{
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
	spare1=spare2=0;
}


Random_Texture_ID_Chunk::Random_Texture_ID_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"RANTEXID")
{
	CHUNK_EXTRACT_STRING(name);
	
	int num_types,type;		
	CHUNK_EXTRACT(num_types,int);
	for(int i=0;i<num_types;i++)
	{
		CHUNK_EXTRACT(type,int);
		random_types.add_entry(type);
	}
	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);

}

Random_Texture_ID_Chunk::~Random_Texture_ID_Chunk()
{
	delete [] name;
}

void Random_Texture_ID_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START

	CHUNK_FILL_STRING(name)

	CHUNK_FILL(random_types.size(),int);
	for(LIF<int> rlif(&random_types);!rlif.done();rlif.next())
	{
		CHUNK_FILL(rlif(),int)
	}

	CHUNK_FILL(spare1,int)
	CHUNK_FILL(spare2,int)

}

size_t Random_Texture_ID_Chunk::size_chunk()
{
	chunk_size=12+12+random_types.size()*4;
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}
///////////////////////////////////////




//Class Special_Objects_Chunk :
RIF_IMPLEMENT_DYNCREATE("SPECLOBJ",Special_Objects_Chunk)

CHUNK_WITH_CHILDREN_LOADER("SPECLOBJ",Special_Objects_Chunk)
/*
Children for Special_Objects_Chunk :
"AVPGENER"		AVP_Generator_Chunk
"SOUNDOB2"		Sound_Object_Chunk
"VIOBJECT"		Virtual_Object_Chunk
"AVPCABLE"		AVP_Power_Cable_Chunk
"AVPSTART"		AVP_Player_Start_Chunk
"AVPPATH2"		AVP_Path_Chunk
"AVPGENEX"		AVP_Generator_Extra_Data_Chunk
"SOUNDEXD"		Sound_Object_Extra_Data_Chunk
"PARGENER"		AVP_Particle_Generator_Chunk
"PLACHIER"		Placed_Hierarchy_Chunk
"CAMORIGN"		Camera_Origin_Chunk
"AVPDECAL"		AVP_Decal_Chunk
"R6WAYPNT"		R6_Waypoint_Chunk

*/


