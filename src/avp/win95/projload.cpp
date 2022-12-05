#define DB_LEVEL 2

#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "list_tem.hpp"
#include "chnkload.hpp"
#include "projload.hpp"
#include "gamedef.h"
#include "shpchunk.hpp"
#include "envchunk.hpp"
#include "obchunk.hpp"
#include "chunkpal.hpp"
#include "bmpnames.hpp"
#include "chnktexi.h"
#include "strachnk.hpp"
#include "animchnk.hpp"
#include "ltchunk.hpp"
#include "avpchunk.hpp"
#include "pathchnk.hpp"

#include "objsetup.hpp"
#include "npcsetup.h"
#include "sprchunk.hpp"
#include "pcmenus.h"
#include <math.h>
#include "wpchunk.hpp"
#include "hierchnk.hpp"
#include "animobs.hpp"
#include "sequnces.h"
#include "fragchnk.hpp"
#include "hierplace.hpp"

#include "pfarlocs.h"
#include "progress_bar.h"
#include "stratdef.h"
#include "equipmnt.h"
#include "bh_ais.h"
#include "bh_types.h"
#include "bh_plachier.h"
#include "bh_marin.h"
#include "pvisible.h"
#include "psndplat.h"
#include "jsndsup.h"
#include "avpreg.hpp"
#include "ffstdio.h"

#include "decal.h"
#include "mempool.h"
#include "db.h"
#include "pldnet.h"

extern "C" {
#include "inventry.h"

extern int VideoMode;
extern int ScanDrawMode;
extern int ZBufferMode;
extern unsigned char *PaletteRemapTable;
extern unsigned char **PaletteShadingTableArray;
extern int HWAccel;

extern VECTORCH PlayerStartLocation;
extern MATRIXCH PlayerStartMat;

extern FARENTRYPOINTSHEADER *FALLP_EntryPoints;

extern RIFFHANDLE env_rif;

extern void NewOnScreenMessage(char *messagePtr);

extern BOOL KeepMainRifFile;

BOOL LevelHasStars;

};

// these are to link with chnkimag.cpp
const char * ToolsTex_Directory = "\\\\Kate\\Kate Share\\avp\\ToolsTex\\";
const char * GenTex_Directory = "\\\\Kate\\Kate Share\\avp\\GenG-Tex\\";
const char * SubShps_Directory = "SubShps\\All\\";
// const char * GenTex_Directory = 0;
const char * FixTex_Directory = "\\\\Kate\\Kate Share\\avp\\Fix-Tex\\";
const char * GameTex_Directory = "\\\\Kate\\Kate Share\\avp\\game-tex\\";
// new directories for new-style graphics - to be determined properly
char const * FirstTex_Directory = "Graphics"; // currently relative to cwd
char const * SecondTex_Directory = 0; // will be the src safe shadow for development builds
								//used for cd graphics directory in final version

char* Rif_Sound_Directory=0;//set for the main level rif

static char * light_set_name = "NORMALLT";

static Object_Chunk * * o_chunk_array;
static int * aimodule_indeces; //array parallel to o_chunk_array

extern void setup_placed_hierarchies(Environment_Data_Chunk * envd);
void setup_preplaced_decals(File_Chunk* fc,Environment_Data_Chunk* edc);
/////////////////////////////////////////
// Functions which operate on RIFFHANDLEs
/////////////////////////////////////////

void setup_start_position(RIFFHANDLE h)
{
	Chunk * pChunk = h->envd->lookup_single_child("SPECLOBJ");
	AVP_Player_Start_Chunk* start_chunk=0;
	if(pChunk)
	{
		List<Chunk*> start_list;
		((Chunk_With_Children*)pChunk)->lookup_child("AVPSTART",start_list);

		if(start_list.size())
			start_chunk=(AVP_Player_Start_Chunk*)start_list.first_entry();
		
	}
	if(start_chunk)
	{
		PlayerStartLocation.vx=(int)(start_chunk->location.x*local_scale);
		PlayerStartLocation.vy=(int)(start_chunk->location.y*local_scale);
		PlayerStartLocation.vz=(int)(start_chunk->location.z*local_scale);

		PlayerStartMat.mat11=start_chunk->orientation.mat11;
		PlayerStartMat.mat12=start_chunk->orientation.mat12;
		PlayerStartMat.mat13=start_chunk->orientation.mat13;
		PlayerStartMat.mat21=start_chunk->orientation.mat21;
		PlayerStartMat.mat22=start_chunk->orientation.mat22;
		PlayerStartMat.mat23=start_chunk->orientation.mat23;
		PlayerStartMat.mat31=start_chunk->orientation.mat31;
		PlayerStartMat.mat32=start_chunk->orientation.mat32;
		PlayerStartMat.mat33=start_chunk->orientation.mat33;
	}
	else
	{
		PlayerStartLocation.vx=0;
		PlayerStartLocation.vy=0;
		PlayerStartLocation.vz=0;
		
		PlayerStartMat.mat11=65536;
		PlayerStartMat.mat12=0;
		PlayerStartMat.mat13=0;
		PlayerStartMat.mat21=0;
		PlayerStartMat.mat22=65536;
		PlayerStartMat.mat23=0;
		PlayerStartMat.mat31=0;
		PlayerStartMat.mat32=0;
		PlayerStartMat.mat33=65536;
	}	
}

void setup_paths(RIFFHANDLE h)
{
	PathArraySize=0;
	PathArray=0;
	
	Chunk * pChunk = h->envd->lookup_single_child("SPECLOBJ");
	if(!pChunk) return;
	
	List<Chunk*> pathlist;
	((Chunk_With_Children*)pChunk)->lookup_child("AVPPATH2",pathlist);

	if(!pathlist.size()) return;
	
	//find the highest path index
	LIF<Chunk*> plif(&pathlist);
	
	for(; !plif.done(); plif.next())
	{
		AVP_Path_Chunk* apc=(AVP_Path_Chunk*) plif();
		PathArraySize=max(PathArraySize,apc->PathID+1);	
	}

	PathArray=(PATHHEADER*)PoolAllocateMem(sizeof(PATHHEADER)*PathArraySize);
	for(int i=0;i<PathArraySize;i++)
	{
		PathArray[i].path_length=0;
		PathArray[i].modules_in_path=0;
	}
	
	for(plif.restart();!plif.done();plif.next())
	{
		AVP_Path_Chunk* apc=(AVP_Path_Chunk*) plif();
		if(!apc->PathLength) continue;

		PATHHEADER* path=&PathArray[apc->PathID];
		
		int length=apc->PathLength;
		if(apc->flags & PathFlag_BackAndForth)
		{
			length=max(length,(length-1)*2);
		}
		
		path->modules_in_path=(AIMODULE**)PoolAllocateMem(sizeof(AIMODULE*)*length);
	
		for(int i=0;i<apc->PathLength;i++)
		{
			Object_Chunk* path_object=h->fc->get_object_by_index(apc->Path[i].module_index);
			if(!path_object)continue;
			if(path_object->program_object_index==-1) continue;
			AIMODULE* path_module=&AIModuleArray[aimodule_indeces[path_object->program_object_index]];				
			
			//if this ai module is the same as the previous one in the path , ignore it
			if(path->path_length)
			{
				if(path_module==path->modules_in_path[path->path_length-1])continue;
			}

			path->modules_in_path[path->path_length]=path_module;
			path->path_length++;
		}

		if(apc->flags & PathFlag_BackAndForth)
		{
			for(int i=path->path_length-2;i>0;i--)
			{
				path->modules_in_path[path->path_length]=path->modules_in_path[i];
				path->path_length++;
			}
		}
	}
	
}

extern "C"
{
extern int SkyColour_R;
extern int SkyColour_G;
extern int SkyColour_B;
};

void set_environment_properties(Environment_Data_Chunk* edc)
{
	GLOBALASSERT(edc);
	AVP_Environment_Settings_Chunk* env_set=GetAVPEnvironmentSettings(edc);
	GLOBALASSERT(env_set);

//set sky colour 
	SkyColour_R=env_set->settings->sky_colour_red;
	SkyColour_G=env_set->settings->sky_colour_green;
	SkyColour_B=env_set->settings->sky_colour_blue;

	LevelHasStars=env_set->settings->stars_in_sky;

//get starting equipment data
	StartingEquipment.marine_jetpack=env_set->settings->marine_jetpack;

	StartingEquipment.predator_pistol=env_set->settings->predator_pistol;
	StartingEquipment.predator_plasmacaster=env_set->settings->predator_plasmacaster;
	StartingEquipment.predator_disc=env_set->settings->predator_disc;
	StartingEquipment.predator_medicomp=env_set->settings->predator_medicomp;
	StartingEquipment.predator_grappling_hook=env_set->settings->predator_grappling_hook;
	StartingEquipment.predator_num_spears=env_set->settings->predator_num_spears;

	if(AvP.Network != I_No_Network)
	{
		StartingEquipment.predator_pistol=netGameData.allowPistol;	
		StartingEquipment.predator_plasmacaster=netGameData.allowPlasmaCaster;	
		StartingEquipment.predator_disc=netGameData.allowDisc;	
		StartingEquipment.predator_medicomp=netGameData.allowMedicomp;	
		
		if(!netGameData.allowSpeargun)
		{
			StartingEquipment.predator_num_spears=0;
		}
	}
	
}



int ConvertObjectIndexToPathIndex(int path_index,int object_index)
{
	if(path_index<0 || path_index>=PathArraySize)
	{
		return -1; //path doesn't exist
	}
	
	Object_Chunk* oc=env_rif->fc->get_object_by_index(object_index);
	if(!oc)
	{
		return -1; //object doesn't exist
	}
	int module_index=oc->program_object_index;
	if(module_index==-1)
	{
		return -1; //object isn't a module
	}
	int aimodule_index=aimodule_indeces[module_index];


	PATHHEADER* path=&PathArray[path_index];
	
	for(int i=0;i<path->path_length;i++)
	{
		if(path->modules_in_path[i]==&AIModuleArray[aimodule_index])
		{
			return i; //found the module in the path
		}
	}

	return -1; //module isn't in path
	
}


void pre_process_shape (RIFFHANDLE, ChunkShape & cs, Chunk_With_Children * shape_chunk, int /*flags*/)
{
	Shape_Merge_Data_Chunk * smdc = 0;

	Chunk * pChunk = shape_chunk->lookup_single_child("SHPMRGDT");
	if (pChunk)
	{
		smdc = (Shape_Merge_Data_Chunk *) pChunk;
	}

	if (smdc)
		merge_polygons_in_chunkshape (cs,smdc);
}

void set_local_scale(RIFFHANDLE h, int /*flags*/)
{
	local_scale = GlobalScale;

	if (h->envd)
	{
		Chunk * pChunk = h->envd->lookup_single_child ("ENVSDSCL");
		if (pChunk)
			local_scale *= ((Environment_Scale_Chunk *)pChunk)->scale;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
//placed hierarchy stuff

extern LOADED_SOUND const * GetSoundFromSoundPath(char* wavname);


extern void add_placed_hierarchy(Placed_Hierarchy_Chunk* phc,const char* fname,const char* hname);

struct LoadedPlacedHierarchy
{
	void load_rif();
	void unload();
	
	const char* file_name;
	const char* hier_name;
	RIFFHANDLE placed_rif;
};

#define NumPlacedHierarchy 3
LoadedPlacedHierarchy PlacedHierarchyArray[NumPlacedHierarchy]=
{
	{ "dropship","dropship",INVALID_RIFFHANDLE },
	{ "pred ship fury","pred ship fury",INVALID_RIFFHANDLE },
	{ "pred ship ob","pred ship ob",INVALID_RIFFHANDLE },
};


void LoadedPlacedHierarchy::load_rif()
{
	if(placed_rif!=INVALID_RIFFHANDLE) return;
	char file_path[100];

	sprintf(file_path,"avp_huds/%s.rif",file_name);
	
	placed_rif=avp_load_rif_non_env(file_path);
	if(placed_rif!=INVALID_RIFFHANDLE)
	{
		#if MaxImageGroups>1
		SetCurrentImageGroup(2); // load into environment image group
		#endif
		copy_rif_data(placed_rif,CCF_IMAGEGROUPSET + CCF_LOAD_AS_HIERARCHY_IF_EXISTS+CCF_DONT_INITIALISE_TEXTURES,0,0);
		unload_rif(placed_rif);
	}
}
void LoadedPlacedHierarchy::unload()
{
	if(placed_rif!=INVALID_RIFFHANDLE)
	{
		avp_undo_rif_load(placed_rif); // destroys copied shapes
	}
	placed_rif=INVALID_RIFFHANDLE;
}



void setup_placed_hierarchies(Environment_Data_Chunk * envd)
{
	GLOBALASSERT(envd);
	Special_Objects_Chunk* soc=(Special_Objects_Chunk*)envd->lookup_single_child("SPECLOBJ");
	if(!soc) return;

	List<Chunk*> chlist;
	soc->lookup_child("PLACHIER",chlist);

	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		Placed_Hierarchy_Chunk* phc=(Placed_Hierarchy_Chunk*)chlif();
		Placed_Hierarchy_Data_Chunk* phdc=phc->get_data_chunk();

		if(phdc->hierarchy_index<0 || phdc->hierarchy_index>=NumPlacedHierarchy) continue;

		//need to store the current sound directory , since it will get altered when loading the 
		//hierarchy rif file.
		char* Temp_Rif_Sound_Directory = Rif_Sound_Directory;
		
		PlacedHierarchyArray[phdc->hierarchy_index].load_rif();	

		//restore the sound directory
		Rif_Sound_Directory = Temp_Rif_Sound_Directory;

		add_placed_hierarchy(phc,PlacedHierarchyArray[phdc->hierarchy_index].file_name,PlacedHierarchyArray[phdc->hierarchy_index].hier_name);

	}	
}

void unload_placed_hierarchies()
{
	for(int i=0;i<NumPlacedHierarchy;i++)
	{
		PlacedHierarchyArray[i].unload();
	}
}




///////////////////////////////////////////////////////////////////////////////
// stuff for handling hierarchies

extern int GetSequenceID(int sequence_type,int sub_sequence);
extern "C" void MulQuat(QUAT *q1,QUAT *q2,QUAT *output);


List <Global_Hierarchy_Store *> Global_Hierarchy_Library;
List<Hierarchy_ID_Time_Pair*> Global_Hierarchy_Store::time_list;

Global_Hierarchy_Store::Global_Hierarchy_Store (RIFFHANDLE h)
{
	GLOBALASSERT (h->envd);
	
	Chunk * pChunk = h->envd->lookup_single_child ("RIFFNAME");
	
	RIF_Name_Chunk * rnc = (RIF_Name_Chunk *) pChunk;
	
	GLOBALASSERT (rnc);
	
	riffname = (char*) PoolAllocateMem(strlen (rnc->rif_name)+1);
	strcpy (riffname, rnc->rif_name);

	rif_hand=h;

	sound_array=0;
	
	//load the sounds used by this file
	List<Chunk*> chlist;
	h->fc->lookup_child("INDSOUND",chlist);

	//find the highest sound index
	int max_index=-1;
	LIF<Chunk*> chlif(&chlist);
	
	for(; !chlif.done(); chlif.next())
	{
		Indexed_Sound_Chunk* isc=(Indexed_Sound_Chunk*)chlif();
		max_index=max(max_index,isc->index);
	}
	//now create a large enough array , and fill it in
   	num_sounds=max_index+1;
	if(max_index>=0)
	{
		//find this rif's sound directory
		Sound_Directory_Chunk* dir_chunk=(Sound_Directory_Chunk*) h->envd->lookup_single_child("SOUNDDIR");
		char wavname[200];
		
		VECTORCH ZeroVector={0,0,0};
		
		sound_array=(HIERARCHY_SOUND*) PoolAllocateMem(sizeof(HIERARCHY_SOUND)*(max_index+1));
		for(int i=0;i<=max_index;i++)
		{
			sound_array[i].sound_loaded=0;
		}
	
		for(chlif.restart();!chlif.done();chlif.next())
		{
			Indexed_Sound_Chunk* isc=(Indexed_Sound_Chunk*)chlif();
			int index=isc->index;

			GLOBALASSERT(sound_array[index].sound_loaded==0);
			
			sound_array[index].s3d.inner_range=(int)(isc->inner_range*local_scale);
			sound_array[index].s3d.outer_range=(int)(isc->outer_range*local_scale);
			sound_array[index].s3d.velocity=ZeroVector;
			sound_array[index].s3d.position=ZeroVector;

			sound_array[index].pitch=isc->pitch;
			sound_array[index].volume=isc->max_volume;
			if(dir_chunk)
			{
				sprintf(wavname,"%s\\%s",dir_chunk->directory,isc->wav_name);
				sound_array[index].sound_loaded=GetSound(wavname);
			}
			else
			{
				sound_array[index].sound_loaded=GetSound(isc->wav_name);
			}
			if(sound_array[index].sound_loaded)
			{
				sound_array[index].sound_index=(SOUNDINDEX)sound_array[index].sound_loaded->sound_num;
			}		
		}
	}
}


Global_Hierarchy_Store::~Global_Hierarchy_Store()
{
	#if !USE_LEVEL_MEMORY_POOL
	if (riffname)
	{
		DeallocateMem(riffname);
	}
	

	while (hierarchy_list.size())
	{
		Hierarchy_Descriptor * hd = hierarchy_list.first_entry();
		
		
		if (hd->hierarchy_name)
			DeallocateMem (hd->hierarchy_name);
		
		if (hd->hierarchy_root)
			delete_section (hd->hierarchy_root);
			
		DeallocateMem (hd);
		
		hierarchy_list.delete_first_entry();
	}
	
	while(alternate_shape_set_list.size())
	{
		Hierarchy_Alternate_Shape_Set* hass=alternate_shape_set_list.first_entry();
		HIERARCHY_SHAPE_REPLACEMENT* hsr=hass->replaced_shapes;
		
		while(hsr->replaced_section_name)
		{
			DeallocateMem (hsr->replaced_section_name);
			hsr++;
		}
		
		DeallocateMem (hass->replaced_shapes);
		DeallocateMem (hass->shape_set_name);
		DeallocateMem (hass);
		alternate_shape_set_list.delete_first_entry();
	}

	if(shape_collections)
	{
		for(int i=0;i<num_shape_collections;i++)
		{
			DeallocateMem (shape_collections[i].replacements);			
		}
		DeallocateMem (shape_collections);
	}
	#else
	while (hierarchy_list.size())
	{
		hierarchy_list.delete_first_entry();
	}
	while(alternate_shape_set_list.size())
	{
		alternate_shape_set_list.delete_first_entry();
	}
	#endif

	//get rid of any loaded sounds
	#if !NEW_DEALLOCATION_ORDER
	if(sound_array)
	{
		for(int i=0;i<num_sounds;i++)	
		{
			if(sound_array[i].sound_loaded)
			{
				LoseSound(sound_array[i].sound_loaded);
			}
		}
		#if !USE_LEVEL_MEMORY_POOL
		DeallocateMem (sound_array);
		#endif
	}
	#endif
}

void Global_Hierarchy_Store::add_hierarchy (List <Object_ShapeNum_Pair *> & osnp_lst, Object_Hierarchy_Chunk * ohc)
{
	List <Object_Hierarchy_Chunk *> ohcl = ohc->list_h_children();
		
	GLOBALASSERT (ohcl.size() == 1);

	
	//check to see if this hierarchy has any sequences
	{
		Object_Hierarchy_Chunk* ohc=(Object_Hierarchy_Chunk*) ohcl.first_entry();
		if(!Get_Object_Animation_All_Sequence_Chunk(ohc))
		{
			List <Object_Animation_Sequence_Chunk *> seql;
			Chunk * pChunk = ohc->lookup_single_child ("OBANSEQS");
			if (pChunk)
			{
				((Object_Animation_Sequences_Chunk *)pChunk)->list_sequences(&seql);
			}
			if(!seql.size()) return;
		}
	}
	

	Hierarchy_Descriptor * hd =(Hierarchy_Descriptor*) PoolAllocateMem(sizeof(Hierarchy_Descriptor));
	
	Object_Hierarchy_Name_Chunk * ohnc = ohc->get_name();
	if (ohnc)
	{
		hd->hierarchy_name = (char*)PoolAllocateMem(strlen (ohnc->hierarchy_name) + 1);
		strcpy (hd->hierarchy_name, ohnc->hierarchy_name);
	}
	else
	{ // throw back from old system, no new hierarchies should be created with no name
		hd->hierarchy_name = (char*) PoolAllocateMem(strlen ("Template") + 1);
		strcpy (hd->hierarchy_name, "Template");
	}
	
	
	build_time_list(ohcl.first_entry());	
	hd->hierarchy_root = build_hierarchy (ohcl.first_entry(),hd->hierarchy_name);
	while(time_list.size())
	{
		delete time_list.first_entry();
		time_list.delete_first_entry();
	}
	
	hd->hierarchy_root->flags |= section_is_master_root;
	
	
	hierarchy_list.add_entry (hd);
	
	Preprocess_HModel (hd->hierarchy_root ,riffname);
}

static List<int> random_marine_texturings;
static List<int> random_civilian_texturings;

void Global_Hierarchy_Store::setup_alternate_shape_sets(List <Object_ShapeNum_Pair *> & osnp_lst, File_Chunk * fc)
{
	List<Chunk*> chlist;
	fc->lookup_child("OBHALTSH",chlist);
	
	LIF<Chunk*> chlif(&chlist);
	for(; !chlif.done(); chlif.next())
	{
		Object_Hierarchy_Alternate_Shape_Set_Chunk* ohassc=(Object_Hierarchy_Alternate_Shape_Set_Chunk*) chlif();

		Hierarchy_Alternate_Shape_Set* hass=(Hierarchy_Alternate_Shape_Set*) PoolAllocateMem(sizeof(Hierarchy_Alternate_Shape_Set));
		
		hass->shape_set_name=(char*) PoolAllocateMem(strlen(ohassc->Shape_Set_Name)+1);
		hass->index=ohassc->Shape_Set_Num;
		hass->num_replaced_shapes=ohassc->Replaced_Shape_List.size();
		strcpy(hass->shape_set_name,ohassc->Shape_Set_Name);

		hass->flags=ohassc->flags;

		hass->replaced_shapes=(HIERARCHY_SHAPE_REPLACEMENT*) PoolAllocateMem(sizeof(HIERARCHY_SHAPE_REPLACEMENT)*(ohassc->Replaced_Shape_List.size()+1));
		
		HIERARCHY_SHAPE_REPLACEMENT* hsr=hass->replaced_shapes;
		for(LIF<Replaced_Shape_Details*> rlif(&ohassc->Replaced_Shape_List);!rlif.done();rlif.next(),hsr++)
		{
			hsr->replaced_section_name=(char*) PoolAllocateMem(strlen(rlif()->old_object_name)+1);
			strcpy(hsr->replaced_section_name,rlif()->old_object_name);

			hsr->replacement_id = 0;
			
			//find the shape num for the new shape
			LIF<Object_ShapeNum_Pair*> olif(&osnp_lst);
			
			for(; !olif.done(); olif.next())
			{
				if(!strcmp(olif()->ob->object_data.o_name,rlif()->new_object_name))
				{
					hsr->replacement_shape=mainshapelist[olif()->sh_num];
					hsr->replacement_shape_index=olif()->sh_num;
					break;
				}
			}
			if(olif.done())
			{
				//alternate shape set removes this object
				hsr->replacement_shape=0;
				hsr->replacement_shape_index=0;
			}
			
		}
		hsr->replaced_section_name=0;
		hsr->replacement_shape=0;
	
		alternate_shape_set_list.add_entry(hass);
	}

	
	num_shape_collections=0;
	shape_collections=0;

	//now set up the shape set collections
	List_Hierarchy_Shape_Set_Collection_Chunk(fc,chlist);
		
	//find the highest index
	int max_index=-1;		
	for(chlif.restart();!chlif.done();chlif.next())
	{
		Hierarchy_Shape_Set_Collection_Chunk* coll=(Hierarchy_Shape_Set_Collection_Chunk*)chlif();
		max_index=max(max_index,coll->Set_Collection_Num);	
	}

	if(max_index==-1) return;

	//allocate array for shape sets and initialise to zero
	num_shape_collections=max_index+1;
	shape_collections=(HIERARCHY_VARIANT_DATA*) PoolAllocateMem(sizeof(HIERARCHY_VARIANT_DATA)*num_shape_collections);
	for(int i=0;i<num_shape_collections;i++)
	{
		shape_collections[i].replacements=0;
		shape_collections[i].voice=0;
		shape_collections[i].female=0;

	}
	//find the collections that can be randomly generated
	List<int>* ran_list=0;
	if(!_stricmp(riffname,"hnpcmarine"))	
	{
		if(random_marine_texturings.size()) ran_list=&random_marine_texturings;
	}
	else if(!_stricmp(riffname,"hnpc_civvie"))
	{
		if(random_civilian_texturings.size()) ran_list=&random_civilian_texturings;
	}

	if(!ran_list || ran_list->contains(1))
		random_shape_colls.add_entry(1);

	//now go through the list of shape set collections again
	Hierarchy_Alternate_Shape_Set* found_sets[20];
	int num_found_sets;
	int num_shapes_to_replace;
	LIF<Hierarchy_Alternate_Shape_Set*> alt_lif(&alternate_shape_set_list);
	for(chlif.restart();!chlif.done();chlif.next())
	{
		Hierarchy_Shape_Set_Collection_Chunk* coll=(Hierarchy_Shape_Set_Collection_Chunk*)chlif();
		GLOBALASSERT(coll->Index_List.size()<20);
		//locate the sets that this collections is using
		num_found_sets=0;
		num_shapes_to_replace=0;
		int combined_flags=0;
			
		for(LIF<int> ind_lif(&coll->Index_List);!ind_lif.done();ind_lif.next())
		{
			for(alt_lif.restart();!alt_lif.done();alt_lif.next())		
			{
				if(ind_lif()==alt_lif()->index)
				{
					found_sets[num_found_sets]=alt_lif();
					num_found_sets++;
					num_shapes_to_replace+=alt_lif()->num_replaced_shapes;
					combined_flags|=alt_lif()->flags;
					break;
				}
			}
		}

		if(num_found_sets)
		{
			HIERARCHY_SHAPE_REPLACEMENT* hsr=(HIERARCHY_SHAPE_REPLACEMENT*) PoolAllocateMem(sizeof(HIERARCHY_SHAPE_REPLACEMENT)*(num_shapes_to_replace+1));
			shape_collections[coll->Set_Collection_Num].replacements=(struct hierarchy_shape_replacement*)hsr;
			
			if(combined_flags & Avp_ShapeSet_Flag_Female)
				shape_collections[coll->Set_Collection_Num].female=1;
			
			shape_collections[coll->Set_Collection_Num].voice=coll->TypeIndex;
			

			int pos=0;
			for(int i=0;i<num_found_sets;i++)
			{
				Hierarchy_Alternate_Shape_Set* hass=found_sets[i];
				for(int j=0;j<hass->num_replaced_shapes;j++)
				{
					//just take a straight copy since the shape set collections won't deallocate the names	
					hsr[pos]=hass->replaced_shapes[j];
					hsr[pos].replacement_id = coll->Set_Collection_Num;
					pos++;
				}
				
			}
			hsr[pos].replaced_section_name=0;
			hsr[pos].replacement_shape=0;
		}

		//see if this collection can appear randomly
		if(!ran_list || ran_list->contains(coll->Set_Collection_Num))
		{
			random_shape_colls.add_entry(coll->Set_Collection_Num);
		}
	}
}

void Global_Hierarchy_Store::delete_section(SECTION * s2d)
{
	#if !USE_LEVEL_MEMORY_POOL
	SECTION ** csp = &s2d->Children[0];
	if (csp)
	{
		while (*csp)
		{
			delete_section (*csp);
			csp ++;
		}
		DeallocateMem ((void *)s2d->Children);
		
	}

	for (int i=0; i<s2d->num_sequences; i++)
	{
		DeallocateMem (s2d->sequence_array[i].first_frame);
	}
	
	DeallocateMem ((void *)s2d->sequence_array);

	if (s2d->Section_Name)
	{
		DeallocateMem ((void *)s2d->Section_Name);
	}

	if (s2d->ShapeName)
	{
		DeallocateMem ((void *)s2d->ShapeName);
	}
	
	DeallocateMem ((void *)s2d);
  	#endif
}


SECTION * Global_Hierarchy_Store::build_hierarchy (Object_Hierarchy_Chunk * ohc,char* hierarchy_name)
{
	// iterative bit
	// should be called with the root node
	// containing an object




	Object_Hierarchy_Data_Chunk * ohdc = ohc->get_data();
	
	GLOBALASSERT (ohdc);
	GLOBALASSERT (ohdc->ob_name);
	
	
	// make a section
	
	SECTION * currsection = (SECTION *)PoolAllocateMem (sizeof (SECTION));

	currsection->flags = 0;
	currsection->ShapeName = 0;
	currsection->Hierarchy_Name=hierarchy_name;	
	currsection->Rif_Name=riffname;
	
	currsection->Section_Name = (char *)PoolAllocateMem (strlen (ohdc->ob_name) + 1);
	strcpy (currsection->Section_Name, ohdc->ob_name);
	
	if (ohdc->object)
	{
		GLOBALASSERT(ohdc->object->program_object_index!=-1);
		currsection->ShapeNum = ohdc->object->program_object_index;
		currsection->Shape = mainshapelist[ohdc->object->program_object_index];
	}
	else
	{
		currsection->ShapeNum = -1;
		currsection->Shape = 0;
	}
	
	Object_Animation_All_Sequence_Chunk* all_seq=Get_Object_Animation_All_Sequence_Chunk(ohc);
	if(all_seq)
	{
		currsection->num_sequences=all_seq->num_sequences;
		currsection->sequence_array = 0;
		if(all_seq->num_sequences)
		{
			currsection->sequence_array = (SEQUENCE *)PoolAllocateMem (sizeof(SEQUENCE) * all_seq->num_sequences);
			SEQUENCE * seqa_p = currsection->sequence_array;
			
			for(int i=0;i<all_seq->num_sequences;i++)
			{
				KEYFRAME_DATA ** next_frame_ptr=&seqa_p->first_frame;
				KEYFRAME_DATA * delta_frame=0;
				
				Object_Animation_Sequence* seq=&all_seq->sequences[i];
							
				seqa_p->sequence_id = GetSequenceID (seq->sequence_number, seq->sub_sequence_number);
				seqa_p->Time = (seq->sequence_time*ONE_FIXED)/1000;
								
				
				KEYFRAME_DATA * kfd=0;
								
				
				for(unsigned int frame_no=0;frame_no<seq->num_frames;)
				{
					Object_Animation_Frame* frame=&seq->frames[frame_no];
					
					int flags=0;
					HIERARCHY_SOUND* sound=0;
					
					//see if there are any flags or sounds on this frame
					flags=frame->flags & HierarchyFrame_FlagMask;
					int sound_index=frame->get_sound_index();
					if(sound_index>0 && sound_index<num_sounds)
					{
						if(sound_array[sound_index].sound_loaded)
						{
							sound=&sound_array[sound_index];
						}
					}

					if(flags || sound)
					{
						//need to use extended keyframe
						KEYFRAME_DATA_EXTENDED* kfd_extended=(KEYFRAME_DATA_EXTENDED*) PoolAllocateMem(sizeof(KEYFRAME_DATA_EXTENDED));
						kfd=(KEYFRAME_DATA*) kfd_extended;

						kfd->frame_has_extended_data=1;
						kfd_extended->sound=sound;
						kfd_extended->flags=flags;

						
					}
					else
					{
						//use standard keyframe
						kfd =(KEYFRAME_DATA*) PoolAllocateMem(sizeof(KEYFRAME_DATA));

						kfd->frame_has_extended_data=0;
					}

					kfd->last_frame=0; //if this is in fact the last frame , then this will be changed later

					//set up previous frames next pointer
					*next_frame_ptr=kfd;
					next_frame_ptr=&kfd->Next_Frame;
										

					if(frame->flags & HierarchyFrameFlag_DeltaFrame)
					{
						GLOBALASSERT(delta_frame==0); //should only be one frame with this flag
						delta_frame=kfd;
					}
										
					VECTORCH offset;
					
					offset.vx = (int)(frame->transform.x * local_scale);
					offset.vy = (int)(frame->transform.y * local_scale);
					offset.vz = (int)(frame->transform.z * local_scale);

					SetKeyFrameOffset(kfd,&offset);
					
					QUAT q;

					q.quatx = (int) -(frame->orientation.x*ONE_FIXED);
					q.quaty = (int) -(frame->orientation.y*ONE_FIXED);
					q.quatz = (int) -(frame->orientation.z*ONE_FIXED);
					q.quatw = (int) (frame->orientation.w*ONE_FIXED);

					CopyIntQuatToShort(&q,&kfd->QOrient);

					
					int this_frame_no = frame->at_frame_no;
									
										
					
					frame_no++;
					
					if (frame_no<seq->num_frames)
					{
						//calculate sequence length , making sure it doesn't overflow an unsigned short
						kfd->Sequence_Length =(unsigned short) min(seq->frames[frame_no].at_frame_no - this_frame_no,65535);
					}
					else
					{
						kfd->Sequence_Length =(unsigned short) min(65536 - this_frame_no,65535);
					}
					
				}
				//sort out some settings for the last frame
				kfd->last_frame=1;
				if(kfd->Sequence_Length<=6)
				{
					//this sequence doesn't loop
					kfd->Next_Frame=kfd;
				}
				else
				{
					//this sequence loops
					kfd->Next_Frame=seqa_p->first_frame;
				}

				seqa_p->last_frame = kfd;

				

				if(delta_frame)
				{
					//this sequence is a delta sequence , so convert all frames to deltas relative
					//to delta_frame
					
					QUAT inverse_delta; 
					
					CopyShortQuatToInt(&delta_frame->QOrient,&inverse_delta);
					
					VECTORCH inverse_offset;
					GetKeyFrameOffset(delta_frame,&inverse_offset);
								
					inverse_delta.quatx = -inverse_delta.quatx;
					inverse_delta.quaty = -inverse_delta.quaty;
					inverse_delta.quatz = -inverse_delta.quatz;
	
					inverse_offset.vx = -inverse_offset.vx;
					inverse_offset.vy = -inverse_offset.vy;
					inverse_offset.vz = -inverse_offset.vz;
			
					kfd = seqa_p->first_frame;
			
					while (1)
					{
						// do deltas
						QUAT q;

						CopyShortQuatToInt(&kfd->QOrient,&q);
						
						MulQuat (&q, &inverse_delta, &q);
						CopyIntQuatToShort(&q,&kfd->QOrient);

						VECTORCH offset;
						GetKeyFrameOffset(kfd,&offset);
						
						offset.vx += inverse_offset.vx;
						offset.vy += inverse_offset.vy;
						offset.vz += inverse_offset.vz;
						
						SetKeyFrameOffset(kfd,&offset);

						if(kfd->last_frame) break;
			
						kfd = kfd->Next_Frame;
					}
					

				}
				
				
				seqa_p ++;
			}
		
		}
	}
	else
	{
		List <Object_Animation_Sequence_Chunk *> seql;
	
		Chunk * pChunk = ohc->lookup_single_child ("OBANSEQS");
	
		if (pChunk)
		{
			((Object_Animation_Sequences_Chunk *)pChunk)->list_sequences(&seql);
		}
	
		currsection->num_sequences = seql.size();
		currsection->sequence_array = 0;

		// fill in sequences


		if (seql.size())
		{
			currsection->sequence_array = (SEQUENCE *)PoolAllocateMem (sizeof(SEQUENCE) * seql.size());
			SEQUENCE * seqa_p = currsection->sequence_array;
			
			for (LIF<Object_Animation_Sequence_Chunk *> seqi(&seql); !seqi.done(); seqi.next())
			{
			
				Object_Animation_Sequence_Header_Chunk * oashc = seqi()->get_header();
				GLOBALASSERT (oashc);
				
				seqa_p->sequence_id = GetSequenceID (oashc->sequence_number, oashc->sub_sequence_number);
				seqa_p->Time = get_time_from_sequence_id(seqa_p->sequence_id);

				List <Object_Animation_Sequence_Frame_Chunk	*> f_list;
				seqi()->get_frames(&f_list);
				
				GLOBALASSERT (f_list.size());
				
				int num_frames=f_list.size();
				Object_Animation_Sequence_Frame_Chunk ** frame_array= new Object_Animation_Sequence_Frame_Chunk*[num_frames];
				
				LIF<Object_Animation_Sequence_Frame_Chunk *> fli(&f_list);
								
				
				for (; !fli.done(); fli.next())
				{
					GLOBALASSERT(fli()->frame_ref_no<num_frames);
					frame_array[fli()->frame_ref_no]=fli();
					
				}
				KEYFRAME_DATA ** next_frame_ptr=&seqa_p->first_frame;
				KEYFRAME_DATA * delta_frame=0;

				KEYFRAME_DATA * kfd=0;
				
				
				for(int frame_no=0;frame_no<num_frames;)
				{
					Object_Animation_Sequence_Frame_Chunk* frame=frame_array[frame_no];
					
					int flags=0;
					HIERARCHY_SOUND* sound=0;
					
					//see if there are any flags or sounds on this frame
					flags=frame->flags & HierarchyFrame_FlagMask;
					int sound_index=frame->get_sound_index();
					if(sound_index>0 && sound_index<num_sounds)
					{
						if(sound_array[sound_index].sound_loaded)
						{
							sound=&sound_array[sound_index];
						}
					}
					
					if(flags || sound)
					{
						//need to use extended keyframe
						KEYFRAME_DATA_EXTENDED* kfd_extended=(KEYFRAME_DATA_EXTENDED*) PoolAllocateMem(sizeof(KEYFRAME_DATA_EXTENDED));
						kfd=(KEYFRAME_DATA*) kfd_extended;

						kfd->frame_has_extended_data=1;
						kfd_extended->sound=sound;
						kfd_extended->flags=flags;
						
					}
					else
					{
						//use standard keyframe
						kfd =(KEYFRAME_DATA*) PoolAllocateMem(sizeof(KEYFRAME_DATA));

						kfd->frame_has_extended_data=0;
		
					}
					kfd->last_frame=0; //if this is in fact the last frame , then this will be changed later

					//set up previous frames next pointer
					*next_frame_ptr=kfd;
					next_frame_ptr=&kfd->Next_Frame;

					if(frame->flags & HierarchyFrameFlag_DeltaFrame)
					{
						GLOBALASSERT(delta_frame==0); //should only be one frame with this flag
						delta_frame=kfd;
					}

					VECTORCH offset;
					
					offset.vx = (int)(frame->transform.x * local_scale);
					offset.vy = (int)(frame->transform.y * local_scale);
					offset.vz = (int)(frame->transform.z * local_scale);

					SetKeyFrameOffset(kfd,&offset);
					
					QUAT q;

					q.quatx = (int) -(frame->orientation.x*ONE_FIXED);
					q.quaty = (int) -(frame->orientation.y*ONE_FIXED);
					q.quatz = (int) -(frame->orientation.z*ONE_FIXED);
					q.quatw = (int) (frame->orientation.w*ONE_FIXED);
					

					CopyIntQuatToShort(&q,&kfd->QOrient);
					
					int this_frame_no = frame->at_frame_no;
									
					frame_no++;
					
					if (frame_no<num_frames)
					{
						//calculate sequence length , making sure it doesn't overflow an unsigned short
						kfd->Sequence_Length =(unsigned short) min(frame_array[frame_no]->at_frame_no - this_frame_no,65535);
					}
					else
					{
						kfd->Sequence_Length =(unsigned short) min(65536 - this_frame_no , 65535);
					}
				}
				//sort out some settings for the last frame
				kfd->last_frame=1;
				if(kfd->Sequence_Length<=6)
				{
					//this sequence doesn't loop
					kfd->Next_Frame=kfd;
				}
				else
				{
					//this sequence loops
					kfd->Next_Frame=seqa_p->first_frame;
				}
				seqa_p->last_frame = kfd;

				delete [] frame_array;
				

				if(delta_frame)
				{
					//this sequence is a delta sequence , so convert all frames to deltas relative
					//to the frame delta_frame

					QUAT inverse_delta; 
					CopyShortQuatToInt(&delta_frame->QOrient,&inverse_delta);
					
					VECTORCH inverse_offset;
					GetKeyFrameOffset(delta_frame,&inverse_offset);
			
			
					inverse_delta.quatx = -inverse_delta.quatx;
					inverse_delta.quaty = -inverse_delta.quaty;
					inverse_delta.quatz = -inverse_delta.quatz;
	
					inverse_offset.vx = -inverse_offset.vx;
					inverse_offset.vy = -inverse_offset.vy;
					inverse_offset.vz = -inverse_offset.vz;
			
					kfd = seqa_p->first_frame;
			
					while (1)
					{
						// do deltas
						QUAT q;

						CopyShortQuatToInt(&kfd->QOrient,&q);
						
						MulQuat (&q, &inverse_delta, &q);
						CopyIntQuatToShort(&q,&kfd->QOrient);

						VECTORCH offset;
						GetKeyFrameOffset(kfd,&offset);
						
						offset.vx += inverse_offset.vx;
						offset.vy += inverse_offset.vy;
						offset.vz += inverse_offset.vz;
						
						SetKeyFrameOffset(kfd,&offset);
			
						if(kfd->last_frame) break;

						kfd = kfd->Next_Frame;
					}

				}
				
				
				seqa_p ++;
			}
		}
	}
	
	currsection->Children = 0;
	
	List <Object_Hierarchy_Chunk *> h_children = ohc->list_h_children();
	
	if (h_children.size())
	{
		currsection->Children = (SECTION **)PoolAllocateMem (sizeof (SECTION *) * (h_children.size()+1));
		
		int i = 0;
		
		for (LIF<Object_Hierarchy_Chunk *> hci(&h_children); !hci.done(); hci.next())
		{
			if ((currsection->Children[i++] = build_hierarchy (hci(),hierarchy_name)) == 0)
				return 0;
		}
		currsection->Children[i] = 0;
	}
	
	
	return(currsection);
	
}

void Global_Hierarchy_Store::build_time_list(Object_Hierarchy_Chunk* ohc)
{
	List <Object_Animation_Sequence_Chunk *> seql;
	Chunk * pChunk = ohc->lookup_single_child ("OBANSEQS");
	if (pChunk)
	{
		((Object_Animation_Sequences_Chunk *)pChunk)->list_sequences(&seql);
	}

	for (LIF<Object_Animation_Sequence_Chunk *> seqi(&seql); !seqi.done(); seqi.next())
	{
		Object_Animation_Sequence_Header_Chunk * oashc = seqi()->get_header();
		GLOBALASSERT (oashc);
		
		int id=GetSequenceID (oashc->sequence_number, oashc->sub_sequence_number);
		int time=(seqi()->get_sequence_time()*ONE_FIXED)/1000;
		if(time)
		{
			Hierarchy_ID_Time_Pair* id_time = new Hierarchy_ID_Time_Pair;
			id_time->id=id;
			id_time->time=time;
			time_list.add_entry(id_time);
		}
	}
}

int Global_Hierarchy_Store::get_time_from_sequence_id(int id)
{
	for(LIF<Hierarchy_ID_Time_Pair*> tlif(&time_list);!tlif.done();tlif.next())
	{
		if(tlif()->id==id)
		{
			return tlif()->time;
		}
	}
	return 0;
}


static BOOL copy_rif_data_as_hierarchy (RIFFHANDLE h, int flags,int progress_start,int progress_interval)
{
 //////////////////////////
 // Sort out local scale //
 //////////////////////////
	int i;
	set_local_scale(h,flags);
	
	Set_Progress_Bar_Position(progress_start);
	
	//SelectGenTexDirectory(ITI_TEXTURE);

	#if 0 //disable the multiple image group stuff
	if(!(flags & CCF_DONT_INITIALISE_TEXTURES))
	{
		InitialiseTextures();
	}
	#endif
	
	/*find this rif's sound directory*/
	Rif_Sound_Directory=0;
	Sound_Directory_Chunk* dir_chunk=(Sound_Directory_Chunk*)h->envd->lookup_single_child("SOUNDDIR");
	if(dir_chunk)
	{
		Rif_Sound_Directory=dir_chunk->directory;
	}


	load_rif_bitmaps(h,flags);

	List <Object_ShapeNum_Pair *> osnp_list;
	List <Object_ShapeNum_Pair *> low_osnp_list;
	List <Object_Chunk *> obl;
	h->fc->list_objects(&obl);
	
 	int NumObjectsToLoad=obl.size(); //for progress_bar
 	int NumObjectsLoaded=0;

	for (LIF<Object_Chunk *> oli(&obl); !oli.done(); oli.next())
	{							  
		if((NumObjectsLoaded &0xf)==0)
		{
			//update bar every 16 objects
			Set_Progress_Bar_Position((int)(progress_start+progress_interval*((.5*NumObjectsLoaded)/NumObjectsToLoad)));
		}
		NumObjectsLoaded++;
		
		
		Shape_Chunk * tmpshp = oli()->get_assoc_shape();
		//ChunkShape cs = tmpshp->shape_data;

		CTM_ReturnType rt_temp = copy_to_mainshapelist(h,tmpshp,flags);
		Object_ShapeNum_Pair * osnp = new Object_ShapeNum_Pair;

		osnp->sh_num = rt_temp.main_list_pos;
		osnp->ob = oli();
		const char* ob_name=osnp->ob->object_data.o_name;
		if(ob_name[0]=='L' && ob_name[1] && ob_name[2]=='#')
		{
			low_osnp_list.add_entry(osnp);
		}
		else
		{
			osnp_list.add_entry (osnp);
		}

		osnp->ob->program_object_index=osnp->sh_num;


		//add the prelighting data to the shape

		Shape_Vertex_Intensities_Chunk * svic = 0;
		
		List<Chunk *> cl;
		osnp->ob->lookup_child ("SHPVTINT",cl);
		
		for (LIF<Chunk *> svici(&cl); !svici.done(); svici.next())
		{
			Shape_Vertex_Intensities_Chunk * temp_svic = (Shape_Vertex_Intensities_Chunk *) svici();
			if (!strncmp(temp_svic->light_set_name, ::light_set_name, 8))
			{
				svic = temp_svic;
				break;
			}
			
		}
		
		if (svic)
		{
		
			mainshapelist[osnp->sh_num]->sh_extraitemdata = (EXTRAITEMDATA *)PoolAllocateMem(12 * svic->num_vertices);
			if (!mainshapelist[osnp->sh_num]->sh_extraitemdata)
			{
				memoryInitialisationFailure = 1;
				return FALSE;
			}					

			for (int vn = 0; vn < svic->num_vertices; vn++) 
			{
				//convert coloured light to a brightness value
				int ir=svic->intensity_array[vn]>>16;
			 	int ig=svic->intensity_array[vn]>>8 &0xff;
			 	int ib=svic->intensity_array[vn] &0xff;
			 	int mag = (int)sqrt((ir*ir+ig*ig+ib*ib)/3.0);
				
			 	mainshapelist[osnp->sh_num]->sh_extraitemdata[vn].EID_VertexI = svic->intensity_array[vn] + (mag<<24);
				
			}
		
			mainshapelist[osnp->sh_num]->shapeflags |= ShapeFlag_PreLit;
		
		}
		
	}
	/*-----------------------------**
	** Load shapes with no objects **
	**-----------------------------*/

	//don't want these to be scaled with the level
	double temp_scale=local_scale;
	local_scale=GlobalScale;

	List<Chunk *> shps;
	h->fc->lookup_child ("REBSHAPE",shps);

	for (LIF<Chunk *> shplst(&shps) ; !shplst.done() ; shplst.next())
	{
 	
 		Shape_Chunk * tmpshp = (Shape_Chunk *)shplst();

		if ( ! tmpshp->list_assoc_objs().size() )
		{
		//	ChunkShape cs = tmpshp->shape_data;
			copy_to_mainshapelist(h,tmpshp,flags);
		}
		
	}
	local_scale=temp_scale;
	
	if(!(flags & CCF_DONT_INITIALISE_TEXTURES))
	{
		//currently on the occasions this flag is used , the textures are shared with the level rif.
		//therefore only clear the fast files if the flag is not set.
		ffclose_almost_all();
	}
	/*-----------------------------**
	** set up hierarchy related stuff **
	**-----------------------------*/
	
	
	Set_Progress_Bar_Position((int)(progress_start+progress_interval*.5));
	//get the distances at which the various detail levels should be used
	int* distance_array=0;
	Hierarchy_Degradation_Distance_Chunk* hddc=(Hierarchy_Degradation_Distance_Chunk*)h->fc->lookup_single_child("HIDEGDIS");
	if(hddc)
	{
		GLOBALASSERT(hddc->num_detail_levels==10);
		distance_array=hddc->distance_array;
	}
	
	//sort out the arrays of low detail shapes
	SHAPEHEADER* low_detail_array[10];
	for(LIF<Object_ShapeNum_Pair *> osnp_lif(&osnp_list);!osnp_lif.done();osnp_lif.next())
	{
		if(!low_osnp_list.size()) break;

		for(i=0;i<10;i++)
		{
			low_detail_array[i]=0;
		}
		int num_detail_level=0;
		
		Object_ShapeNum_Pair* osnp=osnp_lif();
		//find all the low detail shapes for this shape;
		for(LIF<Object_ShapeNum_Pair *> low_osnp_lif(&low_osnp_list);!low_osnp_lif.done();)
		{
			Object_ShapeNum_Pair* low_osnp=low_osnp_lif();
			if(!strcmp(&low_osnp->ob->object_data.o_name[3],osnp->ob->object_data.o_name))
			{
				low_osnp_lif.delete_current();
				int detail=low_osnp->ob->object_data.o_name[1]-'0';
				if(detail>=1 && detail <=9)
				{
					low_detail_array[detail]=mainshapelist[low_osnp->sh_num];
					num_detail_level++;
				}
				delete low_osnp;
			}
			else
			{
				low_osnp_lif.next();
			}
		
		}

		if(num_detail_level)//we have some lower detail shapes
		{
			SHAPEHEADER* main_shape=mainshapelist[osnp->sh_num];
			low_detail_array[0]=main_shape; //detail level 0 is the original shape
			num_detail_level++;

			main_shape->shape_degradation_array=(ADAPTIVE_DEGRADATION_DESC*)PoolAllocateMem(sizeof(ADAPTIVE_DEGRADATION_DESC)*num_detail_level);
			
			ADAPTIVE_DEGRADATION_DESC* deg_ptr=main_shape->shape_degradation_array;
			for(i=9;i>=0;i--) //shapes are entered in ascending order of detail.
			{
				if(low_detail_array[i])
				{
					if(distance_array)
						deg_ptr->distance=distance_array[i];
					else
						deg_ptr->distance=i*10000;
					deg_ptr->shape=low_detail_array[i];

					if (i!=9)
					{
						deg_ptr->shapeCanBeUsedCloseUp = 1;
					}
					else
					{
						deg_ptr->shapeCanBeUsedCloseUp = 0;
					}
					deg_ptr++;
				}
			}

				
		}
		
	}
	//at this point low_osnp_list ought to be empty , but delete the rest anyway
	while(low_osnp_list.size())
	{
		delete low_osnp_list.first_entry();
		low_osnp_list.delete_first_entry();
	}

	
	List <Chunk *> cl;
	h->fc->lookup_child ("OBJCHIER",cl);

	GLOBALASSERT (cl.size());

	Global_Hierarchy_Store * ghs = new Global_Hierarchy_Store (h);

	int NumHierToLoad=cl.size();
	int NumHierLoaded=0;
	
	for (LIF<Chunk *> cli (&cl); !cli.done(); cli.next())
	{
		Set_Progress_Bar_Position((int)(progress_start+progress_interval*(.7+(.2*NumHierLoaded)/NumHierToLoad)));
		NumHierLoaded++;
		
		Object_Hierarchy_Chunk * ohc = (Object_Hierarchy_Chunk *)cli();
		
		ghs->add_hierarchy (osnp_list, ohc);
	}

	ghs->setup_alternate_shape_sets(osnp_list,h->fc);
	
	
	while (osnp_list.size())
	{
		delete osnp_list.first_entry();
		osnp_list.delete_first_entry();
	}

	Global_Hierarchy_Library.add_entry(ghs);

	Set_Progress_Bar_Position((int)(progress_start+progress_interval*.9));

	//reset the sound directory
	Rif_Sound_Directory=0;
	
	return(1);
	

}

///////////////////////////////////////////////////////////////////////////////
// Library management functions

extern "C"
{

SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);

SECTION * GetHierarchyFromLibrary(const char * rif_name)
{
	return (GetNamedHierarchyFromLibrary(rif_name, "Template"));
}

SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name)
{
	for (LIF<Global_Hierarchy_Store *> ghli(&Global_Hierarchy_Library); !ghli.done(); ghli.next())
	{
		if (!_stricmp (ghli()->riffname, rif_name))
		{
			for (LIF<Hierarchy_Descriptor *> hdi (&ghli()->hierarchy_list); !hdi.done(); hdi.next())
			{
				if (!strcmp (hdi()->hierarchy_name, hier_name))
				{
					return(hdi()->hierarchy_root);
				}
			}
			
		}
	}
	return(0);
}

HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeSetFromLibrary(const char* rif_name,const char* shape_set_name)
{
	for (LIF<Global_Hierarchy_Store *> ghli(&Global_Hierarchy_Library); !ghli.done(); ghli.next())
	{
		if (!_stricmp (ghli()->riffname, rif_name))
		{
			for(LIF<Hierarchy_Alternate_Shape_Set*> hlif(&ghli()->alternate_shape_set_list);!hlif.done();hlif.next())
			{
				if(!_stricmp(hlif()->shape_set_name,shape_set_name))
				{
					return hlif()->replaced_shapes;
				}
			}
			return 0;
		}
	}
	return(0);
}
HIERARCHY_VARIANT_DATA* GetHierarchyAlternateShapeSetCollectionFromLibrary(const char* rif_name,int collection_index)
{
	for (LIF<Global_Hierarchy_Store *> ghli(&Global_Hierarchy_Library); !ghli.done(); ghli.next())
	{

		if (!_stricmp (ghli()->riffname, rif_name))
		{
			//found the appropriate hierarchy library
			Global_Hierarchy_Store * hier=ghli();

			if(hier->num_shape_collections==0) return 0;

			if(collection_index<=0) 
			{
				if(hier->random_shape_colls.size()==0) return 0;

				//pick a collection at random.
				int random=FastRandom() % hier->random_shape_colls.size();
				collection_index=hier->random_shape_colls[random];
			}

			collection_index%=hier->num_shape_collections;
			GLOBALASSERT(hier->shape_collections);
			return &hier->shape_collections[collection_index];
		}
	}
	return(0);
}

HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeFromId(const char* rif_name,int replacement_id,char* section_name)
{
	int collection_index = replacement_id;
	if(collection_index == 0 ) return NULL;

	for (LIF<Global_Hierarchy_Store *> ghli(&Global_Hierarchy_Library); !ghli.done(); ghli.next())
	{

		if (!_stricmp (ghli()->riffname, rif_name))
		{
			//found the appropriate hierarchy library
			Global_Hierarchy_Store * hier=ghli();

			if(collection_index<0 || collection_index>=hier->num_shape_collections) return NULL;

			HIERARCHY_VARIANT_DATA* variant = &hier->shape_collections[collection_index];
			HIERARCHY_SHAPE_REPLACEMENT* replacement =(HIERARCHY_SHAPE_REPLACEMENT*) variant->replacements;
			HIERARCHY_SHAPE_REPLACEMENT* returnval = NULL;

			/*
			Search through the list of replacements for the desired shape.
			We need to find the last replacement for a given shape. (One shape may be replaced twice)
			*/

			while(replacement->replaced_section_name)
			{
				if(!strcmp(replacement->replaced_section_name,section_name))
				{
					//found a replacement for this shape
					returnval = replacement;
				}
				replacement++;
			}
			
			return returnval;
		}
	}
	return(0);
}


void EmptyHierarchyLibrary ()
{
	while (Global_Hierarchy_Library.size())
	{
		delete Global_Hierarchy_Library.first_entry();
		Global_Hierarchy_Library.delete_first_entry();
	}
}

void DeleteHierarchyLibraryEntry(RIFFHANDLE h)
{
	for (LIF<Global_Hierarchy_Store *> ghli(&Global_Hierarchy_Library); !ghli.done(); ghli.next())
	{
		if(ghli()->rif_hand==h)
		{
			delete ghli();
			ghli.delete_current();
			break;
		}
	}
}


};
///////////////////////////////////////////////////////////////////////////////


extern void SetupFragmentType(Fragment_Type_Chunk* ftc);

struct Adjacent_AIModule_EP
{
	VECTORCH entry_point;
	int aimodule_index;
	unsigned int alien_only :1;
};

// copies all shapes and objects etc
BOOL copy_rif_data (RIFFHANDLE h, int flags,int progress_start,int progress_interval)
{
	/*close all fastfiles so we don't end up with too many open at once*/
	if(!(flags & CCF_DONT_INITIALISE_TEXTURES))
	{
		//currently on the occasions this flag is used , the textures are shared with the level rif.
		//therefore only clear the fast files if the flag is not set.
		ffclose_almost_all();
	}

	#if 0 //disable the multiple image group stuff
	if (!(flags & CCF_IMAGEGROUPSET))
	{
		GLOBALASSERT(flags & CCF_ENVIRONMENT); //image group should be set for everything else
		#ifdef MaxImageGroups
		#if MaxImageGroups > 2
		SetCurrentImageGroup(flags & CCF_ENVIRONMENT ? 2 : 0);
		#else
		if (flags & CCF_ENVIRONMENT)
			GLOBALASSERT(0=="Requires MaxImageGroups to be > 2 (system.h)");
		#endif
		#else
		if (flags & CCF_ENVIRONMENT)
			GLOBALASSERT(0=="Requires MaxImageGroups to be defined > 2 (system.h)");
		#endif
	}
	#endif
	
	if (INVALID_RIFFHANDLE == h || !h->fc) return(FALSE);
	
	if (flags & CCF_LOAD_AS_HIERARCHY_IF_EXISTS)
	{
		if (h->fc->count_children("OBJCHIER"))
			return(copy_rif_data_as_hierarchy (h,flags,progress_start,progress_interval));
	}

 //////////////////////////
 // Sort out local scale //
 //////////////////////////

	set_local_scale(h,flags);

	/*find this rif's sound directory*/
	Rif_Sound_Directory=0;
	Sound_Directory_Chunk* dir_chunk=(Sound_Directory_Chunk*)h->envd->lookup_single_child("SOUNDDIR");
	if(dir_chunk)
	{
		Rif_Sound_Directory=dir_chunk->directory;
	}

/*---------------**
** Load Textures **
**---------------*/

	//SelectGenTexDirectory(ITI_TEXTURE);

	#if 0 //disable the multiple image group stuff
	if(!(flags & CCF_DONT_INITIALISE_TEXTURES))
	{
		InitialiseTextures();
	}
	#endif
	
	if (flags & CCF_ENVIRONMENT)
	{
		load_rif_bitmaps(h,flags);
		
		// Load in the palette, tlt, and set up lookup table for coloured polys

		PaletteMapTable = 0;
		
		if (h->envd)
		{
			set_quantization_event (h,flags);

			copy_rif_palette (h,flags);
			
			copy_rif_tlt (h,flags);

			get_rif_palette_remap_table (h,flags);

		}

		/*find the default sound settings */
		float env_reverb=-1;
		int env_sound_type=0;
		Environment_Acoustics_Chunk* env_ac=(Environment_Acoustics_Chunk*)h->envd->lookup_single_child("ENVACOUS");
		if(env_ac)
		{
			if(env_ac->reverb<=1) env_reverb=env_ac->reverb;
			env_sound_type=env_ac->env_index;
		}
		
	
	/*-------------------------------------**
	** Load in shapes to the mainshapelist **
	**-------------------------------------*/

	/*-----------------------------------**
	** Set up arrays of modules          **
	**                                   **
	** There should be two arrays,       **
	** one array of modules and a        **
	** pointer to an array of modules    **
	**-----------------------------------*/
			
		
		//count the modules
		int num_modules=0;
		{
			List<Object_Chunk*> object_list;
			h->fc->list_objects(&object_list);
			for(LIF<Object_Chunk*> oblif(&object_list);!oblif.done();oblif.next())
			{
				if(!(oblif()->get_header()->flags & OBJECT_FLAG_PLACED_OBJECT))
				{
					num_modules++;
				}
			}
		
		}
		// One module per object at the moment

		MainScene.sm_module = (MODULE *) PoolAllocateMem (sizeof(MODULE) * (num_modules + 3));
		if (!MainScene.sm_module)
		{
			memoryInitialisationFailure = 1;
			return FALSE;
		}
		
		MainScene.sm_marray = (MODULE **) PoolAllocateMem (sizeof(MODULE) * (num_modules + 3));
		if (!MainScene.sm_marray)
		{
			memoryInitialisationFailure = 1;
			return FALSE;
		}

		// putting an infinite module at the beginning - followed by a term
		// by cheating

		MODULE * sm_module_start = MainScene.sm_module;
		MODULE ** sm_marray_start = MainScene.sm_marray;

		MainScene.sm_module += 2;
		MainScene.sm_marray += 1;
		
		int i;
		for (i=0; i<num_modules; i++)
		{
			MainScene.sm_module[i] = Empty_Module;
			MainScene.sm_marray[i] = &MainScene.sm_module[i];

			MainScene.sm_module[i].m_sound_reverb=env_reverb;
			MainScene.sm_module[i].m_sound_env_index=env_sound_type;

		}

		MainScene.sm_module[i] = Term_Module;

		AIModuleArraySize=0;

		// we'll set the rest up later !!!
		
	/*-------------------------------------**
	** Load in shapes to the mainshapelist **
	**-------------------------------------*/
		o_chunk_array = new Object_Chunk * [num_modules];
		aimodule_indeces=new int[num_modules];

		int mod_pos = 0;


		List<Shape_Chunk*> shape_list;
		List<Object_Chunk*> object_list;
		h->fc->list_shapes(&shape_list);
		
		int NumShapesToLoad=shape_list.size(); //for progress_bar
		int NumShapesLoaded=0;
		
		for(LIF<Shape_Chunk*> shplif(&shape_list);!shplif.done();shplif.next())
		{
			
			if((NumShapesLoaded & 0xf)==0)
			{
				//update bar every 16 objects
				Set_Progress_Bar_Position((int)(progress_start+progress_interval*((.6*NumShapesLoaded)/NumShapesToLoad)));
			}
			
			NumShapesLoaded++;
			
			object_list=shplif()->list_assoc_objs();
			if(object_list.size())
			{
				if(shplif()->get_header()->flags & SHAPE_FLAG_EXTERNALFILE)	
				{
					//only create one copy of the shape for imported objects
					
					#if SupportMorphing && LOAD_MORPH_SHAPES
					db_logf3(("Copying shape to shape list"));
					CTM_ReturnType rt_temp = copy_to_mainshapelist(h,shplif(),flags,0);
					int start_shape_no = rt_temp.start_list_pos;
					int list_pos = rt_temp.main_list_pos;
					db_logf3(("Shape copied to %d",list_pos));
					#else
					int list_pos = copy_to_mainshapelist(h,shplif(),flags,&ob->object_data);
					int start_shape_no = list_pos;
					#endif

					int AnimationShape=-1;
					if (shplif()->lookup_single_child("TEXTANIM"))
					{
						AnimationShape=list_pos;
					}
					
					for(LIF<Object_Chunk*> oblif(&object_list);!oblif.done();oblif.next())
					{
						Object_Chunk* ob=oblif();	
						if (ob->get_header()->flags & OBJECT_FLAG_PLACED_OBJECT)
						{
							deal_with_placed_object(ob, start_shape_no, AnimationShape);
						}
						else 
						{
							GLOBALASSERT(0=="Shouldn't be any modules using imported shapes");
						}
					}

				}
				else
				{
					//create one shape per object
					for(LIF<Object_Chunk*> oblif(&object_list);!oblif.done();oblif.next())
					{
						Object_Chunk* ob=oblif();
						#if SupportMorphing && LOAD_MORPH_SHAPES
						db_logf3(("Copying shape for object %s",ob->object_data.o_name));
						CTM_ReturnType rt_temp = copy_to_mainshapelist(h,shplif(),flags,&ob->object_data);
						int start_shape_no = rt_temp.start_list_pos;
						int list_pos = rt_temp.main_list_pos;
						db_logf3(("Shape copied to %d",list_pos));
						MORPHCTRL * mc = rt_temp.mc;
						#else
						int list_pos = copy_to_mainshapelist(h,shplif(),flags,&ob->object_data);
						int start_shape_no = list_pos;
						#endif
				
						//see if object has prelighting data
						Shape_Vertex_Intensities_Chunk * svic = 0;
				
						List<Chunk *> cl;
						ob->lookup_child ("SHPVTINT",cl);
				
						for (LIF<Chunk *> svici(&cl); !svici.done(); svici.next())
						{
							Shape_Vertex_Intensities_Chunk * temp_svic = (Shape_Vertex_Intensities_Chunk *) svici();
							if (!strncmp(temp_svic->light_set_name, ::light_set_name, 8))
							{
								svic = temp_svic;
								break;
							}
							
						}
				
						if (svic)
						{
							//this object has prelighting , so set up extra item data
							mainshapelist[list_pos]->sh_extraitemdata = (EXTRAITEMDATA *)PoolAllocateMem(12 * svic->num_vertices);
							if (!mainshapelist[list_pos]->sh_extraitemdata)
							{
								memoryInitialisationFailure = 1;
								return FALSE;
							}					

							for (int vn = 0; vn < svic->num_vertices; vn++) 
							{
								//convert coloured light to a brightness value
								int ir=svic->intensity_array[vn]>>16;
							 	int ig=svic->intensity_array[vn]>>8 &0xff;
							 	int ib=svic->intensity_array[vn] &0xff;
							 	int mag =(int)sqrt((ir*ir+ig*ig+ib*ib)/3.0);
								
							 	mainshapelist[list_pos]->sh_extraitemdata[vn].EID_VertexI = svic->intensity_array[vn] + (mag<<24);
								
							}
				
							mainshapelist[list_pos]->shapeflags |= ShapeFlag_PreLit;
				
						}
						#if 1
						int AnimationShape=-1;
						if (shplif()->lookup_single_child("TEXTANIM"))
						{
							AnimationShape=list_pos;
						}
						#endif
				
						if (ob->get_header()->flags & OBJECT_FLAG_PLACED_OBJECT)
						{
							
							deal_with_placed_object(ob, start_shape_no, AnimationShape);
						}
						else 
						{
							
							copy_to_module (ob, mod_pos, start_shape_no);
							
							int shape2 = -1;
							if (mc)
							{
								if(mc->ObMorphHeader)
								{
									if(mc->ObMorphHeader->mph_frames)
									{
										shape2 = mc->ObMorphHeader->mph_frames[0].mf_shape2;
										DeallocateMem (mc->ObMorphHeader->mph_frames);
									}	
									DeallocateMem(mc->ObMorphHeader);
								}
								DeallocateMem (mc);
								mc = 0;
							}
							deal_with_module_object (ob, start_shape_no, AnimationShape, shape2, &MainScene.sm_module[mod_pos]);
							
						   	Object_Module_Data_Chunk* omdc=(Object_Module_Data_Chunk*)ob->lookup_single_child("MODULEDT");
						   	if(omdc)
							{
						   		if(!omdc->lookup_single_child("AIMODSLA"))
						   			AIModuleArraySize++;
							}
							else
			   					AIModuleArraySize++;
							o_chunk_array[mod_pos] = ob; //for finding object_chunk from module
							ob->program_object_index=mod_pos; //for finding module from object_chunk
							aimodule_indeces[mod_pos]=-1;
							mod_pos ++;
						}
					}
				}
			}
			else
			{
				//shape has no objects , load it , but don't scale it
				double temp_scale=local_scale;
				local_scale=GlobalScale;
				
				copy_to_mainshapelist(h,shplif(),flags);
				
				local_scale=temp_scale;
			}
		}
		GLOBALASSERT(num_modules==mod_pos);
	/*--------------**
	** Module Stuff **
	**--------------*/

		if (flags & CCF_ENVIRONMENT)
		{
			Set_Progress_Bar_Position((int)(progress_start+progress_interval*.6));
		}
		MainScene.sm_module[mod_pos] = Term_Module;
		MainScene.sm_marray[mod_pos] = 0;

		AIModuleArraySize++;
		AIModuleArray=(AIMODULE*)PoolAllocateMem(sizeof(AIMODULE)*AIModuleArraySize);
			
		AIModuleArray[0].m_link_ptrs=0;
		AIModuleArray[0].m_module_ptrs=0;
		AIModuleArray[0].m_waypoints=0;
		AIModuleArray[0].m_index=0;

		
		List<Adjacent_AIModule_EP*>* entry_points=new List<Adjacent_AIModule_EP*>[AIModuleArraySize];
		
		int ai_mod_pos=1;
		
		//setup aimodules and the aimodule conversion array
		for(i=0;i<mod_pos;i++)
		{
	   		Object_Module_Data_Chunk * omdc =(Object_Module_Data_Chunk*) o_chunk_array[i]->lookup_single_child("MODULEDT");
			AI_Module_Master_Chunk* ammc=0;
			if(omdc)
			{
				if(omdc->lookup_single_child("AIMODSLA"))continue;
				ammc=(AI_Module_Master_Chunk*)omdc->lookup_single_child("AIMODMAS");
				
			}

			AIMODULE* aim=&AIModuleArray[ai_mod_pos];
			aim->m_index=ai_mod_pos;
			aim->m_world=o_chunk_array[i]->object_data.location*local_scale;

			aim->m_link_ptrs=0;
			if(ammc)
			{
				aim->m_module_ptrs=(MODULE**)PoolAllocateMem(sizeof(MODULE*)*(2+ammc->ModuleList.size()));
				aim->m_module_ptrs[0]=&MainScene.sm_module[i];
				MainScene.sm_module[i].m_aimodule=aim;
				aimodule_indeces[i]=ai_mod_pos;
				
				int pos=1;
				for(LIF<Object_Chunk*> modlif(&ammc->ModuleList);!modlif.done();modlif.next())
				{
				  	for(int obj=0;obj<mod_pos;obj++)
				  	{
						if(o_chunk_array[obj]==modlif())
						{
							aim->m_module_ptrs[pos++]=&MainScene.sm_module[obj];
							MainScene.sm_module[obj].m_aimodule=aim;
							aimodule_indeces[obj]=ai_mod_pos;
							break;
						}

					}
					
				}
				aim->m_module_ptrs[pos++]=0;
			}
			else
			{
				aim->m_module_ptrs=(MODULE**)PoolAllocateMem(2*sizeof(MODULE*));
				
				aim->m_module_ptrs[0]=&MainScene.sm_module[i];
				MainScene.sm_module[i].m_aimodule=aim;
				aimodule_indeces[i]=ai_mod_pos;
				
				aim->m_module_ptrs[1]=0;
			}
			
			ai_mod_pos++;
		}
		
		FALLP_EntryPoints = (FARENTRYPOINTSHEADER *)AllocateMem(AIModuleArraySize*sizeof(FARENTRYPOINTSHEADER));		
		if(!FALLP_EntryPoints) 
		{
			memoryInitialisationFailure = 1;
			return FALSE;
		}
		for(i=0;i<AIModuleArraySize;i++)
		{
			FALLP_EntryPoints[i].entryPointsList=0;
			FALLP_EntryPoints[i].numEntryPoints=0;
		}

		if (flags & CCF_ENVIRONMENT)
		{
			Set_Progress_Bar_Position((int)(progress_start+progress_interval*.7));
		}
		
		for (i=0; i<mod_pos; i++)
		{
		
			Object_Module_Data_Chunk * omdc = 0;
			Adjacent_Module_Entry_Points_Chunk * amc = 0;
			VModule_Array_Chunk * vmac = 0;
			Module_Flag_Chunk * mfc=0;
			Module_Waypoint_Chunk* mwc=0;
			
			//fill in module position and extents
			{
				MODULE* mod=&MainScene.sm_module[i];
				Object_Chunk* oc=o_chunk_array[i];
				Shape_Chunk* sc=oc->get_assoc_shape();
				
				mod->m_world=oc->object_data.location*local_scale;
				
				//extents calculated in this way so that if there are no gaps in the extents when stored as doubles ,
				//there shouldn't be any gaps after they have been converted to ints.
				VECTORCH world_min=(oc->object_data.location+sc->shape_data.min)*local_scale;
				VECTORCH world_max=(oc->object_data.location+sc->shape_data.max)*local_scale;

				mod->m_maxx=world_max.vx-mod->m_world.vx;
				mod->m_maxy=world_max.vy-mod->m_world.vy;
				mod->m_maxz=world_max.vz-mod->m_world.vz;

				mod->m_minx=world_min.vx-mod->m_world.vx;
				mod->m_miny=world_min.vy-mod->m_world.vy;
				mod->m_minz=world_min.vz-mod->m_world.vz;
			}
			
			Chunk * pChunk = o_chunk_array[i]->lookup_single_child ("MODULEDT");
			if (pChunk)
			{
				omdc = (Object_Module_Data_Chunk *) pChunk;
			}

			if (omdc)
			{
				pChunk = omdc->lookup_single_child("VMDARRAY");

				if (pChunk)
				{
					vmac = (VModule_Array_Chunk *) pChunk;
				}
				
				
				amc=(Adjacent_Module_Entry_Points_Chunk*) omdc->lookup_single_child("ADJMDLEP");

				if(omdc->lookup_single_child("ADJMDLST"))
				{
					GLOBALASSERT(0=="Please load this file into module adjacency so that entry points can be calculated.");
				}

				pChunk = omdc->lookup_single_child("MODFLAGS");
				if(pChunk)
				{
					mfc = (Module_Flag_Chunk*)pChunk;
				}

				pChunk = omdc->lookup_single_child("WAYPOINT");
				if(pChunk)
				{
					mwc = (Module_Waypoint_Chunk*)pChunk;
				}

				/*see if this module has its own sound settings*/
				Module_Acoustics_Chunk* mod_ac=(Module_Acoustics_Chunk*)omdc->lookup_single_child("MODACOUS");
				if(mod_ac)
				{
					if(mod_ac->env_index>=0)
					{
						MainScene.sm_module[i].m_sound_env_index=mod_ac->env_index;
					}
					if(mod_ac->reverb>=0)
					{
						if(mod_ac->reverb>1)
							MainScene.sm_module[i].m_sound_reverb=-1;//reverb depends on distance (I think)
						else
							MainScene.sm_module[i].m_sound_reverb=mod_ac->reverb;
					}
				}

			}

			//Deal with module linking
			if (vmac)
			{
				if(KeepMainRifFile)
				{
					//use standard memory allocation , since this may need to be deallocated when updating module linking
					MainScene.sm_module[i].m_vmptr = (VMODULE *)AllocateMem(sizeof(VMODULE) * (vmac->num_array_items+1));
				}
				else
				{
					MainScene.sm_module[i].m_vmptr = (VMODULE *)PoolAllocateMem(sizeof(VMODULE) * (vmac->num_array_items+1));
				}
				if (!MainScene.sm_module[i].m_vmptr)
				{
					memoryInitialisationFailure = 1;
					return FALSE;
				}	

				int vmac_no = 0;
				int vmod_no = 0;
				while (vmac_no < vmac->num_array_items)
				{
					Object_Chunk* linked_module=h->fc->get_object_by_index(vmac->vmod_array[vmac_no].object_index);
					
					if (linked_module && linked_module->program_object_index!=-1)
					{
						
						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_type = vmtype_vmodule;
						*((int *)MainScene.sm_module[i].m_vmptr[vmod_no].vmod_name) = vmac_no;
						if (vmac->vmod_array[vmac_no].branch_no)
						{
							MainScene.sm_module[i].m_vmptr[vmod_no].vmod_instr = vmodi_bra_vc;

							#if (StandardStrategyAndCollisions || IntermediateSSACM)
							MainScene.sm_module[j].m_mapptr->MapStrategy = StrategyI_DoorPROX;
							#endif //(StandardStrategyAndCollisions || IntermediateSSACM)
						}
						else
						{
							MainScene.sm_module[i].m_vmptr[vmod_no].vmod_instr = vmodi_null;
						}

						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_data.vmodidata = vmac->vmod_array[vmac_no].branch_no;

						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_mref.mref_ptr = &MainScene.sm_module[linked_module->program_object_index];
						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_dir.vx = 0;
						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_dir.vy = 0;
						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_dir.vz = 0;

						MainScene.sm_module[i].m_vmptr[vmod_no].vmod_angle = 0;
		   				MainScene.sm_module[i].m_vmptr[vmod_no].vmod_flags = vmac->vmod_array[vmac_no].flags;
						vmod_no ++;
					}

					vmac_no ++;

				}
				MainScene.sm_module[i].m_vmptr[vmod_no].vmod_type = vmtype_term;
				*((int *)MainScene.sm_module[i].m_vmptr[vmod_no].vmod_name) = vmac_no;
			}
			else
			{
				MainScene.sm_module[i].m_vmptr = (VMODULE *)PoolAllocateMem(sizeof(VMODULE) * (2));
				if (!MainScene.sm_module[i].m_vmptr)
				{
					memoryInitialisationFailure = 1;
					return FALSE;
				}	

				MainScene.sm_module[i].m_vmptr[0].vmod_type = vmtype_vmodule;
				*((int *)MainScene.sm_module[i].m_vmptr[0].vmod_name) = 0;
				MainScene.sm_module[i].m_vmptr[0].vmod_instr = vmodi_null;
				MainScene.sm_module[i].m_vmptr[0].vmod_data.vmodidata = 0;

				MainScene.sm_module[i].m_vmptr[0].vmod_mref.mref_ptr = &MainScene.sm_module[i];
				MainScene.sm_module[i].m_vmptr[0].vmod_dir.vx = 0;
				MainScene.sm_module[i].m_vmptr[0].vmod_dir.vy = 0;
				MainScene.sm_module[i].m_vmptr[0].vmod_dir.vz = 0;

				MainScene.sm_module[i].m_vmptr[0].vmod_angle = 0;
		   		MainScene.sm_module[i].m_vmptr[0].vmod_flags = 0;
				
				MainScene.sm_module[i].m_vmptr[1].vmod_type = vmtype_term;
				*((int *)MainScene.sm_module[i].m_vmptr[1].vmod_name) = 1;
			}
			MainScene.sm_module[i].m_link_ptrs=0;

			if(mfc)
			{
				MainScene.sm_module[i].m_flags|=mfc->Flags;
			}

			AI_Module_Master_Chunk* ammc=0;
			if(omdc)
			{
				if(omdc->lookup_single_child("AIMODSLA"))continue;
				ammc=(AI_Module_Master_Chunk*)omdc->lookup_single_child("AIMODMAS");
			}
			
			int this_ai_module_index=aimodule_indeces[i];
			AIMODULE* aim=&AIModuleArray[this_ai_module_index];

			
			//build list of adjacent ai_modules
			List<int> adjacent_aimodule_list; 
			
			if (amc)
			{
				
				for (LIF<Adjacent_Module> ami(&amc->adjacent_modules_list); !ami.done(); ami.next())
				{
					Object_Chunk* adjacent_module=h->fc->get_object_by_index(ami().object_index);
					
					if (adjacent_module && adjacent_module->program_object_index!=-1)
					{
						int adj_ai_module=aimodule_indeces[adjacent_module->program_object_index];
						if(adj_ai_module!=this_ai_module_index)
						{
							//make sure not already in list
							LIF<int> adjlif(&adjacent_aimodule_list);
							for(; !adjlif.done();adjlif.next())
							{
								if(adjlif()==adj_ai_module) break;
							}
							if(adjlif.done())
							{
								if(ami().flags & AdjacentModuleFlag_InsideAdjacentModule)
								{
									continue;	
								}
								if(ami().flags & AdjacentModuleFlag_AdjacentModuleInsideMe)
								{
									continue;	
								
								}
								Adjacent_AIModule_EP* ad_aim=new Adjacent_AIModule_EP;
								ad_aim->entry_point=ami().entry_point*local_scale;
								ad_aim->aimodule_index=this_ai_module_index;
								ad_aim->alien_only=(ami().flags & AdjacentModuleFlag_Vertical)!=0;

								entry_points[adj_ai_module].add_entry(ad_aim);
								adjacent_aimodule_list.add_entry(adj_ai_module);

							}
							
						}
					}
					
				}


			}

			if(ammc)
			{
				for(LIF<Object_Chunk*> oblif(&ammc->ModuleList);!oblif.done();oblif.next())
				{
					Object_Module_Data_Chunk* omdc2=(Object_Module_Data_Chunk*)oblif()->lookup_single_child("MODULEDT");
					if(!omdc2)continue;
					amc=(Adjacent_Module_Entry_Points_Chunk*)omdc2->lookup_single_child("ADJMDLEP");
					if(amc)
					{
						for (LIF<Adjacent_Module> ami(&amc->adjacent_modules_list); !ami.done(); ami.next())
						{
							Object_Chunk* adjacent_module=h->fc->get_object_by_index(ami().object_index);
							
							if (adjacent_module && adjacent_module->program_object_index!=-1)
							{
								int adj_ai_module=aimodule_indeces[adjacent_module->program_object_index];
								if(adj_ai_module!=this_ai_module_index)
								{
									//make sure not already in list
									LIF<int> adjlif(&adjacent_aimodule_list);
									for(; !adjlif.done();adjlif.next())
									{
										if(adjlif()==adj_ai_module) break;
									}
									if(adjlif.done())
									{
										if(ami().flags & AdjacentModuleFlag_InsideAdjacentModule)
										{
											continue;	
										}
										if(ami().flags & AdjacentModuleFlag_AdjacentModuleInsideMe)
										{
											continue;	
										
										}
										
										Adjacent_AIModule_EP* ad_aim=new Adjacent_AIModule_EP;
										ad_aim->entry_point=(ami().entry_point)*local_scale;
										ad_aim->aimodule_index=this_ai_module_index;
										ad_aim->alien_only=(ami().flags & AdjacentModuleFlag_Vertical)!=0;

										entry_points[adj_ai_module].add_entry(ad_aim);
										adjacent_aimodule_list.add_entry(adj_ai_module);
									}
									
								}
							}
							
						}
					}
				}	
			}

			//setup adjacent modules
			int adj_pos=0;
			aim->m_link_ptrs=(AIMODULE**)PoolAllocateMem(sizeof(AIMODULE*)*(1+adjacent_aimodule_list.size()));
			if(adjacent_aimodule_list.size())
			{
			
				while(adjacent_aimodule_list.size())
				{
					aim->m_link_ptrs[adj_pos]=&AIModuleArray[adjacent_aimodule_list.first_entry()];
					adj_pos++;
					adjacent_aimodule_list.delete_first_entry();
					
				}
			}
			aim->m_link_ptrs[adj_pos]=0;


			
			
			//Deal with waypoints
			if(mwc && mwc->NumAlienWaypoints)
			{
				WAYPOINT_HEADER* wh=(WAYPOINT_HEADER*)PoolAllocateMem(sizeof(WAYPOINT_HEADER));
				aim->m_waypoints=wh;
			   	MainScene.sm_module[i].m_waypoints=wh;//temporary
				
				wh->num_waypoints=mwc->NumAlienWaypoints;

				wh->first_waypoint=(WAYPOINT_VOLUME*)PoolAllocateMem(sizeof(WAYPOINT_VOLUME)*wh->num_waypoints);
				for(int j=0;j<wh->num_waypoints;j++)
				{
					WAYPOINT_VOLUME* wv=&wh->first_waypoint[j];
					ChunkWaypoint* cw=&mwc->AlienWaypoints[j];

					VECTORCH ObCentre=o_chunk_array[i]->object_data.location*local_scale;
					wv->centre=cw->centre*local_scale;
					wv->min_extents=cw->min*local_scale;
					SubVector(&wv->centre,&wv->min_extents);
					wv->max_extents=cw->max*local_scale;
					SubVector(&wv->centre,&wv->max_extents);
					wv->max_extents.vx--;
					wv->max_extents.vy--;
					wv->max_extents.vz--;
					SubVector(&ObCentre,&wv->centre);
					wv->flags=cw->flags;
					wv->num_links=cw->NumWPLinks;
					if(wv->num_links)
						wv->first_link=(WAYPOINT_LINK*)PoolAllocateMem(sizeof(WAYPOINT_LINK)*wv->num_links);
					else
						wv->first_link=0;
					for(int k=0;k<wv->num_links;k++)
					{
						wv->first_link[k].link_flags=cw->WayLinks[k].flags;
						wv->first_link[k].link_target_index=cw->WayLinks[k].index;
					}
					wv->workspace=0;
					wv->weighting=5;
				}

			}
			else
			{
				aim->m_waypoints=0;
			}
			
						
		}

		if (flags & CCF_ENVIRONMENT)
		{
			Set_Progress_Bar_Position((int)(progress_start+progress_interval*.8));
		}
		//setup entry points
		for(i=0;i<AIModuleArraySize;i++)
		{
			int num_ep=entry_points[i].size();
			if(num_ep)
			{
				FALLP_EntryPoints[i].entryPointsList=(FARENTRYPOINT*)AllocateMem(sizeof(FARENTRYPOINT)*num_ep);
				FALLP_EntryPoints[i].numEntryPoints=num_ep;

				int adj_pos=0;
				while(entry_points[i].size())
				{
					Adjacent_AIModule_EP* ad_aim=entry_points[i].first_entry();

					FALLP_EntryPoints[i].entryPointsList[adj_pos].position=ad_aim->entry_point;
					SubVector(&AIModuleArray[i].m_world,&FALLP_EntryPoints[i].entryPointsList[adj_pos].position);
					FALLP_EntryPoints[i].entryPointsList[adj_pos].donorIndex=ad_aim->aimodule_index;
					FALLP_EntryPoints[i].entryPointsList[adj_pos].alien_only=ad_aim->alien_only;

					#if 0
					//test
					{
						VECTORCH loc=FALLP_EntryPoints[i].entryPointsList[adj_pos].position;

						MODULE* mod=AIModuleArray[i].m_module_ptrs[0];
						SHAPEHEADER* shp=mainshapelist[mod->m_mapptr->MapShape];

						if(loc.vx< shp->shapeminx||
						   loc.vy< shp->shapeminy||
						   loc.vz< shp->shapeminz||
						   loc.vx> shp->shapemaxx||
						   loc.vy> shp->shapemaxy||
						   loc.vz> shp->shapemaxz)
						{
							GLOBALASSERT(0=="The entry points probably need to be recalculated");
						}
					}
					#endif
					adj_pos++;
					entry_points[i].delete_first_entry();
					delete ad_aim;
				}
			}
		}
		delete [] entry_points;


		// putting an infinite module at the beginning - followed by a term
		// and setting the arrays back to point at them !!
		
		MainScene.sm_module = sm_module_start;
		MainScene.sm_marray = sm_marray_start;
		
		MainScene.sm_module[0] = Empty_Module;
		MainScene.sm_module[0].m_flags |= m_flag_infinite;
		MainScene.sm_module[0].m_vptr.mref_ptr = &MainScene.sm_module[2];
		*((int *)MainScene.sm_module[0].m_name) = 1;
		MainScene.sm_module[0].m_vmptr = (VMODULE *)PoolAllocateMem(sizeof(VMODULE) * (mod_pos+1));
		if (!MainScene.sm_module[0].m_vmptr)
		{
			memoryInitialisationFailure = 1;
			return FALSE;
		}
		
		for (i=0; i<mod_pos; i++)
		{
			MainScene.sm_module[0].m_vmptr[i].vmod_type = vmtype_vmodule;
			*((int *)MainScene.sm_module[0].m_vmptr[i].vmod_name) = i + 1000;
			MainScene.sm_module[0].m_vmptr[i].vmod_instr = vmodi_null;
			MainScene.sm_module[0].m_vmptr[i].vmod_data.vmodidata = 0;
			MainScene.sm_module[0].m_vmptr[i].vmod_mref.mref_ptr = &MainScene.sm_module[i+2];
			MainScene.sm_module[0].m_vmptr[i].vmod_dir.vx = 0;
			MainScene.sm_module[0].m_vmptr[i].vmod_dir.vy = 0;
			MainScene.sm_module[0].m_vmptr[i].vmod_dir.vz = 0;
			MainScene.sm_module[0].m_vmptr[i].vmod_angle = 0;
			MainScene.sm_module[0].m_vmptr[i].vmod_flags = 0;
			
		  //	MainScene.sm_module[i]->m_flags|=m_flag_gotptrs;
		}

		MainScene.sm_module[0].m_vmptr[i].vmod_type = vmtype_term;
		*((int *)MainScene.sm_module[0].m_vmptr[i].vmod_name) = i + 1000;
		
		MainScene.sm_marray[0] = &MainScene.sm_module[0];
		
		
		MainScene.sm_module[1] = Term_Module;
		*((int *)MainScene.sm_module[1].m_name) = 2;

		setup_start_position(h);
		
		//setup ai paths
		setup_paths(h);
		
		setup_placed_hierarchies(h->envd);
		
		
		//alien power cables
		setup_cables(h->envd);

		//particle generators
		setup_particle_generators(h->envd);

		// External lifts / airlocks
		DealWithExternalObjectStategies (h->envd);
		
		setup_sounds (h->envd);

		setup_preplaced_decals(h->fc,h->envd);

	//	create_strategies_from_list(); //now called later
		
		//set sky colour , and other envionmental properties
		set_environment_properties(h->envd);


		
		Set_Progress_Bar_Position((int)(progress_start+progress_interval*.9));
	}
	else
	{
		//not the environment rif , so just load shapes without objects	
		//don't want these to be scaled with the level
		double temp_scale=local_scale;
		local_scale=GlobalScale;

		List<Chunk *> shps;
		h->fc->lookup_child ("REBSHAPE",shps);

		for (LIF<Chunk *> shplst(&shps) ; !shplst.done() ; shplst.next())
		{
			
 			Shape_Chunk * tmpshp = (Shape_Chunk *)shplst();

			if ( ! tmpshp->list_assoc_objs().size() )
			{
		  //		ChunkShape cs = tmpshp->shape_data;
				copy_to_mainshapelist(h,tmpshp,flags);
			}
			
		}
		local_scale=temp_scale;
	}

/*--------------**
** Load sprites **
**--------------*/
	//SelectGenTexDirectory(ITI_SPRITE);

	Chunk * pChunk = h->fc->lookup_single_child ("RSPRITES");
	if (pChunk)
	{
		List<Chunk *> sprs;
		((Chunk_With_Children *)pChunk)->lookup_child("SPRIHEAD",sprs);
		for (LIF<Chunk *> sprlst(&sprs) ; !sprlst.done() ; sprlst.next())
		{
			Sprite_Header_Chunk* shc=(Sprite_Header_Chunk*)sprlst();

			RIF_Name_Chunk * rnc = 0;

			pChunk = shc->lookup_single_child("RIFFNAME");
			if (pChunk)
			{
				rnc = (RIF_Name_Chunk *) pChunk;
			}
			else
			{
				GLOBALASSERT(0=="RIF name not found in sprite");
			}
			
			
			copy_sprite_to_mainshapelist(h,shc,flags);
		}
		
	}

	//setup shape fragments;
	{
		List<Chunk*> chlist;
		h->envd->lookup_child("FRAGTYPE",chlist);
		for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
		{
			SetupFragmentType((Fragment_Type_Chunk*)chlif());
		}
	}

	if (flags & CCF_ENVIRONMENT)
	{
		SetUpRunTimeLights();
		// setup_generators ,and load hierarchies
		//take copy of scale since it is changed by InitNPCs
		setup_generators (h->envd);

		delete[] o_chunk_array;
		delete[] aimodule_indeces;

		//find the marine/civvie types that can be randomly generated
		List<Chunk*> rlist;
		h->envd->lookup_child("RANTEXID",rlist);
		for(LIF<Chunk*> rlif(&rlist);!rlif.done();rlif.next())
		{
			Random_Texture_ID_Chunk* ran_chunk=(Random_Texture_ID_Chunk*)rlif();
			if(!_stricmp(ran_chunk->name,"hnpcmarine"))
			{
				random_marine_texturings=ran_chunk->random_types;
			}
			else if(!_stricmp(ran_chunk->name,"hnpc_civvie"))
			{
				random_civilian_texturings=ran_chunk->random_types;
			}
		}
		
		double env_scale=local_scale;
		InitNPCs(h);
		local_scale=env_scale;
	
		while (random_marine_texturings.size())random_marine_texturings.delete_first_entry();
		while (random_civilian_texturings.size())random_civilian_texturings.delete_first_entry();		
	}

	//reset sound diretory pointer
	Rif_Sound_Directory=0;

	return TRUE;
}

// hook to load a bitmap - so you can load them from test directories, etc. should return tex index
int load_rif_bitmap (char const * fname, BMPN_Flags flags)
{
	return CL_LoadImageOnce
		(
			fname,
			(ScanDrawDirectDraw == ScanDrawMode ? LIO_CHIMAGE : LIO_D3DTEXTURE)
			|(flags & ChunkBMPFlag_IFF ? LIO_RELATIVEPATH : LIO_RIFFPATH)
			|(flags & ChunkBMPFlag_UsesTransparency ? LIO_TRANSPARENT : LIO_NONTRANSP)
			|(flags & (cl_pszGameMode ?
				ChunkBMPFlag_RequireGameMipMaps
				: ChunkBMPFlag_RequireToolsMipMaps)
				? LIO_LOADMIPMAPS : LIO_NOMIPMAPS)
			|LIO_RESTORABLE
		);
}

////////////////////////////////////////////////////////////////////////
// Functions which do not operate on RIFFHANDLEs and may become obsolete
////////////////////////////////////////////////////////////////////////

// these functions work on the current rif; they only remain for historical reasons
// copies all shapes and objects etc
BOOL copy_chunks_from_environment(int flags)
{
	return copy_rif_data(current_rif_handle,flags,0,0);
}

/////////////////////////////////////////////
// Functions for handling the main shape list
/////////////////////////////////////////////

static SHAPEHEADER null_shape; // static to ensure it is initialized to 0s

#define FREE_SHAPE (&null_shape)

int start_of_loaded_shapes;
static int msl_term_pos;
static int first_free_pos = GLS_NOTINLIST;

// reserves the next avaialbe position in the main shape list and returns it
int GetMSLPos(void)
{
	int pos;
	
	if (GLS_NOTINLIST == first_free_pos)
		first_free_pos = msl_term_pos = start_of_loaded_shapes = load_precompiled_shapes();

	for (pos = first_free_pos; pos < msl_term_pos && FREE_SHAPE != mainshapelist[pos]; ++pos)
		;

	first_free_pos = pos+1;
	if (pos >= msl_term_pos)
	{
		msl_term_pos = first_free_pos;
		if(msl_term_pos>=maxshapes)
		{
			//alocate another 50 slots on the mainshapelist
			mainshapelist=(SHAPEHEADER**)realloc(mainshapelist,sizeof(SHAPEHEADER*)*(maxshapes+50));
			LOCALASSERT(mainshapelist);
			if(!mainshapelist)
			{
	   			ReleaseDirect3D();
				exit(0x74363); 
			}
			for(int i=maxshapes;i<maxshapes+50;i++)
			{
				mainshapelist[i]=0;
			}
			maxshapes+=50;

		}
		mainshapelist[msl_term_pos]=0;
	}

	return pos;
}

// frees a position in the main shape list
void FreeMSLPos(int pos)
{
	mainshapelist[pos] = FREE_SHAPE;

	if (first_free_pos > pos) first_free_pos = pos;

	if (pos+1 == msl_term_pos)
	{
		while (msl_term_pos > 0 && FREE_SHAPE == mainshapelist[msl_term_pos-1])
		{
			mainshapelist[--msl_term_pos] = 0;
		}
	}
}

////////////////////////////////////////////////
// Functions retrieving data about loaded shapes
////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Initializing, deallocating of shapes, mainly hooks for project specific fns
//////////////////////////////////////////////////////////////////////////////

// hook to perhaps scale the uv coordinates - should return new value
int ProcessUVCoord(RIFFHANDLE,UVCOORDTYPE uvct,int uv_value,int /*image_num*/)
{
//	return (int)(uv_value*GetUVScale(UVC_SPRITE_U==uvct||UVC_SPRITE_V==uvct ? ITI_SPRITE : ITI_TEXTURE));
	return uv_value;
}


// perform initial post processing on shape just after loading
// note that the copy named shape functions will not call this
void post_process_shape (SHAPEHEADER * shp)
{
	for (int i=0; i<shp->numitems; i++)
	{
		switch (shp->items[i][0])
		{
			case I_Polygon:
				if (!(shp->items[i][2] & iflag_nolight))
					shp->items[i][0] = I_GouraudPolygon;
				break;
			case I_3dTexturedPolygon:
			case I_2dTexturedPolygon:
			case I_Gouraud3dTexturedPolygon:
			case I_Gouraud2dTexturedPolygon:
				if (shp->shapeflags & ShapeFlag_Sprite)
				{
					shp->items[i][0] = I_2dTexturedPolygon;
					shp->items[i][2] &= ~iflag_tx2dor3d;
				}
				else
				{
					shp->items[i][0] = I_Gouraud3dTexturedPolygon;
					shp->items[i][2] |= iflag_tx2dor3d;
				}
		}
		shp->items[i][2] |= iflag_gsort_ptest | iflag_linear_s;
		//shp->items[i][2] &= ~iflag_transparent; // this causes _translucency_ on direct 3d
		//shp->items[i][2] &= ~iflag_drawtx3das2d;
		
		#if SupportZBuffering
		if (ZBufferOn==ZBufferMode)
		{
			switch (shp->items[i][0])
			{
				case I_Polygon:
					shp->items[i][0] = I_ZB_Polygon;
					break;
				case I_GouraudPolygon:
					shp->items[i][0] = I_ZB_GouraudPolygon;
					break;
				case I_PhongPolygon:
					shp->items[i][0] = I_ZB_PhongPolygon;
					break;
				case I_2dTexturedPolygon:
					shp->items[i][0] = I_ZB_2dTexturedPolygon;
					break;
				case I_Gouraud2dTexturedPolygon:
					shp->items[i][0] = I_ZB_Gouraud2dTexturedPolygon;
					break;
				case I_3dTexturedPolygon:
					shp->items[i][0] = I_ZB_3dTexturedPolygon;
					break;
				case I_Gouraud3dTexturedPolygon:
					shp->items[i][0] = I_ZB_Gouraud3dTexturedPolygon;
					break;
			}
		}
		#endif
	}

	shp->shapeflags |= ShapeFlag_AugZ | ShapeFlag_AugZ_Lite;
}

// your function could perform any extra tidying up you need
void DeallocateLoadedShapeheader(SHAPEHEADER * shp)
{
	#if !NEW_DEALLOCATION_ORDER
	DeallocateRifLoadedShapeheader(shp);
	#endif
}


void DeallocateModules()
{
	#if !USE_LEVEL_MEMORY_POOL
	MODULE ** m_arrayPtr = MainScene.sm_marray;

	while (*m_arrayPtr)
	{
		List<Light_Chunk *> lights_for_this_module;
		
		MODULE * this_mod = *m_arrayPtr++;
		
		if(this_mod->m_mapptr) DeallocateMem(this_mod->m_mapptr);
		this_mod->m_mapptr=0;
		if(this_mod->name) DeallocateMem(this_mod->name);
		this_mod->name=0;
		if(this_mod->m_vmptr) DeallocateMem(this_mod->m_vmptr);
		this_mod->m_vmptr=0;
		if(this_mod->m_lightarray) DeallocateMem(this_mod->m_lightarray);
		this_mod->m_lightarray=0;
	}
	if(MainScene.sm_module)DeallocateMem(MainScene.sm_module);
	if(MainScene.sm_marray)DeallocateMem(MainScene.sm_marray);
	#endif
	MainScene.sm_module=0;
	MainScene.sm_marray=0;


	#if !USE_LEVEL_MEMORY_POOL
	for(int i=0;i<AIModuleArraySize;i++)
	{
		AIMODULE* aim=&AIModuleArray[i];
		if(aim->m_link_ptrs) DeallocateMem(aim->m_link_ptrs);
		if(aim->m_module_ptrs) DeallocateMem(aim->m_module_ptrs);
		if(aim->m_waypoints)
		{
			WAYPOINT_HEADER* wh=aim->m_waypoints;
			for(int j=0;j<wh->num_waypoints;j++)
			{
				WAYPOINT_VOLUME* wv=&wh->first_waypoint[j];
				if(wv->first_link)DeallocateMem(wv->first_link);	
			}
			if(wh->first_waypoint)DeallocateMem(wh->first_waypoint);
			DeallocateMem(wh);
  		}
	}
	DeallocateMem(AIModuleArray);
	#endif
	AIModuleArray=0;

	//delete any paths
	#if !USE_LEVEL_MEMORY_POOL
	if(PathArray)
	{
		for(int i=0;i<PathArraySize;i++)
		{
			if(PathArray[i].modules_in_path)
			{
				DeallocateMem(PathArray[i].modules_in_path);
			}
		}
		DeallocateMem(PathArray);
	}
	#endif
	
	PathArray=0;
	PathArraySize=0;

	//may as well unload hierarchies for placed objects here as well
	unload_placed_hierarchies();

	//and get rid of the strategy lists
	deallocate_behaviour_list();
}

void avp_undo_rif_load(RIFFHANDLE h)
{
	DeleteHierarchyLibraryEntry(h);
   	undo_rif_load(h);
}

RIFFHANDLE avp_load_rif (const char * fname)
{
	//see if there is a local copy of the rif file
	FILE* rifFile = OpenGameFile(fname, FILEMODE_READONLY, FILETYPE_PERM);

/* TODO: Let's find a better method */
	if (!rifFile && AvpCDPath)
	{
		//try and load rif file from cd instead
		char RifName[200];
		sprintf(RifName, "%s%s", AvpCDPath, fname);
		return load_rif(RifName);

	}
	if (rifFile)
		fclose(rifFile);
	return load_rif(fname); 
}

RIFFHANDLE avp_load_rif_non_env (const char * fname)
{
	//see if there is a local copy of the rif file
	FILE* rifFile = OpenGameFile(fname, FILEMODE_READONLY, FILETYPE_PERM);
	
/* TODO: Let's find a better method */	
	if (!rifFile && AvpCDPath)
	{
		//try and load rif file from cd instead
		char RifName[200];
		sprintf(RifName, "%s%s", AvpCDPath, fname);
		return load_rif_non_env(RifName);

	}
	if (rifFile)
		fclose(rifFile);
	
	return load_rif_non_env(fname); 
}



#if debug
extern "C"{
 extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
}
void LoadModuleData()
{
 	GLOBALASSERT(env_rif);

/* TODO: dir separator */
 	HANDLE file = CreateFile ("avp_rifs/module.bbb", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 
 					FILE_FLAG_RANDOM_ACCESS, 0);
	unsigned long byteswritten;
	WriteFile(file,&Global_VDB_Ptr->VDB_World,sizeof(VECTORCH),&byteswritten,0);
	WriteFile(file,&Global_VDB_Ptr->VDB_Mat,sizeof(MATRIXCH),&byteswritten,0);
 	
 	CloseHandle(file);

/* TODO: dir separator */
 	file = CreateFile ("avp_rifs/module.aaa", GENERIC_READ, 0, 0, OPEN_EXISTING, 
 					FILE_FLAG_RANDOM_ACCESS, 0);

	if(file==INVALID_HANDLE_VALUE) return;

	if(!env_rif->fc)
	{
	 	CloseHandle(file);
		NewOnScreenMessage("MODULE UPDATING REQUIRES -KEEPRIF OPTION.");
		return;
	}

	int file_size=GetFileSize(file,0);
	GLOBALASSERT((file_size % 4)==0);
	int pos=0;
	unsigned long bytesread;
	{
		char name[60];
		ReadFile(file,name,60,&bytesread,0);
		
		int i=0;
		char* name1=&name[0];
		while(name[i])
		{
			if(name[i]=='\\' || name[i]==':' /* || name[i]=='/' */)
			{
				name1=&name[i+1];
			}
			i++;
		}
		i=0;
		char* name2=&env_rif->fc->filename[0];
		while(env_rif->fc->filename[i])
		{
			if(env_rif->fc->filename[i]=='\\' || env_rif->fc->filename[i]==':')
			{
				name2=&env_rif->fc->filename[i+1];
			}
			i++;
		}

		if(_stricmp(name1,name2))
		{
			CloseHandle(file);
			DeleteFile("avp_rifs\\module.aaa");
			return;
		}

	}
	pos+=60;


	while(pos<file_size)
	{
		int obj_index;
		ReadFile(file,&obj_index,4,&bytesread,0);
		pos+=4;

		Object_Chunk* obj=env_rif->fc->get_object_by_index(obj_index);
		GLOBALASSERT(obj);
		GLOBALASSERT(obj->program_object_index!=-1);
		MODULE* this_mod=&MainScene.sm_module[obj->program_object_index+2];

		int numlinks;
		ReadFile(file,&numlinks,4,&bytesread,0);
		pos+=4;

		if(!numlinks) continue;

		if(this_mod->m_vmptr)
			DeallocateMem(this_mod->m_vmptr);

		this_mod->m_vmptr = (VMODULE *)AllocateMem(sizeof(VMODULE) * (numlinks+1));
	

		int vmac_no = 0;
		int vmod_no = 0;
		while (vmac_no < numlinks)
		{
			int linked_index;
			int branch_no;
			ReadFile(file,&linked_index,4,&bytesread,0);
			ReadFile(file,&branch_no,4,&bytesread,0);
			pos+=8;
			
			Object_Chunk* linked_module=env_rif->fc->get_object_by_index(linked_index);
			
			GLOBALASSERT(linked_module);
			if (linked_module && linked_module->program_object_index!=-1)
			{
				
				this_mod->m_vmptr[vmod_no].vmod_type = vmtype_vmodule;
				*((int *)this_mod->m_vmptr[vmod_no].vmod_name) = vmac_no;
				if (branch_no)
				{
					this_mod->m_vmptr[vmod_no].vmod_instr = vmodi_bra_vc;
				}
				else
				{
					this_mod->m_vmptr[vmod_no].vmod_instr = vmodi_null;
				}

				this_mod->m_vmptr[vmod_no].vmod_data.vmodidata = branch_no;


				this_mod->m_vmptr[vmod_no].vmod_mref.mref_ptr = &MainScene.sm_module[linked_module->program_object_index+2];
				this_mod->m_vmptr[vmod_no].vmod_dir.vx = 0;
				this_mod->m_vmptr[vmod_no].vmod_dir.vy = 0;
				this_mod->m_vmptr[vmod_no].vmod_dir.vz = 0;

				this_mod->m_vmptr[vmod_no].vmod_angle = 0;
		 		this_mod->m_vmptr[vmod_no].vmod_flags = 0;
				vmod_no ++;
			}

			vmac_no ++;

		}
		for(int j=0;j<vmac_no;j++)
		{
			if(this_mod->m_vmptr[j].vmod_data.vmodidata)
			{
				int k;
				
				for(k=j+1;k<vmod_no;k++)
				{
					if(*((int *)this_mod->m_vmptr[k].vmod_name)>=this_mod->m_vmptr[j].vmod_data.vmodidata)
					{
						this_mod->m_vmptr[j].vmod_data.vmodidata_ptr=&this_mod->m_vmptr[k];
						break;
					}
				}
				if(k==vmod_no)
				{
					this_mod->m_vmptr[j].vmod_data.vmodidata_ptr=&this_mod->m_vmptr[k];
				}
			}
		}
		this_mod->m_vmptr[vmod_no].vmod_type = vmtype_term;
		*((int *)this_mod->m_vmptr[vmod_no].vmod_name) = vmac_no;
	}
	
	CloseHandle(file);
	DeleteFile("avp_rifs\\module.aaa");
}
#endif






void setup_preplaced_decals(File_Chunk* fc,Environment_Data_Chunk* edc)
{
	NumFixedDecals=0;
	CurrentFixedDecalIndex=0;

	GLOBALASSERT(edc);
	GLOBALASSERT(fc);

	Special_Objects_Chunk* soc=(Special_Objects_Chunk*) edc->lookup_single_child("SPECLOBJ");
	if(!soc) return;

	AVP_Decal_Chunk* decal_chunk=(AVP_Decal_Chunk*) soc->lookup_single_child("AVPDECAL");
	if(!decal_chunk) return;

	GLOBALASSERT(decal_chunk->decals);

	NumFixedDecals=0;

	
	for(int i=0;i<decal_chunk->num_decals;i++)
	{
		FIXED_DECAL* fd=&FixedDecalStorage[NumFixedDecals];
		AVP_Decal * ad=&decal_chunk->decals[i];

		//find the decal's module
		Object_Chunk* module=fc->get_object_by_index(ad->object_index) ;
		if(!module) continue; //module not found
		if(module->program_object_index==-1) continue;
		
		fd->DecalID=(DECAL_ID)ad->DecalID;
		for(int j=0;j<4;j++)
		{
			fd->Vertices[j].vx=ad->Vertices[j].x;
			fd->Vertices[j].vy=ad->Vertices[j].y;
			fd->Vertices[j].vz=ad->Vertices[j].z;
		}

		fd->UOffset=ad->UOffset;
		fd->ModuleIndex=module->program_object_index+2;

		NumFixedDecals++;
	}
	
	
	CurrentFixedDecalIndex=NumFixedDecals;

}

int get_object_index_from_module_index(List<Object_Chunk*>& ob_list,int index)
{
	for(LIF<Object_Chunk*> ob_lif(&ob_list);!ob_lif.done();ob_lif.next())
	{
		if(ob_lif()->program_object_index==index)
		{
			return ob_lif()->object_data.index_num;
		}
	}
	return -1;
}


static BOOL WarnedAboutDiskSpace=FALSE;
static void MakeBackupFile(File_Chunk* fc)
{
	unsigned long spc,bps,numclust,total;
	if(GetDiskFreeSpace(0,&spc,&bps,&numclust,&total))
	{
		unsigned int freespace=spc*bps*numclust;
		if(freespace<40000000)
		{
			if(!WarnedAboutDiskSpace)
			{
				WarnedAboutDiskSpace=TRUE;
				NewOnScreenMessage("LESS THAN 40MB FREE DISC SPACE");
				NewOnScreenMessage("NO BACKUP WILL BE MADE");
			}
			return;
		}
	}
	WarnedAboutDiskSpace=FALSE;

	CreateDirectory("avp_rifs\\Backup",0);
	int length=strlen(fc->filename);
	int pos=length;
	while(pos>=0 && fc->filename[pos]!='\\')pos--;
	char* filename=&fc->filename[pos+1];
	length=strlen(filename)-4;

	char* Name1=new char[length+30];
	char* Name2=new char[length+30];
	strncpy(Name1,"avp_rifs\\Backup\\",16);
	strncpy(&Name1[16],filename,length);
	length+=16;
	strncpy(Name2,Name1,length);
	strncpy(&Name1[length],"B0.rif",7);
	strncpy(&Name2[length],"B1.rif",7);
	
	DeleteFile(Name1);
	
	for (int i=0;i<9;i++)
	{
		Name1[length+1]=i+'0';
		Name2[length+1]=i+'1';
		MoveFile(Name2,Name1);
	}
	Name2[length+1]='9';
	CopyFile(fc->filename,Name2,FALSE);

	delete [] Name1;
	delete [] Name2;
}
extern "C"
{


void save_preplaced_decals()
{
	
	GLOBALASSERT(env_rif);
	if(!env_rif->fc)
	{
		NewOnScreenMessage("CAN'T SAVE DECALS. USE -KEEPRIF");
		return;
	}
	
	{
		
		DWORD attributes = GetFileAttributes(env_rif->fc->filename);
		if (0xffffffff!=attributes)
		{
			if (attributes & FILE_ATTRIBUTE_READONLY)
			{
				NewOnScreenMessage("CAN'T SAVE DECALS.");
				NewOnScreenMessage("FILE IS READ ONLY");
				return;
			}
		}
	}
	MakeBackupFile(env_rif->fc);

	Environment_Data_Chunk* edc = env_rif->envd;
	GLOBALASSERT(edc);

	Special_Objects_Chunk* soc=(Special_Objects_Chunk*) edc->lookup_single_child("SPECLOBJ");
	if(!soc) soc=new Special_Objects_Chunk(edc);
	
	//delete the old decal chunk
	delete soc->lookup_single_child("AVPDECAL");

	if(NumFixedDecals)
	{
		List<Object_Chunk*> ob_list;
		env_rif->fc->list_objects(&ob_list);
				
		
		AVP_Decal_Chunk* decal_chunk=new AVP_Decal_Chunk(soc,NumFixedDecals);
		for(int i=0;i<decal_chunk->num_decals;i++)
		{
			FIXED_DECAL* fd=&FixedDecalStorage[i];
			AVP_Decal * ad=&decal_chunk->decals[i];

			ad->DecalID=(int)fd->DecalID;
			for(int j=0;j<4;j++)
			{
				ad->Vertices[j].x=fd->Vertices[j].vx;
				ad->Vertices[j].y=fd->Vertices[j].vy;
				ad->Vertices[j].z=fd->Vertices[j].vz;

			}

			ad->UOffset=fd->UOffset;

			ad->object_index=get_object_index_from_module_index(ob_list,fd->ModuleIndex-2);
		}

	}

	if(!env_rif->fc->update_file())
	{
		NewOnScreenMessage("ERROR SAVING DECALS!!!!!!");
	}
	else
	{
		NewOnScreenMessage("DECALS SAVED.");
	
	}
}


void check_preplaced_decal_modules()
{
/*-------------------------------------------------------------------------------**
** 	go through the list of decals and make sure their module indeces are correct **
**-------------------------------------------------------------------------------*/

	for(int index=0;index<NumFixedDecals;index++)
	{
		FIXED_DECAL* fd=&FixedDecalStorage[index];

		MODULE* mod=MainScene.sm_marray[fd->ModuleIndex];
		mod=ModuleFromPosition(&fd->Vertices[0],mod);

		if(mod)
		{
			fd->ModuleIndex=mod->m_index;	
		}
	}
}

};


extern void DeallocateAllFragments();
extern void LoseAllNonCommonSounds();
extern void deallocate_behaviour_list();
extern void PurgeMSLShapeList();

extern "C"
{
void DeallocateSoundsAndPoolAllocatedMemory()
{
	deallocate_behaviour_list();

	ClearMemoryPool();

	LoseAllNonCommonSounds();
	
	DeallocateAllFragments();

	DeallocateAllImages();

	PurgeMSLShapeList();

}
};
