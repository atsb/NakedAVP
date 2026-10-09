#ifndef _animchnk_hpp
#define _animchnk_hpp
#include "chunk.hpp"
#include "chnktype.hpp"

struct TEXANIM;


class Animation_Chunk : public Chunk
{
public :
	Animation_Chunk(Chunk_With_Children* parent,const char*,size_t);
	Animation_Chunk(Chunk_With_Children* parent);
	~Animation_Chunk();
	
	virtual BOOL output_chunk (HANDLE &hand);
	
	virtual size_t size_chunk();
	
	virtual void fill_data_block(char* data_start);

	int NumPolys; //with animation in this shape
	TEXANIM** AnimList;
	
};

#define txa_flag_nointerptofirst 0x80000000

struct FrameList
{
	~FrameList();
	FrameList(TEXANIM*);
	FrameList(TEXANIM* p,FrameList* fl,int* conv);
	int Speed;
	int Flags;
	
	int NumFrames;
	int CurFrame;
	TEXANIM* parent;

	int* Textures;
	int* UVCoords;
	int spare1,spare2;
};

#define AnimFlag_NotPlaying 0x00000001
struct TEXANIM
{
	TEXANIM(TEXANIM*);
	TEXANIM();
	~TEXANIM();
	
	int shape;
	int poly;
	int NumVerts;
	int ID;
	int NumSeq;//number of sequences
	int CurSeq;
	int AnimFlags;
	int Identifier;
	FrameList** Seq;

	void CopyAnimData(TEXANIM* ta,int* conv);
};

#endif
