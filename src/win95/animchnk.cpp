#include "animchnk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(animchnk)

RIF_IMPLEMENT_DYNCREATE("TEXTANIM",Animation_Chunk)

Animation_Chunk::Animation_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "TEXTANIM")
{
	NumPolys=0;
	AnimList=0;
}
Animation_Chunk::Animation_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "TEXTANIM")
{
	NumPolys=*((int*)data);
	data+=4;
	if(NumPolys)
	{
		AnimList=(TEXANIM**)malloc(NumPolys*sizeof(TEXANIM*));
		for(int i=0;i<NumPolys;i++)
		{
			AnimList[i]=new TEXANIM;
			TEXANIM* ta=AnimList[i];
			ta->poly=*((int*)data);
			data+=4;
			ta->ID=*((int*)data);
			data+=4;
			ta->NumSeq=*((int*)data);
			data+=4;
			ta->NumVerts=*((int*)data);
			data+=4;
			ta->AnimFlags=*((int*)data);
			data+=4;
			ta->Identifier=*((int*)data);
			data+=4;
			ta->CurSeq=0;
			ta->Seq=new FrameList*[ta->NumSeq];
			for(int j=0;j<ta->NumSeq;j++)
			{
				ta->Seq[j]=new FrameList(ta);
				FrameList* fl=ta->Seq[j];
				fl->Speed=*((int*)data);
				data+=4;
				fl->Flags=*((int*)data);
				data+=4;
	
				fl->NumFrames=*((int*)data);
				data+=4;
				fl->spare1=*((int*)data);
				data+=4;
				fl->spare2=*((int*)data);
				data+=4;
				fl->CurFrame=0;
				
				fl->Textures=new int[fl->NumFrames];
				fl->UVCoords=new int[(2*ta->NumVerts)*fl->NumFrames];
				for(int k=0;k<fl->NumFrames;k++)
				{
					fl->Textures[k]=*((int*)data);
					data+=4;
				}
				for(int k=0;k<(2*ta->NumVerts)*fl->NumFrames;k++)
				{
					fl->UVCoords[k]=*((int*)data);
					data+=4;
				}
			}

		}	
	}
	else
		AnimList=0;
	
}

Animation_Chunk::~Animation_Chunk()
{
	for(int i=0;i<NumPolys;i++)
	{
		delete AnimList[i];
	}
	free(AnimList);
}


size_t Animation_Chunk::size_chunk()
{
	chunk_size=12+4;
	for(int i=0;i<NumPolys;i++)
	{
		chunk_size+=24;
		TEXANIM* ta=AnimList[i];
		for(int j=0;j<ta->NumSeq;j++)
		{
			chunk_size+=20;
			chunk_size+=4*(1+2*ta->NumVerts)*ta->Seq[j]->NumFrames;
		}
	}
	return chunk_size;
}
	
BOOL Animation_Chunk::output_chunk (HANDLE &hand)
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

void Animation_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*(int*)data_start=NumPolys;
	data_start+=4;
	for(int i=0;i<NumPolys;i++)
	{
		TEXANIM* ta=AnimList[i];
		*(int*)data_start=ta->poly;		
		data_start+=4;
		*(int*)data_start=ta->ID;		
		data_start+=4;
		*(int*)data_start=ta->NumSeq;		
		data_start+=4;
		*(int*)data_start=ta->NumVerts;		
		data_start+=4;
		*(int*)data_start=ta->AnimFlags;		
		data_start+=4;
		*(int*)data_start=ta->Identifier;		
		data_start+=4;

		for(int j=0;j<ta->NumSeq;j++)
		{
			FrameList* fl=ta->Seq[j];
			*(int*)data_start=fl->Speed;		
			data_start+=4;
			*(int*)data_start=fl->Flags;		
			data_start+=4;
			*(int*)data_start=fl->NumFrames;		
			data_start+=4;
			*(int*)data_start=fl->spare1;		
			data_start+=4;
			*(int*)data_start=fl->spare2;		
			data_start+=4;
			for(int k=0;k<fl->NumFrames;k++)
			{
				*(int*)data_start=fl->Textures[k];		
				data_start+=4;
			}
			for(int k=0;k<(2*ta->NumVerts)*fl->NumFrames;k++)
			{
				*(int*)data_start=fl->UVCoords[k];		
				data_start+=4;
			}
		}

	}
}

FrameList::FrameList(TEXANIM* p)
{
	Speed=65536;
	Flags=0;
	NumFrames=0;
	CurFrame=-1;
	parent=p;
	Textures=0;
	UVCoords=0;
	spare1=spare2=0;
}
FrameList::FrameList(TEXANIM* p,FrameList* fl,int* conv)
{
	Speed=fl->Speed;
	Flags=fl->Flags;
	NumFrames=fl->NumFrames;
	parent=p;
	Textures=new int[NumFrames];
	UVCoords=new int[NumFrames*2*p->NumVerts];
	spare1=fl->spare1;
	spare2=fl->spare2;
	if(conv)
	{
		for(int i=0;i<NumFrames;i++)
		{
			Textures[i]=conv[fl->Textures[i]];
		}
	}
	else
	{
		for(int i=0;i<NumFrames;i++)
		{
			Textures[i]=fl->Textures[i];
		}
	}
	for(int i=0;i<NumFrames*2*p->NumVerts;i++)
	{
		UVCoords[i]=fl->UVCoords[i];
	}
	CurFrame=0;
}
FrameList::~FrameList()
{
	delete [] Textures;
	delete [] UVCoords;
}

TEXANIM::TEXANIM()
{
	shape=0;
	NumSeq=0;
	CurSeq=-1;
	Seq=0;
	NumVerts=3;
	AnimFlags=Identifier=0;
}
TEXANIM::TEXANIM(TEXANIM* ta)
{
	shape=0;
	NumSeq=0;
	CurSeq=-1;
	Seq=0;
	NumVerts=3;
	AnimFlags=Identifier=0;
	CopyAnimData(ta,0);
}

TEXANIM::~TEXANIM()
{
	for(int i=0;i<NumSeq;i++)
	{
		delete Seq[i];
	}
	delete [] Seq;	
}

void TEXANIM::CopyAnimData(TEXANIM* ta,int*conv)
{
	shape=ta->shape;
	poly=ta->poly;
	ID=ta->ID;
	NumSeq=ta->NumSeq;
	CurSeq=0;
	Seq=new FrameList*[NumSeq];
	NumVerts=ta->NumVerts;
	AnimFlags=ta->AnimFlags;
	Identifier=ta->Identifier;
	for(int i=0;i<NumSeq;i++)
	{
		Seq[i]=new FrameList(this,ta->Seq[i],conv);	
	}

}

