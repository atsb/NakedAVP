/***** HModel.h *****/

#ifndef _hmodel_h
#define _hmodel_h 1

#ifdef __cplusplus
extern "C" {
#endif
#include "decal.h"
#include "psnd.h"

/* CDF 15/9/97 New Damage System */
typedef struct sb_health_bitfield
{
	unsigned int AcidResistant:1;
	unsigned int FireResistant:1;
	unsigned int ElectricResistant:1;
	unsigned int PerfectArmour:1; /* For true aliens! */
	unsigned int ElectricSensitive:1; /* For Xenoborgs */
	unsigned int Combustability:2;
	unsigned int Indestructable:1;
}SBHEALTHFLAGS;
/*
Combustability:
0=Non: Will not burn.
1=Combustable: Normal rules.
2=Sensitive: Normal rules?, + double damage.
3=Flammable: Always ignites + double damage.
*/

typedef struct damageblock {
	/* CDF 15/9/97 New Damage System */
	int Health;
	int Armour;
	unsigned int IsOnFire:1; 

	SBHEALTHFLAGS SB_H_flags;
	/* CDF 15/9/97 New Damage System */
} DAMAGEBLOCK;

/* CDF 15/9/97 New Damage System */
/* Moved to this file 12/11/97 */

typedef struct Hierarchy_Sound
{
	SOUND3DDATA s3d;
	struct loaded_sound const * sound_loaded;
	SOUNDINDEX sound_index;
	int pitch;
	int volume;	
}HIERARCHY_SOUND;


/*
I'm going to try storing the quaternions as shorts within the keyframes , 
because there are loads of them.
-Richard.
*/
typedef struct quat_short
{
	short quatx;
	short quaty;
	short quatz;
	short quatw;
} QUAT_SHORT;

/*A couple of conversion functions */
extern void CopyShortQuatToInt(QUAT_SHORT* qs_from,QUAT* q_to);
extern void CopyIntQuatToShort(QUAT* q_from,QUAT_SHORT* qs_to);


#define KEYFRAME_VECTOR_SHIFT 4

//make sure the keyframe structure packs as much as possible
// packing pragmas removed to fix alignment issues.

typedef struct keyframe_data {
	short Offset_x; /*Offset values may need to be scaled*/
	short Offset_y;	/*In practice scaling should only be needed for 'placed' hierarchies*/
	short Offset_z;

	/* Quats */
	QUAT_SHORT QOrient;

	/*
	  These have been moved from the end to here
	  to reduce padding.
	 */
	unsigned short Sequence_Length; /* Time between these values and the next ones. */
	struct keyframe_data *Next_Frame; /*This is no longer Null for the last frame - look at the last_frame setting instead*/

	/*
	int oneoversinomega;
	Removed oneoversinomega , since I can save a far amount of memory by doing so ,
	and the value is just calculated using a lookup table anyway. -Richard
	*/
	int oneoversequencelength;

	unsigned short omega:12;
	/* Quats */
	unsigned short slerp_to_negative_quat:1; /*Should the slerping use the negative version of the next quaternion*/
	unsigned short frame_has_extended_data:1; /*frame can be cast to a KEYFRAME_DATA_EXTENDED in order to access flag and sound info*/
	unsigned short shift_offset:1; /*does offset need to be scaled*/

	unsigned short last_frame:1; /*Is this the last frame?*/
} KEYFRAME_DATA;

/*Two functions for extracting and setting the key frame offset */
extern void GetKeyFrameOffset(KEYFRAME_DATA* frame,VECTORCH* output_vector);
extern void SetKeyFrameOffset(KEYFRAME_DATA* frame,VECTORCH* input_vector);

/*
If a keyframe has frame_has_extended_data set , then it can be cast to a 
KEYFRAME_DATA_EXTENDED in order to access  the flags and sound.
*/
typedef struct keyframe_data_extended {
	KEYFRAME_DATA frame_data;
	/* Keyframe flagging! */
	int flags;
	/*sound to be played this frame*/
	HIERARCHY_SOUND* sound;
} KEYFRAME_DATA_EXTENDED;

typedef struct sequence {
	int sequence_id;
	KEYFRAME_DATA *first_frame;
	KEYFRAME_DATA *last_frame;
	int Time;
} SEQUENCE;

typedef struct Hierarchy_Shape_Replacement
{
	char* replaced_section_name;
	int replacement_shape_index;
	SHAPEHEADER* replacement_shape;
	int replacement_id;
}HIERARCHY_SHAPE_REPLACEMENT;

typedef struct section {
	int ShapeNum; /* Just in case */
	SHAPEHEADER *Shape;
	char *ShapeName;
	char *Section_Name;
	char *Hierarchy_Name;
	char *Rif_Name;
	struct section **Children;
	int num_sequences;
	SEQUENCE *sequence_array;
	struct damageblock StartingStats;
	VECTORCH gore_spray_direction;
	int flags;
	int IDnumber;
} SECTION;

typedef struct section_attachment {
	char *Riffname;
	char *Section_Name;
	char **Hierarchy_Name;
	struct damageblock StartingStats;
	int flags;
} SECTION_ATTACHMENT;

/* Nature flags */
#define section_has_shape_animation		0x00000001
#define section_has_sparkoflife			0x00000002
#define section_is_master_root			0x00000004

/* Section behaviour flags */
#define section_sprays_blood			0x00000008
#define section_sprays_acid				0x00000010
#define section_sprays_predoblood		0x00000020
#define section_sprays_sparks			0x00000040
#define section_flag_nofurthergibbing	0x00000080
#define section_flag_doesnthurtsb		0x00000100
#define section_flag_heatsource			0x00000200
#define section_flag_affectedbyheat		0x00000400
#define section_flag_never_frag			0x00000800
#define section_flag_fragonlyfordisks	0x00001000
#define section_flag_gibbwhenfragged	0x00002000
#define section_flag_passdamagetoparent	0x00004000

#define NORMAL_GORE_RATE	10000

/* Wounding flags */
#define section_flag_left_arm			0x00010000
#define section_flag_right_arm			0x00020000
#define section_flag_left_leg			0x00040000
#define section_flag_right_leg			0x00080000
#define section_flag_left_foot			0x00100000
#define section_flag_right_foot			0x00200000
#define section_flag_left_hand			0x00400000
#define section_flag_right_hand			0x00800000
#define section_flag_head				0x01000000
#define section_flag_tail				0x02000000

/* Multipart flags */
#define section_flags_wounding	(section_flag_left_arm|section_flag_right_arm|section_flag_left_leg|section_flag_right_leg\
	|section_flag_left_foot|section_flag_right_foot|section_flag_left_hand|section_flag_right_hand|section_flag_head\
	|section_flag_tail|section_has_sparkoflife)
#define section_sprays_anything (section_sprays_blood|section_sprays_acid|section_sprays_predoblood|section_sprays_sparks)

typedef struct section_data {
	int ShapeNum; /* Just in case */
	SHAPEHEADER *Shape;
	
	VECTORCH Offset;
	VECTORCH World_Offset;
	VECTORCH Last_World_Offset;
	VECTORCH View_Offset;
	MATRIXCH RelSecMat;
	MATRIXCH SecMat;
	
	struct damageblock current_damage;
	struct hmodelcontroller *my_controller;
	SHAPEANIMATIONCONTROLLER *sac_ptr;
	TXACTRLBLK *tac_ptr;
	SECTION *sempai;

	SEQUENCE *current_sequence;
	KEYFRAME_DATA *current_keyframe;
	int accumulated_timer;
	int freezeframe_timer;
	int lastframe_timer;
	int gore_timer;

	int flags;

	struct section_data *First_Child;
	struct section_data *Prev_Sibling;
	struct section_data *My_Parent;
	struct section_data *Next_Sibling;

	/* KJL 16:56:51 31/07/98 - decal support */
	OBJECT_DECAL Decals[MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION];
	int	NumberOfDecals;
	int NextDecalToUse;

	/*For remembering the alternate shape sets*/
	int replacement_id;

	/* Tweening */
	VECTORCH stored_offset;
	VECTORCH target_offset;
	VECTORCH delta_offset;
	QUAT stored_quat;
	QUAT target_quat;
	int omega;
	int oneoversinomega;
	int oneovertweeninglength;	
	unsigned int Tweening:1;

} SECTION_DATA;


#define section_data_master_root	0x00000001
#define section_data_false_root		0x00000002
#define section_data_notreal		0x00000004
#define section_data_terminate_here	0x00000008
#define section_data_view_init		0x40000000
#define section_data_initialised	0x80000000

typedef struct delta_controller {
	char *id;
	int timer;
	int lastframe_timer;
	int sequence_type;
	int sub_sequence;
	int seconds_for_sequence;
	int timer_increment;
	int Looped:1;
	int Playing:1;
	int Active:1;
	struct delta_controller *next_controller;
	struct hmodelcontroller *my_hmodel_controller;
} DELTA_CONTROLLER;

typedef struct hmodelcontroller {
	
	int Seconds_For_Sequence;
	int timer_increment;
	int Sequence_Type;
	int	Sub_Sequence;
	int sequence_timer;
	int FrameStamp;
	int View_FrameStamp;
	VECTORCH Computed_Position;
	/* For keeping track of the state of the viewspace coords. */
	int keyframe_flags;

	DELTA_CONTROLLER *Deltas;

	SECTION *Root_Section;
	SECTION_DATA *section_data;

	int After_Tweening_Sequence_Type;
	int After_Tweening_Sub_Sequence;
	int AT_seconds_for_sequence;
	int AT_sequence_timer;

	unsigned int Playing:1;
	unsigned int Reversed:1;
	unsigned int Looped:1;
	unsigned int Tweening:2;
	unsigned int LoopAfterTweening:1;
	unsigned int StopAfterTweening:1;
	unsigned int ElevationTweening:1;
	unsigned int DisableBleeding:1;
	unsigned int LockTopSection:1;
	unsigned int ZeroRootDisplacement:1;
	unsigned int ZeroRootRotation:1;
	unsigned int DisableSounds:1;
	/* Bear in mind that 'Reversed' carries A LOT
	of overhead.  In fact, it does it the old way,
	looking down the entire frame list each frame. */

} HMODELCONTROLLER;

#define Controller_NoTweening	0
#define Controller_Tweening		1
#define Controller_EndTweening	2

typedef struct hitlocationtableentry {
	char *section_name;
	int aspect;
	int cprob;
} HITLOCATIONTABLEENTRY;

#define centre_aspect	0
#define front_aspect	1
#define back_aspect		2

typedef struct hitlocationtable {

	char *id;
	int index; //for loading and saving
	HITLOCATIONTABLEENTRY *CentreLocs;
	HITLOCATIONTABLEENTRY *TopLocs;
	HITLOCATIONTABLEENTRY *BaseLocs;
	HITLOCATIONTABLEENTRY *LeftLocs;
	HITLOCATIONTABLEENTRY *RightLocs;
	HITLOCATIONTABLEENTRY *TopLeftLocs;
	HITLOCATIONTABLEENTRY *TopRightLocs;
	HITLOCATIONTABLEENTRY *BaseLeftLocs;
	HITLOCATIONTABLEENTRY *BaseRightLocs;

} HITLOCATIONTABLE;

extern SECTION_ATTACHMENT Global_Section_Attachments[];
extern SECTION_ATTACHMENT Default_Stats;

struct strategyblock;

extern void Preprocess_HModel(SECTION *root,char *riffname);
extern void Create_HModel(HMODELCONTROLLER *controller,SECTION *root);
extern void InitHModelSequence(HMODELCONTROLLER *controller, int sequence_type, int subsequence, int seconds_for_sequence);
extern void DoHModel(HMODELCONTROLLER *controller, struct displayblock *dptr);
extern void DoHModelTimer(HMODELCONTROLLER *controller);
extern void ProveHModel(HMODELCONTROLLER *controller, struct displayblock *dptr);
extern void ProveHModel_Far(HMODELCONTROLLER *controller, struct strategyblock *sbPtr);
extern void Dispel_HModel(HMODELCONTROLLER *controller);
extern int Prune_HModel_Virtual(SECTION_DATA *top_section);
extern void Correlate_HModel_Instances(SECTION_DATA *victim,SECTION_DATA *templat);
extern int GetSequenceID(int sequence_type,int sub_sequence);
extern SEQUENCE *GetSequencePointer(int sequence_type,int sub_sequence,SECTION *this_section);
extern SECTION_ATTACHMENT *GetThisSectionAttachment(char *riffname,char *section_name,char *hierarchy_name);
extern void Extreme_Gibbing(struct strategyblock *sbPtr,SECTION_DATA *this_section_data, int probability);
extern int Splice_HModels(HMODELCONTROLLER *new_controller,SECTION_DATA *top_section_data);
extern void HModel_ChangeSpeed(HMODELCONTROLLER *controller, int seconds_for_sequence);
extern SECTION_DATA *GetThisSectionData(SECTION_DATA *root,char *name);
extern SECTION *GetThisSection(SECTION *root,char *name);
extern void InitHModelTweening(HMODELCONTROLLER *controller, int seconds_for_tweening, int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int loop);
extern void InitHModelTweening_Backwards(HMODELCONTROLLER *controller, int seconds_for_tweening, int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int loop);
extern void InitHModelTweening_ToTheMiddle(HMODELCONTROLLER *controller, int seconds_for_tweening,int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int target_sequence_timer, int loop);
extern DELTA_CONTROLLER *Add_Delta_Sequence(HMODELCONTROLLER *controller,char *id,int sequence_type,int sub_sequence, int seconds_for_sequence);
extern DELTA_CONTROLLER *Get_Delta_Sequence(HMODELCONTROLLER *controller,char *id);
extern void Remove_Delta_Sequence(HMODELCONTROLLER *controller,char *id);
extern void Transmogrify_HModels(struct strategyblock *sbPtr,HMODELCONTROLLER *controller,SECTION *new_template, int frag, int newsections, int regrowsections);
extern void TrimToTemplate(struct strategyblock *sbPtr,HMODELCONTROLLER *controller,SECTION *new_template, int frag);
extern void Start_Delta_Sequence(DELTA_CONTROLLER *delta_controller,int sequence_type,int sub_sequence,int seconds_for_sequence);
extern void Delta_Sequence_ChangeSpeed(DELTA_CONTROLLER *delta_controller,int seconds_for_sequence);
extern int HModelSequence_Exists(HMODELCONTROLLER *controller,int sequence_type,int sub_sequence);
extern int HModelSequence_Exists_FromRoot(SECTION *root,int sequence_type,int sub_sequence);
extern void RemoveAllDeltas(HMODELCONTROLLER *controller);
extern void KillRandomSections(SECTION_DATA *this_section_data, int probability);
extern void HModel_Regen(HMODELCONTROLLER *controller,int time);
extern int HModelAnimation_IsFinished(HMODELCONTROLLER *controller);
extern int DeltaAnimation_IsFinished(DELTA_CONTROLLER *controller);
extern SECTION_DATA *PointInHModel(HMODELCONTROLLER *controller,VECTORCH *point);
extern SECTION *Get_Corresponding_Section_Recursive(SECTION *this_section,char *Name);
extern SECTION_DATA *GetThisSectionData_FromID(SECTION_DATA *root,int IDnumber);
extern void PlayHierarchySound(HIERARCHY_SOUND* sound,VECTORCH* location);
extern void HModel_SetToolsRelativeSpeed(HMODELCONTROLLER *controller, int factor);
extern void Setup_Texture_Animation_For_Section(SECTION_DATA *this_section_data);
extern void Verify_Positions_In_HModel(struct strategyblock *sbPtr,HMODELCONTROLLER *controller,char *callCode);
extern int HModel_DepthTest(HMODELCONTROLLER *controller,SECTION_DATA *test_section_data,int depth);
extern void DeInitialise_HModel(HMODELCONTROLLER *controller);


struct save_block_header; // savegame.h
extern void LoadHierarchy(struct save_block_header* header,HMODELCONTROLLER* controller);
extern void SaveHierarchy(HMODELCONTROLLER* controller);

#ifdef __cplusplus
}
#endif

#endif
