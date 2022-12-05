#ifndef _sndchunk_hpp
#define _sndchunk_hpp 1
#include "chnktype.hpp"

#define SoundObjectFlag_NotPlayingAtStart 0x00000001
#define SoundObjectFlag_NoLoop			  0x00000002
	 
class Object_Alternate_Locations_Chunk;
class Sound_Object_Extra_Data_Chunk;

class Sound_Object_Chunk : public Chunk
{
public:
	
	Sound_Object_Chunk (Chunk_With_Children * parent, 
											ChunkVectorInt & pos,
											const char * name
											);
	// constructor from buffer
	Sound_Object_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/);

	~Sound_Object_Chunk ();
	
	ChunkVectorInt position;

	unsigned long inner_range; //millimetres
	unsigned long outer_range; //millimetres
	int max_volume; //from 0 to 127
	int	pitch;	   //pitch shift in 1/1536ths of an actave
	
	int flags;
	int probability;
	int pad3;
	
	char * snd_name;
	char * wav_name;
	
	size_t size_chunk ()
	{
		chunk_size = 12 + 3*4 + 7*4 + strlen(snd_name) + 1;
		if (wav_name)
		{
			chunk_size += strlen (wav_name);
		}
		else
		{
			chunk_size += 1;
		}
		
		chunk_size +=  4 - chunk_size%4;
		return (chunk_size);
	}
	
	void fill_data_block (char *);
	
	ObjectID CalculateID();

	Sound_Object_Extra_Data_Chunk* get_extra_data_chunk();
	Sound_Object_Extra_Data_Chunk* create_extra_data_chunk(); //gets chunk if it exists , otherwise creates a new one

	Object_Alternate_Locations_Chunk* get_alternate_locations_chunk();
	Object_Alternate_Locations_Chunk* create_alternate_locations_chunk(); //gets chunk if it exists , otherwise creates a new one

private:

	friend class Special_Objects_Chunk;


	
};

//For attaching extra data to the sound objects
class Sound_Object_Extra_Data_Chunk : public Chunk_With_Children
{
public:
	Sound_Object_Extra_Data_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	Sound_Object_Extra_Data_Chunk (Chunk_With_Children * parent,Sound_Object_Chunk*);
	
	Sound_Object_Chunk* get_sound_chunk();
};
	
//Needed so I can match the extra data chunk with the appropriate sound chunk
class Sound_Object_Extra_Name_Chunk : public Chunk
{
public :	
	Sound_Object_Extra_Name_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sound_Object_Extra_Name_Chunk(Chunk_With_Children* parent,const char* _name);
	~Sound_Object_Extra_Name_Chunk();
	
	char* name;

	
	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);
};



#define IndexedSoundFlag_Loop 0x00000001
class Indexed_Sound_Chunk : public Chunk
{
public :

	Indexed_Sound_Chunk(Chunk_With_Children* parent,const char*,const size_t);
	Indexed_Sound_Chunk(Chunk_With_Children* parent);
	~Indexed_Sound_Chunk();

	size_t size_chunk();
	void fill_data_block(char*);

	int index;
	char* wav_name;
	unsigned long inner_range; //millimetres
	unsigned long outer_range; //millimetres
	int max_volume; //from 0 to 127
	int	pitch; //pitch shift in 1/1536ths of an actave
	int flags;
	
	int num_extra_data;
	int* extra_data;
};



/*
struct ChunkSoundWeighting
{
	int index;
	int weighting;
};

//a collection of indeces of possible sounds to play
class Sound_Collection_Chunk : public Chunk
{
public :

	Sound_Collection_Chunk(Chunk_With_Children* parent,const char*,const size_t);
	Sound_Collection_Chunk(Chunk_With_Children* parent);
	~Sound_Collection_Chunk();

	size_t size_chunk();
	void fill_data_block(char*);

	int index;
	
	int num_sounds;
	ChunkSoundWeighting* sounds;

	int spare;

	
};
*/


#endif
