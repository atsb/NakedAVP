#ifndef _projload_hpp
#define _projload_hpp 1

#include "chnkload.h"
#include "hmodel.h"



#ifdef __cplusplus

#include "hierchnk.hpp"

class File_Chunk;

// project specifc structure contained in rif handle
struct Project_RifHandle {};

struct Object_ShapeNum_Pair
{
	int sh_num;
	Object_Chunk * ob;
};

struct Hierarchy_ID_Time_Pair
{
	int id;
	int time;
};


struct Hierarchy_Alternate_Shape_Set
{
	char* shape_set_name;
	int index;
	int num_replaced_shapes;
	int flags;
	//terminator has 0 for name and replacement_shape
	HIERARCHY_SHAPE_REPLACEMENT* replaced_shapes;	
};

struct Hierarchy_Descriptor
{
	// this is null if the sequence is a template
	// or contains the name of the SECTION that it
	// attachs to
	
	SECTION * hierarchy_root;
	
	char * hierarchy_name;

	
};

extern "C"
{
struct hierarchy_variant_data;
};

class Global_Hierarchy_Store
{

public:

	// in a vain hope
	Global_Hierarchy_Store (RIFFHANDLE h);
	
	~Global_Hierarchy_Store();
	
	void add_hierarchy (List <Object_ShapeNum_Pair *> & osnp_lst, Object_Hierarchy_Chunk * ohc);
	void setup_alternate_shape_sets (List <Object_ShapeNum_Pair *> & osnp_lst, File_Chunk * fc);
	
	List <Hierarchy_Descriptor *> hierarchy_list;
	
	List<Hierarchy_Alternate_Shape_Set*> alternate_shape_set_list;

	int num_shape_collections;
	struct hierarchy_variant_data* shape_collections;
	List<int> random_shape_colls;


	RIFFHANDLE rif_hand;
	
	char * riffname;

	private:
	
	int num_sounds;
	HIERARCHY_SOUND* sound_array;
		
	static List<Hierarchy_ID_Time_Pair*> time_list;

	int get_time_from_sequence_id(int id);
	void build_time_list(Object_Hierarchy_Chunk* ohc);
	
	SECTION * build_hierarchy (Object_Hierarchy_Chunk * ohc,char* hierarchy_name);
	
	void delete_section(SECTION *);
	
	
};

extern "C" {

#endif

#define LOAD_MORPH_SHAPES 1 // you can compile out the code that copies morph data

#define CL_SUPPORT_ALTTAB 0 // textures and surfaces loaded with CL_LoadImageOnce with LIO_RESTORABLE set will be added to ALT+TAB lists
#define CL_SUPPORT_FASTFILE 1 // AvP uses fastfiles (but if the gfx aren't in them, it'll try the actual files
#ifdef AVP_DEBUG_VERSION
#define CL_SUPPORTONLY_FASTFILE 0 // for release, milestones, CDs, demos, may want this to be non-zero
#else
//allow loading from outside of fastfiles to help with custom levels
#define CL_SUPPORTONLY_FASTFILE 0 // for release, milestones, CDs, demos, may want this to be non-zero
#endif
// project specific copy chunks flags
#define CCF_ENVIRONMENT   0x00000001
#define CCF_IMAGEGROUPSET 0x00000002
#define CCF_LOAD_AS_HIERARCHY_IF_EXISTS	0x00000004
#define CCF_DONT_INITIALISE_TEXTURES 0x00000008

extern void setup_shading_tables(void);
extern void SetUpRunTimeLights ();

// copies all shapes and objects etc
extern BOOL copy_chunks_from_environment(int flags);

// set the local_scale
extern void set_local_scale(RIFFHANDLE, int flags);
#define SetLocalScale(h,f) set_local_scale(h,f)

// copies all shapes and objects etc
extern BOOL copy_rif_data (RIFFHANDLE, int flags,int progress_start,int progress_interval);
#define CopyRIFData(h,f) copy_rif_data(h,f)

// this should return the next free main shape list position for loaded shapes; it may be called more than once
extern int load_precompiled_shapes();
#define LoadPrecompiledShapes() load_precompiled_shapes()

void avp_undo_rif_load(RIFFHANDLE h);
RIFFHANDLE avp_load_rif (const char * fname);
RIFFHANDLE avp_load_rif_non_env (const char * fname);

void DeallocateModules();

void EmptyHierarchyLibrary ();

void DeleteHierarchyLibraryEntry(RIFFHANDLE);

SECTION * GetHierarchyFromLibrary(const char * rif_name);
SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char *);
HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeSetFromLibrary(const char* rif_name,const char* shape_set_name);


extern void save_preplaced_decals();

#ifdef __cplusplus

};
#endif


#endif
