/***** HModel.c *****/
/***** CDF 21/10/97 *****/

#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pfarlocs.h"
#include "pheromon.h"
#include "bh_types.h"
#include "pvisible.h"
#include "bh_far.h"
#include "bh_debri.h"
#include "bh_pred.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "psnd.h"
#include "load_shp.h"
#include "plat_shp.h"
#include "avp_userprofile.h"
#include "maths.h"
#include "opengl.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "particle.h"
#include "kshape.h"
#include "sfx.h"
#include "dxlog.h"
#include <math.h>

#define AUTODETECT 1
#define ADD_ELEVATION_OFFSETS 1
#define SPARKS_FOR_A_SPRAY 15

int Simplify_HModel_Rendering=0;

extern enum PARTICLE_ID GetBloodType(STRATEGYBLOCK *sbPtr);
extern void DoShapeAnimation (DISPLAYBLOCK * dptr);
extern void RenderThisHierarchicalDisplayblock(DISPLAYBLOCK *dbPtr);
void MatToQuat (MATRIXCH *m, QUAT *quat);

/* protos for this file */
void New_Preprocess_Keyframe(KEYFRAME_DATA *this_keyframe, KEYFRAME_DATA *next_keyframe,int one);
void MulQuat(QUAT *q1,QUAT *q2,QUAT *output);
void Slerp(KEYFRAME_DATA *input,int lerp,QUAT *output);
void Analyse_Tweening_Data(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,int base_timer,VECTORCH *output_offset,MATRIXCH *output_matrix);
static void FindHeatSource_Recursion(HMODELCONTROLLER *controllerPtr, SECTION_DATA *sectionDataPtr);
void Budge_HModel(HMODELCONTROLLER *controller,VECTORCH *offset);

/* external globals */
extern int NormalFrameTime;
extern int GlobalFrameCounter;
extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;

int GlobalGoreRate=NORMAL_GORE_RATE;
int incIDnum; /* Has to be global. */
int GlobalLevelOfDetail_Hierarchical;

STRATEGYBLOCK *Global_HModel_Sptr;
HMODELCONTROLLER *Global_Controller_Ptr;
static DISPLAYBLOCK *Global_HModel_DispPtr;

DAMAGEBLOCK Default_Damageblock = {
	5,	/* Health */
	0,		/* Armour */
	0, /* IsOnFire */
	{
		0,	/* Acid Resistant */
		0,	/* Fire Resistant */
		0,	/* Electric Resistant */
		0,	/* Perfect Armour */
		0,	/* Electric Sensitive */
		0,	/* Combustability */
		0,	/* Indestructable */
	},
};

MATRIXCH Identity_RotMat = {
	ONE_FIXED,0,0,
	0,ONE_FIXED,0,
	0,0,ONE_FIXED,
};

void QNormalise(QUAT *q)
{
	float nw = q->quatw;
	float nx = q->quatx;
	float ny = q->quaty;
	float nz = q->quatz;
	
	float m = sqrt(nw*nw+nx*nx+ny*ny+nz*nz);
	
	if (!m) return;
		
	m = 65536.0/m;

	f2i(q->quatw,nw * m);
	f2i(q->quatx,nx * m);
	f2i(q->quaty,ny * m);
	f2i(q->quatz,nz * m);
}


int GetSequenceID(int sequence_type,int sub_sequence) {
	return( (sub_sequence<<16)+sequence_type);
}

SEQUENCE *GetSequencePointer(int sequence_type,int sub_sequence,SECTION *this_section) {

	int sequence_id,a;
	SEQUENCE *sequence_pointer;

	sequence_id=GetSequenceID(sequence_type,sub_sequence);

	for (a=0; a<this_section->num_sequences; a++) {
		if (this_section->sequence_array[a].sequence_id==sequence_id) {
			sequence_pointer=&(this_section->sequence_array[a]);
			break;
		}
	}
	
	if (a==this_section->num_sequences) {
		//textprint("Unknown HModel sequence! %d,%d\n",sequence_type,sub_sequence);
		return(&(this_section->sequence_array[0]));
		//GLOBALASSERT(0);
		//return(NULL);
	}

	return(sequence_pointer);

}
#if 0
void Preprocess_Keyframe(KEYFRAME_DATA *this_keyframe, KEYFRAME_DATA *next_keyframe,int one) {
{
	
	New_Preprocess_Keyframe(this_keyframe,next_keyframe,one);		
}
#endif
int QDot(QUAT *this_quat, QUAT *next_quat) {

	int qdot;

	qdot=( (MUL_FIXED(this_quat->quatx,next_quat->quatx)) +
		 	(MUL_FIXED(this_quat->quaty,next_quat->quaty)) +
		 	(MUL_FIXED(this_quat->quatz,next_quat->quatz)) +
		 	(MUL_FIXED(this_quat->quatw,next_quat->quatw)) );

	return(qdot);
}

void New_Preprocess_Keyframe(KEYFRAME_DATA *this_keyframe, KEYFRAME_DATA *next_keyframe,int one) {
	
	int cosom;
	QUAT_SHORT *this_quat, *next_quat;

	LOCALASSERT(this_keyframe);
	LOCALASSERT(next_keyframe);

	this_quat=&this_keyframe->QOrient;
	next_quat=&next_keyframe->QOrient;

	//QNormalise(this_quat);
	//QNormalise(next_quat);

	
	//cosom=QDot(this_quat,next_quat);
	cosom=( (MUL_FIXED((int)this_quat->quatx,(int)next_quat->quatx)) +
		 	(MUL_FIXED((int)this_quat->quaty,(int)next_quat->quaty)) +
		 	(MUL_FIXED((int)this_quat->quatz,(int)next_quat->quatz)) +
		 	(MUL_FIXED((int)this_quat->quatw,(int)next_quat->quatw)))<<1;

	if (cosom<0) {
		this_keyframe->slerp_to_negative_quat=1;
		cosom=-cosom;
	}
	else{
		this_keyframe->slerp_to_negative_quat=0;
	}

	
	this_keyframe->omega=(unsigned short)ArcCos(cosom);

	GLOBALASSERT(this_keyframe->Sequence_Length>0);
	this_keyframe->oneoversequencelength=DIV_FIXED(one,(int)this_keyframe->Sequence_Length);

}

void Preprocess_Section(SECTION *this_section, char *riffname ,VECTORCH* offset_to_return) {

	KEYFRAME_DATA *this_keyframe;
	VECTORCH  cumulative_gore_direction;
	VECTORCH  next_one_down;
	SECTION_ATTACHMENT *sec_att;
	int a;

	if (this_section->ShapeName!=NULL) {
		this_section->ShapeNum=GetLoadedShapeMSL(this_section->ShapeName);
		if (this_section->ShapeNum==-1) {
			textprint("\n\n\n\nShape has a name! %s\n",this_section->ShapeName);
			GLOBALASSERT(0);
		}
		this_section->Shape=GetShapeData(this_section->ShapeNum);
	} else {
		/* Do nothing, as requested by John. */
	}

	/* Get section attachment... */

	sec_att=GetThisSectionAttachment(riffname,this_section->Section_Name,this_section->Hierarchy_Name);
	if (sec_att==NULL) {
		/* Try for the general one. */
		sec_att=GetThisSectionAttachment(riffname,this_section->Section_Name,NULL);
	}

	if (sec_att) {
		this_section->StartingStats=sec_att->StartingStats;
		this_section->flags&=~section_is_master_root;
		this_section->flags|=sec_att->flags;
	} else {
		/* Default! */
		this_section->StartingStats=Default_Stats.StartingStats;
		this_section->flags|=Default_Stats.flags;
	}

	/* Now ID number. */

	this_section->IDnumber=incIDnum;
	incIDnum++;

	/* Okay. */
	
	for (a=0; a<this_section->num_sequences; a++) {

		int one=0;
		
		/* Pass through to find sequence length. */

		this_keyframe=this_section->sequence_array[a].first_frame;

		while(1){

			one+=this_keyframe->Sequence_Length;

			/*See if we have got to the end*/
			if(this_keyframe->last_frame) break;
			/* Next frame */
			this_keyframe=this_keyframe->Next_Frame;
	
		}

		/* Second pass. */

		this_keyframe=this_section->sequence_array[a].first_frame;

		while(1) {
			
			New_Preprocess_Keyframe(this_keyframe, this_keyframe->Next_Frame,one);
			

			/*See if we have got to the end*/
			if(this_keyframe->last_frame) break;
			/* Next frame */
			this_keyframe=this_keyframe->Next_Frame;
	
		}

	}

	cumulative_gore_direction.vx=0;
	cumulative_gore_direction.vy=0;
	cumulative_gore_direction.vz=0;

	/* Now call recursion... */

	if (this_section->Children!=NULL) {
		SECTION **child_list_ptr;

		child_list_ptr=this_section->Children;

		while (*child_list_ptr!=NULL) {
			Preprocess_Section(*child_list_ptr,riffname,&next_one_down);
			child_list_ptr++;

			cumulative_gore_direction.vx+=next_one_down.vx;
			cumulative_gore_direction.vy+=next_one_down.vy;
			cumulative_gore_direction.vz+=next_one_down.vz;
		}
	}

	if ( (cumulative_gore_direction.vx==0) 
		&& (cumulative_gore_direction.vy==0) 
		&& (cumulative_gore_direction.vz==0) ) {
		/* Darn.  Have a default. */
		cumulative_gore_direction.vx=4096;
	}

	cumulative_gore_direction.vx=-cumulative_gore_direction.vx;
	cumulative_gore_direction.vy=-cumulative_gore_direction.vy;
	cumulative_gore_direction.vz=-cumulative_gore_direction.vz;

	Normalise(&cumulative_gore_direction);

	this_section->gore_spray_direction=cumulative_gore_direction;

	if(offset_to_return)
	{
		GetKeyFrameOffset(this_section->sequence_array[0].first_frame,offset_to_return);
	}

}

void Preprocess_HModel(SECTION *root, char *riffname) {
	/* One-time preprocessor, prerocesses the 'deltas' for all sequences.  */

	GLOBALASSERT(root); /* Stop messin' about... */

	incIDnum=0;
	/* Root can't spray gore... */
	Preprocess_Section(root,riffname,0);
	root->gore_spray_direction.vx=0;
	root->gore_spray_direction.vy=0;
	root->gore_spray_direction.vz=0;

	/* Shouldn't this be set anyway? */
	root->flags|=section_is_master_root;

}

void Setup_Texture_Animation_For_Section(SECTION_DATA *this_section_data)
{
	GLOBALASSERT(this_section_data);

	if(this_section_data->tac_ptr)
	{
		//get rid of old animation control blocks
		TXACTRLBLK *tac_next;
		tac_next=this_section_data->tac_ptr;
		while (tac_next) {
			TXACTRLBLK *tac_temp;

			tac_temp=tac_next->tac_next;
			DeallocateMem(tac_next);
			tac_next=tac_temp;
			
		}
		this_section_data->tac_ptr=0;
	}
	
	if (this_section_data->Shape) {
		if (this_section_data->Shape->shapeflags & ShapeFlag_HasTextureAnimation) {
	
			TXACTRLBLK **pptxactrlblk;		
			int item_num;
			int shape_num = this_section_data->ShapeNum;
			SHAPEHEADER *shptr = GetShapeData(shape_num);
			pptxactrlblk = &this_section_data->tac_ptr;
			for(item_num = 0; item_num < shptr->numitems; item_num ++)
			{
				POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
				LOCALASSERT(poly);

				SetupPolygonFlagAccessForShape(shptr);
					
				if((Request_PolyFlags((void *)poly)) & iflag_txanim)
					{
						TXACTRLBLK *pnew_txactrlblk;

						pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
						if(pnew_txactrlblk)
						{
							pnew_txactrlblk->tac_flags = 0;										
							pnew_txactrlblk->tac_item = item_num;										
							pnew_txactrlblk->tac_sequence = 0;										
							pnew_txactrlblk->tac_node = 0;										
							pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);										
							pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

							*pptxactrlblk = pnew_txactrlblk;
							pptxactrlblk = &pnew_txactrlblk->tac_next;
						}
						else *pptxactrlblk = NULL; 
					}
			}
			*pptxactrlblk=0;
		}
	}
}

SECTION_DATA *Create_New_Section(SECTION *this_section) {

	SECTION_DATA *this_section_data;
	int num_children;

	if (this_section->ShapeName!=NULL) {
		this_section->ShapeNum=GetLoadedShapeMSL(this_section->ShapeName);
		if (this_section->ShapeNum==-1) {
			GLOBALASSERT(0);
		}
		this_section->Shape=GetShapeData(this_section->ShapeNum);
	} else {
		/* Do nothing, as requested by John. */
	}

	/* Create SECTION_DATA. */

	this_section_data=(SECTION_DATA *)AllocateMem(sizeof(SECTION_DATA));
	GLOBALASSERT(this_section_data);

	this_section_data->sac_ptr=NULL;
	this_section_data->tac_ptr=NULL;
	this_section_data->sempai=this_section;
	
	this_section_data->current_damage.Health=this_section_data->sempai->StartingStats.Health<<16;
	this_section_data->current_damage.Armour=this_section_data->sempai->StartingStats.Armour<<16;
	this_section_data->current_damage.SB_H_flags=this_section_data->sempai->StartingStats.SB_H_flags;

	if (this_section_data->current_damage.Health<=0) {
		/* Wrong! */

		this_section_data->current_damage=Default_Damageblock;
	}

	this_section_data->my_controller=Global_Controller_Ptr;
	/* Note not initialised! */
	this_section_data->flags=0;
	
	/* KJL 17:04:41 31/07/98 - Decal support */
	this_section_data->NumberOfDecals = 0;
	this_section_data->NextDecalToUse = 0;
	
	
	this_section_data->ShapeNum=this_section->ShapeNum;
	this_section_data->Shape=this_section->Shape;

	/* This just so it's not uninitialised. */
	this_section_data->Prev_Sibling=NULL;
	this_section_data->My_Parent=NULL;
	this_section_data->Next_Sibling=NULL;

	this_section_data->replacement_id=0;

	/* Init shape animations... */

	#if AUTODETECT
	if (this_section->Shape) {
		if (this_section->Shape->animation_header) {
	#else
		if (this_section->flags&section_has_shape_animation) {
	#endif

			GLOBALASSERT(this_section->Shape->animation_header);
			
			this_section_data->sac_ptr=AllocateMem(sizeof(SHAPEANIMATIONCONTROLLER));

			InitShapeAnimationController (this_section_data->sac_ptr, this_section->Shape);
	
		}
	#if AUTODETECT
	}
	#endif

	/* Init texture animations. */
	this_section_data->tac_ptr=0;
	Setup_Texture_Animation_For_Section(this_section_data);

	/* Now call recursion... */

	num_children=0;

	if (this_section->Children!=NULL) {
		SECTION **child_list_ptr;
		SECTION_DATA *new_child_list_ptr;

		SECTION_DATA *last_child,*first_child;
		
		last_child=NULL;
		first_child=NULL;

		/* Create subsections. */

		child_list_ptr=this_section->Children;

		while (*child_list_ptr!=NULL) {

			(new_child_list_ptr)=Create_New_Section(*child_list_ptr);

			(new_child_list_ptr)->Prev_Sibling=last_child;
			(new_child_list_ptr)->My_Parent=this_section_data;
			(new_child_list_ptr)->Next_Sibling=NULL; /* For now... */

			if (first_child==NULL) {
				first_child=new_child_list_ptr;
			}

			if (last_child) {
				last_child->Next_Sibling=(new_child_list_ptr);
			}
			last_child=(new_child_list_ptr);			

			child_list_ptr++;
		}
		(new_child_list_ptr)=NULL;
		
		this_section_data->First_Child=first_child;
	} else {
		this_section_data->First_Child=NULL;
	}

	return(this_section_data);

}

void Create_HModel(HMODELCONTROLLER *controller,SECTION *root) {

	/* Connects sequence to controller and must generate 
	the list of section_data structures. */

	GLOBALASSERT(root); /* Stop messin' about... */

	Global_Controller_Ptr=controller;

	controller->Root_Section=root; /* That's a given. */		
	controller->Seconds_For_Sequence=ONE_FIXED;
	controller->timer_increment=ONE_FIXED;
	/* Seconds_For_Sequence and timer_increment are dealt with elsewhere. */
	controller->sequence_timer=0;
	
	controller->FrameStamp=-1;
	controller->View_FrameStamp=-1;
	controller->Computed_Position.vx=0;
	controller->Computed_Position.vy=0;
	controller->Computed_Position.vz=0;

	controller->Playing=0;
	controller->Reversed=0;
	controller->Looped=0;

	controller->After_Tweening_Sequence_Type=-1;
	controller->After_Tweening_Sub_Sequence=-1;
	controller->AT_seconds_for_sequence=ONE_FIXED;
	controller->AT_sequence_timer=0;
	controller->Tweening=Controller_NoTweening;
	controller->LoopAfterTweening=0;
	controller->StopAfterTweening=0;

	controller->DisableBleeding=0;
	controller->LockTopSection=0;
	controller->ZeroRootDisplacement=0;
	controller->ZeroRootRotation=0;
	controller->DisableSounds=0;

	/* Controller elevation now removed.  All done through delta sequences, 8/4/98. */
	controller->ElevationTweening=0;

	controller->Deltas=NULL;

	/* Every time a section is preprocessed, it must generate a section_data for
	itself, and clip it to the last section_data that was generated. */

	controller->section_data=Create_New_Section(controller->Root_Section);

	controller->section_data->Prev_Sibling=NULL;
	controller->section_data->My_Parent=NULL;
	controller->section_data->Next_Sibling=NULL;

	if (root->flags&section_is_master_root) {
		controller->section_data->flags|=section_data_master_root;
	}

}

void Destructor_Recursion(SECTION_DATA *doomed_section_data) {

	/* Remove other bits. */

	if (doomed_section_data->sac_ptr) {
		DeallocateMem(doomed_section_data->sac_ptr);
	}

	if (doomed_section_data->tac_ptr) {
		TXACTRLBLK *tac_next;
		tac_next=doomed_section_data->tac_ptr;
		while (tac_next) {
			TXACTRLBLK *tac_temp;

			tac_temp=tac_next->tac_next;
			DeallocateMem(tac_next);
			tac_next=tac_temp;
			
		}
	}

	/* Recurse. */

	if (doomed_section_data->First_Child!=NULL) {
	
		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=doomed_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {
			/* Remove each child... */
			/* JH - 19/2/98 store the next sibling so that we don't access dealloced memory */
			SECTION_DATA * next_sibling_ptr = child_list_ptr->Next_Sibling;
			Destructor_Recursion(child_list_ptr);
			child_list_ptr=next_sibling_ptr;
		}

	}	

	/* Now remove the section... */

	DeallocateMem(doomed_section_data);

}

void Prune_Section(SECTION_DATA *doomed_section_data) {

	GLOBALASSERT(doomed_section_data);
	/* Destroys section, and all children. */

	if (doomed_section_data->Prev_Sibling) {
		GLOBALASSERT(doomed_section_data->Prev_Sibling->Next_Sibling==doomed_section_data);
		doomed_section_data->Prev_Sibling->Next_Sibling=doomed_section_data->Next_Sibling;
	}

	if (doomed_section_data->Next_Sibling) {
		GLOBALASSERT(doomed_section_data->Next_Sibling->Prev_Sibling==doomed_section_data);
		doomed_section_data->Next_Sibling->Prev_Sibling=doomed_section_data->Prev_Sibling;
	}

	if (doomed_section_data->My_Parent) {
		if (doomed_section_data->My_Parent->First_Child==doomed_section_data) {
			doomed_section_data->My_Parent->First_Child=doomed_section_data->Next_Sibling;
		}
	}

	/* Now destroy. */
	Destructor_Recursion(doomed_section_data);

}

void Delete_Deltas_Recursion(DELTA_CONTROLLER *delta_controller) {

	if (delta_controller==NULL) {
		return;
	}
	
	if (delta_controller->next_controller) {
		Delete_Deltas_Recursion(delta_controller->next_controller);
	}

	DeallocateMem(delta_controller->id);
	DeallocateMem(delta_controller);

}

void Dispel_HModel(HMODELCONTROLLER *controller) {

	/* For getting rid of the section_data. */

	if (controller->section_data!=NULL) {

		Destructor_Recursion(controller->section_data);

		controller->section_data=NULL;

	}

	Delete_Deltas_Recursion(controller->Deltas);

}

void RemoveAllDeltas(HMODELCONTROLLER *controller) {

	/* Pretty self explainatory. */
	GLOBALASSERT(controller);

	Delete_Deltas_Recursion(controller->Deltas);
	controller->Deltas=NULL;

}

/* SetElevationDeltas removed, 8.4.98. CDF. */

void Process_Delta_Controller(SECTION_DATA *this_section_data,DELTA_CONTROLLER *delta_controller,VECTORCH *output_offset,QUAT *output_quat) {

	SEQUENCE *delta_sequence;
	int a;
	int working_timer,lerp,lastframe_working_timer;
	KEYFRAME_DATA *this_frame,*next_frame;
	int sequence_type,sub_sequence,timer;

	if (delta_controller==NULL) {
		return;
	}

	sequence_type=delta_controller->sequence_type;
	sub_sequence=delta_controller->sub_sequence;
	timer=delta_controller->timer;

	GLOBALASSERT(sequence_type>-1);
	GLOBALASSERT(sub_sequence>-1);
	
	delta_sequence=GetSequencePointer(sequence_type,sub_sequence,this_section_data->sempai);

	GLOBALASSERT(delta_sequence);

	/* Final Frame Correction. */
		
	this_frame=delta_sequence->first_frame;

	a=0;

	while (!this_frame->last_frame) {
		a+=this_frame->Sequence_Length;
		this_frame=this_frame->Next_Frame;
	}
	
	/* Now a is the 'real' sequence length. */

	working_timer=MUL_FIXED(timer,a);

	lastframe_working_timer=delta_controller->lastframe_timer;
	lastframe_working_timer=MUL_FIXED(lastframe_working_timer,a);

	/* Now do that thing. */

	this_frame=delta_sequence->first_frame;

	a=0; /* Status flag... */

	while (a==0) {
		next_frame=this_frame->Next_Frame;
		if (working_timer>=this_frame->Sequence_Length) {
			/* We've gone beyond this frame: get next keyframe. */
			working_timer-=this_frame->Sequence_Length;
			lastframe_working_timer-=this_frame->Sequence_Length;
			
			/* Have we looped? */
			if (this_frame->last_frame) {
				/* Some deltas are really fast. */
				this_frame=delta_sequence->first_frame;
			}
			else{
				/* Advance frame... */
				this_frame=next_frame;
			}

			if (lastframe_working_timer<=0) {
				if(this_frame->frame_has_extended_data)
				{
					KEYFRAME_DATA_EXTENDED* this_frame_extended=(KEYFRAME_DATA_EXTENDED*) this_frame;
					/* Check flags... */
					this_section_data->my_controller->keyframe_flags|=this_frame_extended->flags;
					/* ...And keyframe sounds... */
					if (this_frame_extended->sound) {
						if (this_section_data->my_controller->DisableSounds==0) {
							PlayHierarchySound(this_frame_extended->sound,&this_section_data->Last_World_Offset);
						}
					}
				}
			}

		} else {
			a=1; /* Exit loop with success. */
		}	
		/* Better make sure the last 'frame' has 65536 length... */
	}
			
	/* Now, this_frame and next_frame are set, and working_timer<this_frame->Sequence_Length. */
	lerp=MUL_FIXED(working_timer,this_frame->oneoversequencelength);
	
	GetKeyFrameOffset(this_frame,output_offset);
	if(next_frame->shift_offset)
	{
		VECTORCH next_offset;
		GetKeyFrameOffset(next_frame,&next_offset);
		output_offset->vx+=MUL_FIXED(next_offset.vx - output_offset->vx,lerp);
		output_offset->vy+=MUL_FIXED(next_offset.vy - output_offset->vy,lerp);
		output_offset->vz+=MUL_FIXED(next_offset.vz - output_offset->vz,lerp);
	}
	else
	{
		output_offset->vx+=MUL_FIXED((int)next_frame->Offset_x - output_offset->vx,lerp);
		output_offset->vy+=MUL_FIXED((int)next_frame->Offset_y - output_offset->vy,lerp);
		output_offset->vz+=MUL_FIXED((int)next_frame->Offset_z - output_offset->vz,lerp);
	}
		
	
	/* Now deal with orientation. */


	Slerp(this_frame,lerp,output_quat);

	delta_controller->lastframe_timer=delta_controller->timer;

}

void Handle_Section_Timer(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,KEYFRAME_DATA *input_frame,int base_timer, int *working_timer) {

	KEYFRAME_DATA *this_frame,*next_frame;
	int a;

	if (this_section_data->freezeframe_timer!=-1) {
		(*working_timer)=this_section_data->freezeframe_timer;
	} else {
		(*working_timer)=base_timer;
	}
		
	if (this_section_data->accumulated_timer>(*working_timer)) {
		/* We must have looped. Or be reversed.... ! */
		this_section_data->accumulated_timer=0;
		this_section_data->current_keyframe=input_frame;
		if(input_frame->frame_has_extended_data)
		{
			/* Check start flags... */
			KEYFRAME_DATA_EXTENDED* input_frame_extended=(KEYFRAME_DATA_EXTENDED*) input_frame;
			controller->keyframe_flags|=input_frame_extended->flags;
			/* And Keyframe Sounds. */
			if (input_frame_extended->sound) {
				if (controller->DisableSounds==0) {
					PlayHierarchySound(input_frame_extended->sound,&this_section_data->World_Offset);
				}
			}
		}
	}

	this_frame=this_section_data->current_keyframe;
	(*working_timer)-=this_section_data->accumulated_timer;

	/* Now, a lot like the old way, but we shouldn't need to loop more than once. */
	/* If we do, we have a game framerate slower than the anim framerate. */
	
	a=0; /* Status flag... */

	while (a==0) {
		if (this_frame==NULL) {
			this_frame=input_frame; // Heaven help us.
		}
		if (this_frame->last_frame) {
			/* Low framerate loop? */
			next_frame=input_frame;
		}
		else{
			next_frame=this_frame->Next_Frame;
		}

		if ((*working_timer)>=this_frame->Sequence_Length) {
			/* We've gone beyond this frame: get next keyframe. */
			(*working_timer)-=this_frame->Sequence_Length;
			/* Add sequence length to accumulated_timer. */
			this_section_data->accumulated_timer+=this_frame->Sequence_Length;
			/* Advance frame... */
			this_frame=next_frame;
			if (controller->Reversed==0) {
				if(this_frame->frame_has_extended_data)
				{
					KEYFRAME_DATA_EXTENDED* this_frame_extended=(KEYFRAME_DATA_EXTENDED*) this_frame;
					/* Check flags... */
					controller->keyframe_flags|=this_frame_extended->flags;
					/* ...And keyframe sounds... */
					if (this_frame_extended->sound) {
						if (controller->DisableSounds==0) {
							PlayHierarchySound(this_frame_extended->sound,&this_section_data->World_Offset);
						}
					}
				}
			}
			/* Update current keyframe. */
			this_section_data->current_keyframe=this_frame;
		} else {
			a=1; /* Exit loop with success. */
		}	
		/* Better make sure the last 'frame' has 65536 length... */
	}

	if (controller->Reversed) {
		/* Okay, maybe a bit cheesy.  Trigger flags and sounds... */
		KEYFRAME_DATA *cthis_frame,*cnext_frame;
		int b,rev_working_timer;

		rev_working_timer=this_section_data->lastframe_timer-this_section_data->accumulated_timer;
		cthis_frame=this_frame;
		b=0;

		while (b==0) {
			if (cthis_frame==NULL) {
				cthis_frame=input_frame; // Heaven help us.
			}
			if (cthis_frame->last_frame) {
				/* Low framerate loop? */
				cnext_frame=input_frame;
			}
			else{
				cnext_frame=cthis_frame->Next_Frame;
			}
			if (rev_working_timer>=cthis_frame->Sequence_Length) {
				/* We've gone beyond this frame: get next keyframe. */
				rev_working_timer-=cthis_frame->Sequence_Length;
				/* Advance frame... */
				cthis_frame=cnext_frame;
				/* Check flags... */
				if(this_frame->frame_has_extended_data)
				{
					KEYFRAME_DATA_EXTENDED* this_frame_extended=(KEYFRAME_DATA_EXTENDED*) this_frame;
					controller->keyframe_flags|=this_frame_extended->flags;
					/* ...And keyframe sounds... */
					if (this_frame_extended->sound) {
						if (controller->DisableSounds==0) {
							PlayHierarchySound(this_frame_extended->sound,&this_section_data->World_Offset);
						}
					}
				}
			} else {
				b=1; /* Exit loop with success. */
			}	
			/* Better make sure the last 'frame' has 65536 length... */
		}

	}

	/* Check for looping? */
	if (this_frame->last_frame) {
		if (controller->Looped==0) {
			/* Stop at last frame. */
			(*working_timer)=0;
		}
	}

	this_section_data->lastframe_timer=base_timer;
}

void New_Analyse_Keyframe_Data(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,KEYFRAME_DATA *input_frame,int base_timer,VECTORCH *output_offset,MATRIXCH *output_matrix) {

	KEYFRAME_DATA *this_frame;
	QUAT output_quat;
	int working_timer,lerp;

	/* This *should* never fire, should it? */

	GLOBALASSERT(input_frame);

	/* Check the input is in a sensible place. */
	#if 0 //this can't occur anymore with the new way of storing offsets
	if ( !(	(input_frame->Offset.vx<1000000 && input_frame->Offset.vx>-1000000)
		 &&	(input_frame->Offset.vy<1000000 && input_frame->Offset.vy>-1000000)
		 &&	(input_frame->Offset.vz<1000000 && input_frame->Offset.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("First Tests in NEW_ANALYSE_KEYFRAME_DATA.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
			if (Global_HModel_Sptr->SBdptr) {
				LOGDXFMT(("Object is Near.\n"));
			} else {
				LOGDXFMT(("Object is Far.\n"));
			}
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
		LOGDXFMT(("Tweening flags %d\n",controller->Tweening));

		if (this_section_data->My_Parent) {
			LOGDXFMT(("Parent Position %d,%d,%d\n",this_section_data->My_Parent->World_Offset.vx,this_section_data->My_Parent->World_Offset.vy,this_section_data->My_Parent->World_Offset.vz));
		} else {
			LOGDXFMT(("No parent.\n"));
		}
		LOGDXFMT(("This Position %d,%d,%d\n",this_section_data->World_Offset.vx,this_section_data->World_Offset.vy,this_section_data->World_Offset.vz));
		LOGDXFMT(("This Keyframe Position %d,%d,%d\n",input_frame->Offset.vx,input_frame->Offset.vy,input_frame->Offset.vz));

		LOCALASSERT(input_frame->Offset.vx<1000000 && input_frame->Offset.vx>-1000000);
		LOCALASSERT(input_frame->Offset.vy<1000000 && input_frame->Offset.vy>-1000000);
		LOCALASSERT(input_frame->Offset.vz<1000000 && input_frame->Offset.vz>-1000000);
		
	}
	#endif

	/* First find the current frame. */

	if (input_frame->last_frame) {
		/* This is rigid. */
		GetKeyFrameOffset(input_frame,output_offset);
	
		CopyShortQuatToInt(&input_frame->QOrient,&output_quat);

		/* Elevation gone! Now deltas? */

		{
			DELTA_CONTROLLER *dcon;
			QUAT elevation_quat,temp_quat;
			VECTORCH elevation_offset;

			dcon=controller->Deltas;

			while (dcon) {
				if (dcon->Active) {
					Process_Delta_Controller(this_section_data,dcon,&elevation_offset,&elevation_quat);
					output_offset->vx+=elevation_offset.vx;
					output_offset->vy+=elevation_offset.vy;
					output_offset->vz+=elevation_offset.vz;
			
					temp_quat.quatw=output_quat.quatw;
					temp_quat.quatx=output_quat.quatx;
					temp_quat.quaty=output_quat.quaty;
					temp_quat.quatz=output_quat.quatz;
	
					MulQuat(&elevation_quat,&temp_quat,&output_quat);
				}
				dcon=dcon->next_controller;
			}
		}

		QuatToMat(&output_quat,output_matrix);
		
		/* Check the output is in a sensible place. */
		if ( !(	(output_offset->vx<1000000 && output_offset->vx>-1000000)
			 &&	(output_offset->vy<1000000 && output_offset->vy>-1000000)
			 &&	(output_offset->vz<1000000 && output_offset->vz>-1000000) 
			 ) ) {
			VECTORCH frame_offset;
			GetKeyFrameOffset(input_frame,&frame_offset);
		
			LOGDXFMT(("Second Tests in NEW_ANALYSE_KEYFRAME_DATA.\n"));
			if (Global_HModel_Sptr) {
				LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
				if (Global_HModel_Sptr->SBdptr) {
					LOGDXFMT(("Object is Near.\n"));
				} else {
					LOGDXFMT(("Object is Far.\n"));
				}
			} else {
				LOGDXFMT(("Misplaced object has no SBptr.\n"));
			}
			LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
			LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
			LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
			LOGDXFMT(("Tweening flags %d\n",controller->Tweening));
	
			if (this_section_data->My_Parent) {
				LOGDXFMT(("Parent Position %d,%d,%d\n",this_section_data->My_Parent->World_Offset.vx,this_section_data->My_Parent->World_Offset.vy,this_section_data->My_Parent->World_Offset.vz));
				} else {
				LOGDXFMT(("No parent.\n"));
			}
			LOGDXFMT(("This Position %d,%d,%d\n",this_section_data->World_Offset.vx,this_section_data->World_Offset.vy,this_section_data->World_Offset.vz));
			LOGDXFMT(("This Keyframe Position %d,%d,%d\n",frame_offset.vx,frame_offset.vy,frame_offset.vz));
			LOGDXFMT(("Output Offset %d,%d,%d\n",output_offset->vx,output_offset->vy,output_offset->vz));
	
			LOCALASSERT(output_offset->vx<1000000 && output_offset->vx>-1000000);
			LOCALASSERT(output_offset->vy<1000000 && output_offset->vy>-1000000);
			LOCALASSERT(output_offset->vz<1000000 && output_offset->vz>-1000000);
			
		}
		
		return;
	}

	/* New way. */

	Handle_Section_Timer(controller,this_section_data,input_frame,base_timer,&working_timer);
	this_frame=this_section_data->current_keyframe;

	/* Now, this_frame and next_frame are set, and working_timer<this_frame->Sequence_Length. */
	

	lerp=MUL_FIXED(working_timer,this_frame->oneoversequencelength);

	{
		KEYFRAME_DATA* next_frame=this_frame->Next_Frame;
		
		GetKeyFrameOffset(this_frame,output_offset);
		if(next_frame->shift_offset)
		{
			VECTORCH next_offset;
			GetKeyFrameOffset(next_frame,&next_offset);
			output_offset->vx+=MUL_FIXED(next_offset.vx - output_offset->vx,lerp);
			output_offset->vy+=MUL_FIXED(next_offset.vy - output_offset->vy,lerp);
			output_offset->vz+=MUL_FIXED(next_offset.vz - output_offset->vz,lerp);
		}
		else
		{
			output_offset->vx+=MUL_FIXED((int)next_frame->Offset_x - output_offset->vx,lerp);
			output_offset->vy+=MUL_FIXED((int)next_frame->Offset_y - output_offset->vy,lerp);
			output_offset->vz+=MUL_FIXED((int)next_frame->Offset_z - output_offset->vz,lerp);
		}
	}
	/* Now deal with orientation. */


	Slerp(this_frame,lerp,&output_quat);
	
	/* Elevation gone! Now deltas? */
	{
		DELTA_CONTROLLER *dcon;
		QUAT elevation_quat,temp_quat;
		VECTORCH elevation_offset;

		dcon=controller->Deltas;

		while (dcon) {
			if (dcon->Active) {
				Process_Delta_Controller(this_section_data,dcon,&elevation_offset,&elevation_quat);
				output_offset->vx+=elevation_offset.vx;
				output_offset->vy+=elevation_offset.vy;
				output_offset->vz+=elevation_offset.vz;
		
				temp_quat.quatw=output_quat.quatw;
				temp_quat.quatx=output_quat.quatx;
				temp_quat.quaty=output_quat.quaty;
				temp_quat.quatz=output_quat.quatz;
	
				MulQuat(&elevation_quat,&temp_quat,&output_quat);
			}
			dcon=dcon->next_controller;
		}
	}

	QuatToMat(&output_quat,output_matrix);

	/* Check the output is in a sensible place. */
	if ( !(	(output_offset->vx<1000000 && output_offset->vx>-1000000)
		 &&	(output_offset->vy<1000000 && output_offset->vy>-1000000)
		 &&	(output_offset->vz<1000000 && output_offset->vz>-1000000) 
		 ) ) {
	
		VECTORCH frame_offset;
		GetKeyFrameOffset(input_frame,&frame_offset);
		
		LOGDXFMT(("Third Tests in NEW_ANALYSE_KEYFRAME_DATA.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
			if (Global_HModel_Sptr->SBdptr) {
				LOGDXFMT(("Object is Near.\n"));
			} else {
				LOGDXFMT(("Object is Far.\n"));
			}
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
		LOGDXFMT(("Tweening flags %d\n",controller->Tweening));
	
		if (this_section_data->My_Parent) {
			LOGDXFMT(("Parent Position %d,%d,%d\n",this_section_data->My_Parent->World_Offset.vx,this_section_data->My_Parent->World_Offset.vy,this_section_data->My_Parent->World_Offset.vz));
		} else {
			LOGDXFMT(("No parent.\n"));
		}
		LOGDXFMT(("This Position %d,%d,%d\n",this_section_data->World_Offset.vx,this_section_data->World_Offset.vy,this_section_data->World_Offset.vz));
		LOGDXFMT(("This Keyframe Position %d,%d,%d\n",frame_offset.vx,frame_offset.vy,frame_offset.vz));
		LOGDXFMT(("Output Offset %d,%d,%d\n",output_offset->vx,output_offset->vy,output_offset->vz));
	
		LOCALASSERT(output_offset->vx<1000000 && output_offset->vx>-1000000);
		LOCALASSERT(output_offset->vy<1000000 && output_offset->vy>-1000000);
		LOCALASSERT(output_offset->vz<1000000 && output_offset->vz>-1000000);
		
	}

}

SHAPEHEADER *Get_Degraded_Shape(SHAPEHEADER *base_shape)
{
	VECTORCH *worldposition,viewposition;
	ADAPTIVE_DEGRADATION_DESC *array_ptr;
	int lodScale;
	extern float CameraZoomScale;

	if ((base_shape->shape_degradation_array==NULL)||(Global_HModel_Sptr==NULL)) {
		return(base_shape);
	}

	worldposition=&Global_HModel_Sptr->DynPtr->Position;

	MakeVector(worldposition, &Global_VDB_Ptr->VDB_World, &viewposition);
	RotateVector(&viewposition, &Global_VDB_Ptr->VDB_Mat);


	array_ptr=base_shape->shape_degradation_array;

	{
		lodScale = (float)GlobalLevelOfDetail_Hierarchical*CameraZoomScale;

	}
	/* Now walk array. */
	{

		int objectDistance = viewposition.vz;

		if (lodScale!=ONE_FIXED)
		{
			objectDistance = MUL_FIXED(objectDistance,lodScale);
		}
		if (objectDistance<=0) objectDistance=1;
		f2i(viewposition.vz, viewposition.vz*CameraZoomScale);

		/* KJL 12:30:37 09/06/98 - the object distance is scaled by a global variable
		so that the level of detail can be changed on the fly. 
		
		N.B. The last distance stored in the shape_degradation_array is always zero, so that
		the while loop below will always terminate. */
		if (!array_ptr->shapeCanBeUsedCloseUp)
		{
			if(array_ptr->distance<viewposition.vz)
			{
				return (array_ptr->shape);
			}
			else
			{
				array_ptr++;
			}
			
		}
		while (array_ptr->distance>objectDistance) array_ptr++;
	}
	/* Should have a valid entry now. */
	return(array_ptr->shape);
}

void Process_Section(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,VECTORCH *parent_position,MATRIXCH *parent_orientation,int frame_timer, int sequence_type, int subsequence, int render) {

	KEYFRAME_DATA *sequence_start;
	SECTION *this_section;
	SEQUENCE *this_sequence;

	VECTORCH diagnostic_vector;
	int Use_GoreRate;

	/* Work out which SECTION to use. */

	this_section=this_section_data->sempai;

	if (controller!=this_section_data->my_controller) {
		LOGDXFMT(("Wrong Controller assert in PROCESS_SECTION.w\n"));
		LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
		LOGDXFMT(("Tweening flags %d\n",controller->Tweening));
		GLOBALASSERT(controller==this_section_data->my_controller);
	}

	/* We can't just stop at a terminate_here, because that will 
	do over the section_data list. */

	/* Well, actually, now we can. */

	/* Put in a sequence switcher!!! */

	/* There now is one. */

	/* Quick auto-correction... */
	if (frame_timer<0) {
		frame_timer=0;
	}

	if (frame_timer>=65536) {
		frame_timer=65535;
	}

	diagnostic_vector.vx=0;
	diagnostic_vector.vy=0;
	diagnostic_vector.vz=0;
	
	if ((controller->FrameStamp!=GlobalFrameCounter)
		||((this_section_data->flags&section_data_initialised)==0)) {

		int fake_frame_timer;

		/* Positions not computed yet this frame. */

		this_sequence=GetSequencePointer(sequence_type,subsequence,this_section);
		sequence_start=this_sequence->first_frame;
		
		if ((controller->LockTopSection)&&(this_section_data==controller->section_data)) {
			fake_frame_timer=0;
		} else {
			fake_frame_timer=frame_timer;
		}

		/* For this section, find the interpolated offset and eulers. */
		this_section_data->Last_World_Offset=this_section_data->World_Offset;
		
		if (this_section_data->Tweening) {
			Analyse_Tweening_Data(controller,this_section_data,frame_timer,&(this_section_data->World_Offset),&(this_section_data->RelSecMat));
		} else {
			New_Analyse_Keyframe_Data(controller,this_section_data,sequence_start,fake_frame_timer,&(this_section_data->World_Offset),&(this_section_data->RelSecMat));
		}

		if ((controller->ZeroRootDisplacement)&&(this_section_data==controller->section_data)) {
			this_section_data->World_Offset.vx=0;
			this_section_data->World_Offset.vy=0;
			this_section_data->World_Offset.vz=0;
		}
		
		if ((controller->ZeroRootRotation)&&(this_section_data==controller->section_data)) {
			this_section_data->RelSecMat=Identity_RotMat;
		}
						
		this_section_data->Offset=this_section_data->World_Offset;

		diagnostic_vector=this_section_data->World_Offset;
		
		/* The parent's position will be used with the offset value and rotation
		 matrix to determine the position of the new section. */

		RotateVector(&this_section_data->World_Offset,parent_orientation);
		AddVector(parent_position,&(this_section_data->World_Offset));
		
		/* Create the absolute rotation matrix for this section. */
		MatrixMultiply(parent_orientation,&(this_section_data->RelSecMat),&(this_section_data->SecMat));

		/* Set the initialised flag... */
		this_section_data->flags|=section_data_initialised;
	}

	/* Check the object is in a sensible place. */
	if ( !(	(this_section_data->World_Offset.vx<1000000 && this_section_data->World_Offset.vx>-1000000)
		 &&	(this_section_data->World_Offset.vy<1000000 && this_section_data->World_Offset.vy>-1000000)
		 &&	(this_section_data->World_Offset.vz<1000000 && this_section_data->World_Offset.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("Tests in PROCESS_SECTION.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
			if (Global_HModel_Sptr->SBdptr) {
				LOGDXFMT(("Object is Near.\n"));
			} else {
				LOGDXFMT(("Object is Far.\n"));
			}
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
		LOGDXFMT(("Tweening flags %d\n",controller->Tweening));

		LOGDXFMT(("Diagnostic Vector %d,%d,%d\n",diagnostic_vector.vx,diagnostic_vector.vy,diagnostic_vector.vz));
		LOGDXFMT(("Parent Orientation Matrix: %d,%d,%d\n",parent_orientation->mat11,parent_orientation->mat12,parent_orientation->mat13));
		LOGDXFMT(("Parent Orientation Matrix: %d,%d,%d\n",parent_orientation->mat21,parent_orientation->mat22,parent_orientation->mat23));
		LOGDXFMT(("Parent Orientation Matrix: %d,%d,%d\n",parent_orientation->mat31,parent_orientation->mat32,parent_orientation->mat33));
		
		LOGDXFMT(("Parent Position %d,%d,%d\n",parent_position->vx,parent_position->vy,parent_position->vz));
		LOGDXFMT(("This Position %d,%d,%d\n",this_section_data->World_Offset.vx,this_section_data->World_Offset.vy,this_section_data->World_Offset.vz));

		LOCALASSERT(this_section_data->World_Offset.vx<1000000 && this_section_data->World_Offset.vx>-1000000);
		LOCALASSERT(this_section_data->World_Offset.vy<1000000 && this_section_data->World_Offset.vy>-1000000);
		LOCALASSERT(this_section_data->World_Offset.vz<1000000 && this_section_data->World_Offset.vz>-1000000);
		
	}

	/* Now call recursion... */

	if ((this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {

		SECTION_DATA *child_ptr;
	
		child_ptr=this_section_data->First_Child;
	
		while (child_ptr!=NULL) {

			LOCALASSERT(child_ptr->My_Parent==this_section_data);

			Process_Section(controller,child_ptr,&(this_section_data->World_Offset),&(this_section_data->SecMat),frame_timer,sequence_type,subsequence,render);
			child_ptr=child_ptr->Next_Sibling;
		}

	}

	/* Finally, if this section has a shape, and we are rendering, render it. */

	if ((this_section_data->Shape!=NULL)&&((this_section_data->flags&section_data_notreal)==0)
		&&(render)) {
		/* Unreal things don't get plotted, either. */
	
		DISPLAYBLOCK dummy_displayblock;
		SHAPEHEADER *shape_to_use;

		GLOBALASSERT(this_section_data->ShapeNum);

		/* Decide what shape to use. */

		if (this_section_data->Shape) {
			if (this_section_data->Shape->shape_degradation_array==NULL) {
				shape_to_use=this_section_data->Shape;
			} else {
				shape_to_use=Get_Degraded_Shape(this_section_data->Shape);
			}
		} else {
			shape_to_use=NULL;
		}

		/* This really needs to be pre-zeroed, a static
		 in this file... but not yet. */

		if (Simplify_HModel_Rendering) {
			dummy_displayblock.ObShape=GetLoadedShapeMSL("Shell");
			dummy_displayblock.ObShapeData=GetShapeData(dummy_displayblock.ObShape);
		} else {
			dummy_displayblock.ObShape=this_section_data->ShapeNum;
			dummy_displayblock.ObShapeData=shape_to_use;
		}
		dummy_displayblock.name=NULL;
		dummy_displayblock.ObWorld=this_section_data->World_Offset;
		dummy_displayblock.ObEuler.EulerX=0;
		dummy_displayblock.ObEuler.EulerY=0;
		dummy_displayblock.ObEuler.EulerZ=0;
		dummy_displayblock.ObMat=this_section_data->SecMat;
		dummy_displayblock.ObFlags=ObFlag_VertexHazing|ObFlag_MultLSrc;
		dummy_displayblock.ObFlags2=Global_HModel_DispPtr->ObFlags2;
		dummy_displayblock.ObFlags3=0;
		dummy_displayblock.ObNumLights=0;
		dummy_displayblock.ObVDBPtr=NULL;
		if(shape_to_use)
			dummy_displayblock.ObRadius=shape_to_use->shaperadius;
		else
			dummy_displayblock.ObRadius=0;
		dummy_displayblock.ObMaxX=0;
		dummy_displayblock.ObMinX=0;
		dummy_displayblock.ObMaxY=0;
		dummy_displayblock.ObMinY=0;
		dummy_displayblock.ObMaxZ=0;
		dummy_displayblock.ObMinZ=0;
		dummy_displayblock.ObTxAnimCtrlBlks=this_section_data->tac_ptr;
		dummy_displayblock.ObEIDPtr=NULL;
		dummy_displayblock.ObMorphCtrl=NULL;
		dummy_displayblock.ObStrategyBlock=Global_HModel_Sptr;
		dummy_displayblock.ShapeAnimControlBlock=this_section_data->sac_ptr;
		dummy_displayblock.HModelControlBlock=NULL; /* Don't even want to think about that. */
		dummy_displayblock.ObMyModule=NULL;		
		dummy_displayblock.SpecialFXFlags = Global_HModel_DispPtr->SpecialFXFlags;

		MakeVector(&dummy_displayblock.ObWorld, &Global_VDB_Ptr->VDB_World, &dummy_displayblock.ObView);
		RotateVector(&dummy_displayblock.ObView, &Global_VDB_Ptr->VDB_Mat);

		/* Whilst we're here... */

		this_section_data->View_Offset=dummy_displayblock.ObView;
		this_section_data->flags|=section_data_view_init;
		controller->View_FrameStamp=GlobalFrameCounter;

		/* Ho hum. */

		if (dummy_displayblock.ShapeAnimControlBlock) {
			DoShapeAnimation(&dummy_displayblock);
		}

		if(this_section->flags&(section_flag_heatsource|section_flag_affectedbyheat))
		{
			dummy_displayblock.SpecialFXFlags|=SFXFLAG_ISAFFECTEDBYHEAT;
		}
	   
		#if 1
		if (PIPECLEANER_CHEATMODE)
		if (Global_HModel_Sptr)
		if (Global_HModel_Sptr->SBdptr)
		if (! ((Global_HModel_Sptr->SBdptr->ObWorld.vx == parent_position->vx)
			  &&(Global_HModel_Sptr->SBdptr->ObWorld.vy == parent_position->vy)
			  &&(Global_HModel_Sptr->SBdptr->ObWorld.vz == parent_position->vz)) )
		{
			PARTICLE particle;

			particle.Colour = 0xffffffff;
			particle.Size = 30;

			particle.ParticleID = PARTICLE_LASERBEAM;
			particle.Position = this_section_data->World_Offset;
			particle.Offset = *parent_position;;

			D3D_DecalSystem_Setup();
			RenderParticle(&particle);
		
			particle.ParticleID=PARTICLE_ANDROID_BLOOD;

			particle.Size = shape_to_use->shaperadius/8;
			RenderParticle(&particle);
			D3D_DecalSystem_End();
				
		}	
		#endif
		
		RenderThisHierarchicalDisplayblock(&dummy_displayblock);						  

	} else if (controller->View_FrameStamp!=GlobalFrameCounter) {
		this_section_data->flags&=(~section_data_view_init);
	}

	/* Gore spray... */
	
	if (GlobalGoreRate==0) return;
	if (NumberOfBloodParticles>=MAX_NO_OF_BLOOD_PARTICLES) return;
	if (controller->DisableBleeding) return;

	Use_GoreRate=GlobalGoreRate;
	if (SUPERGORE_MODE) {
		Use_GoreRate/=5;
	}

	/* A terminator must spray (backwards) from the origin, to be the stump. */
	
	if (this_section_data->flags&section_data_terminate_here) {
		if (this_section->flags&section_sprays_anything) {
			/* Check for non-zero spray direction... */
			if ( (this_section->gore_spray_direction.vx!=0)
				|| (this_section->gore_spray_direction.vy!=0)
				|| (this_section->gore_spray_direction.vz!=0) ) {

				this_section_data->gore_timer+=NormalFrameTime;
				
				/* New and Special Sparks Code! 16/7/98 */
				if (this_section->flags&section_sprays_sparks) {
					while (this_section_data->gore_timer>(GlobalGoreRate*SPARKS_FOR_A_SPRAY)) {

						VECTORCH final_spray_direction;
						MATRIXCH spray_orient;
						/* Spray is go! */
						
						RotateAndCopyVector(&this_section->gore_spray_direction,&final_spray_direction,&this_section_data->SecMat);
						
						/* Reverse direction for stumps. */
						
						final_spray_direction.vx=-final_spray_direction.vx;
						final_spray_direction.vy=-final_spray_direction.vy;
						final_spray_direction.vz=-final_spray_direction.vz;
						
						/* Scale down. */
						
						final_spray_direction.vx>>=5;
						final_spray_direction.vy>>=5;
						final_spray_direction.vz>>=5;
						
						/* Add random element. */
						
						final_spray_direction.vx+=( (FastRandom()&1023)-512);
						final_spray_direction.vy+=( (FastRandom()&1023)-512);
						final_spray_direction.vz+=( (FastRandom()&1023)-512);
						
						Normalise(&final_spray_direction);
						/* Now convert back to a matrix. */
						MakeMatrixFromDirection(&final_spray_direction,&spray_orient);
												
						/* Call spray function. */
											
						MakeSprayOfSparks(&spray_orient,&this_section_data->World_Offset);
						
						Sound_Play(SID_SPARKS,"d",&this_section_data->World_Offset);

						this_section_data->gore_timer-=(GlobalGoreRate*SPARKS_FOR_A_SPRAY);
					}
					GLOBALASSERT(this_section_data->gore_timer<=(GlobalGoreRate*SPARKS_FOR_A_SPRAY));
				} else {
					while (this_section_data->gore_timer>Use_GoreRate) {
					
						VECTORCH final_spray_direction;
						enum PARTICLE_ID blood_type;
						/* Spray is go! */
						
						RotateAndCopyVector(&this_section->gore_spray_direction,&final_spray_direction,&this_section_data->SecMat);
						
						/* Reverse direction for stumps. */
						
						final_spray_direction.vx=-final_spray_direction.vx;
						final_spray_direction.vy=-final_spray_direction.vy;
						final_spray_direction.vz=-final_spray_direction.vz;
						
						/* Scale down. */
						
						final_spray_direction.vx>>=5;
						final_spray_direction.vy>>=5;
						final_spray_direction.vz>>=5;
						
						/* Add random element. */
						
						final_spray_direction.vx+=( (FastRandom()&1023)-512);
						final_spray_direction.vy+=( (FastRandom()&1023)-512);
						final_spray_direction.vz+=( (FastRandom()&1023)-512);

						if (SUPERGORE_MODE) {
							final_spray_direction.vx<<=1;
							final_spray_direction.vy<<=1;
							final_spray_direction.vz<<=1;
						}

						/* Identify spray type. */
						
						if (this_section->flags&section_sprays_blood) {
							blood_type=GetBloodType(Global_HModel_Sptr);
							/* Er... default? */
						} else if (this_section->flags&section_sprays_acid) {
							blood_type=PARTICLE_ALIEN_BLOOD;
						} else if (this_section->flags&section_sprays_predoblood) {
							blood_type=PARTICLE_PREDATOR_BLOOD;
						} else if (this_section->flags&section_sprays_sparks) {
							blood_type=PARTICLE_SPARK;
						} else {
							blood_type=PARTICLE_FLAME;
							/* Distinctive. */
						}
						
						/* Call spray function. */
											
						MakeParticle(&this_section_data->World_Offset, &final_spray_direction, blood_type);
					
						this_section_data->gore_timer-=Use_GoreRate;
					}
				}
			}
		}
	}

	/* A false root must also spray from the origin... to be the missing part. */

	if (this_section_data->flags&section_data_false_root) {
		if (this_section->flags&section_sprays_anything) {
			/* Check for non-zero spray direction... */
			if ( (this_section->gore_spray_direction.vx!=0)
				|| (this_section->gore_spray_direction.vy!=0)
				|| (this_section->gore_spray_direction.vz!=0) ) {

				this_section_data->gore_timer+=NormalFrameTime;

				/* I don't *think* a section can be both... */
				/* But if it is, we'll just have to live with it. */

				/* New and Special Sparks Code! 16/7/98 */
				if (this_section->flags&section_sprays_sparks) {
					while (this_section_data->gore_timer>(GlobalGoreRate*SPARKS_FOR_A_SPRAY)) {

						VECTORCH final_spray_direction;
						MATRIXCH spray_orient;

						/* Spray is go! */
						
						RotateAndCopyVector(&this_section->gore_spray_direction,&final_spray_direction,&this_section_data->SecMat);
						
						/* Scale down. */
						
						final_spray_direction.vx>>=5;
						final_spray_direction.vy>>=5;
						final_spray_direction.vz>>=5;
						
						/* Add random element. */
						
						final_spray_direction.vx+=( (FastRandom()&1023)-512);
						final_spray_direction.vy+=( (FastRandom()&1023)-512);
						final_spray_direction.vz+=( (FastRandom()&1023)-512);
						Normalise(&final_spray_direction);

						/* Now convert back to a matrix. */
						MakeMatrixFromDirection(&final_spray_direction,&spray_orient);
												
						/* Call spray function. */
											
						MakeSprayOfSparks(&spray_orient,&this_section_data->World_Offset);

						Sound_Play(SID_SPARKS,"d",&this_section_data->World_Offset);

						this_section_data->gore_timer-=(GlobalGoreRate*SPARKS_FOR_A_SPRAY);
					}
					GLOBALASSERT(this_section_data->gore_timer<=(GlobalGoreRate*SPARKS_FOR_A_SPRAY));
				} else {

					while (this_section_data->gore_timer>Use_GoreRate) {
					
						enum PARTICLE_ID blood_type;
						VECTORCH final_spray_direction;
						/* Spray is go! */
						
						RotateAndCopyVector(&this_section->gore_spray_direction,&final_spray_direction,&this_section_data->SecMat);
						
						/* Scale down. */
						
						final_spray_direction.vx>>=5;
						final_spray_direction.vy>>=5;
						final_spray_direction.vz>>=5;
						
						/* Add random element. */
						
						final_spray_direction.vx+=( (FastRandom()&1023)-512);
						final_spray_direction.vy+=( (FastRandom()&1023)-512);
						final_spray_direction.vz+=( (FastRandom()&1023)-512);
						
						if (SUPERGORE_MODE) {
							final_spray_direction.vx<<=1;
							final_spray_direction.vy<<=1;
							final_spray_direction.vz<<=1;
						}

						/* Identify spray type. */
						
						if (this_section->flags&section_sprays_blood) {
							blood_type=GetBloodType(Global_HModel_Sptr);
							/* Er... default? */
						} else if (this_section->flags&section_sprays_acid) {
							blood_type=PARTICLE_ALIEN_BLOOD;
						} else if (this_section->flags&section_sprays_predoblood) {
							blood_type=PARTICLE_PREDATOR_BLOOD;
						} else if (this_section->flags&section_sprays_sparks) {
							blood_type=PARTICLE_SPARK;
						} else {
							blood_type=PARTICLE_FLAME;
							/* Distinctive. */
						}
						
						/* Call spray function. */
						
						MakeParticle(&this_section_data->World_Offset, &final_spray_direction, blood_type);
						
						this_section_data->gore_timer-=Use_GoreRate;
					}
				}
			}
		}
	}

	/* Part three... wounded section. */

	if (this_section_data->current_damage.Health<(this_section->StartingStats.Health<<ONE_FIXED_SHIFT)) {
		if (this_section->flags&section_sprays_anything) {
			int bleeding_rate;

			if (this_section_data->current_damage.Health==0) {
				/* Bleed a lot. */
				bleeding_rate=Use_GoreRate;
			} else {
				/* Just bleed a bit. */
				bleeding_rate=(Use_GoreRate<<1);
			}

			/* Don't care about non-zero spray direction here. */

			this_section_data->gore_timer+=NormalFrameTime;

			/* I don't *think* a section can be both... */
			/* But if it is, we'll just have to live with it. */

			/* New and Special Sparks Code! 16/7/98 */
			if (this_section->flags&section_sprays_sparks) {
				while (this_section_data->gore_timer>(bleeding_rate*SPARKS_FOR_A_SPRAY)) {

					VECTORCH final_spray_direction;
					MATRIXCH spray_orient;

					/* Spray is go! */
					
					/* Zero base direction. */
				
					final_spray_direction.vx=0;
					final_spray_direction.vy=0;
					final_spray_direction.vz=0;
					
					/* Add random element. */
					
					while ( (final_spray_direction.vx==0)
						&&(final_spray_direction.vy==0)
						&&(final_spray_direction.vz==0)) {
						final_spray_direction.vx+=( (FastRandom()&1023)-512);
						final_spray_direction.vy+=( (FastRandom()&1023)-512);
						final_spray_direction.vz+=( (FastRandom()&1023)-512);
					}
					Normalise(&final_spray_direction);

					/* Now convert back to a matrix. */
					MakeMatrixFromDirection(&final_spray_direction,&spray_orient);
											
					/* Call spray function. */
										
					MakeSprayOfSparks(&spray_orient,&this_section_data->World_Offset);

					Sound_Play(SID_SPARKS,"d",&this_section_data->World_Offset);

					this_section_data->gore_timer-=(bleeding_rate*SPARKS_FOR_A_SPRAY);
				}
				GLOBALASSERT(this_section_data->gore_timer<=(bleeding_rate*SPARKS_FOR_A_SPRAY));
			} else {

				while (this_section_data->gore_timer>bleeding_rate) {
				
					enum PARTICLE_ID blood_type;
					VECTORCH final_spray_direction;
					/* Spray is go! */
					
					/* Zero base direction. */
				
					final_spray_direction.vx=0;
					final_spray_direction.vy=0;
					final_spray_direction.vz=0;
					
					/* Add random element. */
					
					final_spray_direction.vx+=( (FastRandom()&1023)-512);
					final_spray_direction.vy+=( (FastRandom()&1023)-512);
					final_spray_direction.vz+=( (FastRandom()&1023)-512);

					/* Identify spray type. */
					
					if (this_section->flags&section_sprays_blood) {
						blood_type=GetBloodType(Global_HModel_Sptr);
						/* Er... default? */
					} else if (this_section->flags&section_sprays_acid) {
						blood_type=PARTICLE_ALIEN_BLOOD;
					} else if (this_section->flags&section_sprays_predoblood) {
						blood_type=PARTICLE_PREDATOR_BLOOD;
					} else if (this_section->flags&section_sprays_sparks) {
						blood_type=PARTICLE_SPARK;
					} else {
						blood_type=PARTICLE_FLAME;
						/* Distinctive. */
					}
					
					/* Call spray function. */
					
					MakeParticle(&this_section_data->World_Offset, &final_spray_direction, blood_type);
					
					this_section_data->gore_timer-=bleeding_rate;
				}
				GLOBALASSERT(this_section_data->gore_timer<=bleeding_rate);
			}
		}
	}

}

void Init_Sequence_Recursion(SECTION_DATA *this_section_data, int sequence_type,int subsequence, int seconds_for_sequence) {

	SEQUENCE *sequence_ptr;

	sequence_ptr=GetSequencePointer(sequence_type,subsequence,this_section_data->sempai);

	/* Init fields. */

	this_section_data->current_sequence=sequence_ptr;
	this_section_data->current_keyframe=sequence_ptr->first_frame;
	this_section_data->accumulated_timer=0;
	this_section_data->freezeframe_timer=-1;
	this_section_data->lastframe_timer=0;
	this_section_data->gore_timer=0; /* As good a time as any. */

	/* Zero last world offset, just for the record. */
	this_section_data->Last_World_Offset.vx=0;
	this_section_data->Last_World_Offset.vy=0;
	this_section_data->Last_World_Offset.vz=0;
	
	/* Deinit tweening. */

	this_section_data->Tweening=0;

	/* Keyframe and Sound Flags. */
	if(sequence_ptr->first_frame->frame_has_extended_data)
	{
		KEYFRAME_DATA_EXTENDED* first_frame_extended=(KEYFRAME_DATA_EXTENDED*) sequence_ptr->first_frame;
		this_section_data->my_controller->keyframe_flags|=first_frame_extended->flags;
		if (first_frame_extended->sound) {
			if (this_section_data->flags&section_data_initialised) {
				if (this_section_data->my_controller->DisableSounds==0) {
					PlayHierarchySound(first_frame_extended->sound,&this_section_data->World_Offset);
				}
			}
		}
	}
	/* Animation. */
	
	if (this_section_data->sac_ptr) {
	
		SHAPEANIMATIONCONTROLDATA sacd ;
		SHAPEANIMATIONSEQUENCE *sequence_pointer;
		int num_frames,sequence;
	
		sequence=(sequence_type<<16)+subsequence;
	
		InitShapeAnimationControlData(&sacd);
		/* Compute rate. */
		
		sequence_pointer=this_section_data->sac_ptr->anim_header->anim_sequences;
		GLOBALASSERT(sequence<this_section_data->sac_ptr->anim_header->num_sequences);
		num_frames=sequence_pointer[sequence].num_frames;
								
		sacd.seconds_per_frame = seconds_for_sequence/num_frames;
		/* Okay. */
		sacd.sequence_no = sequence; 
		sacd.default_start_and_end_frames = 1;
		sacd.reversed = 0;
		sacd.stop_at_end = 0;
		SetOrphanedShapeAnimationSequence (this_section_data->sac_ptr, &sacd);
	
	}

	/* Recurse. */

	if ( (this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {
		/* Respect the terminator! */

		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=this_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {
			Init_Sequence_Recursion(child_list_ptr,sequence_type,subsequence,seconds_for_sequence);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

}

void HModel_ChangeSpeed(HMODELCONTROLLER *controller, int seconds_for_sequence) {

	/* Simple enough... */

	controller->Seconds_For_Sequence=seconds_for_sequence;
	controller->timer_increment=DIV_FIXED(ONE_FIXED,controller->Seconds_For_Sequence);

}

void HModel_SetToolsRelativeSpeed(HMODELCONTROLLER *controller, int factor) {

	KEYFRAME_DATA *sequence_start;
	SEQUENCE *this_sequence;
	int real_time;

	GLOBALASSERT(controller);
	GLOBALASSERT(factor>0);
	/* Find the tools speed for this sequence... */

	this_sequence=GetSequencePointer(controller->Sequence_Type,controller->Sub_Sequence,controller->Root_Section);
	sequence_start=this_sequence->first_frame;
	GLOBALASSERT(sequence_start);

	real_time=this_sequence->Time;
	if (real_time<=0) {
		real_time=ONE_FIXED;
		/* Might want to assert here? */
	}
	
	real_time=MUL_FIXED(real_time,factor);
	HModel_ChangeSpeed(controller,real_time);

}

void InitHModelSequence(HMODELCONTROLLER *controller, int sequence_type,int subsequence, int seconds_for_sequence) {

	KEYFRAME_DATA *sequence_start;
	SEQUENCE *this_sequence;
	int real_time;

	GLOBALASSERT(controller);
	GLOBALASSERT(seconds_for_sequence);

	/* Check this sequence exists... */
	
	this_sequence=GetSequencePointer(sequence_type,subsequence,controller->Root_Section);
	sequence_start=this_sequence->first_frame;
	GLOBALASSERT(sequence_start);

	if (seconds_for_sequence>0) {
		real_time=seconds_for_sequence;
	} else {
		real_time=this_sequence->Time;
		if (real_time<=0) {
			real_time=ONE_FIXED;
			/* Might want to assert here. */
		}
	}

	/* Now set it up. */

	controller->Sequence_Type=sequence_type;
	controller->Sub_Sequence=subsequence;
	controller->Seconds_For_Sequence=real_time;
	controller->timer_increment=DIV_FIXED(ONE_FIXED,controller->Seconds_For_Sequence);
	controller->sequence_timer=0;
	controller->Playing=1;
	controller->Reversed=0;
	controller->Looped=1;
	
	controller->keyframe_flags=0;

	controller->After_Tweening_Sequence_Type=-1;
	controller->After_Tweening_Sub_Sequence=-1;
	controller->AT_seconds_for_sequence=ONE_FIXED;
	controller->AT_sequence_timer=0;
	controller->Tweening=Controller_NoTweening;

	/* Recurse though hierarchy, setting up all the section_data sequence stores? */

	Init_Sequence_Recursion(controller->section_data, sequence_type, subsequence,seconds_for_sequence);
	
}

static void HMTimer_Kernel(HMODELCONTROLLER *controller) {

	/* Core of the timer code. */

	if (controller->Tweening==Controller_EndTweening) {
		int reversed, AT_sequence_timer;

		reversed=controller->Reversed; /* Remember this! */
		AT_sequence_timer=controller->AT_sequence_timer; /* Remember this, too! */

		if (controller->AT_seconds_for_sequence==0) {
			GLOBALASSERT(controller->AT_seconds_for_sequence);
		}

		InitHModelSequence(controller,controller->After_Tweening_Sequence_Type,
			controller->After_Tweening_Sub_Sequence,controller->AT_seconds_for_sequence);
		
		if (reversed) {
			controller->Reversed=1;
			#if 0
			controller->sequence_timer=(ONE_FIXED-1);
			#endif
		}
		
		/* This should now always be set, though InitHModelSequence zeros it (D'oh!). */
		controller->sequence_timer=AT_sequence_timer;
				
		if (controller->LoopAfterTweening) {
			controller->Looped=1;
		} else {
			controller->Looped=0;
		}
		if (controller->StopAfterTweening) {
			/* Hey ho. */
			controller->Playing=0;
		}
		controller->Tweening=Controller_NoTweening;
		controller->ElevationTweening=0;
	}
	
	if (controller->Playing) {
		if (controller->Reversed) {
			if (controller->Tweening!=Controller_NoTweening) {
				GLOBALASSERT(controller->Tweening==Controller_Tweening);
				/* Still tween forwards. */
				controller->sequence_timer+=MUL_FIXED(controller->timer_increment,NormalFrameTime);
				if (controller->sequence_timer>=ONE_FIXED) {
					controller->sequence_timer=ONE_FIXED;
					controller->Tweening=Controller_EndTweening;
				}
			} else {
				GLOBALASSERT(controller->Tweening==Controller_NoTweening);
				controller->sequence_timer-=MUL_FIXED(controller->timer_increment,NormalFrameTime);
				if (controller->Looped) {
					while (controller->sequence_timer<0) {
						/* Might lose count of how many times we've looped, but who's counting? */
						controller->sequence_timer+=ONE_FIXED;
					}
				} else {
					if (controller->sequence_timer<0) {
						controller->sequence_timer=0;
					}
				}
			}
		} else {
			controller->sequence_timer+=MUL_FIXED(controller->timer_increment,NormalFrameTime);
	
			if (controller->Tweening!=Controller_NoTweening) {
				GLOBALASSERT(controller->Tweening==Controller_Tweening);
				if (controller->sequence_timer>=ONE_FIXED) {
					controller->sequence_timer=ONE_FIXED;
					controller->Tweening=Controller_EndTweening;
				}
	
			} else if (controller->Looped) {
				while (controller->sequence_timer>=ONE_FIXED) {
					/* Might lose count of how many times we've looped, but who's counting? */
					controller->sequence_timer-=ONE_FIXED;
				}
			} else {
				if (controller->sequence_timer>=ONE_FIXED) {
					controller->sequence_timer=ONE_FIXED-1;
				}
			}
		}
	}	

	/* Do delta timers, too. */

	{
		DELTA_CONTROLLER *dcon;

		dcon=controller->Deltas;

		while (dcon) {

			if ((dcon->seconds_for_sequence)&&(dcon->Playing)&&(dcon->Active)) {
				dcon->timer+=MUL_FIXED(dcon->timer_increment,NormalFrameTime);
				if (dcon->Looped) {
					if (dcon->timer>=ONE_FIXED) dcon->timer-=ONE_FIXED;
				} else {
					if (dcon->timer>=ONE_FIXED) dcon->timer=ONE_FIXED-1;
				}
			}

			dcon=dcon->next_controller;
		}
	}

}

void DoHModel(HMODELCONTROLLER *controller, DISPLAYBLOCK *dptr) {

	GLOBALASSERT(controller);
	GLOBALASSERT(dptr);

	Global_HModel_Sptr=dptr->ObStrategyBlock;
	Global_HModel_DispPtr=dptr;

	/* Check the object is in a sensible place. */
	if ( !(	(dptr->ObWorld.vx<1000000 && dptr->ObWorld.vx>-1000000)
		 &&	(dptr->ObWorld.vy<1000000 && dptr->ObWorld.vy>-1000000)
		 &&	(dptr->ObWorld.vz<1000000 && dptr->ObWorld.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("Tests in DOHMODEL.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		
		LOCALASSERT(dptr->ObWorld.vx<1000000 && dptr->ObWorld.vx>-1000000);
		LOCALASSERT(dptr->ObWorld.vy<1000000 && dptr->ObWorld.vy>-1000000);
		LOCALASSERT(dptr->ObWorld.vz<1000000 && dptr->ObWorld.vz>-1000000);
		
	}

	GLOBALASSERT(controller->section_data->my_controller==controller);

	/* Only do timer if you're out of date. */
	
	if (controller->FrameStamp!=GlobalFrameCounter) {

		HMTimer_Kernel(controller);

		controller->keyframe_flags=0;

	} else {
		VECTORCH offset;
		/* Want to budge? */

		offset.vx=dptr->ObWorld.vx-controller->Computed_Position.vx;
		offset.vy=dptr->ObWorld.vy-controller->Computed_Position.vy;
		offset.vz=dptr->ObWorld.vz-controller->Computed_Position.vz;

		if ((offset.vx!=0)||(offset.vy!=0)||(offset.vz!=0)) {
			/* I reckon you'd be better off taking the budge. */
			Budge_HModel(controller,&offset);
		}

	}

	/* That handled the timer.  Now render it. */
	{
		int render;

		if (dptr->ObFlags&ObFlag_NotVis) {
			textprint("HModel NotVis!\n");
			render=0;
		} else {
			render=1;
		}
		Process_Section(controller,controller->section_data,&dptr->ObWorld,&dptr->ObMat,controller->sequence_timer,controller->Sequence_Type,controller->Sub_Sequence,render);

	}
	/* Note braces!  Process_Section is OUTSIDE, 'cos you might still want to render! */

	/* Update frame stamp. */

	controller->FrameStamp=GlobalFrameCounter;
	controller->Computed_Position=dptr->ObWorld;

}

void DoHModelTimer_Recursion(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,int frame_timer, int sequence_type, int subsequence) {

	KEYFRAME_DATA *sequence_start;
	SECTION *this_section;
	SEQUENCE *this_sequence;

	/* Cut down Process_Section. */

	this_section=this_section_data->sempai;
	
	if (controller->FrameStamp!=GlobalFrameCounter) {

		/* Positions not computed yet this frame. */

		this_sequence=GetSequencePointer(sequence_type,subsequence,this_section);
		sequence_start=this_sequence->first_frame;
		
		/* For this section, find the interpolated offset and eulers. */
		
		if (this_section_data->Tweening) {
			/* Err... do nothing. */
		} else {
			int working_timer;

			Handle_Section_Timer(controller,this_section_data,sequence_start,frame_timer, &working_timer);
		}
		
	}

	/* Now call recursion... */

	if ((this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {

		SECTION_DATA *child_ptr;
	
		child_ptr=this_section_data->First_Child;
	
		while (child_ptr!=NULL) {

			LOCALASSERT(child_ptr->My_Parent==this_section_data);

			DoHModelTimer_Recursion(controller,child_ptr,frame_timer,sequence_type,subsequence);
			child_ptr=child_ptr->Next_Sibling;
		}

	}

}

void DoHModelTimer(HMODELCONTROLLER *controller) {

	/* Be VERY careful with this function - it can put the timer and the
	position computations out of step.  Once you've called this, call NO
	OTHER HMODEL FUNCTIONS on this model until the next frame! */

	GLOBALASSERT(controller);

	if (controller->FrameStamp==GlobalFrameCounter) {
		/* Done a timer this frame already! */
		return;
	}

	controller->keyframe_flags=0;

	HMTimer_Kernel(controller);

	/* That handled the timer.  No rendering this time. */

	DoHModelTimer_Recursion(controller,controller->section_data,controller->sequence_timer,controller->Sequence_Type,controller->Sub_Sequence);
	
}

void ProveHModel(HMODELCONTROLLER *controller, DISPLAYBLOCK *dptr) {

	/* Simply to verify a new hmodel, and remove junk. */

	GLOBALASSERT(controller);
	GLOBALASSERT(dptr);

	Global_HModel_Sptr=dptr->ObStrategyBlock;

	/* Check the object is in a sensible place. */
	if ( !(	(dptr->ObWorld.vx<1000000 && dptr->ObWorld.vx>-1000000)
		 &&	(dptr->ObWorld.vy<1000000 && dptr->ObWorld.vy>-1000000)
		 &&	(dptr->ObWorld.vz<1000000 && dptr->ObWorld.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("Tests in PROVEHMODEL.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		
		LOCALASSERT(dptr->ObWorld.vx<1000000 && dptr->ObWorld.vx>-1000000);
		LOCALASSERT(dptr->ObWorld.vy<1000000 && dptr->ObWorld.vy>-1000000);
		LOCALASSERT(dptr->ObWorld.vz<1000000 && dptr->ObWorld.vz>-1000000);
		
	}

	GLOBALASSERT(controller->section_data->my_controller==controller);

	if (controller->FrameStamp==GlobalFrameCounter) {
		VECTORCH offset;
		/* Want to budge? */

		offset.vx=dptr->ObWorld.vx-controller->Computed_Position.vx;
		offset.vy=dptr->ObWorld.vy-controller->Computed_Position.vy;
		offset.vz=dptr->ObWorld.vz-controller->Computed_Position.vz;

		if ((offset.vx!=0)||(offset.vy!=0)||(offset.vz!=0)) {
			/* I reckon you'd be better off taking the budge. */
			Budge_HModel(controller,&offset);
		}
	}

	if (controller->FrameStamp!=GlobalFrameCounter) {
		controller->keyframe_flags=0;
		HMTimer_Kernel(controller);
	}
	/* That handled the timer.  Now update positions. */

	Process_Section(controller,controller->section_data,&dptr->ObWorld,&dptr->ObMat,controller->sequence_timer,controller->Sequence_Type,controller->Sub_Sequence,0);

	controller->FrameStamp=GlobalFrameCounter;
	controller->Computed_Position=dptr->ObWorld;

	GLOBALASSERT(controller->section_data->flags&section_data_initialised);

}

void ProveHModel_Far(HMODELCONTROLLER *controller, STRATEGYBLOCK *sbPtr) {

	/* Simply to verify a new hmodel, and remove junk. */

	GLOBALASSERT(controller);
	GLOBALASSERT(sbPtr);

	Global_HModel_Sptr=sbPtr;

	GLOBALASSERT(sbPtr->DynPtr);

	/* Check the object is in a sensible place. */
	if ( !(	(sbPtr->DynPtr->Position.vx<1000000 && sbPtr->DynPtr->Position.vx>-1000000)
		 &&	(sbPtr->DynPtr->Position.vy<1000000 && sbPtr->DynPtr->Position.vy>-1000000)
		 &&	(sbPtr->DynPtr->Position.vz<1000000 && sbPtr->DynPtr->Position.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("Tests in PROVEHMODEL_FAR.\n"));
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		
		LOCALASSERT(sbPtr->DynPtr->Position.vx<1000000 && sbPtr->DynPtr->Position.vx>-1000000);
		LOCALASSERT(sbPtr->DynPtr->Position.vy<1000000 && sbPtr->DynPtr->Position.vy>-1000000);
		LOCALASSERT(sbPtr->DynPtr->Position.vz<1000000 && sbPtr->DynPtr->Position.vz>-1000000);
		
	}

	GLOBALASSERT(controller->section_data->my_controller==controller);

	if (controller->FrameStamp==GlobalFrameCounter) {
		VECTORCH offset;
		/* Want to budge? */

		offset.vx=controller->Computed_Position.vx-sbPtr->DynPtr->Position.vx;
		offset.vy=controller->Computed_Position.vy-sbPtr->DynPtr->Position.vy;
		offset.vz=controller->Computed_Position.vz-sbPtr->DynPtr->Position.vz;

		if ((offset.vx!=0)||(offset.vy!=0)||(offset.vz!=0)) {
			/* I reckon you'd be better off taking the budge. */
			Budge_HModel(controller,&offset);
		}
	}

	if (controller->FrameStamp!=GlobalFrameCounter) {
		controller->keyframe_flags=0;
		HMTimer_Kernel(controller);
	}
   /* That handled the timer.  Now update positions. */

	Process_Section(controller,controller->section_data,&sbPtr->DynPtr->Position,&sbPtr->DynPtr->OrientMat,controller->sequence_timer,controller->Sequence_Type,controller->Sub_Sequence,0);

	controller->FrameStamp=GlobalFrameCounter;
	controller->Computed_Position=sbPtr->DynPtr->Position;

	GLOBALASSERT(controller->section_data->flags&section_data_initialised);

}


int Prune_Recursion_Virtual(SECTION_DATA *this_section_data) {
	
	int sol;
	SECTION *this_section;

	/* Work out which SECTION_DATA to use. */

	this_section=this_section_data->sempai;

	sol=0;

	if (this_section->flags&section_has_sparkoflife) sol=1;
	
	this_section_data->flags|=section_data_notreal;


	if ( (this_section_data->First_Child!=NULL) 
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {

		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			if (Prune_Recursion_Virtual(child_list_ptr)) sol=1;
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

	return(sol);

}

int Prune_HModel_Virtual(SECTION_DATA *top_section) {

	SECTION *this_section;
	int sol;

   /* To make top_section, and everything below it, unreal. */
   /* Must pass back up the recursion if any section pruned */
   /* has the spark of life. */

	this_section=top_section->sempai;
	sol=0;

	if (Prune_Recursion_Virtual(top_section)) sol=1;
	top_section->flags|=section_data_terminate_here;
	top_section->gore_timer=0;
   	
	return(sol);

}

void Correlation_Recursion(SECTION_DATA *this_section_data, SECTION_DATA *alt_section_data) {

	/* Correlate existance. */

	if (alt_section_data->flags&section_data_notreal) {
		this_section_data->flags|=section_data_notreal;
	}

	if (alt_section_data->flags&section_data_terminate_here) {
		this_section_data->flags|=section_data_terminate_here;
		this_section_data->gore_timer=0; /* As good a time as any. */
	}

	/* Now call recursion... */

	if ((this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {
		
		SECTION_DATA *child_list_ptr,*alt_child_list_ptr;

		child_list_ptr=this_section_data->First_Child;
		alt_child_list_ptr=alt_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			Correlation_Recursion(child_list_ptr,alt_child_list_ptr);
			child_list_ptr=child_list_ptr->Next_Sibling;
			alt_child_list_ptr=alt_child_list_ptr->Next_Sibling;
		}
	}
}

void Correlate_HModel_Instances(SECTION_DATA *victim,SECTION_DATA *templat) {

	GLOBALASSERT(victim->sempai==templat->sempai);

	/* You'd better not be being silly. */

	/* The top section must be a false root. */

	victim->flags|=section_data_false_root;

	/* Start recursion. */

	Correlation_Recursion(victim,templat);
}

void MulQuat(QUAT *q1,QUAT *q2,QUAT *output) {

	VECTORCH v1,v2,v3;
	int tw;

	/* Multiply quats... */

	v1.vx=q1->quatx;
	v1.vy=q1->quaty;
	v1.vz=q1->quatz;

	v2.vx=q2->quatx;
	v2.vy=q2->quaty;
	v2.vz=q2->quatz;
	
	CrossProduct(&v1,&v2,&v3);

	tw=MUL_FIXED(q1->quatw,q2->quatw);
	tw-=DotProduct(&v1,&v2);

	v3.vx+=MUL_FIXED(q1->quatw,v2.vx);
	v3.vx+=MUL_FIXED(q2->quatw,v1.vx);

	v3.vy+=MUL_FIXED(q1->quatw,v2.vy);
	v3.vy+=MUL_FIXED(q2->quatw,v1.vy);

	v3.vz+=MUL_FIXED(q1->quatw,v2.vz);
	v3.vz+=MUL_FIXED(q2->quatw,v1.vz);

	output->quatw=tw;
	output->quatx=v3.vx;
	output->quaty=v3.vy;
	output->quatz=v3.vz;

	QNormalise(output);

}

void Slerp(KEYFRAME_DATA *input,int lerp,QUAT *output) {
	
	int sclp,sclq;
	int omega=input->omega; //probably faster copying to an int , rather than using the bitfield
	KEYFRAME_DATA* next_frame=input->Next_Frame;

	/* First check for special case. */

	if ((lerp<0)||(lerp>=65536)) {
		GLOBALASSERT(lerp>=0);
		GLOBALASSERT(lerp<65536);
	}

	if (omega==2048) {
		int t1,t2;

		output->quatx=((int)-input->QOrient.quaty)<<1;
		output->quaty=((int)input->QOrient.quatx)<<1;
		output->quatz=((int)-input->QOrient.quatw)<<1;
		output->quatw=((int)input->QOrient.quatz)<<1;

		t1=MUL_FIXED((ONE_FIXED-lerp),1024);
		sclp=GetSin(t1);
		
		t2=MUL_FIXED(lerp,1024);
		sclq=GetSin(t2);

		//multiply sclp and sclq by 2 to make up for short quats
		sclp<<=1;
		sclq<<=1;

		output->quatx=(MUL_FIXED((int)input->QOrient.quatx,sclp))+(MUL_FIXED(output->quatx,sclq));
		output->quaty=(MUL_FIXED((int)input->QOrient.quaty,sclp))+(MUL_FIXED(output->quaty,sclq));
		output->quatz=(MUL_FIXED((int)input->QOrient.quatz,sclp))+(MUL_FIXED(output->quatz,sclq));
		output->quatw=(MUL_FIXED((int)input->QOrient.quatw,sclp))+(MUL_FIXED(output->quatw,sclq));

	} else {
		if (omega==0) {
			sclp=ONE_FIXED-lerp;
			sclq=lerp;
		} else {
			int t1,t2;
			int oneoversinomega=GetOneOverSin(omega);
	
			t1=MUL_FIXED((ONE_FIXED-lerp),omega);
			t2=GetSin(t1);
			sclp=MUL_FIXED(t2,oneoversinomega);
	
			t1=MUL_FIXED(lerp,omega);
			t2=GetSin(t1);
			sclq=MUL_FIXED(t2,oneoversinomega);
			
		}
		//multiply sclp and sclq by 2 to make up for short quats
		sclp<<=1;
		sclq<<=1;

		if(input->slerp_to_negative_quat)
		{
			//instead of actually negating the quaternion , negate sclq
			sclq=-sclq;
		}
	
		output->quatx=(MUL_FIXED((int)input->QOrient.quatx,sclp))+(MUL_FIXED((int)next_frame->QOrient.quatx,sclq));
		output->quaty=(MUL_FIXED((int)input->QOrient.quaty,sclp))+(MUL_FIXED((int)next_frame->QOrient.quaty,sclq));
		output->quatz=(MUL_FIXED((int)input->QOrient.quatz,sclp))+(MUL_FIXED((int)next_frame->QOrient.quatz,sclq));
		output->quatw=(MUL_FIXED((int)input->QOrient.quatw,sclp))+(MUL_FIXED((int)next_frame->QOrient.quatw,sclq));
	}

	QNormalise(output);

}

void Slerp2(SECTION_DATA *input,int lerp,QUAT *output) {
	
	/* Just a different input structure. */

	int sclp,sclq;

	/* First check for special case. */

	GLOBALASSERT(lerp>=0);
	GLOBALASSERT(lerp<65536);

	if (input->omega==2048) {
		int t1,t2;

		output->quatx=-input->stored_quat.quaty;
		output->quaty=input->stored_quat.quatx;
		output->quatz=-input->stored_quat.quatw;
		output->quatw=input->stored_quat.quatz;

		t1=MUL_FIXED((ONE_FIXED-lerp),1024);
		sclp=GetSin(t1);
		
		t2=MUL_FIXED(lerp,1024);
		sclq=GetSin(t2);

		output->quatx=(MUL_FIXED(input->stored_quat.quatx,sclp))+(MUL_FIXED(output->quatx,sclq));
		output->quaty=(MUL_FIXED(input->stored_quat.quaty,sclp))+(MUL_FIXED(output->quaty,sclq));
		output->quatz=(MUL_FIXED(input->stored_quat.quatz,sclp))+(MUL_FIXED(output->quatz,sclq));
		output->quatw=(MUL_FIXED(input->stored_quat.quatw,sclp))+(MUL_FIXED(output->quatw,sclq));

	} else {
		if ( (input->omega==0) && (input->oneoversinomega==0) ) {
			sclp=ONE_FIXED-lerp;
			sclq=lerp;
		} else {
			int t1,t2;
	
			t1=MUL_FIXED((ONE_FIXED-lerp),input->omega);
			t2=GetSin(t1);
			sclp=MUL_FIXED(t2,input->oneoversinomega);
	
			t1=MUL_FIXED(lerp,input->omega);
			t2=GetSin(t1);
			sclq=MUL_FIXED(t2,input->oneoversinomega);
			
		}
	
		output->quatx=(MUL_FIXED(input->stored_quat.quatx,sclp))+(MUL_FIXED(input->target_quat.quatx,sclq));
		output->quaty=(MUL_FIXED(input->stored_quat.quaty,sclp))+(MUL_FIXED(input->target_quat.quaty,sclq));
		output->quatz=(MUL_FIXED(input->stored_quat.quatz,sclp))+(MUL_FIXED(input->target_quat.quatz,sclq));
		output->quatw=(MUL_FIXED(input->stored_quat.quatw,sclp))+(MUL_FIXED(input->target_quat.quatw,sclq));
	}

	QNormalise(output);

}

void Gibbing_Recursion(STRATEGYBLOCK *sbPtr,SECTION_DATA *this_section_data, int probability) {

	/* General gibbing function. */


	/* Recurse. */

	if ( (this_section_data->First_Child!=NULL) 
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {


		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=this_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {

			if ( (child_list_ptr->flags&section_data_terminate_here)==0) {
							
				/* Right. Roll some dice... */
				if (((child_list_ptr->sempai->flags&section_flag_never_frag)==0)
					&&((SeededFastRandom()&65535)<probability)) {
					/* Frag this bit... */
					DISPLAYBLOCK *this_debris;
	
					this_debris=MakeHierarchicalDebris(sbPtr,child_list_ptr, &child_list_ptr->World_Offset, &child_list_ptr->SecMat,NULL,4);
					/* Oh Dear!  Every section below and including this one becomes... unreal. 
					And if any of them contain the spark of life, we need to know. */
					
					/* Now, gibb the debris ;-) */
					
					if ( (this_debris) && 
						((this_debris->HModelControlBlock->section_data->sempai->flags&section_flag_nofurthergibbing)==0) )  {
						GLOBALASSERT(this_debris->HModelControlBlock);
						GLOBALASSERT(this_debris->HModelControlBlock->section_data);
						Gibbing_Recursion(sbPtr,this_debris->HModelControlBlock->section_data,probability);			
					}

				} else {
					
					Gibbing_Recursion(sbPtr,child_list_ptr,probability);
				
				}
			}	
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

}

void Extreme_Gibbing(STRATEGYBLOCK *sbPtr,SECTION_DATA *this_section_data, int probability) {

	/* Shell for gibbing. Do gibbing... */

	Gibbing_Recursion(sbPtr,this_section_data,probability);

	/* Now frag the body off. */

	if ( (SeededFastRandom()&65535)<probability) {
		MakeHierarchicalDebris(sbPtr,this_section_data, &this_section_data->World_Offset, &this_section_data->SecMat,NULL,4);
	}
}

int Change_Controller_Recursion(HMODELCONTROLLER *new_controller,SECTION_DATA *this_section_data) {
	
	int wounds;

	this_section_data->my_controller=new_controller;
	wounds=this_section_data->sempai->flags&section_flags_wounding;

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			wounds|=Change_Controller_Recursion(new_controller,child_list_ptr);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}

	return(wounds);

}

int Splice_HModels(HMODELCONTROLLER *new_controller,SECTION_DATA *top_section_data) {
	
	/* Real fragging. */
	int wounds;
	SECTION_DATA *new_top_section;

	/* Init new controller. */

	wounds=0;
	Global_Controller_Ptr=new_controller;

	new_controller->Seconds_For_Sequence=ONE_FIXED;
	new_controller->timer_increment=ONE_FIXED;
	/* Seconds_For_Sequence and timer_increment are dealt with elsewhere. */
	new_controller->sequence_timer=0;
	new_controller->Playing=0;
	new_controller->Reversed=0;
	new_controller->Looped=0;

	new_controller->FrameStamp=-1;
	new_controller->View_FrameStamp=-1;

	if (top_section_data->my_controller) {
		new_controller->DisableBleeding=top_section_data->my_controller->DisableBleeding;
		new_controller->DisableSounds=top_section_data->my_controller->DisableSounds;
	} else {
		new_controller->DisableBleeding=0;
		new_controller->DisableSounds=0;
	}
	/* Probably set on return, but never mind. */
	new_controller->LockTopSection=0;
	new_controller->ZeroRootDisplacement=0;
	new_controller->ZeroRootRotation=0;
	new_controller->AT_sequence_timer=0;

	/* Copy them over, splice them over, or ignore BY HAND. */
	new_controller->Deltas=NULL;

	new_controller->keyframe_flags=0;

	/* Every time a section is preprocessed, it must generate a section_data for
	itself, and clip it to the last section_data that was generated. */

	/* Create a new top section... */

	new_top_section=(SECTION_DATA *)AllocateMem(sizeof(SECTION_DATA));
	GLOBALASSERT(new_top_section);

	/* Now.  Copy the old top_section_data into the new top section. */
	
	*new_top_section=*top_section_data;
	
	top_section_data->tac_ptr=NULL;

	/* Correct for new parentage. */
	new_top_section->My_Parent=NULL;

	if (new_top_section->First_Child!=NULL) {

		SECTION_DATA *child_ptr;
	
		child_ptr=new_top_section->First_Child;
	
		while (child_ptr!=NULL) {

			child_ptr->My_Parent=new_top_section;

			child_ptr=child_ptr->Next_Sibling;
		}

	}
	
	
	/* ...and top_section_data gets no children. */
	
	top_section_data->First_Child=NULL;

	/* Set flags. */	

	new_top_section->flags=top_section_data->flags&(~section_data_initialised);
	top_section_data->flags|=section_data_terminate_here;
	top_section_data->gore_timer=0;
	top_section_data->flags|=section_data_notreal;
	new_top_section->flags|=section_data_false_root;
	new_top_section->gore_timer=0;

	/* Connect to controller. */

	new_controller->section_data=new_top_section;
	new_controller->Root_Section=new_top_section->sempai;

	wounds=Change_Controller_Recursion(new_controller,new_top_section);
	
	return(wounds);

}

SECTION_DATA *GetSectionData_Recursion(SECTION_DATA *this_section_data,char *name) {
	
	SECTION_DATA *sdptr;

	sdptr=NULL;

	if (strcmp(name,this_section_data->sempai->Section_Name)==0) {
		/* We are that section! */
		return(this_section_data);	
	}

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			sdptr=GetSectionData_Recursion(child_list_ptr,name);
			child_list_ptr=child_list_ptr->Next_Sibling;
			
			if (sdptr) {
				return(sdptr); /* We got one! */
			}
		}
	}
	
	return(sdptr);

}

SECTION_DATA *GetThisSectionData(SECTION_DATA *root,char *name) {

	if ((root==NULL)||(name==NULL)) {
		return(NULL);
	}

	return(GetSectionData_Recursion(root,name));

}

SECTION *GetSection_Recursion(SECTION *this_section,char *name) {
	
	SECTION *sptr;

	sptr=NULL;

	if (strcmp(name,this_section->Section_Name)==0) {
		/* We are that section! */
		return(this_section);	
	}

	/* Now call recursion... */

	if (this_section->Children!=NULL) {
		
		SECTION **child_list_ptr;

		child_list_ptr=this_section->Children;

		while (*child_list_ptr!=NULL) {

			sptr=GetSection_Recursion(*child_list_ptr,name);
			child_list_ptr++;
			
			if (sptr) {
				return(sptr); /* We got one! */
			}
		}
	}
	
	return(sptr);

}

SECTION *GetThisSection(SECTION *root,char *name) {

	if ((root==NULL)||(name==NULL)) {
		return(NULL);
	}

	return(GetSection_Recursion(root,name));

}

void MatToQuat (MATRIXCH *m, QUAT *quat)
{
	const int X=0;
	const int Y=1;
	const int Z=2;
	const int W=3;

	double mat[4][4];
	double	q[4];
	
	int i,j,k;
	double tr,s;

	int const nxt[3] = 
	{
		//Y,Z,X
		1,2,0
	};
	
	// we could try transposing the matrix here

//	TransposeMatrixCH(m);
	
	mat[0][0] = (double) m->mat11 / ONE_FIXED;
	mat[1][0] = (double) m->mat21 / ONE_FIXED;
	mat[2][0] = (double) m->mat31 / ONE_FIXED;
	mat[0][1] = (double) m->mat12 / ONE_FIXED;
	mat[1][1] = (double) m->mat22 / ONE_FIXED;
	mat[2][1] = (double) m->mat32 / ONE_FIXED;
	mat[0][2] = (double) m->mat13 / ONE_FIXED;
	mat[1][2] = (double) m->mat23 / ONE_FIXED;
	mat[2][2] = (double) m->mat33 / ONE_FIXED;
	
	
	
	tr= mat[0][0]+mat[1][1]+mat[2][2];
	
	if (tr>0.0)
	{
		s=sqrt(tr+1.0);
		q[W] = s*0.5;
		s = 0.5/s;
		
		q[X] = (mat[1][2] - mat[2][1])*s;
		q[Y] = (mat[2][0] - mat[0][2])*s;
		q[Z] = (mat[0][1] - mat[1][0])*s;
	}
	else
	{
		i = X;
		if (mat[Y][Y] > mat[X][X]) i = Y;
		if (mat[Z][Z] > mat[i][i]) i = Z;
		j = nxt[i]; k = nxt[j];
		
		s = sqrt((mat[i][i] - (mat[j][j]+mat[k][k])) + 1.0);
		
		q[i] = s*0.5;
		s = 0.5/s;
		q[W] = (mat[j][k] - mat[k][j])*s;
		q[j] = (mat[i][j] + mat[j][i])*s;
		q[k] = (mat[i][k] + mat[k][i])*s;
	}
	
	quat->quatx = (int) ((double) q[X]*65536.0);
	quat->quaty = (int) ((double) q[Y]*65536.0);
	quat->quatz = (int) ((double) q[Z]*65536.0);
	quat->quatw = (int) ((double) q[W]*65536.0);

	quat->quatw = -quat->quatw;

	QNormalise(quat);
	
}

void Init_Tweening_Recursion(SECTION_DATA *this_section_data, int target_sequence_type,int target_subsequence, int seconds_for_tweening, int backwards) {

	SEQUENCE *sequence_ptr;

	/* Firstly, store current state. */

	this_section_data->stored_offset=this_section_data->Offset;
	MatToQuat(&this_section_data->RelSecMat,&this_section_data->stored_quat);

	/* Now, get target state. */
	
	sequence_ptr=GetSequencePointer(target_sequence_type,target_subsequence,this_section_data->sempai);

	if (backwards) {
		KEYFRAME_DATA *current_frame;
		/* Deduce last frame. */
		current_frame=sequence_ptr->last_frame;
		
		/* Must now have the last frame. */
		GetKeyFrameOffset(current_frame,&this_section_data->target_offset);
		
		CopyShortQuatToInt(&current_frame->QOrient,&this_section_data->target_quat);
	} else {
		GetKeyFrameOffset(sequence_ptr->first_frame,&this_section_data->target_offset);
		CopyShortQuatToInt(&sequence_ptr->first_frame->QOrient,&this_section_data->target_quat);
	}

	/* Preprocess slerp values. */

	{

		this_section_data->delta_offset.vx=this_section_data->target_offset.vx-this_section_data->stored_offset.vx;
		this_section_data->delta_offset.vy=this_section_data->target_offset.vy-this_section_data->stored_offset.vy;
		this_section_data->delta_offset.vz=this_section_data->target_offset.vz-this_section_data->stored_offset.vz;

	}

	{
		int cosom,sinom;
		QUAT *this_quat, *next_quat;

		this_quat=&this_section_data->stored_quat;
		next_quat=&this_section_data->target_quat;

		
		cosom=QDot(this_quat,next_quat);
		
		if (cosom<0) {
			next_quat->quatx=-next_quat->quatx;
			next_quat->quaty=-next_quat->quaty;
			next_quat->quatz=-next_quat->quatz;
			next_quat->quatw=-next_quat->quatw;

			cosom=-cosom;
		}
	
	
		this_section_data->omega=ArcCos(cosom);
		sinom=GetSin(this_section_data->omega);
		if (sinom) {
			this_section_data->oneoversinomega=GetOneOverSin(this_section_data->omega);
		} else {
			/* Yuk. */
			this_section_data->omega=0;
			this_section_data->oneoversinomega=0;
		}
	
		GLOBALASSERT(seconds_for_tweening>0);
		this_section_data->oneovertweeninglength=DIV_FIXED(ONE_FIXED,seconds_for_tweening);

	}

	/* Init fields... I guess. */

	this_section_data->current_sequence=sequence_ptr;
	this_section_data->current_keyframe=sequence_ptr->first_frame;
	this_section_data->accumulated_timer=0;
	this_section_data->freezeframe_timer=-1;
	this_section_data->lastframe_timer=0;
	this_section_data->gore_timer=0; /* As good a time as any. */

	this_section_data->Tweening=1;

	/* Animation? */

	/* Nah. */

	/* Recurse. */

	if ( (this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {
		/* Respect the terminator! */

		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=this_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {
			Init_Tweening_Recursion(child_list_ptr,target_sequence_type,target_subsequence,seconds_for_tweening,backwards);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

}


void InitHModelTweening(HMODELCONTROLLER *controller, int seconds_for_tweening,
	int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int loop) {

	/* Just set it up... */
	GLOBALASSERT(target_seconds_for_sequence);

	controller->Sequence_Type=target_sequence_type;
	controller->Sub_Sequence=target_subsequence;
	controller->Seconds_For_Sequence=seconds_for_tweening;
	controller->timer_increment=DIV_FIXED(ONE_FIXED,controller->Seconds_For_Sequence);
	controller->sequence_timer=0;
	controller->Playing=1;
	controller->Reversed=0;
	controller->Looped=1;

	controller->After_Tweening_Sequence_Type=target_sequence_type;
	controller->After_Tweening_Sub_Sequence=target_subsequence;
	controller->AT_seconds_for_sequence=target_seconds_for_sequence;
	controller->AT_sequence_timer=0;
	controller->Tweening=Controller_Tweening;
	if (loop) {
		controller->LoopAfterTweening=1;
	} else {
		controller->LoopAfterTweening=0;
	}
	controller->StopAfterTweening=0;
	controller->ElevationTweening=0;

	/* Recurse though hierarchy, setting up all the section_data sequence stores? */

	Init_Tweening_Recursion(controller->section_data, target_sequence_type, target_subsequence,seconds_for_tweening,0);

}

void InitHModelTweening_Backwards(HMODELCONTROLLER *controller, int seconds_for_tweening,
	int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int loop) {

	/* Ooh, yuck. */
	GLOBALASSERT(target_seconds_for_sequence);

	controller->Sequence_Type=target_sequence_type;
	controller->Sub_Sequence=target_subsequence;
	controller->Seconds_For_Sequence=seconds_for_tweening;
	controller->timer_increment=DIV_FIXED(ONE_FIXED,controller->Seconds_For_Sequence);
	controller->sequence_timer=0;
	controller->Playing=1;
	controller->Reversed=1;
	controller->Looped=1;

	controller->After_Tweening_Sequence_Type=target_sequence_type;
	controller->After_Tweening_Sub_Sequence=target_subsequence;
	controller->AT_seconds_for_sequence=target_seconds_for_sequence;
	controller->AT_sequence_timer=(ONE_FIXED-1);
	controller->Tweening=Controller_Tweening;
	if (loop) {
		controller->LoopAfterTweening=1;
	} else {
		controller->LoopAfterTweening=0;
	}
	controller->StopAfterTweening=0;
	controller->ElevationTweening=0;

	/* Recurse though hierarchy, setting up all the section_data sequence stores? */

	Init_Tweening_Recursion(controller->section_data, target_sequence_type, target_subsequence,seconds_for_tweening,1);

}

void ReSnap(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data, int elevation) {

	SEQUENCE *sequence_ptr;
	/* In this procedure, we have to get the new target quat and offset. */

	sequence_ptr=GetSequencePointer(controller->After_Tweening_Sequence_Type,controller->After_Tweening_Sub_Sequence,this_section_data->sempai);

	#if 0
	if (controller->Reversed) {
		/* We're in a backwards tween. */
		KEYFRAME_DATA *current_frame;
		/* Deduce last frame. */
		current_frame=sequence_ptr->last_frame;
	
		/* Must now have the last frame. */
		this_section_data->target_offset=current_frame->Offset;
		CopyShortQuatToInt(&current_frame->QOrient,&this_section_data->target_quat);
	} else {
		this_section_data->target_offset=sequence_ptr->first_frame->Offset;
		CopyShortQuatToInt(&sequence_ptr->first_frame->QOrient,&this_section_data->target_quat);
	}
	#else
	/* Now, irritatingly, we have to put our faith in AT_sequence_timer. */
	if (controller->AT_sequence_timer==(ONE_FIXED-1)) {
		/* We're in a backwards tween. */
		KEYFRAME_DATA *current_frame;
		/* Deduce last frame. */
		current_frame=sequence_ptr->last_frame;

		/* Must now have the last frame. */
		GetKeyFrameOffset(current_frame,&this_section_data->target_offset);
		CopyShortQuatToInt(&current_frame->QOrient,&this_section_data->target_quat);
	} else if (controller->AT_sequence_timer==0) {
		GetKeyFrameOffset(sequence_ptr->first_frame,&this_section_data->target_offset);
		CopyShortQuatToInt(&sequence_ptr->first_frame->QOrient,&this_section_data->target_quat);
	} else {
		int a,working_timer,lerp;
		KEYFRAME_DATA *this_frame,*next_frame;
	
		this_frame=sequence_ptr->first_frame;
		GLOBALASSERT(this_frame);
		working_timer=controller->AT_sequence_timer;

		a=0; /* Status flag... */

		while (a==0) {
			if (this_frame->last_frame) {
				/* Low framerate loop? */
				next_frame=sequence_ptr->first_frame;
			}
			else{
				next_frame=this_frame->Next_Frame;
			}
			
			if (working_timer>=this_frame->Sequence_Length) {
				/* We've gone beyond this frame: get next keyframe. */
				working_timer-=this_frame->Sequence_Length;
				/* Advance frame... */
				this_frame=next_frame;
			} else {
				a=1; /* Exit loop with success. */
			}	
			/* Better make sure the last 'frame' has 65536 length... */
		}
		GLOBALASSERT(working_timer>=0);
		/* Now we should have a frame and a timer. */

		lerp=MUL_FIXED(working_timer,this_frame->oneoversequencelength);

		GetKeyFrameOffset(this_frame,&this_section_data->target_offset);
		
		if(next_frame->shift_offset)
		{
			VECTORCH next_offset;
			GetKeyFrameOffset(next_frame,&next_offset);
			this_section_data->target_offset.vx+=MUL_FIXED(next_offset.vx - this_section_data->target_offset.vx,lerp);
			this_section_data->target_offset.vy+=MUL_FIXED(next_offset.vy - this_section_data->target_offset.vy,lerp);
			this_section_data->target_offset.vz+=MUL_FIXED(next_offset.vz - this_section_data->target_offset.vz,lerp);
		}
		else
		{
			this_section_data->target_offset.vx+=MUL_FIXED((int)next_frame->Offset_x - this_section_data->target_offset.vx,lerp);
			this_section_data->target_offset.vy+=MUL_FIXED((int)next_frame->Offset_y - this_section_data->target_offset.vy,lerp);
			this_section_data->target_offset.vz+=MUL_FIXED((int)next_frame->Offset_z - this_section_data->target_offset.vz,lerp);
		}
		
		/* Now deal with orientation. */

		Slerp(this_frame,lerp,&this_section_data->target_quat);

	}
	#endif

	if (elevation) {

		/* Elevation gone! Now deltas? */
		{
			DELTA_CONTROLLER *dcon;
			QUAT elevation_quat,temp_quat;
			VECTORCH elevation_offset;
		
			dcon=controller->Deltas;
		
			while (dcon) {
				if (dcon->Active) {
					Process_Delta_Controller(this_section_data,dcon,&elevation_offset,&elevation_quat);
					this_section_data->target_offset.vx+=elevation_offset.vx;
					this_section_data->target_offset.vy+=elevation_offset.vy;
					this_section_data->target_offset.vz+=elevation_offset.vz;
			
					temp_quat.quatw=this_section_data->target_quat.quatw;
					temp_quat.quatx=this_section_data->target_quat.quatx;
					temp_quat.quaty=this_section_data->target_quat.quaty;
					temp_quat.quatz=this_section_data->target_quat.quatz;
		
					MulQuat(&elevation_quat,&temp_quat,&this_section_data->target_quat);
				}
				dcon=dcon->next_controller;
			}
		}
		
	}

	/* Now recompute values. */

	{

		this_section_data->delta_offset.vx=this_section_data->target_offset.vx-this_section_data->stored_offset.vx;
		this_section_data->delta_offset.vy=this_section_data->target_offset.vy-this_section_data->stored_offset.vy;
		this_section_data->delta_offset.vz=this_section_data->target_offset.vz-this_section_data->stored_offset.vz;

	}

	{
		int cosom,sinom;
		QUAT *this_quat, *next_quat;

		this_quat=&this_section_data->stored_quat;
		next_quat=&this_section_data->target_quat;

		
		cosom=QDot(this_quat,next_quat);
		
		if (cosom<0) {
			next_quat->quatx=-next_quat->quatx;
			next_quat->quaty=-next_quat->quaty;
			next_quat->quatz=-next_quat->quatz;
			next_quat->quatw=-next_quat->quatw;
			cosom=-cosom;
		}
	
	
		this_section_data->omega=ArcCos(cosom);
		sinom=GetSin(this_section_data->omega);
		if (sinom) {
			this_section_data->oneoversinomega=GetOneOverSin(this_section_data->omega);
		} else {
			/* Yuk. */
			this_section_data->omega=0;
			this_section_data->oneoversinomega=0;
		}

	}

}

void Analyse_Tweening_Data(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,int base_timer,VECTORCH *output_offset,MATRIXCH *output_matrix) {

	QUAT output_quat;
	int working_timer,lerp;

	/* Go go gadget tweening. */

	/* There is only one frame. */

	working_timer=base_timer;
	/* Tests: can't be too careful. */
	if (working_timer>=65536) working_timer=65535;
	if (working_timer<0) working_timer=0;

	/* Now, a lot like the old way, but we shouldn't need to loop more than once. */
	/* If we do, we have a game framerate slower than the anim framerate. */
	
	if (controller->Deltas) {
		/* Resnap with elevation. */
		ReSnap(controller,this_section_data,1);
		
		controller->ElevationTweening=1;

	} else if (controller->ElevationTweening) {
		/* It's all messed up now.  'Resnap' WITHOUT elevation. */
		
		ReSnap(controller,this_section_data,0);
		
		controller->ElevationTweening=0;

	}

	/* Now, this_frame and next_frame are set, and working_timer<this_frame->Sequence_Length. */
	
	output_offset->vx=(MUL_FIXED(this_section_data->delta_offset.vx,working_timer))+this_section_data->stored_offset.vx;
	output_offset->vy=(MUL_FIXED(this_section_data->delta_offset.vy,working_timer))+this_section_data->stored_offset.vy;
	output_offset->vz=(MUL_FIXED(this_section_data->delta_offset.vz,working_timer))+this_section_data->stored_offset.vz;

	/* Now deal with orientation. */

	lerp=working_timer;

	Slerp2(this_section_data,lerp,&output_quat);

	/* NO elevation! */

	QuatToMat(&output_quat,output_matrix);

	/* Just to make sure. */
	MNormalise(output_matrix);
}


/* KJL 16:51:20 10/02/98 - Heat source stuff */

int FindHeatSourcesInHModel(DISPLAYBLOCK *dispPtr)
{
	HMODELCONTROLLER *controllerPtr = dispPtr->HModelControlBlock;

	LOCALASSERT(controllerPtr);
	
	/* KJL 16:36:12 10/02/98 - check positions are up to date */
	ProveHModel(controllerPtr,dispPtr);

	NumberOfHeatSources=0;

	/* KJL 16:36:25 10/02/98 - process model */
	FindHeatSource_Recursion(controllerPtr,controllerPtr->section_data);

	return 0;
}
																	  
static void FindHeatSource_Recursion(HMODELCONTROLLER *controllerPtr, SECTION_DATA *sectionDataPtr)
{
	/* KJL 16:29:40 10/02/98 - Recurse through hmodel */
	if ((sectionDataPtr->First_Child!=NULL)&&(!(sectionDataPtr->flags&section_data_terminate_here)))
	{
		SECTION_DATA *childSectionPtr = sectionDataPtr->First_Child;
	
		while (childSectionPtr!=NULL)
		{
			LOCALASSERT(childSectionPtr->My_Parent==sectionDataPtr);

			FindHeatSource_Recursion(controllerPtr,childSectionPtr);
			childSectionPtr=childSectionPtr->Next_Sibling;
		}
	}

	/* KJL 16:30:03 10/02/98 - record heat source */
	if (sectionDataPtr->sempai->flags&section_flag_heatsource)
	{
		/* KJL 16:36:58 10/02/98 - currently just position; could have size, orientation, etc. */
		HeatSourceList[NumberOfHeatSources].Position.vx = sectionDataPtr->World_Offset.vx;
		HeatSourceList[NumberOfHeatSources].Position.vy = sectionDataPtr->World_Offset.vy;
		HeatSourceList[NumberOfHeatSources].Position.vz = sectionDataPtr->World_Offset.vz;
		TranslatePointIntoViewspace(&(HeatSourceList[NumberOfHeatSources].Position));
		NumberOfHeatSources++;
	}
}

DELTA_CONTROLLER *Get_Delta_Sequence(HMODELCONTROLLER *controller,char *id) {

	DELTA_CONTROLLER *delta_controller;

	/* Get the controller that matches id. */

	delta_controller=controller->Deltas;

	while (delta_controller) {
		
		if (strcmp(id,delta_controller->id)==0) {
			break;
		}
		delta_controller=delta_controller->next_controller;

	}
	return(delta_controller);
}

void Remove_Delta_Sequence(HMODELCONTROLLER *controller,char *id) {

	DELTA_CONTROLLER *delta_controller;
	DELTA_CONTROLLER **source;

	delta_controller=controller->Deltas;
	source=&controller->Deltas;
	
	while (delta_controller) {
		
		if (strcmp(id,delta_controller->id)==0) {
			break;
		}
		source=&delta_controller->next_controller;
		delta_controller=delta_controller->next_controller;

	}

	if (delta_controller) {
		/* Remove it. */
		*source=delta_controller->next_controller;

		DeallocateMem(delta_controller->id);
		DeallocateMem(delta_controller);
	}
}

DELTA_CONTROLLER *Add_Delta_Sequence(HMODELCONTROLLER *controller,char *id,int sequence_type,int sub_sequence, int seconds_for_sequence) {
	
	KEYFRAME_DATA *sequence_start;
	SEQUENCE *this_sequence;
	DELTA_CONTROLLER *delta_controller;

	/* Create a new delta sequence. */

	delta_controller=(DELTA_CONTROLLER *)AllocateMem(sizeof(DELTA_CONTROLLER));

	GLOBALASSERT(delta_controller);

	delta_controller->next_controller=controller->Deltas;
	controller->Deltas=delta_controller;

	delta_controller->id = AllocateMem(strlen(id)+1);
	strcpy(delta_controller->id,id);

	delta_controller->sequence_type=sequence_type;
	delta_controller->sub_sequence=sub_sequence;
	delta_controller->timer=0;
	delta_controller->lastframe_timer=-1;
	delta_controller->Looped=0;
	delta_controller->Playing=0;
	delta_controller->Active=1; /* By default. */
	delta_controller->my_hmodel_controller=controller;

	this_sequence=GetSequencePointer(sequence_type,sub_sequence,delta_controller->my_hmodel_controller->Root_Section);
	sequence_start=this_sequence->first_frame;
	GLOBALASSERT(sequence_start);
	
	if (seconds_for_sequence>=0) {
		delta_controller->seconds_for_sequence=seconds_for_sequence; // Special case, 0 is legal.
	} else {	
		delta_controller->seconds_for_sequence=this_sequence->Time;
		if (delta_controller->seconds_for_sequence<=0) {
			delta_controller->seconds_for_sequence=ONE_FIXED;
			/* Might want to assert here? */
		}
	}

	if (delta_controller->seconds_for_sequence) {
		delta_controller->timer_increment=DIV_FIXED(ONE_FIXED,delta_controller->seconds_for_sequence);
	} else {
		delta_controller->timer_increment=0;
	}

	return(delta_controller);

}

void Start_Delta_Sequence(DELTA_CONTROLLER *delta_controller,int sequence_type,int sub_sequence,int seconds_for_sequence) {

	KEYFRAME_DATA *sequence_start;
	SEQUENCE *this_sequence;
	GLOBALASSERT(delta_controller);

	/* Again, you must start it and loop it by hand. */

	delta_controller->sequence_type=sequence_type;
	delta_controller->sub_sequence=sub_sequence;
	delta_controller->timer=0;
	delta_controller->Looped=0;
	delta_controller->Playing=0;

	this_sequence=GetSequencePointer(sequence_type,sub_sequence,delta_controller->my_hmodel_controller->Root_Section);
	sequence_start=this_sequence->first_frame;
	GLOBALASSERT(sequence_start);
	
	if (seconds_for_sequence>=0) {
		delta_controller->seconds_for_sequence=seconds_for_sequence; // Special case, 0 is legal.
	} else {	
		delta_controller->seconds_for_sequence=this_sequence->Time;
		if (delta_controller->seconds_for_sequence<=0) {
			delta_controller->seconds_for_sequence=ONE_FIXED;
			/* Might want to assert here? */
		}
	}
	
	if (delta_controller->seconds_for_sequence) {
		delta_controller->timer_increment=DIV_FIXED(ONE_FIXED,delta_controller->seconds_for_sequence);
	} else {
		delta_controller->timer_increment=0;
	}

}

void Delta_Sequence_ChangeSpeed(DELTA_CONTROLLER *delta_controller,int seconds_for_sequence) {
	
	GLOBALASSERT(delta_controller);

	delta_controller->seconds_for_sequence=seconds_for_sequence; // Special case.

	if (delta_controller->seconds_for_sequence) {
		delta_controller->timer_increment=DIV_FIXED(ONE_FIXED,delta_controller->seconds_for_sequence);
	} else {
		delta_controller->timer_increment=0;
	}

}

SECTION *Get_Corresponding_Section(SECTION **List_Ptr,char *Name) {

	SECTION **child_list_ptr=List_Ptr;

	while (*child_list_ptr!=NULL) {
		if (strcmp((*child_list_ptr)->Section_Name,Name)==0) {
			break;
		}
		child_list_ptr++;
	}

	return(*child_list_ptr);
}

SECTION_DATA *GetThisSectionData_FromChildrenOnly(SECTION_DATA *parent,char *name) {

	SECTION_DATA *sdptr;

	if ((parent==NULL)||(name==NULL)) {
		return(NULL);
	}

	sdptr=NULL;

	if (parent->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=parent->First_Child;

		while (child_list_ptr!=NULL) {

			if (strcmp(name,child_list_ptr->sempai->Section_Name)==0) {
				/* Got it. */
				return(child_list_ptr);
			}

			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return(sdptr);

}

void Transmogrification_Recursion(STRATEGYBLOCK *sbPtr,HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,SECTION *new_template, SECTION *old_template, int frag, int newsections, int regrowsections) {
	
	/* Doesn't really matter which tree we're walking... does it? */
	
	GLOBALASSERT(new_template);
	GLOBALASSERT(old_template);
	GLOBALASSERT(strcmp(new_template->Section_Name,old_template->Section_Name)==0);

	GLOBALASSERT(this_section_data);

	if ( (new_template->Children!=NULL) && (old_template->Children!=NULL) )  {
		/* Complex.  I'd really like to walk both at the same time. */
		SECTION **new_child_list_ptr,**old_child_list_ptr;

		new_child_list_ptr=new_template->Children;
		old_child_list_ptr=old_template->Children;

		/* First, let's walk the new template. */
		while (*new_child_list_ptr!=NULL) {

			SECTION *corresponding_section;

			corresponding_section=Get_Corresponding_Section(old_child_list_ptr,(*new_child_list_ptr)->Section_Name);

			if (corresponding_section!=NULL) {
				/* Section also exists in old template.  Deal with it and recurse. */
				SECTION_DATA *child_section_data;

				child_section_data=GetThisSectionData_FromChildrenOnly(this_section_data,(*new_child_list_ptr)->Section_Name);

				if (child_section_data) {
					/* Hey, it might be fragged off.  Now deal with it. */

					child_section_data->sempai=*new_child_list_ptr;

					Transmogrification_Recursion(sbPtr,controller,child_section_data,*new_child_list_ptr, corresponding_section, frag, newsections, regrowsections);
				} else if (regrowsections) {
					/* If it is fragged off, put it back. */
					SECTION_DATA *new_child;
			
					new_child=Create_New_Section(*new_child_list_ptr);

					new_child->My_Parent=this_section_data;
					new_child->Prev_Sibling=NULL;
					new_child->Next_Sibling=this_section_data->First_Child;
					if (this_section_data->First_Child) {
						this_section_data->First_Child->Prev_Sibling=new_child;
					}
					this_section_data->First_Child=new_child;
					Init_Sequence_Recursion(new_child, controller->Sequence_Type, controller->Sub_Sequence,controller->Seconds_For_Sequence);
					/* Prove new positions. */
					Process_Section(controller,new_child,&(this_section_data->World_Offset),&(this_section_data->SecMat),0,controller->Sequence_Type,controller->Sub_Sequence,0);
				}
				
			} else if (newsections) {
				/* No corresponding old section: create a new bit. */
				SECTION_DATA *new_child;
			
				new_child=Create_New_Section(*new_child_list_ptr);

				new_child->My_Parent=this_section_data;
				new_child->Prev_Sibling=NULL;
				new_child->Next_Sibling=this_section_data->First_Child;
				if (this_section_data->First_Child) {
					this_section_data->First_Child->Prev_Sibling=new_child;
				}
				this_section_data->First_Child=new_child;
				Init_Sequence_Recursion(new_child, controller->Sequence_Type, controller->Sub_Sequence,controller->Seconds_For_Sequence);
				/* Prove new positions. */
				Process_Section(controller,new_child,&(this_section_data->World_Offset),&(this_section_data->SecMat),0,controller->Sequence_Type,controller->Sub_Sequence,0);
			}

			new_child_list_ptr++;
		}		
		/* Now, let's walk the old template. */
		new_child_list_ptr=new_template->Children;
		while (*old_child_list_ptr!=NULL) {

			SECTION *corresponding_section;

			corresponding_section=Get_Corresponding_Section(new_child_list_ptr,(*old_child_list_ptr)->Section_Name);

			if (corresponding_section!=NULL) {
				/* Section also exists in new template.  Do nothing, it should already have been dealt with. */
			} else {
				/* No corresponding new section: delete this branch. */
				SECTION_DATA *superfluous_section;

				superfluous_section=GetThisSectionData_FromChildrenOnly(this_section_data,(*old_child_list_ptr)->Section_Name);
				if (superfluous_section) {
					if (frag) {
						MakeHierarchicalDebris(sbPtr,superfluous_section, &superfluous_section->World_Offset, &superfluous_section->SecMat,NULL,2);
						Prune_Section(superfluous_section);
					} else {
						Prune_Section(superfluous_section);
					}
				
				}
			}

			old_child_list_ptr++;
		}		
		

	} else if (new_template->Children!=NULL) {
		
		if (newsections) {
			/* A whole lotta new branches. */
			SECTION_DATA *new_child;
			SECTION **new_child_list_ptr;
			
			new_child_list_ptr=new_template->Children;
			
			while (*new_child_list_ptr!=NULL) {
				
				new_child=Create_New_Section(*new_child_list_ptr);
			
				new_child->My_Parent=this_section_data;
				new_child->Prev_Sibling=NULL;
				new_child->Next_Sibling=this_section_data->First_Child;
				if (this_section_data->First_Child) {
					this_section_data->First_Child->Prev_Sibling=new_child;
				}
				this_section_data->First_Child=new_child;
			
				Init_Sequence_Recursion(new_child, controller->Sequence_Type, controller->Sub_Sequence,controller->Seconds_For_Sequence);
				
				new_child_list_ptr++;
			}		
		}
	} else if (old_template->Children!=NULL) {
		/* Remove all branches. */
		SECTION_DATA *data_child_ptr;
	
		data_child_ptr=this_section_data->First_Child;
	
		while (data_child_ptr!=NULL) {

			SECTION_DATA *next_one;

			LOCALASSERT(data_child_ptr->My_Parent==this_section_data);
			next_one=data_child_ptr->Next_Sibling;
			if (frag) {
				MakeHierarchicalDebris(sbPtr,data_child_ptr, &data_child_ptr->World_Offset, &data_child_ptr->SecMat,NULL,2);
				Prune_Section(data_child_ptr);
			} else {
				Prune_Section(data_child_ptr);
			}
			data_child_ptr=next_one;
		}

	} else {
		/* Null case. */
	}

}

void Transmogrify_HModels(STRATEGYBLOCK *sbPtr,HMODELCONTROLLER *controller,SECTION *new_template, int frag, int newsections, int regrowsections) {

	SECTION *old_template;

	/* Convert one HModel to another template... */
	Global_Controller_Ptr=controller;

	old_template=controller->Root_Section;

	/* Compare the two templates to each other. */

	GLOBALASSERT(controller->section_data->sempai==old_template);

	controller->section_data->sempai=new_template;

	Transmogrification_Recursion(sbPtr,controller,controller->section_data,new_template,old_template,frag,newsections,regrowsections);

	controller->Root_Section=new_template;	
}

void TrimToTemplate_Recursion(STRATEGYBLOCK *sbPtr,HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,SECTION *new_template, SECTION *old_template, int frag) {
	
	/* Doesn't really matter which tree we're walking... does it? */
	
	GLOBALASSERT(new_template);
	GLOBALASSERT(old_template);
	GLOBALASSERT(strcmp(new_template->Section_Name,old_template->Section_Name)==0);

	GLOBALASSERT(this_section_data);

	if ( (new_template->Children!=NULL) && (old_template->Children!=NULL) )  {
		/* Complex.  I'd really like to walk both at the same time. */
		SECTION **new_child_list_ptr,**old_child_list_ptr;

		new_child_list_ptr=new_template->Children;
		old_child_list_ptr=old_template->Children;

		/* Let's walk the old template. */
		while (*old_child_list_ptr!=NULL) {

			SECTION *corresponding_section;

			corresponding_section=Get_Corresponding_Section(new_child_list_ptr,(*old_child_list_ptr)->Section_Name);

			if (corresponding_section!=NULL) {
				/* Section also exists in new template.  Recurse. */
				SECTION_DATA *child_section_data;

				child_section_data=GetThisSectionData_FromChildrenOnly(this_section_data,(*old_child_list_ptr)->Section_Name);

				if (child_section_data) {
					/* Hey, it might be fragged off.  Now deal with it. */

					TrimToTemplate_Recursion(sbPtr,controller,child_section_data,corresponding_section,*old_child_list_ptr, frag);
				}
			} else {
				/* No corresponding new section: delete this branch. */
				SECTION_DATA *superfluous_section;

				superfluous_section=GetThisSectionData_FromChildrenOnly(this_section_data,(*old_child_list_ptr)->Section_Name);
				if (superfluous_section) {
					if (frag) {
						MakeHierarchicalDebris(sbPtr,superfluous_section, &superfluous_section->World_Offset, &superfluous_section->SecMat,NULL,2);
						Prune_Section(superfluous_section);
					} else {
						Prune_Section(superfluous_section);
					}
				
				}
			}

			old_child_list_ptr++;
		}		

	} else if (old_template->Children!=NULL) {
		/* Remove all branches. */
		SECTION_DATA *data_child_ptr;
	
		data_child_ptr=this_section_data->First_Child;
	
		while (data_child_ptr!=NULL) {

			SECTION_DATA *next_one;

			LOCALASSERT(data_child_ptr->My_Parent==this_section_data);
			next_one=data_child_ptr->Next_Sibling;
			if (frag) {
				MakeHierarchicalDebris(sbPtr,data_child_ptr, &data_child_ptr->World_Offset, &data_child_ptr->SecMat,NULL,2);
				Prune_Section(data_child_ptr);
			} else {
				Prune_Section(data_child_ptr);
			}
			data_child_ptr=next_one;
		}

	} else {
		/* Null case. */
	}

}

void TrimToTemplate(STRATEGYBLOCK *sbPtr,HMODELCONTROLLER *controller,SECTION *new_template, int frag) {

	SECTION *old_template;

	/* Convert one HModel to another template... */

	old_template=controller->Root_Section;

	/* Compare the two templates to each other. */

	GLOBALASSERT(controller->section_data->sempai==old_template);
	if (strcmp(new_template->Section_Name,old_template->Section_Name)) {
		GLOBALASSERT(strcmp(new_template->Section_Name,old_template->Section_Name)==0);
	}
	TrimToTemplate_Recursion(sbPtr,controller,controller->section_data,new_template,old_template,frag);

}

int HModelSequence_Exists(HMODELCONTROLLER *controller,int sequence_type,int sub_sequence) {

	int sequence_id,a;
	SEQUENCE *sequence_pointer;

	sequence_id=GetSequenceID(sequence_type,sub_sequence);

	for (a=0; a<controller->Root_Section->num_sequences; a++) {
		if (controller->Root_Section->sequence_array[a].sequence_id==sequence_id) {
			sequence_pointer=&(controller->Root_Section->sequence_array[a]);
			break;
		}
	}
	
	if (a==controller->Root_Section->num_sequences) {
		/* No such animal. */
		return(0);
	} else {
		return(1);
	}

}

int HModelSequence_Exists_FromRoot(SECTION *root,int sequence_type,int sub_sequence) {

	int sequence_id,a;
	SEQUENCE *sequence_pointer;

	if (!root) {
		return(0);
	}

	sequence_id=GetSequenceID(sequence_type,sub_sequence);

	for (a=0; a<root->num_sequences; a++) {
		if (root->sequence_array[a].sequence_id==sequence_id) {
			sequence_pointer=&(root->sequence_array[a]);
			break;
		}
	}
	
	if (a==root->num_sequences) {
		/* No such animal. */
		return(0);
	} else {
		return(1);
	}

}

void KRS_Recursion(SECTION_DATA *this_section_data, int probability) {

	/* General gibbing function. */


	/* Recurse. */

	if ( (this_section_data->First_Child!=NULL) 
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {


		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=this_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {

			if ( (child_list_ptr->flags&section_data_terminate_here)==0) {
							
				/* Right. Roll some dice... */
				if ( (SeededFastRandom()&65535)<probability) {
					/* Kill this bit... */

					child_list_ptr->current_damage.Health=0;
					
					KRS_Recursion(child_list_ptr,probability);			
					
				} else {
					
					KRS_Recursion(child_list_ptr,probability);
				
				}
			}	
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

}

void KillRandomSections(SECTION_DATA *this_section_data, int probability) {

	/* A bit like gibbing, but less extreme. */

	KRS_Recursion(this_section_data,probability);

}

void Budge_Recursion(SECTION_DATA *this_section_data,VECTORCH *offset) {
	
	SECTION_DATA *sdptr;

	sdptr=NULL;

	/* Budge! */
	this_section_data->World_Offset.vx+=offset->vx;
	this_section_data->World_Offset.vy+=offset->vy;
	this_section_data->World_Offset.vz+=offset->vz;

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			Budge_Recursion(child_list_ptr,offset);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return;

}

void Budge_HModel(HMODELCONTROLLER *controller,VECTORCH *offset) {

	/* Shift a model. */

	if ((offset==NULL)||(controller==NULL)) {
		return;
	}

	/* */

	Budge_Recursion(controller->section_data,offset);

}

void HModelRegen_Recursion(SECTION_DATA *this_section_data, int time) {
	
	SECTION_DATA *sdptr;
	int health_increment;

	sdptr=NULL;

	/* Regenerate this section. */
	if (this_section_data->current_damage.Health>0) {
	
		health_increment=DIV_FIXED((this_section_data->sempai->StartingStats.Health*NormalFrameTime),time);
		this_section_data->current_damage.Health+=health_increment;	
		
		if (this_section_data->current_damage.Health>(this_section_data->sempai->StartingStats.Health<<ONE_FIXED_SHIFT)) {
			this_section_data->current_damage.Health=(this_section_data->sempai->StartingStats.Health<<ONE_FIXED_SHIFT);
		}
	}

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			HModelRegen_Recursion(child_list_ptr,time);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return;

}

void HModel_Regen(HMODELCONTROLLER *controller,int time) {

	/* Regenerate sections. */

	HModelRegen_Recursion(controller->section_data,time);

}

int HModelAnimation_IsFinished(HMODELCONTROLLER *controller) {

	/* This now gets used all over the place... */

	if (controller->Tweening!=Controller_NoTweening) {
		return(0);
	}
	if (controller->Looped) {
		return(0);
	}
	if (controller->Reversed) {
		if (controller->sequence_timer!=0) {
			return(0);
		}
	} else {
		if (controller->sequence_timer!=(ONE_FIXED-1)) {
			return(0);
		}
	}
	return(1);
}

int DeltaAnimation_IsFinished(DELTA_CONTROLLER *controller) {

	if (controller->Looped) {
		return(0);
	}
	if (controller->Playing==0) {
		return(1);
	}
	if (controller->Active==0) {
		return(1);
	}
	if (controller->timer!=(ONE_FIXED-1)) {
		return(0);
	}
	return(1);
}

SECTION_DATA *PointInHModel_Recursion(SECTION_DATA *this_section_data, VECTORCH *point) {
	
	SECTION_DATA *hit;

	hit=NULL;

	/* Test this section. */
	
	/* Use shape data. */
	if (this_section_data->Shape!=NULL) {
		VECTORCH offset;
		int dist;

		offset.vx=this_section_data->World_Offset.vx-point->vx;
		offset.vy=this_section_data->World_Offset.vy-point->vy;
		offset.vz=this_section_data->World_Offset.vz-point->vz;
		dist=Approximate3dMagnitude(&offset);
		if (dist<this_section_data->Shape->shaperadius) {
			/* Hit! */
			hit=this_section_data;
		}
	}
	
	if (hit) {
		return(hit);
	}
	
	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			hit=PointInHModel_Recursion(child_list_ptr,point);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return(hit);

}

SECTION_DATA *PointInHModel(HMODELCONTROLLER *controller,VECTORCH *point) {

	/* Test for point in model. */

	return(PointInHModel_Recursion(controller->section_data,point));

}

SECTION *Get_Corresponding_Section_Recursive(SECTION *this_section,char *Name) {

	SECTION **child_list_ptr;

	if (strcmp(this_section->Section_Name,Name)==0) {
		return(this_section);
	}

	/* Recurse. */
	child_list_ptr=this_section->Children;
	
	if (child_list_ptr) {
		while (*child_list_ptr!=NULL) {
			SECTION *cosec;
	
			cosec=Get_Corresponding_Section_Recursive(*child_list_ptr,Name);
			if (cosec) {
				/* Back out! */
				return(cosec);
			}
			child_list_ptr++;
		}
	}

	/* No luck. */
	return(NULL);
}

SECTION_DATA *GetSectionFromID_Recursion(SECTION_DATA *this_section_data,int IDnumber) {
	
	SECTION_DATA *sdptr;

	sdptr=NULL;

	if (this_section_data->sempai->IDnumber==IDnumber) {
		/* We are that section! */
		return(this_section_data);	
	}

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			sdptr=GetSectionFromID_Recursion(child_list_ptr,IDnumber);
			child_list_ptr=child_list_ptr->Next_Sibling;
			
			if (sdptr) {
				return(sdptr); /* We got one! */
			}
		}
	}
	
	return(sdptr);

}

SECTION_DATA *GetThisSectionData_FromID(SECTION_DATA *root,int IDnumber) {

	if (root==NULL) {
		return(NULL);
	}

	return(GetSectionFromID_Recursion(root,IDnumber));

}

SECTION *GetThisSection_FromID(SECTION *this_section,int IDnumber)
{
	if (this_section==NULL) {
		return(NULL);
	}

	//is this the section that we're looking for
	if(this_section->IDnumber == IDnumber) return this_section;

	//try this section's children then
	if(this_section->Children)
	{
		SECTION **child_list_ptr = this_section->Children;

		while(*child_list_ptr)
		{
			SECTION* return_section = GetThisSection_FromID(*child_list_ptr,IDnumber);
			if(return_section) return return_section;	
			child_list_ptr++;
		}
	}

	//out of luck
	return NULL;
}


void PlayHierarchySound(HIERARCHY_SOUND* sound,VECTORCH* location)
{
	GLOBALASSERT(sound);
	GLOBALASSERT(location);

	sound->s3d.position=*location;
	
	if (!Global_VDB_Ptr) {
		return;
	}

	/* Marine_ignore, to stop them getting alarmed by their own footsteps! */
   	Sound_Play (sound->sound_index, "nvpm", &sound->s3d,sound->volume,sound->pitch);
}

void Init_Tweening_ToTheMiddle_Recursion(SECTION_DATA *this_section_data, int target_sequence_type,int target_subsequence, int seconds_for_tweening, int target_sequence_timer, int backwards) {

	SEQUENCE *sequence_ptr;

	/* Firstly, store current state. */

	this_section_data->stored_offset=this_section_data->Offset;
	MatToQuat(&this_section_data->RelSecMat,&this_section_data->stored_quat);

	/* Now, get target state. */
	
	sequence_ptr=GetSequencePointer(target_sequence_type,target_subsequence,this_section_data->sempai);

	/* Deduce target positions.  Backwards flag is irrelevant... */
	
	{
		int a,working_timer,lerp;
		KEYFRAME_DATA *this_frame,*next_frame;
	
		this_frame=sequence_ptr->first_frame;
		GLOBALASSERT(this_frame);
		working_timer=target_sequence_timer;

		a=0; /* Status flag... */

		while (a==0) {
			if (this_frame->last_frame) {
				/* Low framerate loop? */
				next_frame=sequence_ptr->first_frame;
			}
			else{
				next_frame=this_frame->Next_Frame;
			}
			if (working_timer>=this_frame->Sequence_Length) {
				/* We've gone beyond this frame: get next keyframe. */
				working_timer-=this_frame->Sequence_Length;
				/* Advance frame... */
				this_frame=next_frame;
			} else {
				a=1; /* Exit loop with success. */
			}	
			/* Better make sure the last 'frame' has 65536 length... */
		}
		GLOBALASSERT(working_timer>=0);
		/* Now we should have a frame and a timer. */
		lerp=MUL_FIXED(working_timer,this_frame->oneoversequencelength);

		GetKeyFrameOffset(this_frame,&this_section_data->target_offset);
		
		if(next_frame->shift_offset)
		{
			VECTORCH next_offset;
			GetKeyFrameOffset(next_frame,&next_offset);
			this_section_data->target_offset.vx+=MUL_FIXED(next_offset.vx - this_section_data->target_offset.vx,lerp);
			this_section_data->target_offset.vy+=MUL_FIXED(next_offset.vy - this_section_data->target_offset.vy,lerp);
			this_section_data->target_offset.vz+=MUL_FIXED(next_offset.vz - this_section_data->target_offset.vz,lerp);
		}
		else
		{
			this_section_data->target_offset.vx+=MUL_FIXED((int)next_frame->Offset_x - this_section_data->target_offset.vx,lerp);
			this_section_data->target_offset.vy+=MUL_FIXED((int)next_frame->Offset_y - this_section_data->target_offset.vy,lerp);
			this_section_data->target_offset.vz+=MUL_FIXED((int)next_frame->Offset_z - this_section_data->target_offset.vz,lerp);
		}

		/* Now deal with orientation. */

		Slerp(this_frame,lerp,&this_section_data->target_quat);

	}

	/* Preprocess slerp values. */

	{

		this_section_data->delta_offset.vx=this_section_data->target_offset.vx-this_section_data->stored_offset.vx;
		this_section_data->delta_offset.vy=this_section_data->target_offset.vy-this_section_data->stored_offset.vy;
		this_section_data->delta_offset.vz=this_section_data->target_offset.vz-this_section_data->stored_offset.vz;

	}

	{
		int cosom,sinom;
		QUAT *this_quat, *next_quat;

		this_quat=&this_section_data->stored_quat;
		next_quat=&this_section_data->target_quat;

		
		cosom=QDot(this_quat,next_quat);
		
		if (cosom<0) {
			next_quat->quatx=-next_quat->quatx;
			next_quat->quaty=-next_quat->quaty;
			next_quat->quatz=-next_quat->quatz;
			next_quat->quatw=-next_quat->quatw;
			cosom=-cosom;
		}
	
	
		this_section_data->omega=ArcCos(cosom);
		sinom=GetSin(this_section_data->omega);
		if (sinom) {
			this_section_data->oneoversinomega=GetOneOverSin(this_section_data->omega);
		} else {
			/* Yuk. */
			this_section_data->omega=0;
			this_section_data->oneoversinomega=0;
		}
	
		GLOBALASSERT(seconds_for_tweening>0);
		this_section_data->oneovertweeninglength=DIV_FIXED(ONE_FIXED,seconds_for_tweening);

	}

	/* Init fields... I guess. */

	this_section_data->current_sequence=sequence_ptr;
	this_section_data->current_keyframe=sequence_ptr->first_frame;
	this_section_data->accumulated_timer=0;
	this_section_data->freezeframe_timer=-1;
	this_section_data->lastframe_timer=0;
	this_section_data->gore_timer=0; /* As good a time as any. */

	this_section_data->Tweening=1;

	/* Animation? */

	/* Nah. */

	/* Recurse. */

	if ( (this_section_data->First_Child!=NULL)
		&&( (this_section_data->flags&section_data_terminate_here)==0)) {
		/* Respect the terminator! */

		SECTION_DATA *child_list_ptr;
	
		child_list_ptr=this_section_data->First_Child;
	
		while (child_list_ptr!=NULL) {
			Init_Tweening_ToTheMiddle_Recursion(child_list_ptr,target_sequence_type,target_subsequence,seconds_for_tweening,target_sequence_timer,backwards);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}

	}

}


void InitHModelTweening_ToTheMiddle(HMODELCONTROLLER *controller, int seconds_for_tweening,
	int target_sequence_type, int target_subsequence, int target_seconds_for_sequence, int target_sequence_timer, int loop) {

	/* Just set it up... */
	GLOBALASSERT(target_seconds_for_sequence);

	controller->Sequence_Type=target_sequence_type;
	controller->Sub_Sequence=target_subsequence;
	controller->Seconds_For_Sequence=seconds_for_tweening;
	controller->timer_increment=DIV_FIXED(ONE_FIXED,controller->Seconds_For_Sequence);
	controller->sequence_timer=0;
	controller->Playing=1;
	controller->Reversed=0;
	controller->Looped=1;

	controller->After_Tweening_Sequence_Type=target_sequence_type;
	controller->After_Tweening_Sub_Sequence=target_subsequence;
	controller->AT_seconds_for_sequence=target_seconds_for_sequence;

	controller->AT_sequence_timer=target_sequence_timer;
	while (controller->AT_sequence_timer>=ONE_FIXED) {
		controller->AT_sequence_timer-=ONE_FIXED;
	}

	controller->Tweening=Controller_Tweening;
	if (loop) {
		controller->LoopAfterTweening=1;
	} else {
		controller->LoopAfterTweening=0;
	}
	controller->StopAfterTweening=0;
	controller->ElevationTweening=0;

	/* Recurse though hierarchy, setting up all the section_data sequence stores? */

	Init_Tweening_ToTheMiddle_Recursion(controller->section_data, target_sequence_type, target_subsequence,seconds_for_tweening,target_sequence_timer,0);

}

void Verify_Positions_Recursion(HMODELCONTROLLER *controller,SECTION_DATA *this_section_data,VECTORCH *parent_position,char *callCode) {
	
	/* Verify positions... */

	if ( !(	(this_section_data->World_Offset.vx<1000000 && this_section_data->World_Offset.vx>-1000000)
		 &&	(this_section_data->World_Offset.vy<1000000 && this_section_data->World_Offset.vy>-1000000)
		 &&	(this_section_data->World_Offset.vz<1000000 && this_section_data->World_Offset.vz>-1000000) 
		 ) ) {
	
		LOGDXFMT(("Tests in VERIFY_POSITIONS_RECURSION.\n"));
		if (callCode) {
			LOGDXFMT(("Call code %s\n",callCode));
		} else {
			LOGDXFMT(("No call code!\n"));
		}
		if (Global_HModel_Sptr) {
			LOGDXFMT(("Misplaced object is of type %d\n",Global_HModel_Sptr->I_SBtype));
			if (Global_HModel_Sptr->SBdptr) {
				LOGDXFMT(("Object is Near.\n"));
			} else {
				LOGDXFMT(("Object is Far.\n"));
			}
		} else {
			LOGDXFMT(("Misplaced object has no SBptr.\n"));
		}
		LOGDXFMT(("Name of section: %s\n",this_section_data->sempai->Section_Name));
		LOGDXFMT(("It was playing sequence: %d,%d\n",controller->Sequence_Type,controller->Sub_Sequence));
		LOGDXFMT(("Sequence Timer = %d\n",controller->sequence_timer));
		LOGDXFMT(("Tweening flags %d\n",controller->Tweening));

		LOGDXFMT(("Parent Position %d,%d,%d\n",parent_position->vx,parent_position->vy,parent_position->vz));
		LOGDXFMT(("This Position %d,%d,%d\n",this_section_data->World_Offset.vx,this_section_data->World_Offset.vy,this_section_data->World_Offset.vz));

		LOCALASSERT(this_section_data->World_Offset.vx<1000000 && this_section_data->World_Offset.vx>-1000000);
		LOCALASSERT(this_section_data->World_Offset.vy<1000000 && this_section_data->World_Offset.vy>-1000000);
		LOCALASSERT(this_section_data->World_Offset.vz<1000000 && this_section_data->World_Offset.vz>-1000000);
		
	}

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			Verify_Positions_Recursion(controller,child_list_ptr,&this_section_data->World_Offset,callCode);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return;

}

void Verify_Positions_In_HModel(STRATEGYBLOCK *sbPtr,HMODELCONTROLLER *controller,char *callCode) {

	/* Verify position integrity. */

	if ((controller==NULL)||(sbPtr==NULL)) {
		return;
	}

	/* */

	Global_HModel_Sptr=sbPtr;
	Global_Controller_Ptr=controller;
	Global_HModel_DispPtr=sbPtr->SBdptr;

	Verify_Positions_Recursion(controller,controller->section_data,&sbPtr->DynPtr->Position,callCode);

}




void CopyShortQuatToInt(QUAT_SHORT* qs,QUAT* q)
{
	q->quatx=((int)qs->quatx)<<1;
	q->quaty=((int)qs->quaty)<<1;
	q->quatz=((int)qs->quatz)<<1;
	q->quatw=((int)qs->quatw)<<1;
}


void CopyIntQuatToShort(QUAT* q,QUAT_SHORT* qs)
{
	if(q->quatx>65535) qs->quatx=32767;
	else if(q->quatx<-65536) qs->quatx=-32768;
	else qs->quatx=q->quatx>>1;

	if(q->quaty>65535) qs->quaty=32767;
	else if(q->quaty<-65536) qs->quaty=-32768;
	else qs->quaty=q->quaty>>1;

	if(q->quatz>65535) qs->quatz=32767;
	else if(q->quatz<-65536) qs->quatz=-32768;
	else qs->quatz=q->quatz>>1;

	if(q->quatw>65535) qs->quatw=32767;
	else if(q->quatw<-65536) qs->quatw=-32768;
 	else qs->quatw=q->quatw>>1;

}

void GetKeyFrameOffset(KEYFRAME_DATA* frame,VECTORCH* output_vector)
{
	if(frame->shift_offset)
	{
		output_vector->vx=((int)frame->Offset_x)<<KEYFRAME_VECTOR_SHIFT;
		output_vector->vy=((int)frame->Offset_y)<<KEYFRAME_VECTOR_SHIFT;
		output_vector->vz=((int)frame->Offset_z)<<KEYFRAME_VECTOR_SHIFT;
	}
	else
	{
		output_vector->vx=(int)frame->Offset_x;
		output_vector->vy=(int)frame->Offset_y;
		output_vector->vz=(int)frame->Offset_z;
	}
}


void SetKeyFrameOffset(KEYFRAME_DATA* frame,VECTORCH* input_vector)
{
	if(input_vector->vx>=-32768 && input_vector->vx<=32767 &&
	   input_vector->vy>=-32768 && input_vector->vy<=32767 &&
	   input_vector->vz>=-32768 && input_vector->vz<=32767)
	{
		frame->Offset_x=(short)input_vector->vx;
		frame->Offset_y=(short)input_vector->vy;
		frame->Offset_z=(short)input_vector->vz;

		frame->shift_offset=0;
	}
	else
	{
		frame->Offset_x=(short)(input_vector->vx >> KEYFRAME_VECTOR_SHIFT);
		frame->Offset_y=(short)(input_vector->vy >> KEYFRAME_VECTOR_SHIFT);
		frame->Offset_z=(short)(input_vector->vz >> KEYFRAME_VECTOR_SHIFT);
		
		frame->shift_offset=1;

	}
}

int HModelDepthTest_Recursion(SECTION_DATA *this_section_data, SECTION_DATA *test_section_data,int depth) {
	
	SECTION_DATA *sdptr;
	int totals;

	sdptr=NULL;
	totals=0;

	/* Return if too deep. */
	if (depth<0) {
		return(0);
	}
	
	/* Test this section? */
	if (this_section_data==test_section_data) {
		/* Success. */
		return(1);
	}

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			totals+=HModelDepthTest_Recursion(child_list_ptr,test_section_data,depth-1);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return(totals);

}

int HModel_DepthTest(HMODELCONTROLLER *controller,SECTION_DATA *test_section_data,int depth) {

	/* Regenerate sections. */

	return(HModelDepthTest_Recursion(controller->section_data,test_section_data,depth));

}

void DeInitialise_Recursion(SECTION_DATA *this_section_data) {
	
	SECTION_DATA *sdptr;

	sdptr=NULL;

	this_section_data->flags=this_section_data->flags&(~section_data_initialised);

	/* Now call recursion... */

	if (this_section_data->First_Child!=NULL) {
		
		SECTION_DATA *child_list_ptr;

		child_list_ptr=this_section_data->First_Child;

		while (child_list_ptr!=NULL) {
			DeInitialise_Recursion(child_list_ptr);
			child_list_ptr=child_list_ptr->Next_Sibling;
		}
	}
	
	return;

}

void DeInitialise_HModel(HMODELCONTROLLER *controller) {

	/* Recursively set all 'initialised' flags to zero. */

	if (controller==NULL) {
		return;
	}

	DeInitialise_Recursion(controller->section_data);

}


static void EnsureChildrenAreInAscendingIDOrder(SECTION_DATA* section)
{
	/*
	This checks all the children of a section to make sure that they are placed in ascending Id order.
	This is needed for the saving and loading process to work properly.
	(In the majority of cases, sections will be in the right order , so this should be fairly quick)
	*/
	
	SECTION_DATA* child_section = section->First_Child;
	
	if(!child_section) return;

	while(child_section->Next_Sibling)
	{
		SECTION_DATA* next_section = child_section->Next_Sibling;
		if(next_section->sempai->IDnumber<child_section->sempai->IDnumber)
		{
			/*
			These two sections are out of order , so we need to swap them
			*/
			
			
			/*First correct the children before and adter the ones being swapped*/
			
			if(child_section->Prev_Sibling)
			{
				child_section->Prev_Sibling->Next_Sibling = next_section;
			}
			if(next_section->Next_Sibling)
			{
				next_section->Next_Sibling->Prev_Sibling = child_section;
			}
			
			/*
			If we are swapping the first child , then we need to alter the parent's pointer
			*/
			if(section->First_Child == child_section)
			{
				section->First_Child = next_section;
			}


			/*
			Give the sections being swapped the pointers to the sections before and after them
			*/
			child_section->Next_Sibling = next_section->Next_Sibling;
			next_section->Prev_Sibling = child_section->Prev_Sibling;

			
			child_section->Prev_Sibling = next_section;
			next_section->Next_Sibling = child_section;


			/*The next section we will have to consider is the one we have just swapped ealier in the list*/
			child_section = next_section;

			/*
			If the section has a previous sibling then we need to look back at that one , since the previous sibling's
			if could be higher than that of the section we have just swapped
			*/

			if(child_section->Prev_Sibling) child_section = child_section->Prev_Sibling;
															
		}
		else
		{
			//No problem with this section , go on to the next
			child_section = child_section->Next_Sibling;
		}
	}
}

/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"

static void LoadHierarchySection(SECTION_DATA* section);
static void SaveHierarchySectionRecursion(SECTION_DATA* section);

static void LoadHierarchySectionDecals(SAVE_BLOCK_HEADER* header,SECTION_DATA* section);
static void SaveHierarchySectionDecals(SECTION_DATA* section);

static void LoadHierarchySectionTween(SAVE_BLOCK_HEADER* header,SECTION_DATA* section);
static void SaveHierarchySectionTween(SECTION_DATA* section);

static void LoadHierarchyDelta(SAVE_BLOCK_HEADER* header,HMODELCONTROLLER* controller);
static void SaveHierarchyDelta(DELTA_CONTROLLER* delta);

typedef struct hierarchy_save_block
{
	SAVE_BLOCK_HEADER header;
	
	int structure_size;

	int Seconds_For_Sequence;
	int timer_increment;
	int Sequence_Type;
	int	Sub_Sequence;
	int sequence_timer;
	int FrameStamp;
	VECTORCH Computed_Position;

	int keyframe_flags;

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

	int root_section_id;
	
}HIERARCHY_SAVE_BLOCK;


//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV controller


void LoadHierarchy(SAVE_BLOCK_HEADER* header,HMODELCONTROLLER* controller)
{
	SECTION *root_section;
	const char *Rif_Name;
	const char *Hierarchy_Name;
	HIERARCHY_SAVE_BLOCK* block = (HIERARCHY_SAVE_BLOCK*) header;

	//make sure the block is the correct size
	if(block->structure_size != sizeof(*block)) return;

	//get the names from just after the block
	{
		char* buffer=(char*) header;
		buffer+=sizeof(*block);

		Hierarchy_Name = buffer;
		buffer+= strlen(Hierarchy_Name)+1;
		Rif_Name = buffer;
  	}
	
	{
		BOOL needToCreateHModel = FALSE;

		//make sure that the initial model has been set up , and is using the correct hierarchy
		if(controller->Root_Section)
		{
			if(strcmp(Rif_Name,controller->Root_Section->Rif_Name) || strcmp(Hierarchy_Name,controller->Root_Section->Hierarchy_Name))
			{
				needToCreateHModel = TRUE;
			}
			else if(controller->Root_Section->IDnumber != block->root_section_id)
			{
				needToCreateHModel = TRUE;
			}
		}
		else
		{
			needToCreateHModel = TRUE;
		}
		
		//check to see if we need to change hierarchy
		if(needToCreateHModel)
		{
			extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
			
			root_section = GetNamedHierarchyFromLibrary(Rif_Name,Hierarchy_Name);
			if(!root_section) return;

			//may not want the 'real' root (hierarchical debris)
			root_section = GetThisSection_FromID(root_section,block->root_section_id);
			if(!root_section) return;

							
			//reinitialise the hierarchy
			Dispel_HModel(controller);
			Create_HModel(controller,root_section);
		}
	}	

	//copy the stuff for the controller
	COPYELEMENT_LOAD(Seconds_For_Sequence)
	COPYELEMENT_LOAD(timer_increment)
	COPYELEMENT_LOAD(Sequence_Type)
	COPYELEMENT_LOAD(Sub_Sequence)
	COPYELEMENT_LOAD(sequence_timer)
	COPYELEMENT_LOAD(FrameStamp)
	COPYELEMENT_LOAD(Computed_Position)
	COPYELEMENT_LOAD(keyframe_flags)
	COPYELEMENT_LOAD(After_Tweening_Sequence_Type)
	COPYELEMENT_LOAD(After_Tweening_Sub_Sequence)
	COPYELEMENT_LOAD(AT_seconds_for_sequence)
	COPYELEMENT_LOAD(AT_sequence_timer)
	COPYELEMENT_LOAD(Playing)
	COPYELEMENT_LOAD(Reversed)
	COPYELEMENT_LOAD(Looped)
	COPYELEMENT_LOAD(Tweening)
	COPYELEMENT_LOAD(LoopAfterTweening)
	COPYELEMENT_LOAD(StopAfterTweening)
	COPYELEMENT_LOAD(ElevationTweening)
	COPYELEMENT_LOAD(DisableBleeding)
	COPYELEMENT_LOAD(LockTopSection)
	COPYELEMENT_LOAD(ZeroRootDisplacement)
	COPYELEMENT_LOAD(ZeroRootRotation)
	COPYELEMENT_LOAD(DisableSounds);

	
	//load the delta sequences
	{
		SAVE_BLOCK_HEADER* delta_header;

		while((delta_header = GetNextBlockIfOfType(SaveBlock_HierarchyDelta)))
		{
			LoadHierarchyDelta(delta_header,controller);
		}
		
	}
	
	///load the section data
	LoadHierarchySection(controller->section_data);

}


void SaveHierarchy(HMODELCONTROLLER* controller)
{
	HIERARCHY_SAVE_BLOCK* block;
	if(!controller) return;
	if(!controller->Root_Section) return;

	GET_SAVE_BLOCK_POINTER(block);
	
	//fill in header
	block->header.type = SaveBlock_Hierarchy;	
	block->header.size = sizeof(*block);
	block->structure_size =block->header.size;

	COPYELEMENT_SAVE(Seconds_For_Sequence)
	COPYELEMENT_SAVE(timer_increment)
	COPYELEMENT_SAVE(Sequence_Type)
	COPYELEMENT_SAVE(Sub_Sequence)
	COPYELEMENT_SAVE(sequence_timer)
	COPYELEMENT_SAVE(FrameStamp)
	COPYELEMENT_SAVE(Computed_Position)
	COPYELEMENT_SAVE(keyframe_flags)
	COPYELEMENT_SAVE(After_Tweening_Sequence_Type)
	COPYELEMENT_SAVE(After_Tweening_Sub_Sequence)
	COPYELEMENT_SAVE(AT_seconds_for_sequence)
	COPYELEMENT_SAVE(AT_sequence_timer)
	COPYELEMENT_SAVE(Playing)
	COPYELEMENT_SAVE(Reversed)
	COPYELEMENT_SAVE(Looped)
	COPYELEMENT_SAVE(Tweening)
	COPYELEMENT_SAVE(LoopAfterTweening)
	COPYELEMENT_SAVE(StopAfterTweening)
	COPYELEMENT_SAVE(ElevationTweening)
	COPYELEMENT_SAVE(DisableBleeding)
	COPYELEMENT_SAVE(LockTopSection)
	COPYELEMENT_SAVE(ZeroRootDisplacement)
	COPYELEMENT_SAVE(ZeroRootRotation)
	COPYELEMENT_SAVE(DisableSounds);

	block->root_section_id = controller->Root_Section->IDnumber;

	{
		char* buffer;
		char* Hierarchy_Name = controller->Root_Section->Hierarchy_Name;
		char* Rif_Name = controller->Root_Section->Rif_Name;

		//increase the block size by enough to hold these names
		block->header.size+= strlen(Hierarchy_Name)+strlen(Rif_Name)+2;

		//alllocate memoey , and copy names;
		buffer = GetPointerForSaveBlock(strlen(Hierarchy_Name)+1);
		strcpy(buffer,Hierarchy_Name);

		buffer = GetPointerForSaveBlock(strlen(Rif_Name)+1);
		strcpy(buffer,Rif_Name);
		
	}

	//save the delta sequences
	{
		DELTA_CONTROLLER* delta = controller->Deltas; 
		
		while(delta)
		{
			SaveHierarchyDelta(delta);		
			delta=delta->next_controller;
		}
	}
	
	//now save the section data
	SaveHierarchySectionRecursion(controller->section_data);
}

typedef struct hierarchy_delta_save_block
{
	SAVE_BLOCK_HEADER header;

	int timer;
	int lastframe_timer;
	int sequence_type;
	int sub_sequence;
	int seconds_for_sequence;
	int timer_increment;
	int Looped:1;
	int Playing:1;
	int Active:1;

}HIERARCHY_DELTA_SAVE_BLOCK;

#undef SAVELOAD_BEHAV
//defines for load/save macros
#define SAVELOAD_BEHAV delta

static void LoadHierarchyDelta(SAVE_BLOCK_HEADER* header,HMODELCONTROLLER* controller)
{
	DELTA_CONTROLLER* delta;
	HIERARCHY_DELTA_SAVE_BLOCK* block = (HIERARCHY_DELTA_SAVE_BLOCK*) header;	
	char* name = (char*) (block+1);

	delta = Get_Delta_Sequence(controller,name);
	if(!delta)
	{
		delta = Add_Delta_Sequence(controller,name,block->sequence_type,block->sub_sequence,block->seconds_for_sequence);
	}

	
	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(lastframe_timer)
	COPYELEMENT_LOAD(timer_increment)
	COPYELEMENT_LOAD(Looped)
	COPYELEMENT_LOAD(Playing)
	COPYELEMENT_LOAD(Active)
	COPYELEMENT_LOAD(sequence_type)
	COPYELEMENT_LOAD(sub_sequence)
	COPYELEMENT_LOAD(seconds_for_sequence)
	

	
	
}

static void SaveHierarchyDelta(DELTA_CONTROLLER* delta)
{
	HIERARCHY_DELTA_SAVE_BLOCK* block;
	int size;

	//work out memory needed
	size=sizeof(*block) + strlen(delta->id) +1;
	block = (HIERARCHY_DELTA_SAVE_BLOCK*) GetPointerForSaveBlock(size);

	//fill in the header
	block->header.size = size;
	block->header.type = SaveBlock_HierarchyDelta;

	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(lastframe_timer)
	COPYELEMENT_SAVE(timer_increment)
	COPYELEMENT_SAVE(Looped)
	COPYELEMENT_SAVE(Playing)
	COPYELEMENT_SAVE(Active)
	COPYELEMENT_SAVE(sequence_type)
	COPYELEMENT_SAVE(sub_sequence)
	COPYELEMENT_SAVE(seconds_for_sequence)

	//tack the name on the end
	{
		char* name = (char*)(block+1);
		strcpy(name,delta->id);
	}
	
	
}


typedef struct hierarchy_section_save_block
{
	SAVE_BLOCK_HEADER header;

//from section
	int IDnumber;

//from section_data	
	VECTORCH Offset;
	VECTORCH World_Offset;
	VECTORCH Last_World_Offset;
	MATRIXCH RelSecMat;
	MATRIXCH SecMat;
	
	struct damageblock current_damage;

	int accumulated_timer;
	int freezeframe_timer;
	int lastframe_timer;
	int gore_timer;

	int flags;

	int sequence_id;
	int keyframe_time;


	int replacement_id;
	
}HIERARCHY_SECTION_SAVE_BLOCK;


#undef SAVELOAD_BEHAV
//defines for load/save macros
#define SAVELOAD_BEHAV section

extern HIERARCHY_SHAPE_REPLACEMENT* GetHierarchyAlternateShapeFromId(const char* rif_name,int replacement_id,char* section_name);


static void LoadHierarchySection(SECTION_DATA* section)
{
	SAVE_BLOCK_HEADER* header;
	SAVE_BLOCK_HEADER* decal_header;
	SAVE_BLOCK_HEADER* tween_header;

	HIERARCHY_SECTION_SAVE_BLOCK* block;

	header=GetNextBlockIfOfType(SaveBlock_HierarchySection);
	decal_header=GetNextBlockIfOfType(SaveBlock_HierarchyDecals);
	tween_header=GetNextBlockIfOfType(SaveBlock_HierarchyTween);

	/*
	In this bit we go through the hierechy section data , and saved section data in increasing
	ID number order. Whenever we find any sections without saved data , we need to prunce them
	(since they will have been blown off the original hierarchy)
	*/
	
	while(header && section)
	{
		block = (HIERARCHY_SECTION_SAVE_BLOCK*) header;

		if(block->header.size!=sizeof(*block)) return;

		//compare section id numbers
		if(block->IDnumber == section->sempai->IDnumber)
		{
			//copy stuff for this section then
			COPYELEMENT_LOAD(Offset);
			COPYELEMENT_LOAD(World_Offset);
			COPYELEMENT_LOAD(Last_World_Offset);
			COPYELEMENT_LOAD(RelSecMat);
			COPYELEMENT_LOAD(SecMat);
		
			COPYELEMENT_LOAD(current_damage);

			COPYELEMENT_LOAD(accumulated_timer);
			COPYELEMENT_LOAD(freezeframe_timer);
			COPYELEMENT_LOAD(lastframe_timer);
			COPYELEMENT_LOAD(gore_timer);

			COPYELEMENT_LOAD(flags);
			COPYELEMENT_LOAD(replacement_id);


			//see if this section is using an alternate shape set
			{
				int desiredShapeIndex = section->sempai->ShapeNum;
				HIERARCHY_SHAPE_REPLACEMENT* replacement = GetHierarchyAlternateShapeFromId(section->sempai->Rif_Name,block->replacement_id,section->sempai->Section_Name);

				if(replacement)
				{
					desiredShapeIndex = replacement->replacement_shape_index;
				}
				
				if(section->ShapeNum != desiredShapeIndex)
				{
					section->ShapeNum = desiredShapeIndex;
					
					if(desiredShapeIndex>=0)
					{
						section->Shape = mainshapelist[desiredShapeIndex];
					}
					else
					{
						section->Shape = 0;
					}
					Setup_Texture_Animation_For_Section(section);

				}
			}


			//load decals
			if(decal_header)
			{
				LoadHierarchySectionDecals(decal_header,section);
			}
			else
			{
				section->NumberOfDecals = 0;
				section->NextDecalToUse	= 0;

			}

			//load tweening data
			if(tween_header)
			{
				LoadHierarchySectionTween(tween_header,section);
			}
			else
			{
				section->Tweening = 0;
			}

   
			//get the current sequence and frame
			
			{
				int a;
				int time;
				SECTION* this_section = section->sempai;
				section->current_sequence=&(this_section->sequence_array[0]);
				
				for (a=0; a<this_section->num_sequences; a++) {
					if (this_section->sequence_array[a].sequence_id==block->sequence_id) {
						section->current_sequence=&(this_section->sequence_array[a]);
						break;
					}
				}

				time = block->keyframe_time;
				section->current_keyframe = section->current_sequence->first_frame;

				while(time>=section->current_keyframe->Sequence_Length)
				{
					time-= section->current_keyframe->Sequence_Length;
					
					if(section->current_keyframe->last_frame)
					{
						break;
					}
					else
					{
						section->current_keyframe = section->current_keyframe->Next_Frame;
					}
				}



				

			}

			//move to the next section data
			if(section->First_Child)
			{
				section = section->First_Child;
			}
			else if (section->Next_Sibling)
			{
				section = section->Next_Sibling;
			}
			else
			{
				BOOL section_found=FALSE;
				while(section && !section_found)
				{
					section = section->My_Parent;
					if(section)
					{
						if(section->Next_Sibling)
						{
							section=section->Next_Sibling;
							section_found = TRUE;
						}
					}
				}
			}

			//move to next saved section
			header=GetNextBlockIfOfType(SaveBlock_HierarchySection);
			decal_header=GetNextBlockIfOfType(SaveBlock_HierarchyDecals);
			tween_header=GetNextBlockIfOfType(SaveBlock_HierarchyTween);
		}
		else if(block->IDnumber > section->sempai->IDnumber)
		{
			/*
				There was no saved data for this section , so we will need to prune it
			*/
			
			SECTION_DATA* pruned_section = section;
			
			if (section->Next_Sibling)
			{
				section = section->Next_Sibling;
			}
			else
			{
				BOOL section_found=FALSE;
				while(section && !section_found)
				{
					section = section->My_Parent;
					if(section)
					{
						if(section->Next_Sibling)
						{
							section=section->Next_Sibling;
							section_found = TRUE;
						}
					}
				}
			}
			Prune_Section(pruned_section);
			
		}
		else
		{
			//move to next saved section (we will need to advance until the saved id number matches
			//the section id number)
			//Nb. This probably never happens anyway
			header=GetNextBlockIfOfType(SaveBlock_HierarchySection);
			decal_header=GetNextBlockIfOfType(SaveBlock_HierarchyDecals);
			tween_header=GetNextBlockIfOfType(SaveBlock_HierarchyTween);
		}
	}

	//prune the remaining sections
	while(section)
	{
		SECTION_DATA* pruned_section = section;
		
		if (section->Next_Sibling)
		{
			section = section->Next_Sibling;
		}
		else
		{
			BOOL section_found=FALSE;
			while(section && !section_found)
			{
				section = section->My_Parent;
				if(section)
				{
					if(section->Next_Sibling)
					{
						section=section->Next_Sibling;
						section_found = TRUE;
					}
				}
			}
		}
		Prune_Section(pruned_section);
	}

}


static void SaveHierarchySectionRecursion(SECTION_DATA* section)
{
	HIERARCHY_SECTION_SAVE_BLOCK* block;

	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_HierarchySection;	
	block->header.size = sizeof(*block);


	block->IDnumber = section->sempai->IDnumber;
	
	//copy stuff
	COPYELEMENT_SAVE(Offset);
	COPYELEMENT_SAVE(World_Offset);
	COPYELEMENT_SAVE(Last_World_Offset);
	COPYELEMENT_SAVE(RelSecMat);
	COPYELEMENT_SAVE(SecMat);
	
	COPYELEMENT_SAVE(current_damage);

	COPYELEMENT_SAVE(accumulated_timer);
	COPYELEMENT_SAVE(freezeframe_timer);
	COPYELEMENT_SAVE(lastframe_timer);
	COPYELEMENT_SAVE(gore_timer);

	COPYELEMENT_SAVE(flags);
	COPYELEMENT_SAVE(replacement_id);
	
	//get current sequence id
	block->sequence_id = section->current_sequence->sequence_id;

	{
		KEYFRAME_DATA* frame = section->current_sequence->first_frame; 
		int time=0;

		while(frame != section->current_keyframe)
		{
			time += frame->Sequence_Length;

			if(frame->last_frame) break;

			frame = frame->Next_Frame;
		}

		block->keyframe_time = time;

	}


	//save decals (if needed)
	if(section->NumberOfDecals)
	{
		SaveHierarchySectionDecals(section);
	}

	//save tweening	(if needed)
	if(section->Tweening)
	{
		SaveHierarchySectionTween(section);
	}

	//recurse down hierarchy
	if (section->First_Child!=NULL) {
		
		SECTION_DATA * child_section;
		/*
		Must make sure the children are in the right order before saving.
		*/
		EnsureChildrenAreInAscendingIDOrder(section);
		
		child_section = section->First_Child;

		while (child_section) 
		{
			SaveHierarchySectionRecursion(child_section);
			child_section = child_section->Next_Sibling;
		}
	}

	
}



//section decal data
typedef struct hierarchy_decal_save_block
{
	SAVE_BLOCK_HEADER header;
	
	int	NumberOfDecals;
	int NextDecalToUse;
	OBJECT_DECAL Decals[MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION];
}HIERARCHY_DECAL_SAVE_BLOCK;





static void LoadHierarchySectionDecals(SAVE_BLOCK_HEADER* header,SECTION_DATA* section)
{
	int i;

	HIERARCHY_DECAL_SAVE_BLOCK* block = (HIERARCHY_DECAL_SAVE_BLOCK*) header;

	COPYELEMENT_LOAD(NumberOfDecals);
	COPYELEMENT_LOAD(NextDecalToUse);

	for(i=0;i<block->NumberOfDecals;i++)
	{
		COPYELEMENT_LOAD(Decals[i]);
	}

}


static void SaveHierarchySectionDecals(SECTION_DATA* section)
{
	HIERARCHY_DECAL_SAVE_BLOCK* block;
	int i;

	//determine how much space is required for the decals present
	int size = sizeof(HIERARCHY_DECAL_SAVE_BLOCK);
	size -= (MAX_NO_OF_DECALS_PER_HIERARCHICAL_SECTION - section->NumberOfDecals) * sizeof(OBJECT_DECAL);

	block = (HIERARCHY_DECAL_SAVE_BLOCK*) GetPointerForSaveBlock(size);

	//fill in the header
	block->header.type = SaveBlock_HierarchyDecals;
	block->header.size = size;

	//copy the decals

	COPYELEMENT_SAVE(NumberOfDecals);
	COPYELEMENT_SAVE(NextDecalToUse);

	for(i=0;i<block->NumberOfDecals;i++)
	{
		COPYELEMENT_SAVE(Decals[i]);
	}


}


//section tweening data
typedef struct hierarchy_tween_save_block
{
	SAVE_BLOCK_HEADER header;

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

	
}HIERARCHY_TWEEN_SAVE_BLOCK;

static void LoadHierarchySectionTween(SAVE_BLOCK_HEADER* header,SECTION_DATA* section)
{
	//see if this section has tweening data saved
	HIERARCHY_TWEEN_SAVE_BLOCK* block = (HIERARCHY_TWEEN_SAVE_BLOCK*) header;
	if(!block) return;

	COPYELEMENT_LOAD(stored_offset);
   	COPYELEMENT_LOAD(target_offset);
   	COPYELEMENT_LOAD(delta_offset);
   	COPYELEMENT_LOAD(stored_quat);
   	COPYELEMENT_LOAD(target_quat);
   	COPYELEMENT_LOAD(omega);
   	COPYELEMENT_LOAD(oneoversinomega);
   	COPYELEMENT_LOAD(oneovertweeninglength);	
	COPYELEMENT_LOAD(Tweening);
}

static void SaveHierarchySectionTween(SECTION_DATA* section)
{
	HIERARCHY_TWEEN_SAVE_BLOCK* block;

	GET_SAVE_BLOCK_POINTER(block);

	//fill in the header
	block->header.type = SaveBlock_HierarchyTween;
	block->header.size = sizeof(*block);

	//copy stuff

	COPYELEMENT_SAVE(stored_offset);
   	COPYELEMENT_SAVE(target_offset);
   	COPYELEMENT_SAVE(delta_offset);
   	COPYELEMENT_SAVE(stored_quat);
   	COPYELEMENT_SAVE(target_quat);
   	COPYELEMENT_SAVE(omega);
   	COPYELEMENT_SAVE(oneoversinomega);
   	COPYELEMENT_SAVE(oneovertweeninglength);	
	COPYELEMENT_SAVE(Tweening);

}
