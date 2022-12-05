#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "string.hpp"

#include "list_tem.hpp"
#include "chnkload.hpp"
#include "oechunk.h"
#include "stratdef.h"
//#include "bh_types.h"
#include "shpchunk.hpp"
#include "envchunk.hpp"
#include "obchunk.hpp"
#include "chunkpal.hpp"
#include "bmpnames.hpp"
#include "ltchunk.hpp"
#include "chnktexi.h"
#include "sprchunk.hpp"
#include "gsprchnk.hpp"
#include "animchnk.hpp"
#include "fragchnk.hpp"
#include "jsndsup.h"
#include "mempool.h"
#include <math.h>
// for log file
void SetupAnimatedTextures(Shape_Chunk* sc,SHAPEHEADER* shp,Animation_Chunk* ac,Shape_Merge_Data_Chunk* smdc);
void SetupAnimOnTriangle(SHAPEHEADER* shp,TEXANIM* ta,int poly);
void SetupAnimOnQuad(Shape_Chunk* sc,SHAPEHEADER* shp,TEXANIM* ta1,TEXANIM* ta2,int poly);

// what we need to do for now is load shapes into the mainshapelist
// and objects into the Mapheader - Map


double local_scale;

File_Chunk * Env_Chunk = 0;

RIFFHANDLE current_rif_handle;

unsigned char const * PaletteMapTable;

//////////////////////////////////////////////////////////
extern LOADED_SOUND const * GetSoundForMainRif(const char* wav_name);
extern char * extsounddir ;
extern char * sounddir ;

struct Shape_Fragment_Type
{
	Shape_Fragment_Type(const char*);
	~Shape_Fragment_Type();
	void AddShape(SHAPEHEADER*);
	void Setup_sh_frags(Fragment_Type_Chunk*);

	char* name;
	List<SHAPEHEADER*> shapelist;
	SHAPEFRAGMENTDESC* sh_fragdesc;
};

Shape_Fragment_Type::Shape_Fragment_Type(const char* _name)
{
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
	sh_fragdesc=0;
}

Shape_Fragment_Type::~Shape_Fragment_Type()
{
	while(shapelist.size())
	{
		shapelist.first_entry()->sh_fragdesc=0;
		delete shapelist.first_entry();
	}
	if(sh_fragdesc)
	{
		#if USE_LEVEL_MEMORY_POOL
		if(sh_fragdesc->sh_fragsound)
		{
			if(sh_fragdesc->sh_fragsound->sound_loaded) 
				LoseSound(sh_fragdesc->sh_fragsound->sound_loaded);
		}
		#else
		if(sh_fragdesc->sh_frags)DeallocateMem(sh_fragdesc->sh_frags);
		if(sh_fragdesc->sh_fragsound)
		{
			if(sh_fragdesc->sh_fragsound->sound_loaded) 
				LoseSound(sh_fragdesc->sh_fragsound->sound_loaded);
			DeallocateMem(sh_fragdesc->sh_fragsound);
		}
		DeallocateMem(sh_fragdesc);
		#endif

	}
	if(name) delete[] name;
}

void Shape_Fragment_Type::AddShape(SHAPEHEADER* shp)
{
	shapelist.add_entry(shp);
	if(sh_fragdesc) shp->sh_fragdesc=sh_fragdesc;
}


void Shape_Fragment_Type::Setup_sh_frags(Fragment_Type_Chunk* ftc)
{
	if(sh_fragdesc) return;
	List<Chunk*> chlist;
	ftc->lookup_child("FRGTYPSC",chlist);

	sh_fragdesc=(SHAPEFRAGMENTDESC*)PoolAllocateMem(sizeof(SHAPEFRAGMENTDESC));
	for(LIF<SHAPEHEADER*> slif(&shapelist);!slif.done();slif.next())
	{
		slif()->sh_fragdesc=sh_fragdesc;
	}
	
	
	sh_fragdesc->sh_frags = (SHAPEFRAGMENT *)PoolAllocateMem((chlist.size()+1) * sizeof(SHAPEFRAGMENT));
	int pos=0;
	while(chlist.size())
	{
		Fragment_Type_Shape_Chunk* ftsc=(Fragment_Type_Shape_Chunk*)chlist.first_entry();
		
		int shapeindex=GetLoadedShapeMSL(ftsc->name);
		if(shapeindex!=-1)
		{
			sh_fragdesc->sh_frags[pos].ShapeIndex=shapeindex;
			sh_fragdesc->sh_frags[pos].NumFrags=ftsc->num_fragments;
			
			sh_fragdesc->sh_frags[pos].x_offset = 0;
			sh_fragdesc->sh_frags[pos].y_offset = 0;
			sh_fragdesc->sh_frags[pos].z_offset = 0;
			pos++;
		}
		
		chlist.delete_first_entry();
	}
	sh_fragdesc->sh_frags[pos].ShapeIndex = -1;	
	sh_fragdesc->sh_frags[pos].NumFrags = -1;

	sh_fragdesc->sh_fragsound=0;
	
	Chunk * pChunk = ftc->lookup_single_child("FRGSOUND");
	if(pChunk)
	{
		Fragment_Type_Sound_Chunk* ftsoc=(Fragment_Type_Sound_Chunk*) pChunk;
		
		
		sh_fragdesc->sh_fragsound=(SHAPEFRAGMENTSOUND*)PoolAllocateMem(sizeof(SHAPEFRAGMENTSOUND));
		sh_fragdesc->sh_fragsound->sound_loaded=GetSoundForMainRif (ftsoc->wav_name);
		sh_fragdesc->sh_fragsound->inner_range=(unsigned long)(ftsoc->inner_range*local_scale);
		sh_fragdesc->sh_fragsound->outer_range=(unsigned long)(ftsoc->outer_range*local_scale);
		sh_fragdesc->sh_fragsound->pitch=ftsoc->pitch;
		sh_fragdesc->sh_fragsound->max_volume=ftsoc->max_volume;

	}

	
}


static List<Shape_Fragment_Type*> FragList;

void ApplyFragTypeToShape(SHAPEHEADER* shp,const char* name)
{
	for(LIF<Shape_Fragment_Type*> flif(&FragList);!flif.done();flif.next())
	{
		if(!_stricmp(flif()->name,name))
		{
			flif()->AddShape(shp);
			return;
		}
	}
	Shape_Fragment_Type* sft=new Shape_Fragment_Type(name);
	sft->AddShape(shp);
	FragList.add_entry(sft);		
}

void SetupFragmentType(Fragment_Type_Chunk* ftc)
{
	const char* name=ftc->get_name();
	if(!name) return;
	
	Shape_Fragment_Type* sft=0;
	for(LIF<Shape_Fragment_Type*> flif(&FragList);!flif.done();flif.next())
	{
		if(!_stricmp(flif()->name,name))
		{
			sft=flif();
			break;
		}
	}
	if(!sft)
	{
		return;
	}

	sft->Setup_sh_frags(ftc);
}

void DeallocateFragments(SHAPEHEADER* shp,SHAPEFRAGMENTDESC* sh_fragdesc)
{
	for(LIF<Shape_Fragment_Type*> flif(&FragList);!flif.done();flif.next())
	{
		if(flif()->sh_fragdesc==sh_fragdesc)
		{
			flif()->shapelist.delete_entry(shp);
			if(!flif()->shapelist.size())
			{
				//no more shapes use this fragment type - so deallocate it
				delete flif();
				flif.delete_current();
			}
			return;
		}
	}
	//sh_fragdesc not generated by a fragment type so deallocate it.
	#if USE_LEVEL_MEMORY_POOL
	if(sh_fragdesc->sh_fragsound)
	{
		if(sh_fragdesc->sh_fragsound->sound_loaded) 
			LoseSound(sh_fragdesc->sh_fragsound->sound_loaded);
	}
	#else
	if(sh_fragdesc->sh_frags)DeallocateMem(sh_fragdesc->sh_frags);
	if(sh_fragdesc->sh_fragsound)
	{
		if(sh_fragdesc->sh_fragsound->sound_loaded) 
			LoseSound(sh_fragdesc->sh_fragsound->sound_loaded);
		DeallocateMem(sh_fragdesc->sh_fragsound);
	}
	DeallocateMem(sh_fragdesc);
	#endif
}

void DeallocateAllFragments()
{
	while(FragList.size())
	{
		Shape_Fragment_Type* frag_type=FragList.first_entry();

		while(frag_type->shapelist.size())
		{
			frag_type->shapelist.delete_first_entry();
		}
		frag_type->sh_fragdesc=0;
		delete frag_type;

		FragList.delete_first_entry();
	}
}

/////////////////////////////////////////
/////////////////////////////////////////
// Hold data about chunk loaded shapes //
/////////////////////////////////////////
/////////////////////////////////////////

class ShapeInMSL
{
private:
	void AddToHashTables();
	void RemoveFromHashTables();

	#define SIM_HASH_BITS 6
	#define SIM_HASH_SIZE (1<<SIM_HASH_BITS)
	#define SIM_HASH_MASK (SIM_HASH_SIZE-1)

	static List<ShapeInMSL const *> hash_msl[];
	static List<ShapeInMSL const *> hash_ptr[];
	static List<ShapeInMSL const *> hash_name[];

	static int HashMSLFunc(int);
	static int HashPtrFunc(SHAPEHEADER *);
	static int HashNameFunc(char const *);


	int listpos;
	SHAPEHEADER * shptr;
	String name;
	BOOL in_hash_table;
	
public:

	inline int Listpos() const { return listpos; }
	inline SHAPEHEADER * Shptr() const { return shptr; }
	inline char const * Name() const { return name; }

	static ShapeInMSL const * GetByName(char const *);
	static ShapeInMSL const * GetByMSL(int);
	static ShapeInMSL const * GetByPtr(SHAPEHEADER *);
	static void PurgeMSLShapeList();

	ShapeInMSL();
	ShapeInMSL(int _p);
	ShapeInMSL(SHAPEHEADER * _s, char const * _n, int _p);
	ShapeInMSL(ShapeInMSL const &);
	ShapeInMSL & operator = (ShapeInMSL const &);
	~ShapeInMSL();

	BOOL operator == (ShapeInMSL const & s2) const
		{ return (GLS_NOTINLIST==listpos && GLS_NOTINLIST==s2.listpos) ? shptr == s2.shptr : listpos == s2.listpos; }
	inline BOOL operator != (ShapeInMSL const & s2) const { return ! operator == (s2); }
};

void ShapeInMSL::AddToHashTables()
{
	if (GLS_NOTINLIST != listpos)
		hash_msl[HashMSLFunc(listpos)].add_entry(this);
	hash_ptr[HashPtrFunc(shptr)].add_entry(this);
	hash_name[HashNameFunc(name)].add_entry(this);

	in_hash_table = TRUE;
}

void ShapeInMSL::RemoveFromHashTables()
{
	if (GLS_NOTINLIST != listpos)
		hash_msl[HashMSLFunc(listpos)].delete_entry(this);
	hash_ptr[HashPtrFunc(shptr)].delete_entry(this);
	hash_name[HashNameFunc(name)].delete_entry(this);

	in_hash_table = FALSE;
}

List<ShapeInMSL const *> ShapeInMSL::hash_msl[SIM_HASH_SIZE];
List<ShapeInMSL const *> ShapeInMSL::hash_ptr[SIM_HASH_SIZE];
List<ShapeInMSL const *> ShapeInMSL::hash_name[SIM_HASH_SIZE];

int ShapeInMSL::HashMSLFunc(int pos)
{
	return pos & SIM_HASH_MASK;
}

int ShapeInMSL::HashPtrFunc(SHAPEHEADER * shp)
{
	size_t p = (size_t)shp;

	while (p>=SIM_HASH_SIZE)
		p = (p & SIM_HASH_MASK) ^ (p>>SIM_HASH_BITS);

	return (int)p;
}

int ShapeInMSL::HashNameFunc(char const * nam)
{
	int v = 0;

	while (*nam) v += (unsigned char)toupper(*nam++);

	return v & SIM_HASH_MASK;
}

ShapeInMSL const * ShapeInMSL::GetByMSL(int pos)
{
	for (LIF<ShapeInMSL const *> i(&hash_msl[HashMSLFunc(pos)]); !i.done(); i.next())
	{
		if (i()->listpos == pos) return i();
	}
	return 0;
}

ShapeInMSL const * ShapeInMSL::GetByPtr(SHAPEHEADER * shp)
{
	for (LIF<ShapeInMSL const *> i(&hash_ptr[HashPtrFunc(shp)]); !i.done(); i.next())
	{
		if (i()->shptr == shp) return i();
	}
	return 0;
}

ShapeInMSL const * ShapeInMSL::GetByName(char const * nam)
{
	for (LIF<ShapeInMSL const *> i(&hash_name[HashNameFunc(nam)]); !i.done(); i.next())
	{
		if (!_stricmp(i()->name,nam)) return i();
	}
	return 0;
}

ShapeInMSL::ShapeInMSL()
: listpos(GLS_NOTINLIST)
, shptr(0)
, in_hash_table(FALSE)
{
}

ShapeInMSL::ShapeInMSL(int _p)
: listpos(_p)
, shptr(0)
, in_hash_table(FALSE)
{
}

ShapeInMSL::ShapeInMSL(SHAPEHEADER * _s, char const * _n, int _p)
: listpos(_p)
, shptr(_s)
, name(_n)
, in_hash_table(FALSE)
{
	AddToHashTables();
}

ShapeInMSL::ShapeInMSL(ShapeInMSL const & sim)
: listpos(sim.listpos)
, shptr(sim.shptr)
, name(sim.name)
, in_hash_table(FALSE)
{
	if (sim.in_hash_table) AddToHashTables();
}

ShapeInMSL & ShapeInMSL::operator = (ShapeInMSL const & sim)
{
	if (&sim != this)
	{
		if (in_hash_table) RemoveFromHashTables();
		shptr = sim.shptr;
		name = sim.name;
		listpos = sim.listpos;
		if (sim.in_hash_table) AddToHashTables();
	}
	return *this;
}

ShapeInMSL::~ShapeInMSL()
{
	if (in_hash_table) RemoveFromHashTables();
}


static List<ShapeInMSL*> msl_shapes;

void ShapeInMSL::PurgeMSLShapeList()
{
	for(int i=0;i<SIM_HASH_SIZE;i++)
	{
		while(hash_msl[i].size())hash_msl[i].delete_first_entry();
		while(hash_ptr[i].size())hash_ptr[i].delete_first_entry();
		while(hash_name[i].size())hash_name[i].delete_first_entry();
	}
	
	while(msl_shapes.size())	
	{
		ShapeInMSL* shp_msl=msl_shapes.first_entry();
		shp_msl->in_hash_table=FALSE;
		delete shp_msl;
		msl_shapes.delete_first_entry();	
	}
}
void PurgeMSLShapeList()
{
	ShapeInMSL::PurgeMSLShapeList();
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

extern "C"
{
	extern unsigned char *TextureLightingTable;
	extern int ScanDrawMode;
};


/////////////////////////////////////////
// Functions which operate on RIFFHANDLEs
/////////////////////////////////////////

// load a rif file into memory
RIFFHANDLE load_rif (const char * fname)
{
	File_Chunk * fc = new File_Chunk (fname);

	if (fc->error_code != 0)
	{
		delete fc;
		#if OUTPUT_LOG
		CL_LogFile.lprintf("FAILED TO LOAD RIF: %s\n",fname);
		#endif
	   	ReleaseDirect3D();

		fprintf(stderr, "load_rif: Error loading %s\n", fname);
		exit(0x111);
		return INVALID_RIFFHANDLE;
	}
	#if OUTPUT_LOG
	CL_LogFile.lprintf("Successfully Loaded RIF: %s\n",fname);
	#endif

	RIFFHANDLE h = current_rif_handle = new _RifHandle;
	h->fc = Env_Chunk = fc;
	
	Chunk * pChunk = fc->lookup_single_child("REBENVDT");
	if (pChunk)
	{
		h->envd = (Environment_Data_Chunk *)pChunk;
	}

	return h;
}

RIFFHANDLE load_rif_non_env (const char * fname)
{
	File_Chunk * fc = new File_Chunk (fname);

	if (fc->error_code != 0)
	{
		delete fc;
		#if OUTPUT_LOG
		CL_LogFile.lprintf("FAILED TO LOAD RIF: %s\n",fname);
		#endif
		
	   	ReleaseDirect3D();
		fprintf(stderr, "load_rif_non_env: Error loading %s\n", fname);		
		exit(0x111);

		return INVALID_RIFFHANDLE;
	}
	#if OUTPUT_LOG
	CL_LogFile.lprintf("Successfully Loaded RIF: %s\n",fname);
	#endif

	RIFFHANDLE h = current_rif_handle = new _RifHandle;
	h->fc = fc;
	
	Chunk * pChunk = fc->lookup_single_child("REBENVDT");
	if (pChunk)
	{
		h->envd = (Environment_Data_Chunk *)pChunk;
	}

	return h;
}


// deallocate the shapes, unload the rif, close the handle
void undo_rif_load (RIFFHANDLE h)
{
	deallocate_loaded_shapes(h);
	unload_rif(h);
	close_rif_handle(h);
}

// deallocate the shapes copied from the rif
void deallocate_loaded_shapes (RIFFHANDLE h)
{
	
	// because the SHAPEHEADER is calloced, we can
	// just delete the arrays we want
	
	LIF<ShapeInMSL*> msl_shape_lif(&msl_shapes);
	
	while (h->shape_nums.size())
	{
		#if !StandardShapeLanguage
		#error Must have standard shape language
		#endif

		int list_pos = h->shape_nums.first_entry();
		h->shape_nums.delete_first_entry();
		
		DeallocateLoadedShapeheader(mainshapelist[list_pos]);

		FreeMSLPos(list_pos);

		for(msl_shape_lif.restart();!msl_shape_lif.done();msl_shape_lif.next())
		{
			if(list_pos==msl_shape_lif()->Listpos())
			{
				delete msl_shape_lif();
				msl_shape_lif.delete_current();
				break;
			}
		}
	}

	// ?????????? FIXME
	if (Map[0].MapType6Objects)
	{
		DeallocateMem (Map[0].MapType6Objects);
		Map[0].MapType6Objects = 0;
	}
}

// unloads the rif but keeps the handle and associated copied shapes
void unload_rif (RIFFHANDLE h)
{
	if (h->fc)
	{
		if (h->fc == Env_Chunk)
			Env_Chunk = 0;
		delete h->fc;
		h->fc = 0;
	}
	h->envd = 0;
	h->palparent = 0;
	h->max_index = 0;
	if (h->tex_index_nos) delete[] h->tex_index_nos;
	h->tex_index_nos = 0;
}

// close the handle - performs tidying up and memory deallocation
void close_rif_handle (RIFFHANDLE h)
{
	delete h;
}


//////////////////////////////////////////////////////////



// copies sprite to msl
int copy_sprite_to_mainshapelist(RIFFHANDLE h, Sprite_Header_Chunk * shc, int/* flags*/)
{
	int list_pos = GetMSLPos();
	
	copy_sprite_to_shapeheader (h, mainshapelist[list_pos], shc, list_pos);
	
	post_process_shape(mainshapelist[list_pos]);

	h->shape_nums.add_entry(list_pos);

	return list_pos;
}

static void setup_tex_conv_array (
	int & max_indices, 
	int * & conv_array, 
	RIFFHANDLE h, 
	Chunk_With_Children * tmpshp
	)
{
	String rif_name;

	max_indices = h->max_index;
	conv_array = h->tex_index_nos;
	
	// find out if its come from elsewhere!!!!!!!
	// Doo Dee Doo Doh
	// Just come back from the pub - sorry

	Chunk * pChunk = tmpshp->lookup_single_child("SHPEXTFL");
	
	Shape_External_File_Chunk * seflc = 0;
	Bitmap_List_Store_Chunk * blsc = 0;
	
	
	if (pChunk) 
	{
		seflc = (Shape_External_File_Chunk *)pChunk;
		pChunk = seflc->lookup_single_child("BMPLSTST");
		if (pChunk)
		{
			blsc = (Bitmap_List_Store_Chunk *) pChunk;
		}
		pChunk = seflc->lookup_single_child("RIFFNAME");
		if (pChunk)
		{
			rif_name = ((RIF_Name_Chunk *)pChunk)->rif_name;
		}
	}	

	if (blsc)	
	{
	
		// load in the textures from the shape
	
		max_indices = 0;
		LIF<BMP_Name> bns (&blsc->bmps);
		
		for (; !bns.done(); bns.next())
		{
			max_indices = max(bns().index,max_indices);
		}
		
		conv_array = new int [max_indices+1];
		for (int i=0; i<=max_indices; i++)
		{
			conv_array[i] = -1;
		}

		if (Env_Chunk == 0)
			Env_Chunk = h->fc;
		// JH 17-2-97 -- image loaders have changed to avoid loading the same image twice
		for (bns.restart() ; !bns.done(); bns.next())
		{
			if (!(bns().flags & ChunkBMPFlag_NotInPC))
			{
				String tex;
				if (bns().flags & ChunkBMPFlag_IFF)
				{
					tex = bns().filename;
				}
				else
				{
					tex = rif_name;
					tex += "\\";
					tex += bns().filename;
				}

				int imgnum = load_rif_bitmap(tex,bns().flags);
				if (GEI_NOTLOADED != imgnum)
					conv_array[bns().index] = imgnum;
			}
		}
		
	}
	
}

void CopyShapeAnimationHeader(SHAPEHEADER* shpfrom,SHAPEHEADER* shpto)
{
	GLOBALASSERT(shpfrom->numitems==shpto->numitems);
	GLOBALASSERT(shpfrom->animation_header);
	shpto->animation_header=shpfrom->animation_header;
	shpto->animation_header->num_shapes_using_this++;
	//find a sequence which has some frames;
	
	shapeanimationsequence* sas=0;
	for(int i=0;i<shpto->animation_header->num_sequences;i++)
	{
		sas=&shpto->animation_header->anim_sequences[i];
		if(sas->num_frames)
			break;
		
	}
	GLOBALASSERT(i<shpto->animation_header->num_sequences);
	
	//copy the pointers for the first frame of this sequence
	#if !USE_LEVEL_MEMORY_POOL
	DeallocateMem(shpto->points[0]);
	DeallocateMem(shpto->sh_normals[0]);
	DeallocateMem(shpto->sh_vnormals[0]);
	#endif

	shpto->points[0]=sas->anim_frames[0].vertices;
	shpto->sh_normals[0]=sas->anim_frames[0].item_normals;
	shpto->sh_vnormals[0]=sas->vertex_normals;

}

// copies shape to msl
CTM_ReturnType copy_to_mainshapelist(RIFFHANDLE h, Shape_Chunk * tmpshp, int flags,const ChunkObject* object)
{
	int local_max_index;
	int * local_tex_index_nos;

	int list_pos = GetMSLPos();
	int main_shape_num = list_pos;
	int start_shape_no = list_pos;
	String rif_name;
	
	setup_tex_conv_array (local_max_index, local_tex_index_nos, h, tmpshp);
	
	
	Shape_Preprocessed_Data_Chunk* spdc=(Shape_Preprocessed_Data_Chunk*)tmpshp->lookup_single_child("SHPPRPRO");
	if(spdc)
	{
		copy_preprocessed_to_shapeheader (
			h, 
			spdc, 
			mainshapelist[list_pos], 
			tmpshp, 
			flags, 
			local_max_index,
			local_tex_index_nos,
			list_pos,
			object
			);
	}
	else
	{

		copy_to_shapeheader (
			h, 
			tmpshp->shape_data, 
			mainshapelist[list_pos], 
			tmpshp, 
			flags, 
			local_max_index,
			local_tex_index_nos,
			list_pos,
			object
			);
	}

	Shape_External_File_Chunk * seflc = 0;
	
	Chunk * pChunk = tmpshp->lookup_single_child("SHPEXTFL");

	if (pChunk) 
	{
		seflc = (Shape_External_File_Chunk *)pChunk;
		rif_name = seflc->get_shape_name();
		msl_shapes.add_entry(new ShapeInMSL(mainshapelist[list_pos],rif_name,list_pos));
	}
	else
	{
		List<Object_Chunk*> const & oblist=tmpshp->list_assoc_objs();
		if(oblist.size())
		{
			Object_Chunk* oc=oblist.first_entry();
			if(oc->get_header()->flags & OBJECT_FLAG_PLACED_OBJECT)
			{
				msl_shapes.add_entry(new ShapeInMSL(mainshapelist[list_pos],oc->object_data.o_name,list_pos));
				
			}
		}	
	}	
	
	post_process_shape(mainshapelist[list_pos]);

	h->shape_nums.add_entry(list_pos);

	if (tmpshp->count_children("ANIMSEQU"))
	{
		//look for alternate texture mappings
		pChunk=tmpshp->lookup_single_child("ASALTTEX");
		if(pChunk)
		{
			List<Chunk *> chlst;
			((Chunk_With_Children*)pChunk)->lookup_child("SUBSHAPE",chlst);
			for(LIF<Chunk*> chlif(&chlst);!chlif.done();chlif.next())
			{
				Shape_Sub_Shape_Chunk* sssc=(Shape_Sub_Shape_Chunk*)chlif();

				list_pos=GetMSLPos();
				copy_to_shapeheader (
					h, 
					sssc->shape_data, 
					mainshapelist[list_pos], 
					sssc, 
					flags, 
					local_max_index,
					local_tex_index_nos,
					list_pos,
					object
					);
				CopyShapeAnimationHeader(mainshapelist[start_shape_no],mainshapelist[list_pos]);
				
				const char* shpname=sssc->get_shape_name();
				GLOBALASSERT(shpname);
				msl_shapes.add_entry(new ShapeInMSL(mainshapelist[list_pos],shpname,list_pos));
				h->shape_nums.add_entry(list_pos);
			   								
				post_process_shape(mainshapelist[list_pos]);
							
			}
		}
	}

	Shape_Fragments_Chunk * sfc = 0;

	pChunk = tmpshp->lookup_single_child ("SHPFRAGS");

	if (pChunk)
	{
		sfc = (Shape_Fragments_Chunk *)pChunk;
		
		pChunk=sfc->lookup_single_child("SHPFRGTP");
		if(pChunk)
		{
			//the shape is using a fragment type
			Shape_Fragment_Type_Chunk* sftc=(Shape_Fragment_Type_Chunk*)pChunk;
			ApplyFragTypeToShape(mainshapelist[main_shape_num],sftc->frag_type_name);
		}
		else
		{
			List<Chunk *> cl;
			sfc->lookup_child ("SUBSHAPE", cl);
			if (cl.size())
			{
				mainshapelist[main_shape_num]->sh_fragdesc = (SHAPEFRAGMENTDESC *)PoolAllocateMem(sizeof(SHAPEFRAGMENTDESC));
				
				mainshapelist[main_shape_num]->sh_fragdesc->sh_frags = (SHAPEFRAGMENT *)PoolAllocateMem((cl.size()+1) * sizeof(SHAPEFRAGMENT));
				mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound = 0;
				
				int fragpos = 0;
				
				for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next(), fragpos++)
				{
					Shape_Sub_Shape_Chunk * sssc = ((Shape_Sub_Shape_Chunk *)cli());
				
					list_pos = GetMSLPos();


					copy_to_shapeheader (
						h, 
						sssc->shape_data,
						mainshapelist[list_pos], 
						sssc, 
						flags, 
						local_max_index,
						local_tex_index_nos,
						list_pos,
						object
						);
					post_process_shape(mainshapelist[list_pos]);
					h->shape_nums.add_entry(list_pos);

					int num_frags = 1;
					
					pChunk = sssc->lookup_single_child("FRAGDATA");
					if (pChunk)
					{
						num_frags = ((Shape_Fragments_Data_Chunk *)pChunk)->num_fragments;
					}
					
					Shape_Fragment_Location_Chunk * sflc = 0;
					
					pChunk = sssc->lookup_single_child("FRAGLOCN");
					if (pChunk)
					{
						sflc = (Shape_Fragment_Location_Chunk *)pChunk;
					}
					
					
					mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].ShapeIndex = list_pos;
					mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].NumFrags = num_frags;
					
					if (sflc)
					{
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].x_offset = (int)(sflc->frag_loc.x * local_scale);
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].y_offset = (int)(sflc->frag_loc.y * local_scale);
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].z_offset = (int)(sflc->frag_loc.z * local_scale);
						
					}
					else
					{
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].x_offset = 0;
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].y_offset = 0;
						mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].z_offset = 0;
					}
					
				}

				mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].ShapeIndex = -1;
				mainshapelist[main_shape_num]->sh_fragdesc->sh_frags[fragpos].NumFrags = -1;

				//see if fragment has a sound to go with it
				Fragment_Type_Sound_Chunk* ftsoc=(Fragment_Type_Sound_Chunk*) sfc->lookup_single_child("FRGSOUND");
				if(ftsoc)
				{
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound=(SHAPEFRAGMENTSOUND*)PoolAllocateMem(sizeof(SHAPEFRAGMENTSOUND));
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound->sound_loaded=GetSoundForMainRif (ftsoc->wav_name);
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound->inner_range=(unsigned long)(ftsoc->inner_range*local_scale);
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound->outer_range=(unsigned long)(ftsoc->outer_range*local_scale);
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound->pitch=ftsoc->pitch;
					mainshapelist[main_shape_num]->sh_fragdesc->sh_fragsound->max_volume=ftsoc->max_volume;
				}


			}
		}
	}

	#if SupportMorphing && LOAD_MORPH_SHAPES
	
	/*-------------------**
	** Morphing stuff    **
	**-------------------*/
	MORPHCTRL * mc = 0;

	if (!(flags & CCF_NOMORPH))
	{
		pChunk = tmpshp->lookup_single_child ("SHPMORPH");
		if (pChunk)
		{
			// this shape has some morphing data
		
			// (store the list no. of the shape)
			Shape_Morphing_Data_Chunk * smdc = (Shape_Morphing_Data_Chunk *)pChunk;
			
			// set all the subshape list_pos numbers to -1
			// so later we can check to see if it has already been loaded
			List<Chunk *> chlst;
			smdc->lookup_child("SUBSHAPE",chlst);
			for (LIF<Chunk *> ssi(&chlst); !ssi.done(); ssi.next())
			{
				((Shape_Sub_Shape_Chunk *)ssi())->list_pos_number = -1;
			}
			
			pChunk = smdc->lookup_single_child("FRMMORPH");
			if (pChunk)
			{
				Shape_Morphing_Frame_Data_Chunk * smfdc = (Shape_Morphing_Frame_Data_Chunk *)pChunk;
				// Check there are some frames!!
				if (smfdc->anim_frames.size())
				{
					mc = (MORPHCTRL *)AllocateMem(sizeof(MORPHCTRL));
					mc->ObMorphFlags = smfdc->a_flags;
					mc->ObMorphSpeed = smfdc->a_speed;
					MORPHHEADER * mh = (MORPHHEADER *)AllocateMem(sizeof(MORPHHEADER));
					mc->ObMorphHeader = mh;
					mh->mph_numframes = 0;
					mh->mph_frames = (MORPHFRAME *)AllocateMem(sizeof(MORPHFRAME) * (smfdc->anim_frames.size()) );
					
					int frame_no = 0;
					
					for (LIF<a_frame *> afi(&smfdc->anim_frames); !afi.done(); afi.next())
					{
						if (afi()->shape1a)
						{
							if (afi()->shape1a->list_pos_number == -1)
							{
								list_pos = GetMSLPos();

								Shape_Preprocessed_Data_Chunk* spdc=(Shape_Preprocessed_Data_Chunk*)afi()->shape1a->lookup_single_child("SHPPRPRO");
								if(spdc)
								{
									copy_preprocessed_to_shapeheader (
										h, 
										spdc, 
										mainshapelist[list_pos], 
										afi()->shape1a, 
										flags, 
										local_max_index,
										local_tex_index_nos,
										list_pos,
										object
										);
								}
								else
								{

									copy_to_shapeheader (
										h, 
										afi()->shape1a->shape_data, 
										mainshapelist[list_pos], 
										afi()->shape1a, 
										flags, 
										local_max_index,
										local_tex_index_nos,
										list_pos,
										object
										);
								}
								
								post_process_shape(mainshapelist[list_pos]);
								afi()->shape1a->list_pos_number = list_pos;
								h->shape_nums.add_entry(list_pos);
								/*
								Copy the item data for this door shape from the main shape. This is largely done to cope
								with the problem of the polygons being merged differently in different morph shapes.
								*/
								SHAPEHEADER* main_shape=mainshapelist[main_shape_num];
								SHAPEHEADER* this_shape=mainshapelist[list_pos];

								this_shape->numitems=main_shape->numitems;
								this_shape->items=main_shape->items;
								this_shape->sh_textures=main_shape->sh_textures;
								this_shape->sh_normals=main_shape->sh_normals;

								//update shape instructions (probably not uses anyway)
								this_shape->sh_instruction[1].sh_numitems=main_shape->numitems;
								this_shape->sh_instruction[1].sh_instr_data=main_shape->sh_normals;

								this_shape->sh_instruction[4].sh_numitems=main_shape->numitems;
								this_shape->sh_instruction[4].sh_instr_data=main_shape->items;
															
							}	
							mh->mph_frames[frame_no].mf_shape1 = afi()->shape1a->list_pos_number;
						}
						else
						{
							mh->mph_frames[frame_no].mf_shape1 = main_shape_num;
						}
						
						if (afi()->shape2a)
						{
							if (afi()->shape2a->list_pos_number == -1)
							{
								list_pos = GetMSLPos();

								Shape_Preprocessed_Data_Chunk* spdc=(Shape_Preprocessed_Data_Chunk*)afi()->shape2a->lookup_single_child("SHPPRPRO");
								if(spdc)
								{
									copy_preprocessed_to_shapeheader (
										h, 
										spdc, 
										mainshapelist[list_pos], 
										afi()->shape1a, 
										flags, 
										local_max_index,
										local_tex_index_nos,
										list_pos,
										object
										);
								}
								else
								{
							
									copy_to_shapeheader (
										h, 
										afi()->shape2a->shape_data, 
										mainshapelist[list_pos], 
										afi()->shape2a, 
										0, 
										local_max_index,
										local_tex_index_nos,
										list_pos,
										object
										);
								}
								post_process_shape(mainshapelist[list_pos]);
								afi()->shape2a->list_pos_number = list_pos;
								h->shape_nums.add_entry(list_pos);
								
								/*
								Copy the item data for this door shape from the main shape. This is largely done to cope
								with the problem of the polygons being merged differently in different morph shapes.
								*/
								SHAPEHEADER* main_shape=mainshapelist[main_shape_num];
								SHAPEHEADER* this_shape=mainshapelist[list_pos];

								this_shape->numitems=main_shape->numitems;
								this_shape->items=main_shape->items;
								this_shape->sh_textures=main_shape->sh_textures;
								this_shape->sh_normals=main_shape->sh_normals;

								//update shape instructions (probably not uses anyway)
								this_shape->sh_instruction[1].sh_numitems=main_shape->numitems;
								this_shape->sh_instruction[1].sh_instr_data=main_shape->sh_normals;

								this_shape->sh_instruction[4].sh_numitems=main_shape->numitems;
								this_shape->sh_instruction[4].sh_instr_data=main_shape->items;
							}	
							mh->mph_frames[frame_no].mf_shape2 = afi()->shape2a->list_pos_number;
						}
						else
						{
							mh->mph_frames[frame_no].mf_shape2 = main_shape_num;
						}
						if (frame_no == 0)
						{
							start_shape_no = mh->mph_frames[frame_no].mf_shape1;
						}
						frame_no ++;
					}
					mh->mph_numframes = frame_no;
					mh->mph_maxframes = frame_no << 16;
				}
			}
		}
	}

	CTM_ReturnType retval = { start_shape_no, main_shape_num, mc };
	if(local_tex_index_nos!=h->tex_index_nos) delete [] local_tex_index_nos;
	return retval;

	#else

	if(local_tex_index_nos!=h->tex_index_nos) delete [] local_tex_index_nos;
	return list_pos;

	#endif
}

// load textures for environment
BOOL load_rif_bitmaps (RIFFHANDLE h, int/* flags*/)
{
	Global_BMP_Name_Chunk * gbnc = 0;

	if (h->envd)
	{
		Chunk * pChunk = h->envd->lookup_single_child ("BMPNAMES");
		if (pChunk) gbnc = (Global_BMP_Name_Chunk *) pChunk;
	}

	h->max_index = 0;

	if (gbnc)
	{
		LIF<BMP_Name> bns (&gbnc->bmps);
		
		for (; !bns.done(); bns.next())
		{
			h->max_index = max(bns().index,h->max_index);
		}

		if (h->tex_index_nos) delete h->tex_index_nos;
		h->tex_index_nos = new int [h->max_index+1];
		for (int i=0; i<=h->max_index; i++)
		{
			h->tex_index_nos[i] = -1;
		}
		
		if (Env_Chunk == 0)
			Env_Chunk = h->fc;
		for (bns.restart() ; !bns.done(); bns.next())
		{
			if (!(bns().flags & ChunkBMPFlag_NotInPC))
			{
				// JH 17-2-97 -- image loaders have changed to avoid loading the same image twice
				int imgnum = load_rif_bitmap(bns().filename,bns().flags);
				if (GEI_NOTLOADED != imgnum)
					h->tex_index_nos[bns().index] = imgnum;
			}
		}

		return TRUE;
	}
	else return FALSE;
}

// set the quantization event depending on cl_pszGameMode
BOOL set_quantization_event(RIFFHANDLE h, int /*flags*/)
{
	if (h->envd)
	{
		h->palparent = h->envd;

		if (cl_pszGameMode)
		{
			List<Chunk *> egmcs;
			h->envd->lookup_child("GAMEMODE",egmcs);
			
			for (LIF<Chunk *> egmcLIF(&egmcs); !egmcLIF.done(); egmcLIF.next())
			{
				Environment_Game_Mode_Chunk * egmcm = (Environment_Game_Mode_Chunk *) egmcLIF();
				if (egmcm->id_equals(cl_pszGameMode))
				{
					h->palparent = egmcm;
					break;
				}
			}

		}

		return TRUE;
	}
	else
	{
		h->palparent = 0;
		return FALSE;
	}
}

// copy palette
BOOL copy_rif_palette (RIFFHANDLE h, int /*flags*/)
{
	if (h->palparent)
	{
		List<Chunk *> chlst;
		h->palparent->lookup_child("ENVPALET",chlst);
		for (LIF<Chunk *> i_ch(&chlst); !i_ch.done(); i_ch.next())
		{
			Environment_Palette_Chunk * palch = (Environment_Palette_Chunk *)i_ch();
			if (!(palch->flags & EnvPalFlag_V2) && palch->width*palch->height <= 256 )
			{
				for (int i=0; i<palch->width*palch->height*3; i++)
				{
					TestPalette[i] = (unsigned char)(palch->pixel_data[i] >> 2);
				}
				return TRUE;
			}
		}
	}

	return FALSE;
}

// copy texture lighting table
BOOL copy_rif_tlt (RIFFHANDLE h, int /*flags*/)
{
	if (h->palparent)
	{
		List<Chunk *> chlst;
		h->palparent->lookup_child("ENVTXLIT",chlst);
		if(TextureLightingTable)
		{
			DeallocateMem(TextureLightingTable);
			TextureLightingTable = 0;
		}
		for (LIF<Chunk *> i_ch(&chlst); !i_ch.done(); i_ch.next())
		{
			Environment_TLT_Chunk * tltch = (Environment_TLT_Chunk *)i_ch();

			if ((tltch->flags & ChunkTLTFlag_V2 &&
			     ScreenDescriptorBlock.SDB_Flags & SDB_Flag_TLTPalette ||
			     !(tltch->flags & ChunkTLTFlag_V2) &&
			     !(ScreenDescriptorBlock.SDB_Flags & SDB_Flag_TLTPalette)) &&
				tltch->table
			){
				TextureLightingTable = (unsigned char*)AllocateMem(tltch->width * tltch->num_levels);
				memcpy(TextureLightingTable,tltch->table,tltch->width*tltch->num_levels);
				if (ScreenDescriptorBlock.SDB_Flags & SDB_Flag_TLTPalette)
				{
					ScreenDescriptorBlock.SDB_Flags &= ~(SDB_Flag_TLTSize|SDB_Flag_TLTShift);
					if (tltch->width != 256)
					{
						int shft;
						
						ScreenDescriptorBlock.SDB_Flags |= SDB_Flag_TLTSize;
						ScreenDescriptorBlock.TLTSize = tltch->width;
						
						for (shft = 0; 1<<shft < tltch->width; ++shft)
							;
						if (1<<shft==tltch->width)
						{
							ScreenDescriptorBlock.SDB_Flags |= SDB_Flag_TLTShift;
							ScreenDescriptorBlock.TLTShift = shft;
						}
					}
				}
				return TRUE;
			}
			
		}
	}

	return FALSE;
}

// copy palette remap table (15-bit) - post_process_shape may use it
BOOL get_rif_palette_remap_table (RIFFHANDLE h, int /*flags*/)
{
	PaletteMapTable = 0;
	if (h->palparent)
	{
		Chunk * pChunk = h->palparent->lookup_single_child("CLRLOOKP");
		if (pChunk)
		{
			Coloured_Polygons_Lookup_Chunk * cplook = (Coloured_Polygons_Lookup_Chunk *)pChunk;
			
			if (cplook->table)
			{
				PaletteMapTable = cplook->table;
				return TRUE;
			}
			
		}
	}

	return FALSE;
}

// copy one named shape or sprite; intended to go in position listpos
static SHAPEHEADER * CreateShapeFromRif (RIFFHANDLE h, char const * shapename, int listpos = GLS_NOTINLIST)
{
	if (!h->fc) return 0; // no rif file loaded

	List<Chunk *> shape_chunks;
	h->fc->lookup_child("REBSHAPE",shape_chunks);

	for (LIF<Chunk *> search(&shape_chunks); !search.done(); search.next())
	{
		Shape_Chunk * cur_shape = (Shape_Chunk *) search();

		Chunk * pShpextfile = cur_shape->lookup_single_child("SHPEXTFL");
		
		if (pShpextfile)
		{
			Shape_External_File_Chunk * shexdata = (Shape_External_File_Chunk *) pShpextfile;

			Chunk * pRnc = shexdata->lookup_single_child("RIFFNAME");
			if (pRnc)
			{
				RIF_Name_Chunk * rnc = (RIF_Name_Chunk *) pRnc;
				if (!_stricmp(rnc->rif_name,shapename)) // haha! matching shape found
				{
					SHAPEHEADER * shptr = 0;
					int local_max_index;
					int * local_tex_index_nos;
					setup_tex_conv_array (local_max_index, local_tex_index_nos, h, cur_shape);
					
					copy_to_shapeheader(
						h,
						cur_shape->shape_data, 
						shptr, 
						cur_shape, 
						0, 
						local_max_index,
						local_tex_index_nos,
						listpos
						);
					if(local_tex_index_nos!=h->tex_index_nos) delete [] local_tex_index_nos;

					return shptr;
				}
			}
		}
	}
	//look to see if is a sprite
	Chunk * pSprite_chunks = h->fc->lookup_single_child("RSPRITES");
	if(pSprite_chunks)
	{
		AllSprites_Chunk* asc=(AllSprites_Chunk*) pSprite_chunks;
		List<Chunk *> sprite_chunks;
		asc->lookup_child("SPRIHEAD",sprite_chunks);
		for(LIF<Chunk*> slif(&sprite_chunks);!slif.done();slif.next())
		{
			Sprite_Header_Chunk* shc=(Sprite_Header_Chunk*)slif();
			Chunk * pRn=shc->lookup_single_child("RIFFNAME");
			if(pRn)
			{
				RIF_Name_Chunk* rnc=(RIF_Name_Chunk*)pRn;
				if (!_stricmp(rnc->rif_name,shapename)) // haha! matching sprite found
				{
					SHAPEHEADER * shptr = 0;
					copy_sprite_to_shapeheader(h,shptr, shc, listpos);
					return shptr;
				}
			}

		}
	}
	return 0; // could not match shape
}

// copy one named shape or sprite; does not put in main shape list
SHAPEHEADER * CopyNamedShapePtr (RIFFHANDLE h, char const * shapename)
{
	return CreateShapeFromRif(h,shapename);
}

// copy one named shape or sprite; put it in the main shape list
int CopyNamedShapeMSL (RIFFHANDLE h, char const * shapename)
{
	int listpos = GetMSLPos();
	SHAPEHEADER * shp = CreateShapeFromRif(h,shapename,listpos);
	if (shp)
	{
		h->shape_nums.add_entry(listpos);
		mainshapelist[listpos] = shp;
		return listpos;
	}
	else
	{
		FreeMSLPos(listpos);
		return GLS_NOTINLIST;
	}
}

////////////////////////////////////////////////////////////////////////
// Functions which do not operate on RIFFHANDLEs and may become obsolete
////////////////////////////////////////////////////////////////////////

SHAPEHEADER * CopyNamedShape (char const * shapename)
{
	return CopyNamedShapePtr (current_rif_handle,shapename);
}

/////////////////////////////////////////////
// Functions for handling the main shape list
/////////////////////////////////////////////

////////////////////////////////////////////////
// Functions retrieving data about loaded shapes
////////////////////////////////////////////////

// gets the main shape list position of a shape loaded into the msl
int GetLoadedShapeMSL(char const * shapename)
{
	ShapeInMSL const * sim = ShapeInMSL::GetByName(shapename);
	
	if (sim)
		return sim->Listpos();
	else
		return GLS_NOTINLIST;
}

// ditto, but returns a pointer; the shape need not be in the msl
SHAPEHEADER * GetLoadedShapePtr(char const * shapename)
{
	ShapeInMSL const * sim = ShapeInMSL::GetByName(shapename);
	
	if (sim)
		return sim->Shptr();
	else
		return 0;
}

// gets name of shape from msl pos
char const * GetMSLLoadedShapeName(int listpos)
{
	ShapeInMSL const * sim = ShapeInMSL::GetByMSL(listpos);

	if (sim)
		return sim->Name();
	else
		return 0;
}

// gets name of shape from pointer; the shape need not be in msl
char const * GetPtrLoadedShapeName(SHAPEHEADER * shptr)
{
	ShapeInMSL const * sim = ShapeInMSL::GetByPtr(shptr);

	if (sim)
		return sim->Name();
	else
		return 0;
}

// free a reference to a named shape if it exists - not necessary since these are all tidied up
void FreeShapeNameReference(SHAPEHEADER * shptr)
{
	for (LIF<ShapeInMSL*> search(&msl_shapes); !search.done(); search.next())
	{
		if (search()->Shptr() == shptr)
		{
			delete search();
			search.delete_current();
			break;
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////
// Initializing, deallocating of shapes, mainly hooks for project specific fns
//////////////////////////////////////////////////////////////////////////////

// delete a shape by the shapeheader
void DeallocateLoadedShapePtr(SHAPEHEADER * shp)
{
	DeallocateLoadedShapeheader(shp);

	FreeShapeNameReference(shp);
}

// delete a shape by the shape list number
void DeallocateLoadedShapeMSL(RIFFHANDLE h, int list_pos)
{
	h->shape_nums.delete_entry(list_pos);
	
	DeallocateLoadedShapeheader(mainshapelist[list_pos]);

	FreeMSLPos(list_pos);

	for(LIF<ShapeInMSL*> msl_shape_lif(&msl_shapes);!msl_shape_lif.done();msl_shape_lif.next())
	{
		if(list_pos==msl_shape_lif()->Listpos())
		{
			delete msl_shape_lif();
			msl_shape_lif.delete_current();
			break;
		}
	}
}

void DeallocateRifLoadedShapeheader(SHAPEHEADER * shp)
{
	// because the SHAPEHEADER is calloced, we can
	// just delete the arrays we want
	
	#if !StandardShapeLanguage
	#error Must have standard shape language
	#endif
	
	if(shp->animation_header)
	{
		// so it gets deallocated properly
		shp->points[0] = 0;
		shp->sh_normals[0] = 0;
		shp->sh_vnormals[0] = 0;
	}
	if (shp->sh_fragdesc)
	{
		DeallocateFragments(shp,shp->sh_fragdesc);
	}
	
	#if !USE_LEVEL_MEMORY_POOL
	int max_num_texs = 0;
	int i;
	
	if (shp->points)
	{
		if (*shp->points) DeallocateMem(*shp->points);
		DeallocateMem(shp->points);
	}
	if (shp->sh_normals)
	{
		if (*shp->sh_normals) DeallocateMem(*shp->sh_normals);
		DeallocateMem(shp->sh_normals);
	}
	if (shp->sh_vnormals)
	{
		if (*shp->sh_vnormals) DeallocateMem(*shp->sh_vnormals);
		DeallocateMem(shp->sh_vnormals);
	}
	if (shp->sh_extraitemdata)
		DeallocateMem(shp->sh_extraitemdata);
	/* the items are allocated in one big bunch
	// 9 int's per item (to make bsp simple)
	// this should be changed if it is to be done 
	// a different way
	*/
	if (shp->items)
	{
		if(shp->shapeflags & ShapeFlag_MultiViewSprite)
		{
			TXANIMHEADER** thlist=(TXANIMHEADER**)shp->sh_textures[0];
			for(int j=1;thlist[j]!=0;j++)
			{
				int k;
				TXANIMHEADER* th=thlist[j];
				for(k=0;k<th->txa_numframes;k++)
				{
					txanimframe_mvs* tf=(txanimframe_mvs*)&th->txa_framedata[k];
					if(tf->txf_uvdata[0])DeallocateMem(tf->txf_uvdata[0]);
					if(tf->txf_uvdata)DeallocateMem(tf->txf_uvdata);
					if(tf->txf_images)DeallocateMem(tf->txf_images);
				}
				if(th->txa_framedata)DeallocateMem (th->txa_framedata);
				DeallocateMem (th);
			}
			DeallocateMem (thlist);
			shp->sh_textures[0]=0;
		}
		else
		{
			for (i=0; i<shp->numitems; i++)
			{
				if (is_textured(shp->items[i][0]))
				{
					int UVIndex =  (shp->items[i][3] &0xffff0000) >> 16;
					max_num_texs = max (max_num_texs, shp->items[i][3] &0x7fff);
					if(shp->items[i][2]& iflag_txanim)
					{
						int j;
						TXANIMHEADER** thlist=(TXANIMHEADER**)shp->sh_textures[UVIndex];
						for(j=1;thlist[j]!=0;j++)
						{
							int k;
							TXANIMHEADER* th=thlist[j];
							for(k=0;k<th->txa_numframes;k++)
							{
								if(th->txa_framedata[k].txf_uvdata)DeallocateMem(th->txa_framedata[k].txf_uvdata);
							}
							if(th->txa_framedata)DeallocateMem (th->txa_framedata);
							DeallocateMem (th);
						}
						DeallocateMem (thlist);
						shp->sh_textures[UVIndex]=0;
					}
					else
					{
						if(shp->sh_textures[UVIndex])DeallocateMem(shp->sh_textures[UVIndex]);
					}
				}
			}
		}
		DeallocateMem (*shp->items);
		DeallocateMem (shp->items);
	}

	if (shp->sh_textures)
	{
		DeallocateMem (shp->sh_textures);
	}

	if (shp->sh_localtextures)
	{
		for (i=0; i<(max_num_texs+1); i++)
		{
			DeallocateMem (shp->sh_localtextures[i]);
		}
		DeallocateMem (shp->sh_localtextures);
	}

	
	
	#if SupportTrackOptimisation
	if (shp->sh_track_data)
		DeallocateMem(shp->sh_track_data);
	#endif
	if (shp->sh_instruction)
		DeallocateMem(shp->sh_instruction);
	#if SupportBSP
	if (shp->sh_bsp_blocks)
		DeallocateMem(shp->sh_bsp_blocks);
	#endif

	if(shp->animation_header)
	{
		shp->animation_header->num_shapes_using_this--;
		if(shp->animation_header->num_shapes_using_this==0)
		{
			shapeanimationheader* sah=shp->animation_header;
			for(int i=0;i<sah->num_sequences;i++)
			{
				shapeanimationsequence* sas=&sah->anim_sequences[i];
				for(int j=0;j<sas->num_frames;j++)
				{
					shapeanimationframe*saf=&sas->anim_frames[j];
					DeallocateMem(saf->vertices);
					DeallocateMem(saf->item_normals);
				}
				if(sas->vertex_normals)DeallocateMem(sas->vertex_normals);
				if(sas->anim_frames)DeallocateMem(sas->anim_frames);
			}
			DeallocateMem(sah->anim_sequences);	
			DeallocateMem(sah);
		}
	}

	if(shp->shape_degradation_array)
	{
		DeallocateMem(shp->shape_degradation_array);
	}
	
	DeallocateMem(shp);
	#endif	//!USE_LEVEL_MEMORY_POOL
}

///////
// Misc
///////

// return TRUE if the poly item type corresponds to a textured polygon
BOOL is_textured (int type)
{
	if (
		type == I_2dTexturedPolygon
		|| type == I_Gouraud2dTexturedPolygon 
		|| type == I_3dTexturedPolygon
		|| type == I_Gouraud3dTexturedPolygon
		|| type == I_ZB_2dTexturedPolygon
		|| type == I_ZB_Gouraud2dTexturedPolygon
		|| type == I_ZB_3dTexturedPolygon
		|| type == I_ZB_Gouraud3dTexturedPolygon
		)
		{
			return(TRUE);
		}
	return(FALSE);
}


#if SupportModules

// static Object_Chunk ** o_chunk_array;

void copy_to_module (Object_Chunk * ob, int mod_pos, int shplst_pos)
{
	Object_Project_Data_Chunk * opdc = 0;
	Map_Block_Chunk * mapblok = 0;
	Strategy_Chunk * strat = 0;

	MODULEMAPBLOCK * Map = (MODULEMAPBLOCK *) PoolAllocateMem (sizeof(MODULEMAPBLOCK));

	*Map = Empty_Module_Map;

	MainScene.sm_module[mod_pos].m_mapptr = Map;
	MainScene.sm_module[mod_pos].name = (char *) PoolAllocateMem (strlen (ob->object_data.o_name)+1);
	strcpy (MainScene.sm_module[mod_pos].name, ob->object_data.o_name);

	*((int *)MainScene.sm_module[mod_pos].m_name) = mod_pos + ONE_FIXED;
	// add 65536 to this value to this value to preserve 0

	Chunk * pChunk = ob->lookup_single_child("OBJPRJDT");
	if (pChunk) opdc = (Object_Project_Data_Chunk *)pChunk;
	if (opdc)
	{
		pChunk = opdc->lookup_single_child("MAPBLOCK");
		if (pChunk) mapblok = (Map_Block_Chunk *)pChunk;
		pChunk = opdc->lookup_single_child("STRATEGY");
		if (pChunk) strat = (Strategy_Chunk *)pChunk;
	}

	if (mapblok)
	{
		Map->MapType = mapblok->map_data.MapType;
		Map->MapFlags= mapblok->map_data.MapFlags;
		#if (StandardStrategyAndCollisions || IntermediateSSACM)
		Map->MapCType = mapblok->map_data.MapCType;
		Map->MapCGameType = mapblok->map_data.MapCGameType;
		Map->MapCStrategyS = mapblok->map_data.MapCStrategyS;
		Map->MapCStrategyL = mapblok->map_data.MapCStrategyL;
		#endif
		Map->MapInteriorType = mapblok->map_data.MapInteriorType;
//		Map->MapLightType = mapblok->map_data.MapLightType;
//		Map->MapMass = mapblok->map_data.MapMass;
//		Map->MapNewtonV.vx = mapblok->map_data.MapNewtonV.vx;
//		Map->MapNewtonV.vy = mapblok->map_data.MapNewtonV.vy;
//		Map->MapNewtonV.vz = mapblok->map_data.MapNewtonV.vz;
//		Map->MapOrigin.vx = mapblok->map_data.MapOrigin.vx;
//		Map->MapOrigin.vy = mapblok->map_data.MapOrigin.vy;
//		Map->MapOrigin.vz = mapblok->map_data.MapOrigin.vz;
//		Map->MapViewType = mapblok->map_data.MapViewType;
	}

	#if (StandardStrategyAndCollisions || IntermediateSSACM)
	if (strat)
	{
		Map->MapStrategy = strat->strategy_data.Strategy;
	}
	#endif

	Map->MapShape = shplst_pos;

	Map->MapWorld.vx = (int) (ob->object_data.location.x*local_scale);
	Map->MapWorld.vy = (int) (ob->object_data.location.y*local_scale);
	Map->MapWorld.vz = (int) (ob->object_data.location.z*local_scale);

#if 0
	QUAT q;

	q.quatx = (int) (ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int) (ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int) (ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int) (ob->object_data.orientation.w*ONE_FIXED);


	MATRIXCH m;

	QuatToMat (&q, &m);

	EULER e;

	MatrixToEuler(&m, &e);

	Map->MapEuler.EulerX = -e.EulerX;
	Map->MapEuler.EulerY = -e.EulerY;
	Map->MapEuler.EulerZ = -e.EulerZ;

#endif


}

#endif

void SetupAnimOnTriangle(SHAPEHEADER* shp,TEXANIM* ta,int poly, int * local_tex_index_nos)
{
	if(!is_textured(shp->items[poly][0]))return;
	txanimheader** thlist=(txanimheader**)PoolAllocateMem((ta->NumSeq+2)*sizeof(txanimheader*));
	thlist[0]=0;
	thlist[ta->NumSeq+1]=0;
	for(int i=0;i<ta->NumSeq;i++)
	{
		thlist[i+1]=(txanimheader*)PoolAllocateMem(sizeof(txanimheader));
		txanimheader* th=thlist[i+1];
		
		FrameList* fl=ta->Seq[i];
		th->txa_flags=fl->Flags;
		if(!(ta->AnimFlags & AnimFlag_NotPlaying))th->txa_flags|=txa_flag_play;
		th->txa_numframes=fl->NumFrames+1;
		if(fl->Flags & txa_flag_nointerptofirst)
		{
			th->txa_flags&=~txa_flag_nointerptofirst;
			th->txa_numframes--;				
		}
		th->txa_currentframe=0;
		th->txa_state=0;
		th->txa_maxframe=(th->txa_numframes-1)<<16;
		th->txa_speed=fl->Speed;
		th->txa_framedata=(txanimframe*)PoolAllocateMem(th->txa_numframes*sizeof(txanimframe));
		th->txa_anim_id=ta->Identifier;

		txanimframe* tf;
		for(int j=0;j<th->txa_numframes;j++)
		{
			tf=&th->txa_framedata[j];
			tf->txf_flags=0;
			tf->txf_scale=ONE_FIXED;
			tf->txf_scalex=0;
			tf->txf_scaley=0;
			tf->txf_orient=0;
			tf->txf_orientx=0;
			tf->txf_orienty=0;
			tf->txf_numuvs=3;
			tf->txf_uvdata=(int*)PoolAllocateMem(6*sizeof(int));
			if(j==fl->NumFrames)
			{
				tf->txf_image=local_tex_index_nos[fl->Textures[0]];
				for(int k=0;k<6;k++)
				{
					tf->txf_uvdata[k]=fl->UVCoords[k]<<16;
				}	
			}
			else
			{
				tf->txf_image=local_tex_index_nos[fl->Textures[j]];
				for(int k=0;k<6;k++)
				{
					tf->txf_uvdata[k]=fl->UVCoords[j*6+k]<<16;
				}	
			}
		}
	}
	int UVIndex=shp->items[poly][3]>>16;
	#if !USE_LEVEL_MEMORY_POOL
	if(shp->sh_textures[UVIndex])DeallocateMem(shp->sh_textures[UVIndex]);
	#endif
	shp->sh_textures[UVIndex]=(int*)thlist;
	shp->items[poly][2]|=iflag_txanim;
	shp->items[poly][3]=UVIndex<<16;

}
void SetupAnimOnQuad(Shape_Chunk* sc,SHAPEHEADER* shp,TEXANIM* ta1,TEXANIM* ta2,int poly, int * local_tex_index_nos)
{
	if(!is_textured(shp->items[poly][0]))return;
	if(ta1->ID!=ta2->ID)return;
	int VertConv[3];//conversion between vert nos in triangles and vert nos in quad
	int VertFrom,VertTo;//for remaining vert in second poly
	int i;
	
	VertTo=6;
	for(i=0;i<3;i++)
	{
		int j;
		
		for(j=0;j<4;j++)
		{
			if(sc->shape_data.poly_list[ta1->poly].vert_ind[i]==(shp->items[poly][j+4]))break;
		}
		if(j==4)return;
		VertConv[i]=j;
		VertTo-=j;
	}
	for(i=0;i<3;i++)
	{
		if(sc->shape_data.poly_list[ta2->poly].vert_ind[i]==(shp->items[poly][4+VertTo]))break;
	}
	if(i==3)return;
	VertFrom=i;
	
	txanimheader** thlist=(txanimheader**)PoolAllocateMem((ta1->NumSeq+2)*sizeof(txanimheader*));
	thlist[0]=0;
	thlist[ta1->NumSeq+1]=0;
	for(i=0;i<ta1->NumSeq;i++)
	{
		thlist[i+1]=(txanimheader*)PoolAllocateMem(sizeof(txanimheader));
		txanimheader* th=thlist[i+1];
		FrameList* fl1=ta1->Seq[i];
		FrameList* fl2=ta2->Seq[i];
		th->txa_flags=fl1->Flags;
		if(!(ta1->AnimFlags & AnimFlag_NotPlaying))th->txa_flags|=txa_flag_play;
		th->txa_numframes=fl1->NumFrames+1;
		if(fl1->Flags & txa_flag_nointerptofirst)
		{
			th->txa_flags&=~txa_flag_nointerptofirst;
			th->txa_numframes--;				
		}
		th->txa_currentframe=0;
		th->txa_state=0;
		th->txa_maxframe=(th->txa_numframes-1)<<16;
		th->txa_speed=fl1->Speed;
		th->txa_framedata=(txanimframe*)PoolAllocateMem(th->txa_numframes*sizeof(txanimframe));
		th->txa_anim_id=ta1->Identifier;

		txanimframe* tf;
		for(int j=0;j<th->txa_numframes;j++)
		{
			tf=&th->txa_framedata[j];
			tf->txf_flags=0;
			tf->txf_scale=ONE_FIXED;
			tf->txf_scalex=0;
			tf->txf_scaley=0;
			tf->txf_orient=0;
			tf->txf_orientx=0;
			tf->txf_orienty=0;
			tf->txf_numuvs=4;
			tf->txf_uvdata=(int*)PoolAllocateMem(8*sizeof(int));
			if(j==fl1->NumFrames)
			{
				tf->txf_image=local_tex_index_nos[fl1->Textures[0]];
				for(int k=0;k<3;k++)
				{
					tf->txf_uvdata[VertConv[k]*2]=fl1->UVCoords[k*2]<<16;
					tf->txf_uvdata[VertConv[k]*2+1]=fl1->UVCoords[k*2+1]<<16;
				}
				tf->txf_uvdata[VertTo*2]=fl2->UVCoords[VertFrom*2]<<16;
				tf->txf_uvdata[VertTo*2+1]=fl2->UVCoords[VertFrom*2+1]<<16;
					
			}
			else
			{
				tf->txf_image=local_tex_index_nos[fl1->Textures[j]];
				for(int k=0;k<3;k++)
				{
					tf->txf_uvdata[VertConv[k]*2]=fl1->UVCoords[j*6+k*2]<<16;
					tf->txf_uvdata[VertConv[k]*2+1]=fl1->UVCoords[j*6+k*2+1]<<16;
				}	
				tf->txf_uvdata[VertTo*2]=fl2->UVCoords[j*6+VertFrom*2]<<16;
				tf->txf_uvdata[VertTo*2+1]=fl2->UVCoords[j*6+VertFrom*2+1]<<16;
			}
		}
	}
	int UVIndex=shp->items[poly][3]>>16;
	#if !USE_LEVEL_MEMORY_POOL
	if(shp->sh_textures[UVIndex])DeallocateMem(shp->sh_textures[UVIndex]);
	#endif
	shp->sh_textures[UVIndex]=(int*)thlist;
	shp->items[poly][2]|=iflag_txanim;
	shp->items[poly][3]=UVIndex<<16;


}
void SetupAnimatedTextures(Shape_Chunk* sc,SHAPEHEADER* shp,Animation_Chunk* ac,Shape_Merge_Data_Chunk* smdc, int * local_tex_index_nos)
{
	//create conversion between unmerged poly nos and merged poly nos
	int* PolyConv=0;
	int* mgd=0;
	
	if(smdc) 
	{
		mgd=smdc->merge_data;
		PolyConv=new int[smdc->num_polys];
		for(int i=0, j=0;i<smdc->num_polys;i++)
		{
			if(mgd[i]==-1)
			{
				PolyConv[i]=j;
				j++;
			}
			else if(mgd[i]>i)
			{
				if(shp->items[j][7]==-1)
				{
					//quad in merge data,but not actually merged;
					PolyConv[i]=j;
					j++;
					PolyConv[mgd[i]]=j;
					j++;
				}
				else
				{
					PolyConv[i]=j;
					PolyConv[mgd[i]]=j;
					j++;
				}
			}
			
		}

		for(int i=0;i<ac->NumPolys;i++)
		{
			TEXANIM* ta1,*ta2;
			ta1=ac->AnimList[i];
			if(mgd[ta1->poly]==-1)
			{
				SetupAnimOnTriangle(shp,ta1,PolyConv[ta1->poly], local_tex_index_nos);
			}
			else if(mgd[ta1->poly]>ta1->poly)
			{
				int j;
				
				for(j=0;j<ac->NumPolys;j++)
				{
					if(ac->AnimList[j]->poly==mgd[ta1->poly])break;
				}
				if(j<ac->NumPolys)
				{
					ta2=ac->AnimList[j];
					if(PolyConv[ta1->poly]==PolyConv[ta2->poly])
					{
						SetupAnimOnQuad(sc,shp,ta1,ta2,PolyConv[ta1->poly], local_tex_index_nos);
					}
					else
					{
						SetupAnimOnTriangle(shp,ta1,PolyConv[ta1->poly], local_tex_index_nos);
						SetupAnimOnTriangle(shp,ta2,PolyConv[ta2->poly], local_tex_index_nos);
					}
				}
				else if(PolyConv[ta1->poly]!=PolyConv[mgd[ta1->poly]])
				{
					SetupAnimOnTriangle(shp,ta1,PolyConv[ta1->poly], local_tex_index_nos);
				}
			}
		}
		if(PolyConv)delete [] PolyConv;
	}
	else
	{
		for(int i=0;i<ac->NumPolys;i++)
		{
			SetupAnimOnTriangle(shp,ac->AnimList[i],ac->AnimList[i]->poly, local_tex_index_nos);
		}
	}
	shp->shapeflags|=ShapeFlag_HasTextureAnimation;
}									  


void SetupAnimatingShape(Shape_Chunk* sc,SHAPEHEADER* shp, Shape_Merge_Data_Chunk* smdc)
{
	//create conversion between unmerged poly nos and merged poly nos
	int* PolyConv=0;
	int* mgd=0;
	
	PolyConv=new int[smdc->num_polys];
	
	if(smdc) 
	{
		mgd=smdc->merge_data;
		for(int i=0, j=0;i<smdc->num_polys;i++)
		{
			if(mgd[i]==-1)
			{
				PolyConv[i]=j;
				j++;
			}
			else if(mgd[i]>i)
			{
				if(shp->items[j][7]==-1)
				{
					//quad in merge data,but not actually merged;
					PolyConv[i]=j;
					j++;
					PolyConv[mgd[i]]=j;
					j++;
				}
				else
				{
					PolyConv[i]=j;
					PolyConv[mgd[i]]=j;
					j++;
				}
			}
			
		}

	}
	else
	{
		for(int i=0;i<smdc->num_polys;i++)
		{
			PolyConv[i]=i;
		}
	}

	ChunkVectorInt Centre={0,0,0};
	Chunk * pChunk = sc->lookup_single_child("ANSHCEN2");
	if(pChunk)
		Centre=((Anim_Shape_Centre_Chunk*)pChunk)->Centre;
	
	int numseq=0;
	List<Chunk *> chlist;
	sc->lookup_child("ANIMSEQU",chlist);
	
	LIF<Chunk*> chlif(&chlist);
	for(;!chlif.done();chlif.next())
	{
		Anim_Shape_Sequence_Chunk* assc=(Anim_Shape_Sequence_Chunk*)chlif();
		numseq=max(assc->sequence_data.SequenceNum+1,numseq);
	}

	shapeanimationheader* sah=(shapeanimationheader*)PoolAllocateMem(sizeof(shapeanimationheader));
	shp->animation_header=sah;
	sah->num_sequences=numseq;
	sah->anim_sequences=(shapeanimationsequence*)PoolAllocateMem(sizeof(shapeanimationsequence)*numseq);
	sah->num_shapes_using_this=1;
	//sah->vertices_store = shp->points[0];
	//sah->item_normals_store = shp->sh_normals[0];
	//sah->vertex_normals_store = shp->sh_vnormals[0];
	
	for( int i=0;i<numseq;i++)
	{
		sah->anim_sequences[i].num_frames=0;
		sah->anim_sequences[i].anim_frames=0;
	}


	
	for(chlif.restart();!chlif.done();chlif.next())
	{
		Anim_Shape_Sequence_Chunk* assc=(Anim_Shape_Sequence_Chunk*)chlif();
		assc->GenerateInterpolatedFrames();
		const ChunkAnimSequence * cas=& assc->sequence_data;
		if(!cas->NumFrames)continue;
		shapeanimationsequence* sas	=&sah->anim_sequences[cas->SequenceNum];
	
		sas->max_x=(int)((cas->max.x-Centre.x)*local_scale);
		sas->min_x=(int)((cas->min.x-Centre.x)*local_scale);
		sas->max_y=(int)((cas->max.y-Centre.y)*local_scale);
		sas->min_y=(int)((cas->min.y-Centre.y)*local_scale);
		sas->max_z=(int)((cas->max.z-Centre.z)*local_scale);
		sas->min_z=(int)((cas->min.z-Centre.z)*local_scale);
	
		int x=max(-sas->min_x,sas->max_x);
		int y=max(-sas->min_y,sas->max_y);
		int z=max(-sas->min_z,sas->max_z);
		sas->radius=(int)sqrt((double)(x*x+y*y+z*z));
		
		
		sas->vertex_normals=(int*)PoolAllocateMem(sizeof(VECTORCH)*cas->num_verts);
		for(int i=0;i<cas->num_verts;i++)
		{
			sas->vertex_normals[i*3]=(int)(cas->v_normal_list[i].x*ONE_FIXED);
			sas->vertex_normals[i*3+1]=(int)(cas->v_normal_list[i].y*ONE_FIXED);
			sas->vertex_normals[i*3+2]=(int)(cas->v_normal_list[i].z*ONE_FIXED);
		}

		sas->num_frames=cas->NumFrames;
		sas->anim_frames=(shapeanimationframe*)PoolAllocateMem(sizeof(shapeanimationframe)*cas->NumFrames);
		
		for(int i=0;i<cas->NumFrames;i++)
		{
			const ChunkAnimFrame* caf=cas->Frames[i];
			shapeanimationframe* saf=&sas->anim_frames[i];
			
			saf->vertices=(int*)PoolAllocateMem(sizeof(VECTORCH)*caf->num_verts);
			for(int j=0;j<caf->num_verts;j++)
			{
				saf->vertices[j*3]=(int)((caf->v_list[j].x-Centre.x)*local_scale);
				saf->vertices[j*3+1]=(int)((caf->v_list[j].y-Centre.y)*local_scale);
				saf->vertices[j*3+2]=(int)((caf->v_list[j].z-Centre.z)*local_scale);
			}

			saf->item_normals=(int*) PoolAllocateMem(sizeof(VECTORCH)*shp->numitems);
			for(int j=0;j<caf->num_polys;j++)
			{
				saf->item_normals[PolyConv[j]*3]=(int)(caf->p_normal_list[j].x*ONE_FIXED);
				saf->item_normals[PolyConv[j]*3+1]=(int)(caf->p_normal_list[j].y*ONE_FIXED);
				saf->item_normals[PolyConv[j]*3+2]=(int)(caf->p_normal_list[j].z*ONE_FIXED);

			}
		}
	}
	
	delete [] PolyConv;
	

	//find a sequence which has some frames;
	shapeanimationsequence* sas=0;
	int i;
	
	for(i=0;i<shp->animation_header->num_sequences;i++)
	{
		sas=&shp->animation_header->anim_sequences[i];
		if(sas->num_frames)
			break;
		
	}
	GLOBALASSERT(i<shp->animation_header->num_sequences);
	
	//copy the pointers for the first frame of this sequence
	#if !USE_LEVEL_MEMORY_POOL
	DeallocateMem(shp->points[0]);
	DeallocateMem(shp->sh_normals[0]);
	DeallocateMem(shp->sh_vnormals[0]);
	#endif
	
	shp->points[0]=sas->anim_frames[0].vertices;
	shp->sh_normals[0]=sas->anim_frames[0].item_normals;
	shp->sh_vnormals[0]=sas->vertex_normals;
}


BOOL copy_to_shapeheader (
	RIFFHANDLE h, 
	ChunkShape const & cshp, 
	SHAPEHEADER *& shphd, 
	Chunk_With_Children * shape, 
	int flags, 
	int local_max_index,
	int * local_tex_index_nos,
	int /*listpos*/,
	const ChunkObject* object
	)
{
	ChunkShape merged_cshp;
	const ChunkShape* cshp_ptr;

	if(shape->lookup_single_child("SHPMRGDT"))
	{
		merged_cshp=cshp;
		pre_process_shape(h,merged_cshp,shape,flags);
		
		cshp_ptr=&merged_cshp;
	}
	else
	{
		cshp_ptr=&cshp;
	}
	
	shphd = (SHAPEHEADER *) PoolAllocateMem(sizeof(SHAPEHEADER));
	memset(shphd,0,sizeof(SHAPEHEADER));
	
	int i,j;
	int * tptr;

	// header data (note shapeheader is calloced)

	shphd->numpoints = cshp_ptr->num_verts;
	shphd->numitems = cshp_ptr->num_polys;


	shphd->shaperadius = (int) (cshp_ptr->radius*local_scale);

	shphd->shapemaxx = (int) (cshp_ptr->max.x*local_scale);
	shphd->shapeminx = (int) (cshp_ptr->min.x*local_scale);
	shphd->shapemaxy = (int) (cshp_ptr->max.y*local_scale);
	shphd->shapeminy = (int) (cshp_ptr->min.y*local_scale);
	shphd->shapemaxz = (int) (cshp_ptr->max.z*local_scale);
	shphd->shapeminz = (int) (cshp_ptr->min.z*local_scale);

	// AllocateMem arrays

	shphd->points = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->points) = (int *) PoolAllocateMem (sizeof(int) * shphd->numpoints * 3);
	
	shphd->sh_vnormals = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->sh_vnormals) = (int *) PoolAllocateMem (sizeof(int) * shphd->numpoints * 3);
	
	shphd->sh_normals = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->sh_normals) = (int *) PoolAllocateMem (sizeof(int) * shphd->numitems * 3);

	// for textures
	if (cshp_ptr->num_uvs)
		shphd->sh_textures = (int **) PoolAllocateMem (sizeof(int *) * cshp_ptr->num_uvs);
	if (cshp_ptr->num_texfiles)
		shphd->sh_localtextures = (char **) PoolAllocateMem (sizeof(char *) * (cshp_ptr->num_texfiles+1));

	int * item_list;

	shphd->items = (int **) PoolAllocateMem (sizeof(int *) * shphd->numitems);
	item_list = (int *) PoolAllocateMem (sizeof(int) * shphd->numitems * 9);

	tptr = *(shphd->points);

	
	
	if(object && local_scale!=1)
	{
		//convert from floating point to integers using world coordinates  in an attempt to stop
		//tears from being generated
		ChunkVector object_float;
		object_float.x=(double)object->location.x;
		object_float.y=(double)object->location.y;
		object_float.z=(double)object->location.z;

		VECTORCH object_int;
		object_int.vx=(int)(object_float.x*local_scale);
		object_int.vy=(int)(object_float.y*local_scale);
		object_int.vz=(int)(object_float.z*local_scale);

		for (i=0; i<shphd->numpoints; i++) {
			tptr[i*3] = (int) ((cshp_ptr->v_list[i].x+object_float.x)*local_scale);
			tptr[i*3 + 1] = (int) ((cshp_ptr->v_list[i].y+object_float.y)*local_scale);
			tptr[i*3 + 2] = (int) ((cshp_ptr->v_list[i].z+object_float.z)*local_scale);

			tptr[i*3]-=object_int.vx;
			tptr[i*3+1]-=object_int.vy;
			tptr[i*3+2]-=object_int.vz;
		}

	}
	else
	{
		for (i=0; i<shphd->numpoints; i++) {
			tptr[i*3] = (int) (cshp_ptr->v_list[i].x*local_scale);
			tptr[i*3 + 1] = (int) (cshp_ptr->v_list[i].y*local_scale);
			tptr[i*3 + 2] = (int) (cshp_ptr->v_list[i].z*local_scale);
		}
	}
	
	tptr = *(shphd->sh_vnormals);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3]     =(int) (cshp_ptr->v_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] =(int) (cshp_ptr->v_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] =(int) (cshp_ptr->v_normal_list[i].z*ONE_FIXED);
	}

	tptr = *(shphd->sh_normals);

	for (i=0; i<shphd->numitems; i++) {
		tptr[i*3]     =(int) (cshp_ptr->p_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] =(int) (cshp_ptr->p_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] =(int) (cshp_ptr->p_normal_list[i].z*ONE_FIXED);
	}

	
	for (i=0; i<shphd->numitems; i++)
		shphd->items[i] = &item_list[i*9];

	int * uv_imnums = 0;
	if (cshp_ptr->num_uvs)
	{
		uv_imnums = new int[cshp_ptr->num_uvs];
		for (i=0; i<cshp_ptr->num_uvs; ++i)
		{
			uv_imnums[i]=-1;
			shphd->sh_textures[i]=0;
		}
	}
	
	for (i=0; i<shphd->numitems; i++) {

		item_list[i*9] = (cshp_ptr->poly_list[i].engine_type);
		item_list[i*9 + 1] = (cshp_ptr->poly_list[i].normal_index * 3);
		item_list[i*9 + 2] = (cshp_ptr->poly_list[i].flags&~ChunkInternalItemFlags);
		item_list[i*9 + 3] = (cshp_ptr->poly_list[i].colour);
		
		if ( is_textured(item_list[i*9]) && !( item_list[i*9 + 3] & 0x8000 ) ) 
		{
			int texno = item_list[i*9 + 3] & 0x7fff;
			int UVIndex= item_list[i*9 + 3]>>16;

			if (texno <= local_max_index &&
				local_tex_index_nos[texno] != -1 &&
				cshp_ptr->uv_list[UVIndex].num_verts)
				
			{
				item_list[i*9 + 3] &= 0xffff0000;
				uv_imnums[item_list[i*9+3]>>16]=local_tex_index_nos[texno];
				item_list[i*9 + 3] += local_tex_index_nos[texno];

				shphd->sh_textures[UVIndex] = (int *) PoolAllocateMem (sizeof(int *) * cshp_ptr->uv_list[UVIndex].num_verts * 2);
				for (j=0; j<cshp_ptr->uv_list[UVIndex].num_verts; j++) {
					(shphd->sh_textures[UVIndex])[(j*2)] = ProcessUVCoord(h,UVC_POLY_U,(int)cshp_ptr->uv_list[UVIndex].vert[j].u,uv_imnums[UVIndex]);
					(shphd->sh_textures[UVIndex])[(j*2)+1] = ProcessUVCoord(h,UVC_POLY_V,(int)cshp_ptr->uv_list[UVIndex].vert[j].v,uv_imnums[UVIndex]);
				}
				
				
			}
			else
			{
				item_list[i*9] = I_Polyline;
				item_list[i*9 + 2] = (cshp_ptr->poly_list[i].flags&~ChunkInternalItemFlags) | iflag_nolight;
				item_list[i*9 + 3] = 0xffffffff;
			}
		}
			
		
		for (j=0;j<cshp_ptr->poly_list[i].num_verts;j++)
		//	item_list[i*9 + 4 +j] = (cshp_ptr->poly_list[i].vert_ind[j] *3);
			/* KJL 12:21:58 9/17/97 - I've removed the annoying *3 */
			item_list[i*9 + 4 +j] = (cshp_ptr->poly_list[i].vert_ind[j]);
		for (;j<5;j++)
			item_list[i*9 + 4 +j] = -1;
	}

	if (uv_imnums) delete[] uv_imnums;

	if (cshp_ptr->num_texfiles)
	{
		for (i=0; i<cshp_ptr->num_texfiles; i++) {
			#if john
			shphd->sh_localtextures[i] = 
				(char *) PoolAllocateMem (sizeof(char) * (strlen(cshp_ptr->texture_fns[i]) + strlen(TexturesRoot) + 1) );
			sprintf (shphd->sh_localtextures[i],"%s%s",TexturesRoot, cshp_ptr->texture_fns[i]);
			char * dotpos;
			dotpos = strrchr (shphd->sh_localtextures[i], '.');
			sprintf (dotpos,".pg0");
			#else
			shphd->sh_localtextures[i] = 
				(char *) PoolAllocateMem (sizeof(char) * (strlen(cshp_ptr->texture_fns[i]) + 1) );
			strcpy (shphd->sh_localtextures[i], cshp_ptr->texture_fns[i]);
			#endif
		}
		shphd->sh_localtextures[i] = 0;
	}




	SHAPEINSTR * instruct = (SHAPEINSTR *)PoolAllocateMem(sizeof(SHAPEINSTR)*6);
	
	shphd->sh_instruction = instruct;

	
	instruct[0].sh_instr = I_ShapePoints;                                           /*I_shapepoints*/
	instruct[0].sh_numitems = shphd->numpoints;
	instruct[0].sh_instr_data = shphd->points;

	instruct[1].sh_instr = I_ShapeNormals;                                          /*I_shapenormals*/
	instruct[1].sh_numitems = shphd->numitems;
	instruct[1].sh_instr_data = shphd->sh_normals;

	instruct[2].sh_instr = I_ShapeProject;                                          /*I_shapeproject*/
	instruct[2].sh_numitems = shphd->numpoints;
	instruct[2].sh_instr_data = shphd->points;

	instruct[3].sh_instr = I_ShapeVNormals;                                          /*I_shapevnormals*/
	instruct[3].sh_numitems = shphd->numpoints;
	instruct[3].sh_instr_data = shphd->sh_vnormals;

	instruct[4].sh_instr = I_ShapeItems;                                          
	instruct[4].sh_numitems = shphd->numitems;
	instruct[4].sh_instr_data = shphd->items;

	instruct[5].sh_instr = I_ShapeEnd;                                              /*I_shapeEND*/
	instruct[5].sh_numitems = 0;
	instruct[5].sh_instr_data = 0;
	

	Chunk * pchAnim = shape->lookup_single_child("TEXTANIM");
	if(pchAnim)
	{
		Animation_Chunk* ac=(Animation_Chunk*)pchAnim;
		Shape_Merge_Data_Chunk* smdc=0;
		Chunk * pCh = shape->lookup_single_child("SHPMRGDT");
		if(pCh) smdc=(Shape_Merge_Data_Chunk*)pCh;
		SetupAnimatedTextures((Shape_Chunk*)shape,shphd,ac,smdc, local_tex_index_nos);
	}
	
	if(shape->count_children("ANIMSEQU"))
	{
		Shape_Merge_Data_Chunk* smdc=0;
		Chunk * pCh=shape->lookup_single_child("SHPMRGDT");
		if(pCh) smdc=(Shape_Merge_Data_Chunk*)pCh;
		SetupAnimatingShape((Shape_Chunk*)shape,shphd,smdc);
	}

	return TRUE;
	
	
}
BOOL copy_preprocessed_to_shapeheader (
	RIFFHANDLE h, 
	Shape_Preprocessed_Data_Chunk* spdc, 
	SHAPEHEADER *& shphd, 
	Chunk_With_Children * shape, 
	int /*flags*/, 
	int local_max_index,
	int * local_tex_index_nos,
	int /*listpos*/,
	const ChunkObject* object
	)
{
	shphd = (SHAPEHEADER*)spdc->GetMemoryBlock();
	
	for (int i=0; i<shphd->numitems; i++)
	{
		if(is_textured(shphd->items[i][0]))
		{
			int texno = shphd->items[i][3] & 0x7fff;
			if (texno <= local_max_index &&
				local_tex_index_nos[texno] != -1)
				
			{
				shphd->items[i][3] &= 0xffff0000;
				shphd->items[i][3] += local_tex_index_nos[texno];

				
			}
			else
			{
				shphd->items[i][0] = I_Polyline;
				shphd->items[i][2] |=iflag_nolight;
				shphd->items[i][3] = 0xffffffff;
			}
		}
	}


	return TRUE;
	
	
}

BOOL copy_sprite_to_shapeheader (RIFFHANDLE h, SHAPEHEADER *& shphd,Sprite_Header_Chunk* shc, int listpos)
{
	Chunk * pChunk=shc->lookup_single_child("SPRITEPC");
	if(!pChunk)
	{
		return 0;
	}
	PC_Sprite_Chunk* sc=(PC_Sprite_Chunk*)pChunk;
	
	pChunk = shc->lookup_single_child("SPRISIZE");
	if(!pChunk)
	{
		return 0;
	}
	Sprite_Size_Chunk* ssc=(Sprite_Size_Chunk*)pChunk;

	Sprite_Extent_Chunk* sec=0;
	pChunk=shc->lookup_single_child("SPREXTEN");
	if(pChunk)
		sec=(Sprite_Extent_Chunk*)pChunk;
	
	pChunk=shc->lookup_single_child("SPRBMPSC");
	if(!pChunk)
	{
		return 0;
	}
	Sprite_Bitmap_Scale_Chunk* sbsc=(Sprite_Bitmap_Scale_Chunk*)pChunk;
	
	shphd = (SHAPEHEADER *) PoolAllocateMem(sizeof(SHAPEHEADER));
	memset(shphd,0,sizeof(SHAPEHEADER));

	int i;
	int * tptr;
	int * BmpConv=0;
	int local_max_index;
	String sprite_name;
	

	
	Bitmap_List_Store_Chunk * blsc = 0;
	
	
	pChunk = shc->lookup_single_child("BMPLSTST");
	if (pChunk)
	{
		blsc = (Bitmap_List_Store_Chunk *) pChunk;
	}
	pChunk = shc->lookup_single_child("RIFFNAME");
	if (pChunk)
	{
		sprite_name = ((RIF_Name_Chunk *)pChunk)->rif_name;
		msl_shapes.add_entry(new ShapeInMSL(shphd,sprite_name,listpos));
	}

	if (blsc)	
	{
		// load in the textures from the shape
	
		local_max_index = 0;
		LIF<BMP_Name> bns (&blsc->bmps);
		for (; !bns.done(); bns.next())
		{
			local_max_index = max(bns().index,local_max_index);
		}
		
		BmpConv = new int [local_max_index+1];
		for (i=0; i<=local_max_index; i++)
		{
			BmpConv[i] = -1;
		}

		if (Env_Chunk == 0)
			Env_Chunk = h->fc;
		// JH 17-2-97 -- image loaders have changed to avoid loading the same image twice
		for (bns.restart() ; !bns.done(); bns.next())
		{
			if(bns().flags & ChunkBMPFlag_NotInPC) continue;
			
			String tex;
			if (bns().flags & ChunkBMPFlag_IFF)
			{
				tex = bns().filename;
			}
			else
			{
				tex = sprite_name;
				tex += "\\";
				tex += bns().filename;
			}

			int imgnum = load_rif_bitmap(bns().filename,bns().flags);
			if (GEI_NOTLOADED != imgnum)
				BmpConv[bns().index] = imgnum;
		}
		
	}
	// header data (note shapeheader is calloced)
	
	shphd->numpoints = 4;
	shphd->numitems = 1;


	shphd->shaperadius =ssc->radius*GlobalScale;

	if(sec)
	{
		shphd->shapemaxx =(int)(sec->maxx*GlobalScale); 
		shphd->shapeminx =(int)(sec->minx*GlobalScale); 
		shphd->shapemaxy =(int)(sec->maxy*GlobalScale); 
		shphd->shapeminy =(int)(sec->miny*GlobalScale); 
		
	}
	else
	{
		shphd->shapemaxx =(int)(ssc->maxx*GlobalScale); 
		shphd->shapeminx =(int)(-ssc->maxx*GlobalScale); 
		shphd->shapemaxy =(int)(ssc->maxy*GlobalScale); 
		shphd->shapeminy =(int)(-ssc->maxy*GlobalScale); 
	}
	shphd->shapemaxz =501*GlobalScale; 
	shphd->shapeminz =-501*GlobalScale; 
					   
	// AllocateMem arrays

	shphd->points = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->points) = (int *) PoolAllocateMem (sizeof(int) * shphd->numpoints * 3);
	
	shphd->sh_vnormals = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->sh_vnormals) = (int *) PoolAllocateMem (sizeof(int) * shphd->numpoints * 3);
	
	shphd->sh_normals = (int **) PoolAllocateMem (sizeof(int *));
	*(shphd->sh_normals) = (int *) PoolAllocateMem (sizeof(int) * shphd->numitems * 3);

	shphd->sh_textures = (int **) PoolAllocateMem (sizeof(int *));

	int * item_list;

	shphd->items = (int **) PoolAllocateMem (sizeof(int *) * shphd->numitems);
	item_list = (int *) PoolAllocateMem (sizeof(int) * shphd->numitems * 9);

	tptr = *(shphd->points);

	tptr[0]=shphd->shapemaxx;
	tptr[1]=shphd->shapeminy;
	tptr[2]=0;
	tptr[3]=shphd->shapemaxx;
	tptr[4]=shphd->shapemaxy;
	tptr[5]=0;
	tptr[6]=shphd->shapeminx;
	tptr[7]=shphd->shapemaxy;
	tptr[8]=0;
	tptr[9]=shphd->shapeminx;
	tptr[10]=shphd->shapeminy;
	tptr[11]=0;

	tptr = *(shphd->sh_vnormals);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = 0;
		tptr[i*3 + 1] = 0;
		tptr[i*3 + 2] = ONE_FIXED;
	}

	tptr = *(shphd->sh_normals);

	for (i=0; i<shphd->numitems; i++) {
		tptr[i*3] = 0;
		tptr[i*3 + 1] = 0;
		tptr[i*3 + 2] = ONE_FIXED;
	}

	
	for (i=0; i<shphd->numitems; i++)
		shphd->items[i] = &item_list[i*9];


	item_list[0]=I_2dTexturedPolygon;
	item_list[1]=0;
	item_list[2]=iflag_ignore0| iflag_txanim|iflag_no_bfc;
	if(ssc->Flags & SpriteFlag_NoLight) item_list[2]|=iflag_nolight;
	if(ssc->Flags & SpriteFlag_SemiTrans) item_list[2]|=iflag_transparent;
	item_list[3]=0;
	item_list[4]=0;
	item_list[5]=1;
	item_list[6]=2;
	item_list[7]=3;
	item_list[8]=-1;

	SHAPEINSTR * instruct = (SHAPEINSTR *)PoolAllocateMem(sizeof(SHAPEINSTR)*6);
	
	shphd->sh_instruction = instruct;

	instruct[0].sh_instr = I_ShapeSpriteRPoints;                                           /*I_shapepoints*/
	instruct[0].sh_numitems = 4;
	instruct[0].sh_instr_data = shphd->points;

	instruct[1].sh_instr = I_ShapeNormals;                                          /*I_shapenormals*/
	instruct[1].sh_numitems = shphd->numitems;
	instruct[1].sh_instr_data = shphd->sh_normals;

	instruct[2].sh_instr = I_ShapeProject;                                          /*I_shapeproject*/
	instruct[2].sh_numitems = shphd->numpoints;
	instruct[2].sh_instr_data = shphd->points;

	instruct[3].sh_instr = I_ShapeVNormals;                                          /*I_shapevnormals*/
	instruct[3].sh_numitems = shphd->numpoints;
	instruct[3].sh_instr_data = shphd->sh_vnormals;

	instruct[4].sh_instr = I_ShapeItems;                                          
	instruct[4].sh_numitems = shphd->numitems;
	instruct[4].sh_instr_data = shphd->items;

	instruct[5].sh_instr = I_ShapeEnd;                                              /*I_shapeEND*/
	instruct[5].sh_numitems = 0;
	instruct[5].sh_instr_data = 0;
	
	shphd->shapeflags=ShapeFlag_MultiViewSprite|ShapeFlag_SpriteResizing|ShapeFlag_Sprite;
	
	List<Chunk *> chlist;
	sc->lookup_child("SPRACTIO",chlist);

	int MaxSeq=0;
	for(LIF<Chunk*>chlif(&chlist);!chlif.done();chlif.next())
	{
		MaxSeq=max(MaxSeq,((Sprite_Action_Chunk*)chlif())->Action);
	}
	txanimheader** thlist=(txanimheader**)PoolAllocateMem((3+MaxSeq)*sizeof(txanimheader));
	thlist[0]=thlist[MaxSeq+2]=0;
	for(i=1;i<MaxSeq+2;i++)
	{
		thlist[i]=(txanimheader*)PoolAllocateMem(sizeof(txanimheader));
		thlist[i]->txa_numframes=0;
		thlist[i]->txa_maxframe=0;
		thlist[i]->txa_num_mvs_images=0;
		thlist[i]->txa_framedata=0;
	}
	while(chlist.size())
	{
		Sprite_Action_Chunk* sac=(Sprite_Action_Chunk*) chlist.first_entry();
		txanimheader* th=thlist[sac->Action+1];
		
		th->txa_flags=txa_flag_play;
		th->txa_numframes=sac->NumFrames+1;
		th->txa_currentframe=0;
		th->txa_state=0;
		th->txa_maxframe=(th->txa_numframes-1)<<16;
		if(sac->FrameTime)
			th->txa_speed=65536000/sac->FrameTime;
		else
			th->txa_speed=65536*8;
			
		th->txa_num_mvs_images=sac->NumYaw*sac->NumPitch*2;
		
		th->txa_eulerxshift=12;
		int j=sac->NumPitch;
		while(j)
		{
			j=j>>1;
			th->txa_eulerxshift--;
		}
		th->txa_euleryshift=12;
		j=sac->NumYaw;
		while(j)
		{
			j=j>>1;
			th->txa_euleryshift--;
		}
		if(sac->NumYaw==1)
		{
			th->txa_euleryshift=12;
			th->txa_num_mvs_images=sac->NumPitch;
		}

		th->txa_framedata=(txanimframe*)PoolAllocateMem(th->txa_numframes*sizeof(txanimframe_mvs));
		
		txanimframe_mvs* tf;
		for(j=0;j<th->txa_numframes;j++)
		{
			int framefrom=j;
			if(j==sac->NumFrames) framefrom=0;
			tf=(txanimframe_mvs*)&th->txa_framedata[j];
			tf->txf_flags=0;
			tf->txf_scale=ONE_FIXED;
			tf->txf_scalex=0;
			tf->txf_scaley=0;
			tf->txf_orient=0;
			tf->txf_orientx=0;
			tf->txf_orienty=0;
			tf->txf_numuvs=4;
			
			tf->txf_uvdata=(int**)PoolAllocateMem((th->txa_num_mvs_images)*sizeof(int*));
			int* uvdata=(int*)PoolAllocateMem(th->txa_num_mvs_images*16*sizeof(int));
			for(int k=0;k<th->txa_num_mvs_images;k++)
			{
				tf->txf_uvdata[k]=&uvdata[16*k];
			}
			
			tf->txf_images=(int*)PoolAllocateMem(th->txa_num_mvs_images*sizeof(int));
			int ny=2*sac->NumYaw;
			if(sac->NumYaw==1) ny=1;
			int y,y2;
			int pos,pos2;
			for(y=0;y<ny;y+=2)
			{
				for(int p=0;p<sac->NumPitch;p++)
				{
					y2=(y-1+ny)%ny;
					pos=y*sac->NumPitch+p;
					pos2=y2*sac->NumPitch+p;
					Frame* f=&sac->FrameList[y/2][p][framefrom];
				
					GLOBALASSERT(f->Texture<=local_max_index);
					GLOBALASSERT(f->Texture>=0);
					GLOBALASSERT(BmpConv[f->Texture]!=-1);
	
					tf->txf_images[pos]=BmpConv[f->Texture];
					tf->txf_images[pos2]=BmpConv[f->Texture];
				
					float bmpscale=sbsc->Scale[f->Texture];
					if(y>ny/2 && (sac->Flags & SpriteActionFlag_FlipSecondSide))
					{
						for(int l=0;l<4;l++)
						{
							tf->txf_uvdata[pos][l*2]=ProcessUVCoord(h,UVC_SPRITE_U,f->UVCoords[3-l][0]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos][l*2+1]=ProcessUVCoord(h,UVC_SPRITE_V,f->UVCoords[3-l][1]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos2][l*2]=ProcessUVCoord(h,UVC_SPRITE_U,f->UVCoords[3-l][0]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos2][l*2+1]=ProcessUVCoord(h,UVC_SPRITE_V,f->UVCoords[3-l][1]<<16,BmpConv[f->Texture]);
							
							
							tf->txf_uvdata[pos][l*2+8]=(int)(-(f->UVCoords[3-l][0]-f->CentreX)*bmpscale*GlobalScale);
						   	tf->txf_uvdata[pos][l*2+9]=(int)((f->UVCoords[3-l][1]-f->CentreY)*bmpscale*GlobalScale);
							tf->txf_uvdata[pos2][l*2+8]=(int)(-(f->UVCoords[3-l][0]-f->CentreX)*bmpscale*GlobalScale);
							tf->txf_uvdata[pos2][l*2+9]=(int)((f->UVCoords[3-l][1]-f->CentreY)*bmpscale*GlobalScale);
							
						}
					}
					else
					{
						for(int l=0;l<4;l++)
						{
							tf->txf_uvdata[pos][l*2]=ProcessUVCoord(h,UVC_SPRITE_U,f->UVCoords[l][0]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos][l*2+1]=ProcessUVCoord(h,UVC_SPRITE_V,f->UVCoords[l][1]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos2][l*2]=ProcessUVCoord(h,UVC_SPRITE_U,f->UVCoords[l][0]<<16,BmpConv[f->Texture]);
							tf->txf_uvdata[pos2][l*2+1]=ProcessUVCoord(h,UVC_SPRITE_V,f->UVCoords[l][1]<<16,BmpConv[f->Texture]);
							
							
							tf->txf_uvdata[pos][l*2+8]=(int)((f->UVCoords[l][0]-f->CentreX)*bmpscale*GlobalScale);
						   	tf->txf_uvdata[pos][l*2+9]=(int)((f->UVCoords[l][1]-f->CentreY)*bmpscale*GlobalScale);
							tf->txf_uvdata[pos2][l*2+8]=(int)((f->UVCoords[l][0]-f->CentreX)*bmpscale*GlobalScale);
							tf->txf_uvdata[pos2][l*2+9]=(int)((f->UVCoords[l][1]-f->CentreY)*bmpscale*GlobalScale);
							
						}
					}
				}
			}
		}

		chlist.delete_first_entry();
	}
	shphd->sh_textures[0]=(int*)thlist;
	delete [] BmpConv;
	return TRUE;
}



BOOL copy_to_map6(Object_Chunk * ob,MAPBLOCK6* mapblock, int shplst_pos)
{
	
	Object_Project_Data_Chunk * opdc = 0;
	Map_Block_Chunk * mapblok = 0;
	Strategy_Chunk * strat = 0;
	
	if (ob->object_data.is_base_object)
		*mapblock = Empty_Landscape_Type6;
	else *mapblock = Empty_Object_Type6;



	Chunk * pChunk = ob->lookup_single_child("OBJPRJDT");
	if (pChunk) opdc = (Object_Project_Data_Chunk *)pChunk;
	if (opdc) 
	{
		pChunk = opdc->lookup_single_child("MAPBLOCK");
		if (pChunk) mapblok = (Map_Block_Chunk *)pChunk;
		pChunk = opdc->lookup_single_child("STRATEGY");
		if (pChunk) strat = (Strategy_Chunk *)pChunk;
	}
	
	if (mapblok)
	{
		mapblock->MapType = mapblok->map_data.MapType;	
		mapblock->MapFlags= mapblok->map_data.MapFlags;	
	
		#if (StandardStrategyAndCollisions || IntermediateSSACM)
		mapblock->MapCType = mapblok->map_data.MapCType;	
		mapblock->MapCGameType = mapblok->map_data.MapCGameType;	
		mapblock->MapCStrategyS = mapblok->map_data.MapCStrategyS;	
		mapblock->MapCStrategyL = mapblok->map_data.MapCStrategyL;	
		#endif
		
		mapblock->MapInteriorType = mapblok->map_data.MapInteriorType;	
//		mapblock->MapLightType = mapblok->map_data.MapLightType;	
//		mapblock->MapMass = mapblok->map_data.MapMass;	
//		mapblock->MapNewtonV.vx = mapblok->map_data.MapNewtonV.vx;	
//		mapblock->MapNewtonV.vy = mapblok->map_data.MapNewtonV.vy;	
//		mapblock->MapNewtonV.vz = mapblok->map_data.MapNewtonV.vz;	
//		mapblock->MapOrigin.vx = mapblok->map_data.MapOrigin.vx;	
//		mapblock->MapOrigin.vy = mapblok->map_data.MapOrigin.vy;	
//		mapblock->MapOrigin.vz = mapblok->map_data.MapOrigin.vz;	
//		mapblock->MapViewType = mapblok->map_data.MapViewType;	
	}

	#if (StandardStrategyAndCollisions || IntermediateSSACM)
	if (strat)
	{
		mapblock->MapStrategy = strat->strategy_data.Strategy;
	}
	#endif
	
	mapblock->MapShape = shplst_pos;

	mapblock->MapWorld.vx = (int) (ob->object_data.location.x*local_scale);
	mapblock->MapWorld.vy = (int) (ob->object_data.location.y*local_scale);
	mapblock->MapWorld.vz = (int) (ob->object_data.location.z*local_scale);

	QUAT q;

	q.quatx = (int)(-ob->object_data.orientation.x*ONE_FIXED);
	q.quaty = (int)(-ob->object_data.orientation.y*ONE_FIXED);
	q.quatz = (int)(-ob->object_data.orientation.z*ONE_FIXED);
	q.quatw = (int)(ob->object_data.orientation.w*ONE_FIXED);

	MATRIXCH m;
	
	QuatToMat (&q, &m);
	
	EULER e;
	
	MatrixToEuler(&m, &e);
	
	/*
		This function is only being used by the tools.
		At least for the moment I need the Euler to contain
		the 'inverse' rotation ,until I get round to sorting things
		out properly. Richard
	*/
	
	mapblock->MapEuler.EulerX = e.EulerX;
	mapblock->MapEuler.EulerY = e.EulerY;
	mapblock->MapEuler.EulerZ = e.EulerZ;

	return TRUE;
}





BOOL tex_merge_polys ( ChunkPoly & p1, ChunkPoly & p2, ChunkPoly & p, ChunkShape & shp, int * /*mgd*/)
{
	int j;

	if (p1.engine_type != p2.engine_type)
		return(FALSE);
		
	if ((p1.colour & 0xffff) != (p2.colour & 0xffff))
		return(FALSE);
	
	if (p1.flags != p2.flags)
		return(FALSE);

	
	int p1_uvind = p1.colour >> 16;
	int p2_uvind = p2.colour >> 16;

	ChunkUV_List uv;
	p = p1;
	p.num_verts = 4;
	uv.num_verts = 4;
	
	int num_ins = 0;
	int p_onv = 0;
	int new_p_onv;
	int * p_on = p1.vert_ind;
	int * p_oth = p2.vert_ind;
	int * temp;

	ChunkUV * uv_on = shp.uv_list[p1_uvind].vert;
	ChunkUV * uv_oth = shp.uv_list[p2_uvind].vert;
	ChunkUV * uvtemp;
	
	while (num_ins < 4)
	{
		uv.vert[num_ins] = uv_on[p_onv];
		p.vert_ind[num_ins++] = p_on[p_onv];
		for (j=0; j<3; j++)
		{
			if (p_on[p_onv] == p_oth[j] && 
					(uv_on[p_onv].u != uv_oth[j].u || uv_on[p_onv].v != uv_oth[j].v))
			{
				return(FALSE);
			}
			if (p_on[p_onv] == p_oth[j])
				break;
		}
		if (j==3) p_onv = (p_onv+1)%3;
		else
		{
			new_p_onv = j;
			for (j=0; j<3; j++)
			{
				if (p_on[(p_onv+1)%3] == p_oth[j] && 
						(uv_on[(p_onv+1)%3].u != uv_oth[j].u || uv_on[(p_onv+1)%3].v != uv_oth[j].v))
				{
					return (FALSE);
				}
				if (p_on[(p_onv+1)%3] == p_oth[j])
					break;
			}
			if (j==3) p_onv = (p_onv+1)%3;
			else
			{
				temp = p_on;
				p_on = p_oth;
				p_oth = temp;
				p_onv = (new_p_onv+1)%3;

				uvtemp = uv_on;
				uv_on = uv_oth;
				uv_oth = uvtemp;
			}
		}
	}

	shp.uv_list[p1_uvind] = uv;

	return(TRUE);
	
		
}

BOOL merge_polys ( ChunkPoly & p1, ChunkPoly & p2, ChunkPoly & p, int * /*mgd*/)
{
	int j;

	if (p1.engine_type != p2.engine_type)
		return(FALSE);
	
	if (p1.colour != p2.colour)
		return(FALSE);
	
	if (p1.flags != p2.flags)
		return(FALSE);

			
	p = p1;
	p.num_verts = 4;
	int num_ins = 0;
	int p_onv = 0;
	int new_p_onv;
	int * p_on = p1.vert_ind;
	int * p_oth = p2.vert_ind;
	int * temp;
	
	while (num_ins < 4)
	{
		p.vert_ind[num_ins++] = p_on[p_onv];
		for (j=0; j<3; j++)
		{
			if (p_on[p_onv] == p_oth[j])
				break;
		}
		if (j==3) p_onv = (p_onv+1)%3;
		else
		{
			new_p_onv = j;
			for (j=0; j<3; j++)
			{
				if (p_on[(p_onv+1)%3] == p_oth[j])
					break;
			}
			if (j==3) p_onv = (p_onv+1)%3;
			else
			{
				temp = p_on;
				p_on = p_oth;
				p_oth = temp;
				p_onv = (new_p_onv+1)%3;
			}
		}
	}
	return(TRUE);
}	


void merge_polygons_in_chunkshape (ChunkShape & shp, Shape_Merge_Data_Chunk * smdc)
{
	int * mgd = smdc->merge_data;
	
	int p_no = 0;


	ChunkPoly * new_polys = new ChunkPoly [shp.num_polys];
	ChunkVectorFloat * new_pnorms = new ChunkVectorFloat [shp.num_polys];
	
	for (int i = 0; i<shp.num_polys; i++)
	{
		if (mgd[i] == -1)
		{
			new_polys[p_no] = shp.poly_list[i];
			new_polys[p_no].normal_index = p_no;
			new_pnorms[p_no] = shp.p_normal_list[i];
			p_no ++;
		}
		else if (mgd[i]>i)
		{
			ChunkPoly p;
			
			BOOL merged = FALSE;
			
			//make sure points are within 10mm of being planar
			
			int mpoly=mgd[i];
			//find the 'unique vertex' in the second triangle
			int j, k;
			for(j=0;j<3;j++)
			{
				for(k=0;k<3;k++)
				{
					if(shp.poly_list[mpoly].vert_ind[j]==shp.poly_list[i].vert_ind[k])break;
				}
				if(k==3)
				{
					break;
				}
			}
			GLOBALASSERT(j!=3);
			int vert1=shp.poly_list[mpoly].vert_ind[j];
			int vert2=shp.poly_list[mpoly].vert_ind[(j+1)%3];
			ChunkVectorInt diff=shp.v_list[vert1]-shp.v_list[vert2];
			ChunkVectorFloat* norm=&shp.p_normal_list[i];
			
			//take the dot product of the normal and the difference to find the distance form the first 
			//triangles plane		
			float distance= (float)diff.x*norm->x + (float)diff.y*norm->y + (float)diff.z*norm->z;

			if(distance>-1 && distance <1) 
			{
				if (is_textured(shp.poly_list[i].engine_type))
				{
					merged = tex_merge_polys ( shp.poly_list[i], shp.poly_list[mgd[i]], p, shp, mgd);
				}
				else
				{
					merged = merge_polys ( shp.poly_list[i], shp.poly_list[mgd[i]], p, mgd);
				}
			}
			if (merged)
			{
				p.normal_index = p_no;
				new_polys[p_no] = p;
				new_pnorms[p_no] = shp.p_normal_list[i];
				p_no++;
			}
			else
			{
				new_polys[p_no] = shp.poly_list[i];
				new_polys[p_no].normal_index = p_no;
				new_pnorms[p_no] = shp.p_normal_list[i];
				p_no ++;

				new_polys[p_no] = shp.poly_list[mgd[i]];
				new_polys[p_no].normal_index = p_no;
				new_pnorms[p_no] = shp.p_normal_list[mgd[i]];
				p_no ++;
			}
		
		}
	}
	
	delete [] shp.poly_list;
	delete [] shp.p_normal_list;
	shp.poly_list = new_polys;
	shp.p_normal_list = new_pnorms;
	shp.num_polys = p_no;
	
}






#if 0
BOOL copy_to_mainshpl (Shape_Chunk * shp, int list_pos)
{
	SHAPEHEADER * shphd = (SHAPEHEADER *) AllocateMem(sizeof(SHAPEHEADER));
	memset(shphd,0,sizeof(SHAPEHEADER));

	ChunkShape cshp = shp->shape_data;

	Shape_Merge_Data_Chunk * smdc = 0;

	Chunk * pChunk = shp->lookup_single_child("SHPMRGDT");
	if (pChunk)
	{
		smdc = (Shape_Merge_Data_Chunk *) pChunk;
	}

	merge_polygons_in_chunkshape (cshp,smdc);

	int i,j, * tptr;

	mainshapelist[list_pos] = shphd;

	// header data (note shapeheader is calloced)

	shphd->shapeflags = shphd->shapeflags | (ShapeFlag_AugZ | ShapeFlag_AugZ_Lite | 
	                    ShapeFlag_SizeSortItems); // SIZE HACK!!!
	shphd->numpoints = cshp.num_verts;
	shphd->numitems = cshp.num_polys;

	shphd->shaperadius = (int) (cshp.radius*local_scale);

	shphd->shapemaxx = (int) (cshp.max.x*local_scale);
	shphd->shapeminx = (int) (cshp.min.x*local_scale);
	shphd->shapemaxy = (int) (cshp.max.y*local_scale);
	shphd->shapeminy = (int) (cshp.min.y*local_scale);
	shphd->shapemaxz = (int) (cshp.max.z*local_scale);
	shphd->shapeminz = (int) (cshp.min.z*local_scale);

	// AllocateMem arrays

	shphd->points = (int **) AllocateMem (sizeof(int *));
	*(shphd->points) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_vnormals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_vnormals) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_normals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_normals) = (int *) AllocateMem (sizeof(int) * shphd->numitems * 3);

	// for textures
	if (cshp.num_uvs)
		shphd->sh_textures = (int **) AllocateMem (sizeof(int *) * cshp.num_uvs);
	if (cshp.num_texfiles)
		shphd->sh_localtextures = (char **) AllocateMem (sizeof(char *) * (cshp.num_texfiles+1));

	int * item_list;

	shphd->items = (int **) AllocateMem (sizeof(int *) * shphd->numitems);
	item_list = (int *) AllocateMem (sizeof(int) * shphd->numitems * 9);

	tptr = *(shphd->points);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_list[i].x*local_scale);
		tptr[i*3 + 1] = (int) (cshp.v_list[i].y*local_scale);
		tptr[i*3 + 2] = (int) (cshp.v_list[i].z*local_scale);
	}

	tptr = *(shphd->sh_vnormals);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.v_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.v_normal_list[i].z*ONE_FIXED);
	}

	tptr = *(shphd->sh_normals);

	for (i=0; i<shphd->numitems; i++) {
		tptr[i*3] = (int) (cshp.p_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.p_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.p_normal_list[i].z*ONE_FIXED);
	}


	for (i=0; i<shphd->numitems; i++)
		shphd->items[i] = &item_list[i*9];



	for (i=0; i<shphd->numitems; i++) {


		item_list[i*9] = (cshp.poly_list[i].engine_type);
//		item_list[i*9] = I_Polyline;

		/* trap for polylines*/
		if((item_list[i*9] < 2 ) || (item_list[i*9] > 9))
			{
				item_list[i*9] = I_Polygon;
			}
#if 1
		/* try to set gouraud, for hazing*/

		if(item_list[i*9] == I_Polygon)
			{
				item_list[i*9] = I_GouraudPolygon;
			}				

		if(item_list[i*9] == I_2dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				

		if(item_list[i*9] == I_3dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				
#endif

//		if(item_list[i*9] == I_Polygon)
//			{
//				item_list[i*9] = I_ZB_Polygon;
//				/*HACK HACK ROXHACK*/
//			}

		#if SupportZBuffering
		#else
		if(item_list[i*9] == I_ZB_Polygon)
			item_list[i*9] = I_Polygon; 			

		if(item_list[i*9] == I_ZB_GouraudPolygon)
			item_list[i*9] = I_GouraudPolygon;			

		if(item_list[i*9] == I_ZB_PhongPolygon)
			item_list[i*9] = I_PhongPolygon;			

		if(item_list[i*9] == I_ZB_2dTexturedPolygon)
			item_list[i*9] = I_2dTexturedPolygon;			

		if(item_list[i*9] == I_ZB_Gouraud2dTexturedPolygon)
			item_list[i*9] = I_Gouraud2dTexturedPolygon;

		if(item_list[i*9] == I_ZB_3dTexturedPolygon)
			item_list[i*9] = I_3dTexturedPolygon;
		#endif



		item_list[i*9 + 1] = (cshp.poly_list[i].normal_index * 3);
		item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags);
//		item_list[i*9 + 2] = (cshp.poly_list[i].flags) | iflag_nolight;
		item_list[i*9 + 3] = (cshp.poly_list[i].colour);

#if 1
		if ( (item_list[i*9] == I_2dTexturedPolygon
					|| item_list[i*9] == I_Gouraud2dTexturedPolygon
					|| item_list[i*9] == I_3dTexturedPolygon
					|| item_list[i*9] == I_Gouraud3dTexturedPolygon
					)	&&
				!( item_list[i*9 + 3] & 0x8000 ) )
		{
			int texno = item_list[i*9 + 3] & 0x7fff;

			if (texno <= max_index)
				if ((tex_index_nos[texno] != -1))
				{
					item_list[i*9 + 3] &= 0xffff0000;
					item_list[i*9 + 3] += tex_index_nos[texno];

			/* hack in iflag_no_light */
					item_list[i*9 + 2] |= iflag_gsort_ptest /*| iflag_nolight*/ | iflag_linear_s | iflag_tx2dor3d;

				}
				else
				{
					item_list[i*9] = I_Polygon;
					item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags) | iflag_nolight;
					item_list[i*9 + 3] = 0xffffffff;
				}
			else
			{
				item_list[i*9] = I_Polygon;
				item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags) | iflag_nolight;
				item_list[i*9 + 3] = 0xffffffff;
			}
		}
#endif

//		item_list[i*9 + 3] = 0xffffffff;


		for (j=0;j<cshp.poly_list[i].num_verts;j++)
//			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j] *3);
			/* KJL 12:21:58 9/17/97 - I've removed the annoying *3 */
			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j]);
		for (;j<5;j++)
			item_list[i*9 + 4 +j] = -1;
	}

	if (cshp.num_uvs)
	{
		for (i=0; i<cshp.num_uvs; i++) {
			shphd->sh_textures[i] = (int *) AllocateMem (sizeof(int *) * cshp.uv_list[i].num_verts * 2);
			for (j=0; j<cshp.uv_list[i].num_verts; j++) {
				(shphd->sh_textures[i])[(j*2)] = (int)cshp.uv_list[i].vert[j].u;
				(shphd->sh_textures[i])[(j*2)+1] = (int)cshp.uv_list[i].vert[j].v;
			}
		}
	}

	if (cshp.num_texfiles)
	{
		for (i=0; i<cshp.num_texfiles; i++) {
			#if john
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + strlen(TexturesRoot) + 1) );
			sprintf (shphd->sh_localtextures[i],"%s%s",TexturesRoot, cshp.texture_fns[i]);
			char * dotpos;
			dotpos = strrchr (shphd->sh_localtextures[i], '.');
			sprintf (dotpos,".pg0");
			#else
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + 1) );
			strcpy (shphd->sh_localtextures[i], cshp.texture_fns[i]);
			#endif
		}
		shphd->sh_localtextures[i] = 0;
	}




	SHAPEINSTR * instruct = (SHAPEINSTR *)AllocateMem(sizeof(SHAPEINSTR)*6);

	shphd->sh_instruction = instruct;

	
	
	instruct[0].sh_instr = I_ShapePoints;                                           /*I_shapepoints*/
	instruct[0].sh_numitems = shphd->numpoints;
	instruct[0].sh_instr_data = shphd->points;

	instruct[1].sh_instr = I_ShapeNormals;                                          /*I_shapenormals*/
	instruct[1].sh_numitems = shphd->numitems;
	instruct[1].sh_instr_data = shphd->sh_normals;

	instruct[2].sh_instr = I_ShapeProject;                                          /*I_shapeproject*/
	instruct[2].sh_numitems = shphd->numpoints;
	instruct[2].sh_instr_data = shphd->points;

	instruct[3].sh_instr = I_ShapeVNormals;                                          /*I_shapevnormals*/
	instruct[3].sh_numitems = shphd->numpoints;
	instruct[3].sh_instr_data = shphd->sh_vnormals;

	instruct[4].sh_instr = I_ShapeItems;
	instruct[4].sh_numitems = shphd->numitems;
	instruct[4].sh_instr_data = shphd->items;

	instruct[5].sh_instr = I_ShapeEnd;                                              /*I_shapeEND*/
	instruct[5].sh_numitems = 0;
	instruct[5].sh_instr_data = 0;
	

	Chunk * pchAnim = shp->lookup_single_child("TEXTANIM");
	if (pchAnim)
	{
		Shape_Merge_Data_Chunk* smdc=0;
		Chunk * pChunk=shp->lookup_single_child("SHPMRGDT");
		if(pChunk) smdc=(Shape_Merge_Data_Chunk*)pChunk;
		SetupAnimatedTextures(shp,shphd,(Animation_Chunk*)pchAnim,smdc);
	}

	
	
	return TRUE;
		
}
#endif

#if 0
BOOL copy_to_mainshpl (Shape_Sub_Shape_Chunk * shp, int list_pos)
{
	SHAPEHEADER * shphd = (SHAPEHEADER *) AllocateMem(sizeof(SHAPEHEADER));
	memset(shphd,0,sizeof(SHAPEHEADER));

	ChunkShape cshp = shp->shape_data;

	Shape_Merge_Data_Chunk * smdc = 0;

	Chunk * pChunk = shp->lookup_single_child("SHPMRGDT");
	if (pChunk)
	{
		smdc = (Shape_Merge_Data_Chunk *) pChunk;
	}

	merge_polygons_in_chunkshape (cshp,smdc);

	int i,j, * tptr;

	mainshapelist[list_pos] = shphd;

	// header data (note shapeheader is calloced)

	shphd->shapeflags = shphd->shapeflags | (ShapeFlag_AugZ | ShapeFlag_AugZ_Lite | 
	                    ShapeFlag_SizeSortItems); // SIZE HACK!!!
	shphd->numpoints = cshp.num_verts;
	shphd->numitems = cshp.num_polys;

	shphd->shaperadius = (int) (cshp.radius*local_scale);

	shphd->shapemaxx = (int) (cshp.max.x*local_scale);
	shphd->shapeminx = (int) (cshp.min.x*local_scale);
	shphd->shapemaxy = (int) (cshp.max.y*local_scale);
	shphd->shapeminy = (int) (cshp.min.y*local_scale);
	shphd->shapemaxz = (int) (cshp.max.z*local_scale);
	shphd->shapeminz = (int) (cshp.min.z*local_scale);

	// AllocateMem arrays

	shphd->points = (int **) AllocateMem (sizeof(int *));
	*(shphd->points) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_vnormals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_vnormals) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_normals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_normals) = (int *) AllocateMem (sizeof(int) * shphd->numitems * 3);

	// for textures
	if (cshp.num_uvs)
		shphd->sh_textures = (int **) AllocateMem (sizeof(int *) * cshp.num_uvs);
	if (cshp.num_texfiles)
		shphd->sh_localtextures = (char **) AllocateMem (sizeof(char *) * (cshp.num_texfiles+1));

	int * item_list;

	shphd->items = (int **) AllocateMem (sizeof(int *) * shphd->numitems);
	item_list = (int *) AllocateMem (sizeof(int) * shphd->numitems * 9);

	tptr = *(shphd->points);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_list[i].x*local_scale);
		tptr[i*3 + 1] = (int) (cshp.v_list[i].y*local_scale);
		tptr[i*3 + 2] = (int) (cshp.v_list[i].z*local_scale);
	}

	tptr = *(shphd->sh_vnormals);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.v_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.v_normal_list[i].z*ONE_FIXED);
	}

	tptr = *(shphd->sh_normals);

	for (i=0; i<shphd->numitems; i++) {
		tptr[i*3] = (int) (cshp.p_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.p_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.p_normal_list[i].z*ONE_FIXED);
	}


	for (i=0; i<shphd->numitems; i++)
		shphd->items[i] = &item_list[i*9];



	for (i=0; i<shphd->numitems; i++) {


		item_list[i*9] = (cshp.poly_list[i].engine_type);
//		item_list[i*9] = I_Polyline;
#if 1
		/* try to set gouraud, for hazing*/

		if(item_list[i*9] == I_Polygon)
			{
				item_list[i*9] = I_GouraudPolygon;
			}				

		if(item_list[i*9] == I_2dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				

		if(item_list[i*9] == I_3dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				
#endif

//		if((item_list[i*9] < 2 ) || (item_list[i*9] > 9))
//			{
//				item_list[i*9] = I_Polygon;
//			}
#if 1
		/* try to set gouraud, for hazing*/

		if(item_list[i*9] == I_Polygon)
			{
				item_list[i*9] = I_GouraudPolygon;
			}				

		if(item_list[i*9] == I_2dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				

		if(item_list[i*9] == I_3dTexturedPolygon)
			{
				item_list[i*9] = I_Gouraud2dTexturedPolygon;
			}				
#endif

//		if(item_list[i*9] == I_Polygon)
//			{
//				item_list[i*9] = I_ZB_Polygon;
//				/*HACK HACK ROXHACK*/
//			}

		#if SupportZBuffering
		#else
		if(item_list[i*9] == I_ZB_Polygon)
			item_list[i*9] = I_Polygon; 			

		if(item_list[i*9] == I_ZB_GouraudPolygon)
			item_list[i*9] = I_GouraudPolygon;			

		if(item_list[i*9] == I_ZB_PhongPolygon)
			item_list[i*9] = I_PhongPolygon;			

		if(item_list[i*9] == I_ZB_2dTexturedPolygon)
			item_list[i*9] = I_2dTexturedPolygon;			

		if(item_list[i*9] == I_ZB_Gouraud2dTexturedPolygon)
			item_list[i*9] = I_Gouraud2dTexturedPolygon;

		if(item_list[i*9] == I_ZB_3dTexturedPolygon)
			item_list[i*9] = I_3dTexturedPolygon;
		#endif



		item_list[i*9 + 1] = (cshp.poly_list[i].normal_index * 3);
		item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags);
//		item_list[i*9 + 2] = (cshp.poly_list[i].flags) | iflag_nolight;
		item_list[i*9 + 3] = (cshp.poly_list[i].colour);

#if 1
		if ( (item_list[i*9] == 5 || item_list[i*9] == 6
		|| item_list[i*9] == 7)	&&
				!( item_list[i*9 + 3] & 0x8000 ) )
		{
			int texno = item_list[i*9 + 3] & 0x7fff;

			if (texno <= max_index)
				if ((tex_index_nos[texno] != -1))
				{
					item_list[i*9 + 3] &= 0xffff0000;
					item_list[i*9 + 3] += tex_index_nos[texno];

			/* hack in iflag_no_light */
					item_list[i*9 + 2] |= iflag_gsort_ptest /*| iflag_nolight*/ | iflag_linear_s | iflag_tx2dor3d;

				}
				else
				{
					item_list[i*9] = I_Polygon;
					item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags) | iflag_nolight;
					item_list[i*9 + 3] = 0xffffffff;
				}
			else
			{
				item_list[i*9] = I_Polygon;
				item_list[i*9 + 2] = (cshp.poly_list[i].flags &~ChunkInternalItemFlags) | iflag_nolight;
				item_list[i*9 + 3] = 0xffffffff;
			}
		}
#endif

//		item_list[i*9 + 3] = 0xffffffff;


		for (j=0;j<cshp.poly_list[i].num_verts;j++)
//			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j] *3);
			/* KJL 12:23:02 9/17/97 - I've removed the annoying *3 */
			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j]);
		for (;j<5;j++)
			item_list[i*9 + 4 +j] = -1;
	}

	if (cshp.num_uvs)
	{
		for (i=0; i<cshp.num_uvs; i++) {
			shphd->sh_textures[i] = (int *) AllocateMem (sizeof(int *) * cshp.uv_list[i].num_verts * 2);
			for (j=0; j<cshp.uv_list[i].num_verts; j++) {
				(shphd->sh_textures[i])[(j*2)] = (int)cshp.uv_list[i].vert[j].u;
				(shphd->sh_textures[i])[(j*2)+1] = (int)cshp.uv_list[i].vert[j].v;
			}
		}
	}

	if (cshp.num_texfiles)
	{
		for (i=0; i<cshp.num_texfiles; i++) {
			#if john
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + strlen(TexturesRoot) + 1) );
			sprintf (shphd->sh_localtextures[i],"%s%s",TexturesRoot, cshp.texture_fns[i]);
			char * dotpos;
			dotpos = strrchr (shphd->sh_localtextures[i], '.');
			sprintf (dotpos,".pg0");
			#else
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + 1) );
			strcpy (shphd->sh_localtextures[i], cshp.texture_fns[i]);
			#endif
		}
		shphd->sh_localtextures[i] = 0;
	}




	SHAPEINSTR * instruct = (SHAPEINSTR *)AllocateMem(sizeof(SHAPEINSTR)*6);

	shphd->sh_instruction = instruct;

	
	instruct[0].sh_instr = I_ShapePoints;                                           /*I_shapepoints*/
	instruct[0].sh_numitems = shphd->numpoints;
	instruct[0].sh_instr_data = shphd->points;

	instruct[1].sh_instr = I_ShapeNormals;                                          /*I_shapenormals*/
	instruct[1].sh_numitems = shphd->numitems;
	instruct[1].sh_instr_data = shphd->sh_normals;

	instruct[2].sh_instr = I_ShapeProject;                                          /*I_shapeproject*/
	instruct[2].sh_numitems = shphd->numpoints;
	instruct[2].sh_instr_data = shphd->points;

	instruct[3].sh_instr = I_ShapeVNormals;                                          /*I_shapevnormals*/
	instruct[3].sh_numitems = shphd->numpoints;
	instruct[3].sh_instr_data = shphd->sh_vnormals;

	instruct[4].sh_instr = I_ShapeItems;
	instruct[4].sh_numitems = shphd->numitems;
	instruct[4].sh_instr_data = shphd->items;

	instruct[5].sh_instr = I_ShapeEnd;                                              /*I_shapeEND*/
	instruct[5].sh_numitems = 0;
	instruct[5].sh_instr_data = 0;
	


	return TRUE;
#if 0
	SHAPEHEADER * shphd = (SHAPEHEADER *) AllocateMem(sizeof(SHAPEHEADER));
	memset(shphd,0,sizeof(SHAPEHEADER));

	ChunkShape cshp = shp->shape_data;

	Shape_Merge_Data_Chunk * smdc = 0;

	Chunk * pChunk = shp->lookup_single_child("SHPMRGDT");
	if (pChunk)
	{
		smdc = (Shape_Merge_Data_Chunk *) pChunk;
	}

	merge_polygons_in_chunkshape (cshp,smdc);

	int i,j, * tptr;

	mainshapelist[list_pos] = shphd;

	// header data (note shapeheader is calloced)

	shphd->numpoints = cshp.num_verts;
	shphd->numitems = cshp.num_polys;

	shphd->shaperadius = (int) (cshp.radius*local_scale);

	shphd->shapeflags = shphd->shapeflags | (ShapeFlag_AugZ | ShapeFlag_AugZ_Lite | 
	                    ShapeFlag_SizeSortItems); // SIZE HACK!!!

	shphd->shapemaxx = (int) (cshp.max.x*local_scale);
	shphd->shapeminx = (int) (cshp.min.x*local_scale);
	shphd->shapemaxy = (int) (cshp.max.y*local_scale);
	shphd->shapeminy = (int) (cshp.min.y*local_scale);
	shphd->shapemaxz = (int) (cshp.max.z*local_scale);
	shphd->shapeminz = (int) (cshp.min.z*local_scale);

	// AllocateMem arrays

	shphd->points = (int **) AllocateMem (sizeof(int *));
	*(shphd->points) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_vnormals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_vnormals) = (int *) AllocateMem (sizeof(int) * shphd->numpoints * 3);

	shphd->sh_normals = (int **) AllocateMem (sizeof(int *));
	*(shphd->sh_normals) = (int *) AllocateMem (sizeof(int) * shphd->numitems * 3);

	// for textures
	if (cshp.num_uvs)
		shphd->sh_textures = (int **) AllocateMem (sizeof(int *) * cshp.num_uvs);
	if (cshp.num_texfiles)
		shphd->sh_localtextures = (char **) AllocateMem (sizeof(char *) * (cshp.num_texfiles+1));

	int * item_list;

	shphd->items = (int **) AllocateMem (sizeof(int *) * shphd->numitems);
	item_list = (int *) AllocateMem (sizeof(int) * shphd->numitems * 9);

	tptr = *(shphd->points);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_list[i].x*local_scale);
		tptr[i*3 + 1] = (int) (cshp.v_list[i].y*local_scale);
		tptr[i*3 + 2] = (int) (cshp.v_list[i].z*local_scale);
	}

	tptr = *(shphd->sh_vnormals);

	for (i=0; i<shphd->numpoints; i++) {
		tptr[i*3] = (int) (cshp.v_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.v_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.v_normal_list[i].z*ONE_FIXED);
	}

	tptr = *(shphd->sh_normals);

	for (i=0; i<shphd->numitems; i++) {
		tptr[i*3] = (int) (cshp.p_normal_list[i].x*ONE_FIXED);
		tptr[i*3 + 1] = (int) (cshp.p_normal_list[i].y*ONE_FIXED);
		tptr[i*3 + 2] = (int) (cshp.p_normal_list[i].z*ONE_FIXED);
	}


	for (i=0; i<shphd->numitems; i++)
		shphd->items[i] = &item_list[i*9];


	for (i=0; i<shphd->numitems; i++) {
		item_list[i*9] = (cshp.poly_list[i].engine_type);
//		item_list[i*9] = I_Polyline;


		if((item_list[i*9] < 2 ) || (item_list[i*9] > 9))
			{
				item_list[i*9] = I_Polygon;
			}

		if(item_list[i*9] == I_2dTexturedPolygon)
			{
				item_list[i*9] = I_Polygon;
			}				

		if(item_list[i*9] == I_3dTexturedPolygon)
			{
				item_list[i*9] = I_2dTexturedPolygon;
			}				

//		if(item_list[i*9] == I_Polygon)
//			{
//				item_list[i*9] = I_ZB_Polygon;
//				/*HACK HACK ROXHACK*/
//			}

		item_list[i*9 + 1] = (cshp.poly_list[i].normal_index * 3);
		item_list[i*9 + 2] = (cshp.poly_list[i].flags&~ChunkInternalItemFlags);
//		item_list[i*9 + 2] = (cshp.poly_list[i].flags) | iflag_nolight;
		item_list[i*9 + 3] = (cshp.poly_list[i].colour);

		if ( (item_list[i*9] == 5 || item_list[i*9] == 6)	&&
				!( item_list[i*9 + 3] & 0x8000 ) )
		{
			int texno = item_list[i*9 + 3] & 0x7fff;

			if (texno < max_index)
				if (tex_index_nos[texno] != -1)
				{
					item_list[i*9 + 3] &= 0xffff0000;
					item_list[i*9 + 3] += tex_index_nos[texno];
				}
				else
				{
					item_list[i*9] = I_Polyline;
					item_list[i*9 + 2] = (cshp.poly_list[i].flags &~ChunkInternalItemFlags) /*| iflag_nolight*/;
					item_list[i*9 + 3] = 0xffffffff;
				}
			else
			{
				item_list[i*9] = I_Polyline;
				item_list[i*9 + 2] = (cshp.poly_list[i].flags &~ChunkInternalItemFlags) /*| iflag_nolight*/;
				item_list[i*9 + 3] = 0xffffffff;
			}
		}


//		item_list[i*9 + 3] = 0xffffffff;
		for (j=0;j<cshp.poly_list[i].num_verts;j++)
//			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j] *3);
			/* KJL 12:23:02 9/17/97 - I've removed the annoying *3 */
			item_list[i*9 + 4 +j] = (cshp.poly_list[i].vert_ind[j]);
		for (;j<5;j++)
			item_list[i*9 + 4 +j] = -1;
	}

	if (cshp.num_uvs)
	{
		for (i=0; i<cshp.num_uvs; i++) {
			shphd->sh_textures[i] = (int *) AllocateMem (sizeof(int *) * cshp.uv_list[i].num_verts * 2);
			for (j=0; j<cshp.uv_list[i].num_verts; j++) {
				(shphd->sh_textures[i])[(j*2)] = (int)cshp.uv_list[i].vert[j].u;
				(shphd->sh_textures[i])[(j*2)+1] = (int)cshp.uv_list[i].vert[j].v;
			}
		}
	}
 /*
 *  FILENAME: e:\avpcode\3dc\win95\chnkload.cpp
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */


	if (cshp.num_texfiles)
	{
		for (i=0; i<cshp.num_texfiles; i++) {
			#if john
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + strlen(TexturesRoot) + 1) );
			sprintf (shphd->sh_localtextures[i],"%s%s",TexturesRoot, cshp.texture_fns[i]);
			char * dotpos;
			dotpos = strrchr (shphd->sh_localtextures[i], '.');
			sprintf (dotpos,".pg0");
			#else
			shphd->sh_localtextures[i] =
				(char *) AllocateMem (sizeof(char) * (strlen(cshp.texture_fns[i]) + 1) );
			strcpy (shphd->sh_localtextures[i], cshp.texture_fns[i]);
			#endif
		}
		shphd->sh_localtextures[i] = 0;
	}




	SHAPEINSTR * instruct = (SHAPEINSTR *)AllocateMem(sizeof(SHAPEINSTR)*6);

	shphd->sh_instruction = instruct;

	
	instruct[0].sh_instr = I_ShapePoints;                                           /*I_shapepoints*/
	instruct[0].sh_numitems = shphd->numpoints;
	instruct[0].sh_instr_data = shphd->points;

	instruct[1].sh_instr = I_ShapeNormals;                                          /*I_shapenormals*/
	instruct[1].sh_numitems = shphd->numitems;
	instruct[1].sh_instr_data = shphd->sh_normals;

	instruct[2].sh_instr = I_ShapeProject;                                          /*I_shapeproject*/
	instruct[2].sh_numitems = shphd->numpoints;
	instruct[2].sh_instr_data = shphd->points;

	instruct[3].sh_instr = I_ShapeVNormals;                                          /*I_shapevnormals*/
	instruct[3].sh_numitems = shphd->numpoints;
	instruct[3].sh_instr_data = shphd->sh_vnormals;

	instruct[4].sh_instr = I_ShapeItems;
	instruct[4].sh_numitems = shphd->numitems;
	instruct[4].sh_instr_data = shphd->items;

	instruct[5].sh_instr = I_ShapeEnd;                                              /*I_shapeEND*/
	instruct[5].sh_numitems = 0;
	instruct[5].sh_instr_data = 0;
	


	return TRUE;
#endif		
}
#endif





