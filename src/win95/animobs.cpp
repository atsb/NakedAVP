#include "hierchnk.hpp"
#include "animobs.hpp"
#include "list_tem.hpp"
#include <math.h>

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(animobs)

RIF_IMPLEMENT_DYNCREATE("OBASEQFR",Object_Animation_Sequence_Frame_Chunk)

void Object_Animation_Sequence_Frame_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;


	*((float *) data_start) = orientation.x;
	data_start += 4;
	*((float *) data_start) = orientation.y;
	data_start += 4;
	*((float *) data_start) = orientation.z;
	data_start += 4;
	*((float *) data_start) = orientation.w;
	data_start += 4;
	
	*((int *) data_start) = transform.x;
	data_start += 4;
	*((int *) data_start) = transform.y;
	data_start += 4;
	*((int *) data_start) = transform.z;
	data_start += 4;

	*((int *) data_start) = at_frame_no;
	data_start += 4;
	
	*((int *) data_start) = frame_ref_no;
	data_start += 4;
	
	*((int *) data_start) = flags;
	data_start += 4;
	
	*(int*) data_start=num_extra_data;
	data_start+=4;
	
	for (int i=0; i<num_extra_data; i++)
	{
		*((int *) data_start) = extra_data[i];
		data_start += 4;
	}
	
}

Object_Animation_Sequence_Frame_Chunk::Object_Animation_Sequence_Frame_Chunk (Chunk_With_Children * parent,const char *data_start, size_t)
: Chunk (parent, "OBASEQFR")
{
	orientation.x = *((float *) data_start);
	data_start += 4;
	orientation.y = *((float *) data_start);
	data_start += 4;
	orientation.z = *((float *) data_start);
	data_start += 4;
	orientation.w = *((float *) data_start);
	data_start += 4;

	transform.x = *((int *) data_start);
	data_start += 4;
	transform.y = *((int *) data_start);
	data_start += 4;
	transform.z = *((int *) data_start);
	data_start += 4;

	at_frame_no = *((int *) data_start);
	data_start += 4;
	
	frame_ref_no = *((int *) data_start);
	data_start += 4;
	
	flags = *((int *) data_start);
	data_start += 4;
	
	num_extra_data=*(int*) data_start;
	data_start+=4;
	
	if(num_extra_data)
		extra_data=new int[num_extra_data];
	else
		extra_data=0;
	
	
	for (int i=0; i<num_extra_data; i++)
	{
		extra_data[i] =	*((int *) data_start);
		data_start += 4;
	}
	
}

void Object_Animation_Sequence_Frame_Chunk::set_sound_index(int ind)
{
	if(ind>=0 && ind<=127)
	{
		flags &=~HierarchyFrame_SoundIndexMask;
		flags |= (ind<<24);
	}
}

RIF_IMPLEMENT_DYNCREATE("OBASEQHD",Object_Animation_Sequence_Header_Chunk)

void Object_Animation_Sequence_Header_Chunk::fill_data_block (char *data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*((int *) data_start) = num_frames;
	data_start += 4;

	*((int *) data_start) = sequence_number;
	data_start += 4;

	*((int *) data_start) = sub_sequence_number;
	data_start += 4;

	*(int*)data_start=num_extra_data;
	data_start+=4;

	for (int i=0; i<num_extra_data; i++)
	{
		*((int *) data_start) = extra_data[i];
		data_start += 4;
	}
	
	if (sequence_name)
	{
		sprintf (data_start, "%s", sequence_name);
	}
	else
	{
		*data_start = 0;
	}
	
}

Object_Animation_Sequence_Header_Chunk::~Object_Animation_Sequence_Header_Chunk()
{
	if (sequence_name)
		delete[] sequence_name;
	if(extra_data)
		delete[] extra_data;
}

Object_Animation_Sequence_Header_Chunk::Object_Animation_Sequence_Header_Chunk (Chunk_With_Children * parent,const char * data_start, size_t)
: Chunk (parent, "OBASEQHD"), sequence_name (0)
{
	num_frames = *((int *) data_start);
	data_start += 4;

	sequence_number = *((int *) data_start);
	data_start += 4;

	sub_sequence_number = *((int *) data_start);
	data_start += 4;

	num_extra_data=*(int*) data_start;
	data_start+=4;

	if(num_extra_data)
		extra_data=new int[num_extra_data];
	else
		extra_data=0;
	
	
	for (int i=0; i<num_extra_data; i++)
	{
		extra_data[i] =	*((int *) data_start);
		data_start += 4;
	}
	
	
	if (strlen(data_start))
	{
		sequence_name = new char [strlen(data_start) + 1];
		strcpy (sequence_name, data_start);
	}
}

Object_Animation_Sequence_Header_Chunk * Object_Animation_Sequence_Chunk::get_header()
{
	return(Object_Animation_Sequence_Header_Chunk *) lookup_single_child("OBASEQHD");
}

void Object_Animation_Sequence_Chunk::get_frames(List <Object_Animation_Sequence_Frame_Chunk*> *pList)
{
	List <Chunk *> cl;
	lookup_child ("OBASEQFR",cl);
		
	for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
	{
		pList->add_entry((Object_Animation_Sequence_Frame_Chunk *)cli());
	}	
}

////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("OBANSEQS",Object_Animation_Sequences_Chunk)

void Object_Animation_Sequences_Chunk::list_sequences(List <Object_Animation_Sequence_Chunk *> * pList)
{
	List <Chunk *> cl; 
	lookup_child ("OBANSEQC",cl);
	
	for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
	{
		pList->add_entry((Object_Animation_Sequence_Chunk *)cli());
	}
}


Object_Animation_Sequence_Chunk::Object_Animation_Sequence_Chunk(Object_Animation_Sequences_Chunk* parent,Object_Animation_Sequence_Chunk* template_seq,ChunkQuat & orient ,ChunkVectorInt & trans)
:Chunk_With_Children (parent, "OBANSEQC")
{
	Object_Animation_Sequence_Header_Chunk* template_header=template_seq->get_header();
	Object_Animation_Sequence_Header_Chunk* header=new Object_Animation_Sequence_Header_Chunk(this);

	header->num_frames=65536;
	header->sequence_number=template_header->sequence_number;
	header->sub_sequence_number=template_header->sub_sequence_number;
	header->sequence_name=new char[strlen(template_header->sequence_name)+1];
	strcpy(header->sequence_name,template_header->sequence_name);

	Object_Animation_Sequence_Frame_Chunk* oasfc=new Object_Animation_Sequence_Frame_Chunk(this);
	
	oasfc->orientation=orient;
	oasfc->transform=trans;
	oasfc->at_frame_no=0;
	oasfc->frame_ref_no=0;	

	//see if template sequence is a delta sequence
	List<Object_Animation_Sequence_Frame_Chunk*> framelist;
	template_seq->get_frames(&framelist);
	while(framelist.size())
	{
		if(framelist.first_entry()->flags & HierarchyFrameFlag_DeltaFrame)
		{
			oasfc->flags=HierarchyFrameFlag_DeltaFrame;
			break;
		}
		framelist.delete_first_entry();
	}	
}

Object_Animation_Sequence_Chunk * Object_Animation_Sequences_Chunk::get_sequence (int num, int subnum)
{
	List <Object_Animation_Sequence_Chunk *> seq_list;
	list_sequences(&seq_list);
	
	LIF<Object_Animation_Sequence_Chunk *> sli(&seq_list);
	for (; !sli.done(); sli.next())
	{
		Object_Animation_Sequence_Header_Chunk * oashc = sli()->get_header();
		if (oashc)
		{
			if (oashc->sequence_number == num && oashc->sub_sequence_number == subnum)
			{
				break;
			}
		}
	}
	
	if (!sli.done())
	{
		return(sli());
	}
	return 0;
}

int Object_Animation_Sequence_Chunk::get_sequence_time()
{
	Object_Animation_Sequence_Time_Chunk* time_chunk=(Object_Animation_Sequence_Time_Chunk*)lookup_single_child("OBASEQTM");
	if(time_chunk)
	{
		return time_chunk->sequence_time;
	}
	return 0;
}

int Object_Animation_Sequence_Chunk::get_sequence_speed()
{
	Object_Animation_Sequence_Speed_Chunk* speed_chunk=(Object_Animation_Sequence_Speed_Chunk*)lookup_single_child("OBASEQSP");
	if(speed_chunk)
	{
		return speed_chunk->sequence_speed;
	}
	return 0;
}

BOOL Object_Animation_Sequence_Chunk::get_sequence_vector(ChunkVectorFloat* direction)
{
	if(!direction) return FALSE;

	//default direction is forwards
	direction->x=0;
	direction->y=0;
	direction->z=1;

	
	Object_Animation_Sequence_Speed_Chunk* speed_chunk=(Object_Animation_Sequence_Speed_Chunk*)lookup_single_child("OBASEQSP");
	if(speed_chunk)
	{
		double radian_angle=(speed_chunk->angle/360.0)*2*3.1415278;
		direction->x =(float) sin(radian_angle);
		direction->z =(float) cos(radian_angle);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int Object_Animation_Sequence_Chunk::get_sequence_flags()
{
	Object_Animation_Sequence_Flags_Chunk* flag_chunk=(Object_Animation_Sequence_Flags_Chunk*)lookup_single_child("OBASEQFL");
	if(flag_chunk)
	{
		return flag_chunk->flags;
	}
	return 0;
}

void Object_Animation_Sequence_Chunk::set_sequence_flags(int new_flags)
{
	//find existing flag_chunk , or create a new one
	Object_Animation_Sequence_Flags_Chunk* flag_chunk=(Object_Animation_Sequence_Flags_Chunk*)lookup_single_child("OBASEQFL");
	if(flag_chunk)
	{
		//set the flags
		flag_chunk->flags = new_flags;
	}
	else
	{
		//create a new chunk then
		new Object_Animation_Sequence_Flags_Chunk(this,new_flags);
	}
}

Hierarchy_Bounding_Box_Chunk* Object_Animation_Sequence_Chunk::get_bounding_box()
{
   return (Hierarchy_Bounding_Box_Chunk*)lookup_single_child("HIERBBOX");
}

////////////////////////////////////////////////////////////////////////////////
				/*--------------------------------------**
				** Object_Animation_Sequence_Time_Chunk **
				**--------------------------------------*/

RIF_IMPLEMENT_DYNCREATE("OBASEQTM",Object_Animation_Sequence_Time_Chunk)

Object_Animation_Sequence_Time_Chunk::Object_Animation_Sequence_Time_Chunk (Chunk_With_Children * parent,const char * data_start, size_t)
: Chunk (parent, "OBASEQTM")
{
	sequence_time=*(unsigned int*) data_start;	
}

void Object_Animation_Sequence_Time_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(unsigned int*) data_start = sequence_time;
}

////////////////////////////////////////////////////////////////////////////////
				/*---------------------------------------**
				** Object_Animation_Sequence_Speed_Chunk **
				**---------------------------------------*/

RIF_IMPLEMENT_DYNCREATE("OBASEQSP",Object_Animation_Sequence_Speed_Chunk)

Object_Animation_Sequence_Speed_Chunk::Object_Animation_Sequence_Speed_Chunk (Chunk_With_Children * parent,const char * data, size_t)
: Chunk (parent, "OBASEQSP")
{
	CHUNK_EXTRACT(sequence_speed,int)
	CHUNK_EXTRACT(angle,int)
	CHUNK_EXTRACT(spare,int)
}

void Object_Animation_Sequence_Speed_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START
	CHUNK_FILL(sequence_speed,int)
	CHUNK_FILL(angle,int)
	CHUNK_FILL(spare,int)
}

////////////////////////////////////////////////////////////////////////////////
				/*---------------------------------------**
				** Object_Animation_Sequence_Flags_Chunk **
				**---------------------------------------*/


RIF_IMPLEMENT_DYNCREATE("OBASEQFL",Object_Animation_Sequence_Flags_Chunk)

Object_Animation_Sequence_Flags_Chunk::Object_Animation_Sequence_Flags_Chunk (Chunk_With_Children * parent,const char * data, size_t)
: Chunk (parent, "OBASEQFL")
{
	CHUNK_EXTRACT(flags,int)
}

void Object_Animation_Sequence_Flags_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START
	CHUNK_FILL(flags,int)
}

////////////////////////////////////////////////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("OBANALLS",Object_Animation_All_Sequence_Chunk)

Object_Animation_All_Sequence_Chunk::Object_Animation_All_Sequence_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"OBANALLS")
{
	num_sequences=0;
	sequences=0;
}

Object_Animation_All_Sequence_Chunk::Object_Animation_All_Sequence_Chunk(Chunk_With_Children * const parent,const char * data, const size_t)
:Chunk(parent,"OBANALLS")
{
	num_sequences=*(int*)data;
	data+=4;

	if(num_sequences) sequences=new Object_Animation_Sequence[num_sequences];	
	else sequences=0;

	for(int i=0;i<num_sequences;i++)
	{
		Object_Animation_Sequence* seq=&sequences[i];
		seq->num_frames=*(int*)data;
		data+=4;
		seq->sequence_number=*(int*)data;
		data+=4;
		seq->sub_sequence_number=*(int*)data;
		data+=4;
		seq->sequence_time=*(int*)data;
		data+=4;

		if(seq->num_frames) seq->frames=new Object_Animation_Frame[seq->num_frames];
		else seq->frames=0;

		for(unsigned j=0;j<seq->num_frames;j++)
		{
			seq->frames[j]=*(Object_Animation_Frame*)data;
			data+=sizeof(Object_Animation_Frame);
		}
	}
}

void Object_Animation_All_Sequence_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;

	*(int*)data=num_sequences;
	data+=4;

	for(int i=0;i<num_sequences;i++)
	{
		Object_Animation_Sequence* seq=&sequences[i];
		*(int*)data=seq->num_frames;
		data+=4;
		*(int*)data=seq->sequence_number;
		data+=4;
		*(int*)data=seq->sub_sequence_number;
		data+=4;
		*(int*)data=seq->sequence_time;
		data+=4;

		for(unsigned j=0;j<seq->num_frames;j++)
		{
			*(Object_Animation_Frame*)data=seq->frames[j];
			data+=sizeof(Object_Animation_Frame);
		}
		
	}
}

size_t Object_Animation_All_Sequence_Chunk::size_chunk()
{
	chunk_size=12+4;
	chunk_size+=num_sequences*16;
	for(int i=0;i<num_sequences;i++)
	{
		chunk_size+=sequences[i].num_frames*sizeof(Object_Animation_Frame);
	}
	return chunk_size;
}


////////////////////////////////////////////////////////////////////////////////

//loader for Object_Animation_Sequences_Chunk
CHUNK_WITH_CHILDREN_LOADER("OBANSEQS",Object_Animation_Sequences_Chunk)



//Object_Animation_Sequence_Chunk 

RIF_IMPLEMENT_DYNCREATE("OBANSEQC",Object_Animation_Sequence_Chunk)
//loader for Object_Animation_Sequence_Chunk
CHUNK_WITH_CHILDREN_LOADER("OBANSEQC",Object_Animation_Sequence_Chunk)
