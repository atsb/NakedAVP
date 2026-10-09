#ifndef _sprchunk_hpp
#define _sprchunk_hpp
#include "chunk.hpp"
#include "chnktype.hpp"

struct Frame
{
	int Texture;
	int CentreX;
	int CentreY;
	int UVCoords[4][2];
};

class Sprite_Header_Chunk : public Chunk_With_Children
{
public:
	Sprite_Header_Chunk(Chunk_With_Children* const parent,const char*,size_t const);
	Sprite_Header_Chunk(const char* file_name, Chunk_With_Children * parent = NULL);
	Sprite_Header_Chunk()
	: Chunk_With_Children(0,"SPRIHEAD"){}
	Sprite_Header_Chunk(Chunk_With_Children* parent)
	: Chunk_With_Children(parent,"SPRIHEAD"){}
										 
	int write_file(const char* fname);
	virtual BOOL output_chunk(HANDLE &hand);
};

class PC_Sprite_Chunk : public Chunk_With_Children
{
public:
	PC_Sprite_Chunk(Sprite_Header_Chunk* parent,const char*,size_t);
	PC_Sprite_Chunk(Sprite_Header_Chunk* parent)
	: Chunk_With_Children(parent,"SPRITEPC"){}
};

class Saturn_Sprite_Chunk : public Chunk_With_Children
{
public:
	Saturn_Sprite_Chunk(Sprite_Header_Chunk* parent,const char*,size_t);
	Saturn_Sprite_Chunk(Sprite_Header_Chunk* parent)
	: Chunk_With_Children(parent,"SPRITESA"){}
};
class Playstation_Sprite_Chunk : public Chunk_With_Children
{
public:
	Playstation_Sprite_Chunk(Sprite_Header_Chunk* parent,const char*,size_t);
	Playstation_Sprite_Chunk(Sprite_Header_Chunk* parent)
	: Chunk_With_Children(parent,"SPRITEPS"){}
};


#define SpriteActionFlag_FlipSecondSide 	0x00000001 //for actions where only right facing views are available
class Sprite_Action_Chunk  : public Chunk
{
public:
	Sprite_Action_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Action_Chunk(Chunk_With_Children* parent);
	~Sprite_Action_Chunk(); 

	virtual BOOL output_chunk (HANDLE &hand);
	
	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);

public:
	
	int Action;
	int NumYaw;
	int NumPitch;
	int NumFrames;
	Frame*** FrameList;
	int Flags;
	int FrameTime;
	 
};
#define SpriteFlag_NoLight			0x00000001			
#define SpriteFlag_SemiTrans		0x00000002 //for playstation
class Sprite_Size_Chunk  : public Chunk
{
public:
	Sprite_Size_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Size_Chunk(Chunk_With_Children* parent);

	virtual BOOL output_chunk (HANDLE &hand);
	
	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);

public:
	
	double scale;
	double maxy; 
	double maxx;
	int radius;
	int Flags ;
};

class Sprite_Extent_Chunk  : public Chunk
{
public:
	Sprite_Extent_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Extent_Chunk(Chunk_With_Children* parent);

	
	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);

public:
	
	double minx;
	double maxx; 
	double miny;
	double maxy;
	int spare1,spare2;
};

class Sprite_Version_Number_Chunk  : public Chunk
{
public:
	Sprite_Version_Number_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Version_Number_Chunk(Chunk_With_Children* parent);

	virtual BOOL output_chunk (HANDLE &hand);
	
	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);

public:
	int version_num;
};

class Sprite_Bitmap_Scale_Chunk : public Chunk
{
public :
	Sprite_Bitmap_Scale_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Bitmap_Scale_Chunk(Chunk_With_Children* parent);
	~Sprite_Bitmap_Scale_Chunk();

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	int NumBitmaps;
	float* Scale;
};

class Sprite_Bitmap_Centre_Chunk : public Chunk
{
public :
	Sprite_Bitmap_Centre_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Sprite_Bitmap_Centre_Chunk(Chunk_With_Children* parent);
	~Sprite_Bitmap_Centre_Chunk();

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	int NumBitmaps;
	int* CentreX;
	int* CentreY;
	int* OffsetX;
	int* OffsetY;
	int spare;
	
};
#endif
