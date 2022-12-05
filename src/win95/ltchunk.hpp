#ifndef _ltchunk_hpp
#define _ltchunk_hpp 1

#include "chunk.hpp"
#include "chnktype.hpp"


#define LOFlag_On					0x00000001
#define LOFlag_ShadowData	0x00000002
#define LOFlag_Changed		0x00000004
#define LOFlag_Calc_Data	0x00000008
#define LOFlag_Runtime		0x00000010
#define LOFlag_Invisible	0x00000020
#define LOFlag_NoPreLight	0x00000040

class Environment_Data_Chunk;

class Light_Data
{
public:

	int light_number;
	ChunkVectorInt location;
	ChunkMatrix orientation;
	
	int brightness;
	int spread;
	int range;
	
	int colour;
	
	int engine_light_flags;
	int local_light_flags;

	int pad1;
	int pad2;
	
};


#define PlacedLightType_Standard 0
#define PlacedLightType_Strobe   1
#define PlacedLightType_Flicker  2

#define PlacedLightOnOff_Standard 0
#define PlacedLightOnOff_Fade 1
#define PlacedLightOnOff_Flicker 2

#define PlacedLightFlag_On 				   0x00000001
#define PlacedLightFlag_SwapColourBright   0x00000002
#define PlacedLightFlag_NoSpecular 		   0x00000004
class Placed_Object_Light_Data
{
public:

	int brightness;
	int spread;
	int range;
	
	int up_colour;
	int down_colour;
	
	int engine_light_flags;
	int local_light_flags;

	int fade_up_time;
	int fade_down_time;
	int up_time;
	int down_time;
	int start_time;

	int light_type;
	int on_off_type;
	int flags;

	
};

class Light_Set_Chunk : public Chunk_With_Children
{
public:

	// for user

	Light_Set_Chunk (Chunk_With_Children * parent, char * light_set_name);
	Light_Set_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	
private:
	
	friend class Environment_Data_Chunk;
	

	
};


class Light_Set_Header_Chunk : public Chunk
{
public:

	Light_Set_Header_Chunk (Light_Set_Chunk * parent, char l_set_name[8]);
	Light_Set_Header_Chunk (Chunk_With_Children * parent, const char *, size_t const);
	char light_set_name[8];
	
	int pad;
	
	size_t size_chunk()
	{
		return (chunk_size = 12 + 8 + 4);
	}
	
	void fill_data_block ( char * data_start);
	
private:

	friend class Light_Set_Chunk;

	

};


class Light_Chunk : public Chunk
{
public:

	Light_Chunk (Light_Set_Chunk * parent, Light_Data & new_light)
	: Chunk (parent, "STDLIGHT"), light (new_light) {}

	Light_Chunk (Chunk_With_Children * parent, const char *, size_t const);

	Light_Data light;

	BOOL light_added_to_module;
	
	size_t size_chunk()
	{
		return (chunk_size = 12 + 9 * 4 + 3 * 4 + 9 * 4);
	}
	
	void fill_data_block ( char * data_start);

private:

	friend class Light_Set_Chunk;
	
	

};

//should be a child of an object_chunk
class Placed_Object_Light_Chunk : public Chunk
{
public :	
	Placed_Object_Light_Chunk (Chunk_With_Children * parent, Placed_Object_Light_Data & new_light)
	: Chunk (parent, "PLOBJLIT"), light(new_light), extra_data(0), num_extra_data(0) {}

	Placed_Object_Light_Chunk (Chunk_With_Children * parent, const char *, size_t const);
	
	~Placed_Object_Light_Chunk();
	
	Placed_Object_Light_Data light;
	int* extra_data;
	int num_extra_data;

	size_t size_chunk()
	{
		return (chunk_size = 12 +sizeof(Placed_Object_Light_Data) +4 +4*num_extra_data);
	}
	
	void fill_data_block ( char * data_start);
	
};


class Shape_Vertex_Intensities_Chunk : public Chunk
{
public:

	Shape_Vertex_Intensities_Chunk(Chunk_With_Children *, char *, int, int *);
	Shape_Vertex_Intensities_Chunk(Chunk_With_Children *, const char *, size_t const);
	~Shape_Vertex_Intensities_Chunk();
	
	char light_set_name[8];

	int pad;
	
	int num_vertices;
	
	int * intensity_array;

	size_t size_chunk()
	{
		return (chunk_size = 12 + 8 + 4 + 4 + 4 * num_vertices);
	}

	void fill_data_block ( char * data_start);

private:

	friend class Object_Chunk;

	
	
};

class Lighting_Ambience_Chunk : public Chunk
{
public:

	Lighting_Ambience_Chunk (Light_Set_Chunk * parent, int _ambience)
	: Chunk (parent, "AMBIENCE"), ambience (_ambience)
	{}
	Lighting_Ambience_Chunk (Chunk_With_Children * parent, const char * data, size_t const /*size*/)
	: Chunk (parent, "AMBIENCE"), ambience (*((int *)data))
	{}
	int ambience;
	
	size_t size_chunk()
	{
		return (chunk_size = 12 + 4);
	}

	void fill_data_block ( char * data_start);

private:

	friend class Light_Set_Chunk;



};

//details of how light data should be altered for different platforms.
//currently just playstation
class Light_Scale_Chunk : public Chunk
{
public :
	Light_Scale_Chunk(Light_Set_Chunk * parent,int mode);
	Light_Scale_Chunk(Chunk_With_Children* parent,const char* data,size_t);

	void fill_data_block ( char * data_start);
	size_t size_chunk()
	{
		return (chunk_size = 12 + 44);
	}
	
	int LightMode;
	float prelight_multiply;	
	int prelight_multiply_above;
	int prelight_add;			
	
	float runtime_multiply;
	int runtime_multiply_above;
	int runtime_add;


	int ApplyPrelightScale(int l);
	int ApplyRuntimeScale(int l);

	int spare1,spare2,spare3,spare4;	
};

#endif
