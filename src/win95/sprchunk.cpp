#include "sprchunk.hpp"
#include "mishchnk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(sprchunk)

extern Chunk* Parent_File;

RIF_IMPLEMENT_DYNCREATE("SPRACTIO",Sprite_Action_Chunk)

Sprite_Action_Chunk::Sprite_Action_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPRACTIO")
{
	Action=*((int*)data);
	data+=4;
	NumYaw=*((int*)data);
	data+=4;
	NumPitch=*((int*)data);
	data+=4;
	NumFrames=*((int*)data);
	data+=4;
	Flags=*((int*)data);
	data+=4;
	FrameTime=*((int*)data);
	data+=4;
	FrameList=new Frame**[NumYaw];
	for(int i=0;i<NumYaw;i++)
	{
		FrameList[i]=new Frame*[NumPitch];
		for(int j=0;j<NumPitch;j++)
		{
			FrameList[i][j]=new Frame[NumFrames];
			for(int k=0;k<NumFrames;k++)
			{
				Frame* f=&FrameList[i][j][k];
				f->Texture=*((int*)data);
				data+=4;
				f->CentreX=*((int*)data);
				data+=4;
				f->CentreY=*((int*)data);
				data+=4;
				for(int l=0;l<4;l++)
				{
					f->UVCoords[l][0]=*((int*)data);		
					data+=4;
					f->UVCoords[l][1]=*((int*)data);		
					data+=4;
				}
			}
		}
	}
}
Sprite_Action_Chunk::Sprite_Action_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPRACTIO")
{
	Action=-1;
	NumPitch=0;
	NumYaw=0;
	NumFrames=0;
	FrameList=0;
   	Flags=0;
	FrameTime=200;
}
size_t Sprite_Action_Chunk::size_chunk()
{
	chunk_size=36+NumFrames*NumPitch*NumYaw*44;
	return chunk_size;	
}

Sprite_Action_Chunk::~Sprite_Action_Chunk()
{
	for(int i=0;i<NumYaw;i++)
	{
		for(int j=0;j<NumPitch;j++)
		{
			delete [] FrameList[i][j];
		}
		delete[]  FrameList[i];
	}
	delete [] FrameList;
}


BOOL Sprite_Action_Chunk::output_chunk (HANDLE &hand)
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

void Sprite_Action_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start=Action;
	data_start += 4;
	*(int*)data_start=NumYaw;
	data_start += 4;
	*(int*)data_start=NumPitch;
	data_start += 4;
	*(int*)data_start=NumFrames;
	data_start += 4;
	*(int*)data_start=Flags;
	data_start += 4;
	*(int*)data_start=FrameTime;
	data_start += 4; 
	
	for(int i=0;i<NumYaw;i++)
	{
		for(int j=0;j<NumPitch;j++)
		{
			for(int k=0;k<NumFrames;k++)
			{
				Frame* f=&FrameList[i][j][k];
				*(int*)data_start=f->Texture;
				data_start += 4;
				*(int*)data_start=f->CentreX;
				data_start +=4;
				*(int*)data_start=f->CentreY;
				data_start +=4;
				for(int l=0;l<4;l++)
				{
					*(int*)data_start=f->UVCoords[l][0];
					data_start+=4;
					*(int*)data_start=f->UVCoords[l][1];
					data_start+=4;
				}
   			}
		}
	}
}

//////////////////////////////////////////////////
//Class Sprite_Header_Chunk
RIF_IMPLEMENT_DYNCREATE("SPRIHEAD",Sprite_Header_Chunk)
CHUNK_WITH_CHILDREN_LOADER("SPRIHEAD",Sprite_Header_Chunk)

/*
Children for Sprite_Header_Chunk :

"SPRITVER"		Sprite_Version_Number_Chunk 
"SPRITEPC"		PC_Sprite_Chunk 
"SPRITEPS"		Playstation_Sprite_Chunk 
"SPRITESA"		Saturn_Sprite_Chunk 
"BMPLSTST"		Bitmap_List_Store_Chunk 
"BMNAMVER"		BMP_Names_Version_Chunk 
"BMNAMEXT"		BMP_Names_ExtraData_Chunk 
"RIFFNAME"		RIF_Name_Chunk 
"SHPEXTFN"		Shape_External_Filename_Chunk 
"SPRISIZE"		Sprite_Size_Chunk 
"BMPMD5ID"		Bitmap_MD5_Chunk 
"SPRBMPSC"		Sprite_Bitmap_Scale_Chunk 
"SPRBMPCE"		Sprite_Bitmap_Centre_Chunk 
"SPREXTEN"		Sprite_Extent_Chunk 
*/


Sprite_Header_Chunk::Sprite_Header_Chunk(const char * file_name, Chunk_With_Children * parent)
: Chunk_With_Children(parent,"SPRIHEAD")
{
// Load in whole chunk and traverse
	FILE *rif_file;
	DWORD file_size;
	DWORD file_size_from_file;
	char * buffer;
	char * buffer_ptr;
	char id_buffer[9];

	Parent_File = this;

	error_code = 0;


	rif_file = OpenGameFile(file_name, FILEMODE_READONLY, FILETYPE_PERM);	
	if (rif_file == NULL) {
		return;
	}

	fseek(rif_file, 0, SEEK_END);
	file_size = ftell(rif_file);
	rewind(rif_file);
	
	if (fread(id_buffer, 1, 8, rif_file) != 8) {
		error_code = CHUNK_FAILED_ON_LOAD;
		fclose(rif_file);
		return;
	}	

	if (strncmp(id_buffer, "SPRIHEAD", 8) != 0) {
		error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
		fclose(rif_file);
		return;
	}	

	if (fread(&file_size_from_file, 1, 4, rif_file) != 4) {
		error_code = CHUNK_FAILED_ON_LOAD;
		fclose(rif_file);
		return;
	}	

	if (file_size != file_size_from_file) {
		error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
		fclose(rif_file);
		return;
	}	

	buffer = new char [file_size];

	if (fread(buffer, 1, (file_size-12), rif_file) != (file_size-12)) {
		error_code = CHUNK_FAILED_ON_LOAD;
		delete [] buffer;
		
		fclose(rif_file);
		return;
	}

	fclose(rif_file);
	
	// Process the file

	buffer_ptr = buffer;

	
	// The start of the first chunk

	while ((buffer_ptr-buffer)< ((signed) file_size-12) && !error_code) {
		if ((*(int *)(buffer_ptr + 8)) + (buffer_ptr-buffer) > ((signed) file_size-12)) {
			error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		DynCreate(buffer_ptr);
		buffer_ptr += *(int *)(buffer_ptr + 8);
	}

	delete [] buffer;
}


int Sprite_Header_Chunk::write_file(const char* fname)
{
	HANDLE rif_file;

	rif_file = CreateFileA (fname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 
					FILE_FLAG_RANDOM_ACCESS, 0);

	if (rif_file == INVALID_HANDLE_VALUE) {
		return CHUNK_FAILED_ON_WRITE;
	}

	size_chunk();

	if (!(this->output_chunk(rif_file)))
		return CHUNK_FAILED_ON_WRITE;

	CloseHandle (rif_file);

	return 0;
}

BOOL Sprite_Header_Chunk::output_chunk(HANDLE & hand)
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

RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SPRITEPC",PC_Sprite_Chunk,"SPRIHEAD",Sprite_Header_Chunk)

PC_Sprite_Chunk::PC_Sprite_Chunk(Sprite_Header_Chunk* parent,const char* data,size_t datasize)
:Chunk_With_Children(parent,"SPRITEPC")
{
	const char * buffer_ptr = data;

	
	while ((data-buffer_ptr)< (signed) datasize) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) datasize) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		if (!strncmp(data, "SPRACTIO",8)) {
			new Sprite_Action_Chunk (this, (data + 12), (*(int *) (data + 8))-12);
			data += *(int *)(data + 8);
		}
		else {
			new Miscellaneous_Chunk (this, data, (data + 12), (*(int *) (data + 8)) -12 );
			data += *(int *)(data + 8);
		}

	}
}
/////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SPRITESA",Saturn_Sprite_Chunk,"SPRIHEAD",Sprite_Header_Chunk)


Saturn_Sprite_Chunk::Saturn_Sprite_Chunk(Sprite_Header_Chunk* parent,const char* data,size_t datasize)
:Chunk_With_Children(parent,"SPRITESA")
{
	const char * buffer_ptr = data;

	
	while ((data-buffer_ptr)< (signed) datasize) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) datasize) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		if (!strncmp(data, "SPRACTIO",8)) {
			new Sprite_Action_Chunk (this, (data + 12), (*(int *) (data + 8))-12);
			data += *(int *)(data + 8);
		}
		else {
			new Miscellaneous_Chunk (this, data, (data + 12), (*(int *) (data + 8)) -12 );
			data += *(int *)(data + 8);
		}

	}
}
/////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE_DECLARE_PARENT("SPRITEPS",Playstation_Sprite_Chunk,"SPRIHEAD",Sprite_Header_Chunk)

Playstation_Sprite_Chunk::Playstation_Sprite_Chunk(Sprite_Header_Chunk* parent,const char* data,size_t datasize)
:Chunk_With_Children(parent,"SPRITEPS")
{
	const char * buffer_ptr = data;

	
	while ((data-buffer_ptr)< (signed) datasize) {

		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) datasize) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}

		if (!strncmp(data, "SPRACTIO",8)) {
			new Sprite_Action_Chunk (this, (data + 12), (*(int *) (data + 8))-12);
			data += *(int *)(data + 8);
		}
		else {
			new Miscellaneous_Chunk (this, data, (data + 12), (*(int *) (data + 8)) -12 );
			data += *(int *)(data + 8);
		}

	}
}



////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SPRISIZE",Sprite_Size_Chunk)


Sprite_Size_Chunk::Sprite_Size_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPRISIZE")
{
	scale=*(double*)data;
	data+=8;
	maxy=*(double*)data;
	data+=8;
	maxx=*(double*)data;
	data+=8;
	radius=*(int*)data;
	data+=4;
	Flags=*(int*)data;
	data+=4;
}

Sprite_Size_Chunk::Sprite_Size_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPRISIZE")
{
	scale=15.625;
	maxy=1000;
	maxx=500;
	radius=0;
	Flags=0;
}

BOOL Sprite_Size_Chunk::output_chunk (HANDLE &hand)
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

size_t Sprite_Size_Chunk::size_chunk()
{
	chunk_size=44;
	return chunk_size;	
}

void Sprite_Size_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(double*)data_start=scale;
	data_start+=8;
	*(double*)data_start=maxy;
	data_start+=8;
	*(double*)data_start=maxx;
	data_start+=8;
	*(int*)data_start=radius;
	data_start+=4;
	*(int*)data_start=Flags;
	data_start+=4;
}
///////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SPRITVER",Sprite_Version_Number_Chunk)

Sprite_Version_Number_Chunk::Sprite_Version_Number_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPRITVER")
{
	version_num=*(int*)data;
	data+=4;
}

Sprite_Version_Number_Chunk::Sprite_Version_Number_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPRITVER")
{
	version_num=0;
}

BOOL Sprite_Version_Number_Chunk::output_chunk (HANDLE &hand)
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

size_t Sprite_Version_Number_Chunk::size_chunk()
{
	chunk_size=16;
	return chunk_size;	
}

void Sprite_Version_Number_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start=version_num;
}


//////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SPRBMPSC",Sprite_Bitmap_Scale_Chunk)


Sprite_Bitmap_Scale_Chunk::Sprite_Bitmap_Scale_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPRBMPSC")
{
	NumBitmaps=*(int*)data;
	data+=4;
	if(NumBitmaps)
	{
		Scale=new float[NumBitmaps];
		for(int i=0;i<NumBitmaps;i++)
		{
			Scale[i]=*(float*)data;
			data+=sizeof(float);
		}
	}
	else
		Scale=0;
}
Sprite_Bitmap_Scale_Chunk::Sprite_Bitmap_Scale_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPRBMPSC")
{
	NumBitmaps=0;
	Scale=0;
}
Sprite_Bitmap_Scale_Chunk::~Sprite_Bitmap_Scale_Chunk()
{
	delete [] Scale;
}
size_t Sprite_Bitmap_Scale_Chunk::size_chunk()
{
	chunk_size=16+sizeof(float)*NumBitmaps;
	return chunk_size;	
}

void Sprite_Bitmap_Scale_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start=NumBitmaps;
	data_start+=4;
	for(int i=0;i<NumBitmaps;i++)
	{
		*(float*)data_start=Scale[i];
		data_start+=sizeof(float);
	}
}
/////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SPRBMPCE",Sprite_Bitmap_Centre_Chunk)

Sprite_Bitmap_Centre_Chunk::Sprite_Bitmap_Centre_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPRBMPCE")
{
	NumBitmaps=*(int*)data;
	data+=4;
	if(NumBitmaps)
	{
		CentreX=new int[NumBitmaps];
		CentreY=new int[NumBitmaps];
		OffsetX=new int[NumBitmaps];
		OffsetY=new int[NumBitmaps];

		for(int i=0;i<NumBitmaps;i++)
		{
			CentreX[i]=*(int*)data;
			data+=4;
			CentreY[i]=*(int*)data;
			data+=4;
			OffsetX[i]=*(int*)data;
			data+=4;
			OffsetY[i]=*(int*)data;
			data+=4;
		}
	}
	else
	{
		CentreX=0;
		CentreY=0;
		OffsetX=0;
		OffsetY=0;
	}
	spare=*(int*)data;
}
Sprite_Bitmap_Centre_Chunk::Sprite_Bitmap_Centre_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPRBMPCE")
{
	NumBitmaps=0;
	CentreX=CentreY=0;
	OffsetX=OffsetY=0;
	spare=0;
}
Sprite_Bitmap_Centre_Chunk::~Sprite_Bitmap_Centre_Chunk()
{
	delete [] CentreX;
	delete [] CentreY;
	delete [] OffsetX;
	delete [] OffsetY;
}
size_t Sprite_Bitmap_Centre_Chunk::size_chunk()
{
	chunk_size=20+16*NumBitmaps;
	return chunk_size;	
}

void Sprite_Bitmap_Centre_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(int*)data_start=NumBitmaps;
	data_start+=4;
	for(int i=0;i<NumBitmaps;i++)
	{
		*(int*)data_start=CentreX[i];
		data_start+=4;
		*(int*)data_start=CentreY[i];
		data_start+=4;
		*(int*)data_start=OffsetX[i];
		data_start+=4;
		*(int*)data_start=OffsetY[i];
		data_start+=4;
	}
	*(int*)data_start=spare;
}
/////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("SPREXTEN",Sprite_Extent_Chunk)

Sprite_Extent_Chunk::Sprite_Extent_Chunk(Chunk_With_Children* parent,const char* data,size_t /*datasize*/)
: Chunk(parent, "SPREXTEN")
{
	minx=*(double*)data;
	data+=8;
	maxx=*(double*)data;
	data+=8;
	miny=*(double*)data;
	data+=8;
	maxy=*(double*)data;
	data+=8;
	spare1=*(int*)data;
	data+=4;
	spare2=*(int*)data;
	data+=4;
}
Sprite_Extent_Chunk::Sprite_Extent_Chunk(Chunk_With_Children* parent)
: Chunk(parent, "SPREXTEN")
{
	minx=miny=-1000;
	maxx=maxy=1000;
	spare1=spare2=0;
}
size_t Sprite_Extent_Chunk::size_chunk()
{
	chunk_size=12+40;
	return chunk_size;	
}

void Sprite_Extent_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*(double*)data_start=minx;
	data_start+=8;	
	*(double*)data_start=maxx;
	data_start+=8;	
	*(double*)data_start=miny;
	data_start+=8;	
	*(double*)data_start=maxy;
	data_start+=8;	

	*(int*)data_start=spare1;
	data_start+=4;
	*(int*)data_start=spare2;
	data_start+=4;
}
