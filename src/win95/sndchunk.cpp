#include "chunk.hpp"
#include "envchunk.hpp"
#include "sndchunk.hpp"
#include "obchunk.hpp"
#include "md5.h"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(sndchunk)

RIF_IMPLEMENT_DYNCREATE("SOUNDOB2",Sound_Object_Chunk)

Sound_Object_Chunk::Sound_Object_Chunk (Chunk_With_Children * parent, 
	ChunkVectorInt & pos,
	const char * _name
	)
: Chunk (parent, "SOUNDOB2"), position (pos), 
	inner_range (0), outer_range (0), max_volume (0), pitch (0),
	flags (0), probability (0), pad3 (0),
	snd_name (0), wav_name (0)
{
	if (_name) 
	{
		snd_name = new char [strlen(_name)+1];
		strcpy (snd_name, _name);	
	}
	else
	{
		snd_name = new char [1];
		snd_name [0] = 0;
	}
}

Sound_Object_Chunk::~Sound_Object_Chunk()
{
	if (snd_name)
		delete [] snd_name;
	if (wav_name)
		delete [] wav_name;
}

Sound_Object_Chunk::Sound_Object_Chunk (Chunk_With_Children * parent, const char * data, size_t /*ssize*/)
: Chunk (parent, "SOUNDOB2"), snd_name (0), wav_name (0)
{
	position = *((ChunkVectorInt *) data);
	data += sizeof(ChunkVectorInt);

	inner_range = *((int *)data);
	data += 4;
	outer_range = *((int *)data);
	data += 4;
	max_volume = *((int *)data);
	data += 4;
	pitch = *((int *)data);
	data += 4;
	flags = *((int *)data);
	data += 4;
	probability = *((int *)data);
	data += 4;
	pad3 = *((int *)data);
	data += 4;
	snd_name = new char [strlen (data) + 1];
	strcpy (snd_name, data);
	data += strlen (data) + 1;
	if (strlen(data))
	{
		wav_name = new char [strlen (data) + 1];
		strcpy (wav_name, data);
	}
}

void Sound_Object_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((ChunkVectorInt *) data_start) = position;
	data_start += sizeof(ChunkVectorInt);
	

	*((int *)data_start) = inner_range;
	data_start += 4;
	*((int *)data_start) = outer_range;
	data_start += 4;
	*((int *)data_start) = max_volume;
	data_start += 4;
	*((int *)data_start) = pitch;
	data_start += 4;
	*((int *)data_start) = flags;
	data_start += 4;
	*((int *)data_start) = probability;
	data_start += 4;
	*((int *)data_start) = pad3;
	data_start += 4;

	sprintf (data_start, "%s", snd_name);	
	data_start += strlen (snd_name) + 1;
	if (wav_name)
	{
		sprintf (data_start, "%s", wav_name);	
	}
	else
	{
		*data_start = 0;
	}
}


ObjectID Sound_Object_Chunk::CalculateID()
{
	ObjectID retval={0,0};

	char buffer[16];
	md5_buffer(snd_name,strlen(snd_name),&buffer[0]);
	buffer[7]=0;
	retval = *(ObjectID*)&buffer[0];
	return retval;
}

Sound_Object_Extra_Data_Chunk* Sound_Object_Chunk::get_extra_data_chunk()
{
	List<Chunk*> chlist;
	parent->lookup_child("SOUNDEXD",chlist);
	if(!chlist.size())return 0;

	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Sound_Object_Extra_Data_Chunk* soedc=(Sound_Object_Extra_Data_Chunk*)chlif();
		Sound_Object_Extra_Name_Chunk* sname=(Sound_Object_Extra_Name_Chunk*) soedc->lookup_single_child("SOUNDNAM");
		
		if(sname)
		{
			if(!strcmp(snd_name,sname->name))
			{
				return soedc;
			}
		}
	}
	//no extra data chunk found
	return 0;

}
Sound_Object_Extra_Data_Chunk* Sound_Object_Chunk::create_extra_data_chunk()
{
	Sound_Object_Extra_Data_Chunk* soedc=get_extra_data_chunk();
	if(soedc)return soedc;
	
	return new Sound_Object_Extra_Data_Chunk(parent,this);
}

Object_Alternate_Locations_Chunk* Sound_Object_Chunk::get_alternate_locations_chunk()
{
	Sound_Object_Extra_Data_Chunk* soedc=get_extra_data_chunk();
	if(!soedc) return 0;
	return (Object_Alternate_Locations_Chunk*)soedc->lookup_single_child("ALTLOCAT");
}

Object_Alternate_Locations_Chunk* Sound_Object_Chunk::create_alternate_locations_chunk()
{
	Sound_Object_Extra_Data_Chunk* soedc=create_extra_data_chunk();
	
	Object_Alternate_Locations_Chunk* loc_chunk=(Object_Alternate_Locations_Chunk*)soedc->lookup_single_child("ALTLOCAT");
	if(loc_chunk) return loc_chunk;

	return new Object_Alternate_Locations_Chunk(soedc);
}


/////////////////////////////////////////////////////////////////////////////////////
//class Sound_Object_Extra_Data_Chunk
RIF_IMPLEMENT_DYNCREATE("SOUNDEXD",Sound_Object_Extra_Data_Chunk)

CHUNK_WITH_CHILDREN_LOADER("SOUNDEXD",Sound_Object_Extra_Data_Chunk)

/*
Children for Sound_Object_Extra_Data_Chunk :

"SOUNDNAM"		Sound_Object_Extra_Name_Chunk
"ALTLOCAT"		Object_Alternate_Locations_Chunk
*/


Sound_Object_Extra_Data_Chunk::Sound_Object_Extra_Data_Chunk(Chunk_With_Children* parent,Sound_Object_Chunk* soc)
:Chunk_With_Children(parent,"SOUNDEXD")
{
	new Sound_Object_Extra_Name_Chunk(this,soc->snd_name);	
}

Sound_Object_Chunk* Sound_Object_Extra_Data_Chunk::get_sound_chunk()
{
	List<Chunk*> chlist;

	lookup_child("SOUNDNAM",chlist);
	if(!chlist.size()) return 0;
	const char* name=((Sound_Object_Extra_Name_Chunk*)chlist.first_entry())->name;

	parent->lookup_child("SOUNDEXD",chlist);
	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Sound_Object_Chunk* soc=(Sound_Object_Chunk*)chlif();
		if(!strcmp(soc->snd_name,name))
		{
			return soc;
		}
	}
	//no sound chunk found
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SOUNDNAM",Sound_Object_Extra_Name_Chunk)

Sound_Object_Extra_Name_Chunk::Sound_Object_Extra_Name_Chunk(Chunk_With_Children * parent, const char * data, size_t /*size*/)
:Chunk(parent,"SOUNDNAM")
{
	int length=strlen(data);
	name=new char[length+1];
	strcpy(name,data);
}

Sound_Object_Extra_Name_Chunk::Sound_Object_Extra_Name_Chunk(Chunk_With_Children * parent, const char * _name)
:Chunk(parent,"SOUNDNAM")
{
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

Sound_Object_Extra_Name_Chunk::~Sound_Object_Extra_Name_Chunk()
{
	delete name;
}

void Sound_Object_Extra_Name_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,name);
}


size_t Sound_Object_Extra_Name_Chunk::size_chunk()
{
	chunk_size=12;
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}
/////////////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("INDSOUND",Indexed_Sound_Chunk)

Indexed_Sound_Chunk::Indexed_Sound_Chunk(Chunk_With_Children* parent,const char* data,const size_t)
:Chunk(parent,"INDSOUND")
{
	CHUNK_EXTRACT(index,int)
	CHUNK_EXTRACT_STRING(wav_name)
	CHUNK_EXTRACT(inner_range,int)
	CHUNK_EXTRACT(outer_range,int)
	CHUNK_EXTRACT(max_volume,int)
	CHUNK_EXTRACT(pitch,int)
	CHUNK_EXTRACT(flags,int)
	CHUNK_EXTRACT_ARRAY(num_extra_data,extra_data,int)
}

Indexed_Sound_Chunk::Indexed_Sound_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"INDSOUND")
{
	index=0;
	wav_name=0;
	num_extra_data=0;
	extra_data=0;
}

Indexed_Sound_Chunk::~Indexed_Sound_Chunk()
{
	if(extra_data) delete [] extra_data;
	if(wav_name) delete [] wav_name;
}

void Indexed_Sound_Chunk::fill_data_block(char * data)
{
	CHUNK_FILL_START
	CHUNK_FILL(index,int)
	CHUNK_FILL_STRING(wav_name)
	CHUNK_FILL(inner_range,int)
	CHUNK_FILL(outer_range,int)
	CHUNK_FILL(max_volume,int)
	CHUNK_FILL(pitch,int)
	CHUNK_FILL(flags,int)
	CHUNK_FILL_ARRAY(num_extra_data,extra_data,int)
	
}

size_t Indexed_Sound_Chunk::size_chunk()
{
	chunk_size=12+24;
	chunk_size+=4+4*num_extra_data;
	if(wav_name)
		chunk_size+=(strlen(wav_name)+4)&~3;
	else
		chunk_size+=4;

	return chunk_size;
}

/////////////////////////////////////////////////////////////////////////////////////
/*
Sound_Collection_Chunk::Sound_Collection_Chunk(Chunk_With_Children* parent,const char* data,const size_t)
:Chunk(parent,"SOUNDCOL")
{
	CHUNK_EXTRACT(index,int)
	CHUNK_EXTRACT_ARRAY(num_sounds,sounds,ChunkSoundWeighting)
	CHUNK_EXTRACT(spare,int)
}

Sound_Collection_Chunk::Sound_Collection_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"SOUNDCOL")
{
	index=-1;
	num_sounds=0;
	sounds=0;
	spare=0;
}

Sound_Collection_Chunk::~Sound_Collection_Chunk()
{
	if(sounds) delete [] sounds;
}

void Sound_Collection_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START	
	CHUNK_FILL(index,int)
	CHUNK_FILL_ARRAY(num_sounds,sounds,ChunkSoundWeighting)
	CHUNK_FILL(spare,int)
}

size_t Sound_Collection_Chunk::size_chunk()
{
	chunk_size=12+12+num_sounds*sizeof(ChunkSoundWeighting);
	return chunk_size;
}
*/
