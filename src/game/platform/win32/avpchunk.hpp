#ifndef _avpchunk_hpp_
#define _avpchunk_hpp_

#include "chunk.hpp"
#include "chnktype.hpp"
#include "envchunk.hpp"
#include "sndchunk.hpp"

#define AVPGENFLAG_MODEFLAGSSET 0x00000001

// same as obchunk 4 compatibility

#define AVPGENFLAG_AVPGAMEMODEMARINE 		0x00000800
#define AVPGENFLAG_AVPGAMEMODEALIEN 		0x00001000
#define AVPGENFLAG_AVPGAMEMODEPREDATOR	0x00002000

#define AVPGENFLAG_NOTDIFFICULTY1 					0x00100000
#define AVPGENFLAG_NOTDIFFICULTY2 					0x00200000
#define AVPGENFLAG_NOTDIFFICULTY3 					0x00400000

#define AVPGENFLAG_USEOWNLIMIT 0x02000000
#define AVPGENFLAG_USEOWNRATE 0x04000000
#define AVPGENFLAG_ADVANCEDGENERATOR 0x08000000

#define AVPGENFLAG_MULTIPLAYERSTART     0x40000000
#define AVPGENFLAG_GENERATORINACTIVE	0x80000000
enum GenerTypes
{
	GenerType_Intermittent = 0,
	GenerType_BadGuy1,
	GenerType_BadGuy2,
	GenerType_BadGuy3,
};
	
class AVP_Generator_Extra_Data_Chunk;
class AVP_Generator_Extended_Settings_Chunk;
class Object_Alternate_Locations_Chunk;

class AVP_Generator_Chunk : public Chunk
{
public:

	AVP_Generator_Chunk (Chunk_With_Children * parent)
	: Chunk (parent, "AVPGENER"),
		textureID (0),
		sub_type (0),
		extra1 (0),
		extra2 (0),
		name (0)
	{}

	~AVP_Generator_Chunk ();
	
	ChunkVectorInt location;
	
	int orientation; //euler y
	int type;
	
	int flags;
	unsigned char textureID;
	
	unsigned char sub_type;
	unsigned char extra1;
	unsigned char extra2;

	char * name;

	ObjectID CalculateID();
	virtual void fill_data_block (char * data_start);
	virtual size_t size_chunk ();

	AVP_Generator_Extra_Data_Chunk* get_extra_data_chunk();
	AVP_Generator_Extra_Data_Chunk* create_extra_data_chunk();

	AVP_Generator_Extended_Settings_Chunk* get_extended_settings();
	AVP_Generator_Extended_Settings_Chunk* create_extended_settings();

	Object_Alternate_Locations_Chunk* get_alternate_locations_chunk();
	AVP_Generator_Chunk (Chunk_With_Children * parent, const char * data, size_t size);
private:

	friend class Special_Objects_Chunk;
			
	
};

//For attaching extra data to the generators and badguys
class AVP_Generator_Extra_Data_Chunk:public Chunk_With_Children
{
public:

	AVP_Generator_Extra_Data_Chunk(Chunk_With_Children * parent)
	: Chunk_With_Children (parent, "AVPGENEX")
	{}
	
	AVP_Generator_Extra_Data_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	AVP_Generator_Extra_Data_Chunk (Chunk_With_Children * parent,AVP_Generator_Chunk*);
	
	AVP_Generator_Chunk* get_generator_chunk();
};

//Needed so I can match the extra data chunk with the appropriate generator_chunk
class AVP_Generator_Extra_Name_Chunk : public Chunk
{
public :	
	AVP_Generator_Extra_Name_Chunk(Chunk_With_Children* parent,const char*,size_t);
	AVP_Generator_Extra_Name_Chunk(Chunk_With_Children* parent,const char* _name);
	~AVP_Generator_Extra_Name_Chunk();
	
	char* name;

	
	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);
};


struct AVP_Generator_Weighting
{
	int data_size;

	int PulseMarine_Wt;
	int FlameMarine_Wt;
	int SmartMarine_Wt;
	int SadarMarine_Wt;
	int GrenadeMarine_Wt;
	int MinigunMarine_Wt;
	int ShotgunCiv_Wt;
	int PistolCiv_Wt;
	int FlameCiv_Wt;
	int UnarmedCiv_Wt;
	int MolotovCiv_Wt;

	int Alien_Wt;
	int PredAlien_Wt;
	int Praetorian_Wt;

	int PistolMarine_Wt;

};

class AVP_Generator_Extended_Settings_Chunk : public Chunk
{
public :
	AVP_Generator_Extended_Settings_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	AVP_Generator_Extended_Settings_Chunk(Chunk_With_Children* parent);
	~AVP_Generator_Extended_Settings_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();
	
	int GenerationRate;
	int GenRateIncrease;
	unsigned char GenLimit;
	unsigned char pad1,pad2,pad3;

	int spare1;
	int spare2;

	AVP_Generator_Weighting * weights;
	
};

#define R6GENFLAG_OBJECTIVE_MASK 0x00000007
#define R6GENFLAG_BADDY_INDEX_MASK 0xff000000
#define R6GENFLAG_BADDY_INDEX_SHIFT 24

class Rainbow6_Generator_Extra_Data_Chunk : public Chunk
{
public :
	Rainbow6_Generator_Extra_Data_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Rainbow6_Generator_Extra_Data_Chunk(Chunk_With_Children*);
	~Rainbow6_Generator_Extra_Data_Chunk();

	int Get_Baddy_Index(){ return (flags & R6GENFLAG_BADDY_INDEX_MASK)>>R6GENFLAG_BADDY_INDEX_SHIFT;}
	void Set_Baddy_Index(int index) { flags&=~R6GENFLAG_BADDY_INDEX_MASK; flags |=(index<<R6GENFLAG_BADDY_INDEX_SHIFT);}

	int distance;
	int flags;
	int num_extra_data;
	int* extra_data;
	
	size_t size_chunk();
	void fill_data_block(char* data_start);

};

enum Generated_Enemies
{
	Generate_Aliens,
	Generate_Marines,	
};

class Global_Generator_Data_Chunk : public Chunk
{
public :	
	Global_Generator_Data_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Global_Generator_Data_Chunk(Chunk_With_Children* parent);
	
	int EnemyGenerated;
	int MaxNPCSOnLevel;
	int NPCSPerMinute;
	int NPCAcceleration;	 //in npcs per minute per minute
	int HiveStateChangeTime; //in seconds
	int spare1;
	int spare2;
	
	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);
};



class AVP_Player_Start_Chunk : public Chunk
{
public :
	AVP_Player_Start_Chunk (Chunk_With_Children * parent, const char * data, size_t size);
	AVP_Player_Start_Chunk (Chunk_With_Children * parent);
		
	virtual void fill_data_block (char * data);
	virtual size_t size_chunk ();

	ChunkVectorInt location;
	ChunkMatrix orientation;
	
	ObjectID moduleID; //r6 only
};

#define PowerCableFlag_UseDefaultSettings 0x00000001

class AVP_Power_Cable_Chunk : public Chunk
{
public :
	AVP_Power_Cable_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	AVP_Power_Cable_Chunk(Chunk_With_Children* parent,const char* _name,ObjectID _id);

	~AVP_Power_Cable_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();

	ChunkVectorInt location;
	char* name;
	ObjectID id;
	int max_charge;
	int initial_charge;
	int recharge_rate;
	int flags;

	int spare[3];
	

};
class AVP_Environment_Settings_Chunk;

struct AVP_Environment_Settings
{
private :
	friend class AVP_Environment_Settings_Chunk;
	int data_size;
public :

	int sky_colour_red;
	int sky_colour_green;
	int sky_colour_blue;

	//available predator weapons
	unsigned int predator_pistol :1;
	unsigned int predator_plasmacaster :1;
	unsigned int predator_disc :1;
	unsigned int predator_grappling_hook :1;
	unsigned int predator_medicomp :1;
	
	unsigned int marine_jetpack :1;

	unsigned int stars_in_sky :1;
	
	unsigned int spare_bits :25;
	int  predator_num_spears;
};

class AVP_Environment_Settings_Chunk : public Chunk
{
public :
	AVP_Environment_Settings_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	AVP_Environment_Settings_Chunk(Chunk_With_Children* parent);
	~AVP_Environment_Settings_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();

	AVP_Environment_Settings *  settings;

};


AVP_Environment_Settings_Chunk* GetAVPEnvironmentSettings(Environment_Data_Chunk* env_chunk);


struct AVP_Decal
{
	int DecalID;
	ChunkVectorInt Vertices[4];
	int UOffset;
	int object_index;
};

class AVP_Decal_Chunk : public Chunk
{
public :
	AVP_Decal_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	AVP_Decal_Chunk(Chunk_With_Children* parent,int num_dec);
	~AVP_Decal_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();
	
	int num_decals;
	/*the pointer to the array of decals is only set if the loaded decal structure size
	is less than or equal to the current decal structure size*/
	AVP_Decal* decals;

private :	
	int decal_size;
	char* decal_buffer;	
};

/////////////////////Particle Generators////////////////////////////////////////

class AVP_Particle_Generator_Data_Chunk;

class AVP_Particle_Generator_Chunk : public Chunk_With_Children
{
public :

	AVP_Particle_Generator_Chunk(Chunk_With_Children* parent,const char* name,ObjectID& id);
	AVP_Particle_Generator_Chunk(Chunk_With_Children * const parent,const char *, size_t const);

	AVP_Particle_Generator_Data_Chunk* get_data_chunk();
	Object_Alternate_Locations_Chunk* get_alternate_locations_chunk();
	Indexed_Sound_Chunk* get_sound_chunk();
};

#define ParticleGeneratorFlag_Inactive 0x00000001

class AVP_Particle_Generator_Data_Chunk : public Chunk
{
public :
	AVP_Particle_Generator_Data_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	AVP_Particle_Generator_Data_Chunk(Chunk_With_Children* parent,const char* _name,ObjectID& _id);
	~AVP_Particle_Generator_Data_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();
	
	int type;
	unsigned short time; //int tenths of a second
	unsigned short probability;//0-100
	unsigned short speed;//in cm/second
	unsigned short quantity;
	int spare1,spare2;
	int flags;
	ObjectID id;
	ObjectID parent_id; //gnerator can be positioned relative to another object
	ChunkVectorInt location;
	ChunkQuat orientation;

	char* name;

};



#endif
