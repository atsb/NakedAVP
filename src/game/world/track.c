#define DB_LEVEL 1
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "dynblock.h"
#include "stratdef.h"
#include "gamedef.h"
#include "track.h"
#include "jsndsup.h"
#include "psndplat.h"
#include "ourasert.h"
#include "mempool.h"

#include "db.h"

static void Preprocess_Smooth_Track_Controller(TRACK_CONTROLLER* tc);
static void SmoothTrackPosition(TRACK_SECTION_DATA* trackPtr, int u, VECTORCH *outputPositionPtr);
static void SmoothTrackOrientation(TRACK_SECTION_DATA* trackPtr, int lerp, MATRIXCH* outputMatrixPtr);
static void BasicSlerp(QUAT *input1,QUAT *input2,QUAT *output,int lerp);
static void MakeControlQuat(QUAT *control, QUAT *q0, QUAT *q1, QUAT *q2);
static void LnQuat(QUAT *q);
static void ExpPurelyImaginaryQuat(QUAT *q);
extern void MulQuat(QUAT *q1, QUAT *q2, QUAT *output);

extern int NormalFrameTime;
extern void QNormalise(QUAT*);
extern int QDot(QUAT *, QUAT *);

static void TrackSlerp(TRACK_SECTION_DATA* tsd,int lerp,MATRIXCH* output_mat)
{
	int sclp,sclq;
		
	QUAT* input1=&tsd->quat_start;
	QUAT* input2=&tsd->quat_end;
	QUAT output;
	
	if(lerp<0) lerp=0;
	if(lerp>65536)lerp=65536;
	
	/* First check for special case. */

	if (tsd->omega==2048) {
		int t1,t2;

		output.quatx=-input1->quaty;
		output.quaty=input1->quatx;
		output.quatz=-input1->quatw;
		output.quatw=input1->quatz;

		t1=MUL_FIXED((ONE_FIXED-lerp),1024);
		sclp=GetSin(t1);
		
		t2=MUL_FIXED(lerp,1024);
		sclq=GetSin(t2);

		output.quatx=(MUL_FIXED(input1->quatx,sclp))+(MUL_FIXED(output.quatx,sclq));
		output.quaty=(MUL_FIXED(input1->quaty,sclp))+(MUL_FIXED(output.quaty,sclq));
		output.quatz=(MUL_FIXED(input1->quatz,sclp))+(MUL_FIXED(output.quatz,sclq));
		output.quatw=(MUL_FIXED(input1->quatw,sclp))+(MUL_FIXED(output.quatw,sclq));

	} else {
		if ( (tsd->omega==0) && (tsd->oneoversinomega==0) ) {
			sclp=ONE_FIXED-lerp;
			sclq=lerp;
		} else {
			int t1,t2;
	
			t1=MUL_FIXED((ONE_FIXED-lerp),tsd->omega);
			t2=GetSin(t1);
			sclp=MUL_FIXED(t2,tsd->oneoversinomega);
	
			t1=MUL_FIXED(lerp,tsd->omega);
			t2=GetSin(t1);
			sclq=MUL_FIXED(t2,tsd->oneoversinomega);
			
		}
	
		output.quatx=(MUL_FIXED(input1->quatx,sclp))+(MUL_FIXED(input2->quatx,sclq));
		output.quaty=(MUL_FIXED(input1->quaty,sclp))+(MUL_FIXED(input2->quaty,sclq));
		output.quatz=(MUL_FIXED(input1->quatz,sclp))+(MUL_FIXED(input2->quatz,sclq));
		output.quatw=(MUL_FIXED(input1->quatw,sclp))+(MUL_FIXED(input2->quatw,sclq));
	}

	QNormalise(&output);
	QuatToMat(&output,output_mat);
}


void Start_Track_Sound(TRACK_SOUND* ts,VECTORCH * location)
{
	SOUND3DDATA s3d;

	GLOBALASSERT(ts);

	ts->playing=1;
	if(ts->activ_no!=SOUND_NOACTIVEINDEX) return;
	if(!ts->sound_loaded) return;

	if(ts->loop)
	{
		//make sure track is close enough to be heard
		int dist=VectorDistance(&Player->ObWorld,location);
		if(dist>ts->outer_range)
		{
			return;
		}
	}
	
	s3d.position = *location;
	s3d.inner_range = ts->inner_range;
	s3d.outer_range = ts->outer_range;
	s3d.velocity.vx = 0;
	s3d.velocity.vy = 0;
	s3d.velocity.vz = 0;
	
	if(ts->loop)
	{
		Sound_Play ((SOUNDINDEX)ts->sound_loaded->sound_num, "nvpel", &s3d,ts->max_volume,ts->pitch,&ts->activ_no);
	}
	else
	{
		Sound_Play ((SOUNDINDEX)ts->sound_loaded->sound_num, "nvpe", &s3d,ts->max_volume,ts->pitch,&ts->activ_no);
	}
	ts->time_left=GameSounds[ts->sound_loaded->sound_num].length;

	db_logf3(("Playing sound %d\t%s",ts->sound_loaded->sound_num,GameSounds[ts->sound_loaded->sound_num].wavName));
	
}
void Stop_Track_Sound(TRACK_SOUND* ts)
{
	GLOBALASSERT(ts);

	ts->playing=0;
	if(ts->activ_no!=SOUND_NOACTIVEINDEX)
	{
		Sound_Stop(ts->activ_no);
		ts->time_left=0;
	}
}
void Update_Track_Sound(TRACK_SOUND* ts,VECTORCH * location)
{
	GLOBALASSERT(ts);
	
	if(ts->playing)
	{
		ts->time_left-=NormalFrameTime;
		if(ts->loop)
		{
			//check to see if sound is close enough to player
			int dist=VectorDistance(&Player->ObWorld,location);
			if(dist>ts->outer_range)
			{
				//stop playing the sound for the moment
				if(ts->activ_no != SOUND_NOACTIVEINDEX)
				{
					Sound_Stop(ts->activ_no);
				}
			}
			else
			{
				if(ts->activ_no == SOUND_NOACTIVEINDEX)
				{
					//restart the sound
					Start_Track_Sound(ts,location);
					return;
				}
	
				if (ts->activ_no != SOUND_NOACTIVEINDEX)
				{
					//update the sound's location
					SOUND3DDATA s3d;
					s3d.position = *location;
					s3d.inner_range = ts->inner_range;
					s3d.outer_range = ts->outer_range;
					s3d.velocity.vx = 0;
					s3d.velocity.vy = 0;
					s3d.velocity.vz = 0;
	
					Sound_UpdateNew3d (ts->activ_no, &s3d);
				}
			}
		}
		else
		{
			if(ts->activ_no == SOUND_NOACTIVEINDEX)
			{
				//sound has stopped playing.
				ts->playing=0;
			}
		}
	}
}

void Deallocate_Track_Sound(TRACK_SOUND* ts)
{
	GLOBALASSERT(ts);
	
	Stop_Track_Sound(ts);
	
	if(ts->sound_loaded)
	{
		LoseSound(ts->sound_loaded);
	}
	
	#if !USE_LEVEL_MEMORY_POOL
	DeallocateMem (ts);
	#endif
}

void Update_Track_Position_Only(TRACK_CONTROLLER* tc)
{
	TRACK_SECTION_DATA* cur_tsd;
	DYNAMICSBLOCK* dynptr;
	
	GLOBALASSERT(tc);
	
	GLOBALASSERT(tc->sbptr);
	GLOBALASSERT(tc->sections);

	dynptr=tc->sbptr->DynPtr;
	GLOBALASSERT(dynptr);
	
	cur_tsd=&tc->sections[tc->current_section];
	
	
	//adjust timer until time lies within a section
	while(tc->timer>cur_tsd->time_for_section || tc->timer<0)
	{
		if(tc->timer>cur_tsd->time_for_section)
		{
			tc->timer-=cur_tsd->time_for_section;
			
			if(tc->current_section==(tc->num_sections-1) && !tc->loop)
			{
				if(tc->loop_backandforth)
				{
					//turn around
					tc->reverse=1;
					tc->timer=cur_tsd->time_for_section-tc->timer;
				}
				else
				{
					//reached end of track
					tc->timer=cur_tsd->time_for_section;
					tc->playing=0;
					tc->reverse=1; 
				}
			}
			else
			{
				tc->current_section++;
				
				if(tc->current_section==tc->num_sections) tc->current_section=0;
				
				cur_tsd=&tc->sections[tc->current_section];
			}
		
		}
		else
		{
			if(tc->current_section==0 && !tc->loop)
			{
				if(tc->loop_backandforth)
				{
					//turn around
					tc->reverse=0;
					tc->timer=-tc->timer;
				}
				else
				{
					//reached end of track
		  			tc->timer=0;
		  			tc->playing=0;
		  			tc->reverse=0; 
				}
				
			}
			else
			{
				tc->current_section--;

				if(tc->current_section<0) tc->current_section=tc->num_sections-1;

				cur_tsd=&tc->sections[tc->current_section];
				tc->timer+=cur_tsd->time_for_section;
			}
		
		}
		
	}
	
	// now work out the object position at this time

	/* KJL 14:47:44 24/03/98 - check for smoothing; if you've
	less than 3 points then smoothing is a bit pointless */
	if (tc->use_smoothing && tc->num_sections>=3)
	{
		int lerp=MUL_FIXED(tc->timer,cur_tsd->oneovertime);
		SmoothTrackOrientation(cur_tsd,lerp,&(dynptr->OrientMat));
		SmoothTrackPosition(cur_tsd,lerp,&(dynptr->Position));
		
	}
	else
	{
		if(tc->no_rotation)
		{
			int lerp=MUL_FIXED(tc->timer,cur_tsd->oneovertime);
			VECTORCH pivot_pos=cur_tsd->pivot_start;
		
			pivot_pos.vx+=MUL_FIXED(cur_tsd->pivot_travel.vx,lerp);
			pivot_pos.vy+=MUL_FIXED(cur_tsd->pivot_travel.vy,lerp);
			pivot_pos.vz+=MUL_FIXED(cur_tsd->pivot_travel.vz,lerp);
		
			

			dynptr->Position=pivot_pos;
		}
		else
		{
			int lerp=MUL_FIXED(tc->timer,cur_tsd->oneovertime);
			MATRIXCH orient;
			VECTORCH pivot_pos=cur_tsd->pivot_start;
			VECTORCH object_pos=cur_tsd->object_offset;

		  	TrackSlerp(cur_tsd,lerp,&orient);

			pivot_pos.vx+=MUL_FIXED(cur_tsd->pivot_travel.vx,lerp);
			pivot_pos.vy+=MUL_FIXED(cur_tsd->pivot_travel.vy,lerp);
			pivot_pos.vz+=MUL_FIXED(cur_tsd->pivot_travel.vz,lerp);
		
			RotateVector(&object_pos,&orient);

			AddVector(&pivot_pos,&object_pos);


			dynptr->OrientMat=orient;
			dynptr->Position=object_pos;
		}
	}
}

void Update_Track_Position(TRACK_CONTROLLER* tc)
{
	DYNAMICSBLOCK* dynptr;
	
	GLOBALASSERT(tc);

	if(!tc->playing) return;

	GLOBALASSERT(tc->sbptr);
	GLOBALASSERT(tc->sections);

	dynptr=tc->sbptr->DynPtr;
	GLOBALASSERT(dynptr);
	
	if(tc->playing_start_sound)
	{
		Update_Track_Sound(tc->start_sound,&dynptr->Position);
		if(tc->start_sound->playing)
		{
			return;
		}
		tc->playing_start_sound=FALSE;
		if(tc->sound)
		{
			Start_Track_Sound(tc->sound,&tc->sbptr->DynPtr->Position);
		}
	}
	
	//update timer and current section number
	if(tc->reverse)
	{
		if(tc->use_speed_mult)
			tc->timer-=MUL_FIXED(NormalFrameTime,tc->speed_mult);
		else
			tc->timer-=NormalFrameTime;
	}
	else
	{
		if(tc->use_speed_mult)
			tc->timer+=MUL_FIXED(NormalFrameTime,tc->speed_mult);
		else
			tc->timer+=NormalFrameTime;
	}
	
	Update_Track_Position_Only(tc);
		
	if(tc->sound)
	{
		if(tc->playing)
		{
			Update_Track_Sound(tc->sound,&dynptr->Position);
		}
		else
		{
			Stop_Track_Sound(tc->sound);
			if(tc->end_sound)
			{
				Start_Track_Sound(tc->end_sound,&tc->sbptr->DynPtr->Position);
			}
		}
	}
}

void Preprocess_Track_Controller(TRACK_CONTROLLER* tc)
{
	int i;
	GLOBALASSERT(tc->sections);
	
	/* KJL 14:47:44 24/03/98 - check for smoothing; if you've
	less than 3 points then smoothing is a bit pointless */
	if (tc->use_smoothing && tc->num_sections>=3)
	{
		Preprocess_Smooth_Track_Controller(tc);
		return;
	}

	for(i=0;i<tc->num_sections;i++)
	{
		
		TRACK_SECTION_DATA* tsd=&tc->sections[i];

		tsd->oneovertime=DIV_FIXED(ONE_FIXED,tsd->time_for_section);
	
		if(!tc->no_rotation)
		{
			int cosom,sinom;
			cosom=QDot(&tsd->quat_start,&tsd->quat_end);
	
			if (cosom<0) {
				tsd->quat_end.quatx=-tsd->quat_end.quatx;
				tsd->quat_end.quaty=-tsd->quat_end.quaty;
				tsd->quat_end.quatz=-tsd->quat_end.quatz;
				tsd->quat_end.quatw=-tsd->quat_end.quatw;
				cosom=-cosom;
			}


			tsd->omega=ArcCos(cosom);
			sinom=GetSin(tsd->omega);
			if (sinom) {
				tsd->oneoversinomega=DIV_FIXED(ONE_FIXED,sinom);
			} else {
				tsd->omega=0;
				tsd->oneoversinomega=0;
			}
		}
	}

}



void Start_Track_Playing(TRACK_CONTROLLER* tc)
{
	GLOBALASSERT(tc);
	GLOBALASSERT(tc->sbptr);
	GLOBALASSERT(tc->sbptr->DynPtr);

	if(tc->playing) return;
	
	tc->playing=1;
	if(tc->start_sound)
	{
		tc->playing_start_sound=1;
		Start_Track_Sound(tc->start_sound,&tc->sbptr->DynPtr->Position);
	}
	else if(tc->sound)
	{
		Start_Track_Sound(tc->sound,&tc->sbptr->DynPtr->Position);
	}

}

void Stop_Track_Playing(TRACK_CONTROLLER* tc)
{
	GLOBALASSERT(tc);
	GLOBALASSERT(tc->sbptr);
	GLOBALASSERT(tc->sbptr->DynPtr);

	if(!tc->playing) return;

	tc->playing=0;
	tc->playing_start_sound=0;
		
	if(tc->sound)
	{
		Stop_Track_Sound(tc->sound);
	}
	if(tc->start_sound)
	{
		Stop_Track_Sound(tc->sound);
	}
	if(tc->end_sound)
	{
		Start_Track_Sound(tc->end_sound,&tc->sbptr->DynPtr->Position);
	}
}


void Reset_Track(TRACK_CONTROLLER* tc)
{
	GLOBALASSERT(tc);
		
	if(tc->sound)
	{
		Stop_Track_Sound(tc->sound);
	}
	if(tc->start_sound)
	{
		Stop_Track_Sound(tc->start_sound);
	}
	if(tc->end_sound)
	{
		Stop_Track_Sound(tc->end_sound);
	}
	tc->timer=tc->initial_state_timer;
	tc->playing=tc->initial_state_playing;
	tc->reverse=tc->initial_state_reverse;
	tc->current_section=0;	

	if(tc->playing && tc->sound)
	{
		tc->sound->playing=1;
	}

}

void Deallocate_Track(TRACK_CONTROLLER* tc)
{
	GLOBALASSERT(tc);

	
	if(tc->sound)
	{
		Deallocate_Track_Sound(tc->sound);
	}
	if(tc->start_sound)
	{
		Deallocate_Track_Sound(tc->start_sound);
	}
	if(tc->end_sound)
	{
		Deallocate_Track_Sound(tc->end_sound);
	}
	
	#if !USE_LEVEL_MEMORY_POOL
	if(tc->sections) DeallocateMem(tc->sections);
	DeallocateMem(tc);
	#endif
	
}


/*KJL*******************************************************************
*                                                                      *
* Smooth track system - this code overlays cubic splines over the lerp *
* and slerp code used by the original track system                     *
*                                                                      *
*******************************************************************KJL*/
static void Preprocess_Smooth_Track_Controller(TRACK_CONTROLLER* tc)
{
	int i;
	GLOBALASSERT(tc->sections);
	
	for(i=0;i<tc->num_sections;i++)
	{
		TRACK_SECTION_DATA* tsd=&tc->sections[i];
		tsd->oneovertime=DIV_FIXED(ONE_FIXED,tsd->time_for_section);

		if (i==0)
		{
			tsd->quat_prev = tsd->quat_start;
			tsd->pivot_0 = tsd->pivot_start;
			tsd->pivot_1 = tsd->pivot_start;
			tsd->pivot_2 = tc->sections[1].pivot_start;
			tsd->pivot_3 = tc->sections[2].pivot_start;
		}
		else if (i==tc->num_sections-1)
		{
			tsd->quat_prev = tc->sections[i-1].quat_start;
			tsd->pivot_0 = tc->sections[i-1].pivot_1;
			tsd->pivot_1 = tsd->pivot_start;

			tsd->pivot_2 = tsd->pivot_1;
			tsd->pivot_2.vx += tsd->pivot_travel.vx;
			tsd->pivot_2.vy += tsd->pivot_travel.vy;
			tsd->pivot_2.vz += tsd->pivot_travel.vz;

			tsd->pivot_3 = tsd->pivot_2;
		}
		else if (i==tc->num_sections-2)
		{
			tsd->quat_prev = tc->sections[i-1].quat_start;
			tsd->pivot_0 = tc->sections[i-1].pivot_1;
			tsd->pivot_1 = tsd->pivot_start;
			tsd->pivot_2 = tc->sections[i+1].pivot_start;

			tsd->pivot_3 = tsd->pivot_2;
			tsd->pivot_3.vx += tc->sections[i+1].pivot_travel.vx;
			tsd->pivot_3.vy += tc->sections[i+1].pivot_travel.vy;
			tsd->pivot_3.vz += tc->sections[i+1].pivot_travel.vz;
		}
		else
		{
			tsd->quat_prev = tc->sections[i-1].quat_start;
			tsd->pivot_0 = tc->sections[i-1].pivot_start;
			tsd->pivot_1 = tsd->pivot_start;
			tsd->pivot_2 = tc->sections[i+1].pivot_start;
			tsd->pivot_3 = tc->sections[i+2].pivot_start;
		}
	}

	for(i=0;i<tc->num_sections;i++)
	{
		TRACK_SECTION_DATA* tsd=&tc->sections[i];
		if (QDot(&tsd->quat_prev,&tsd->quat_start)<0)
		{
			tsd->quat_start.quatx=-tsd->quat_start.quatx;
			tsd->quat_start.quaty=-tsd->quat_start.quaty;
			tsd->quat_start.quatz=-tsd->quat_start.quatz;
			tsd->quat_start.quatw=-tsd->quat_start.quatw;
		}
		if (QDot(&tsd->quat_start,&tsd->quat_end)<0)
		{
			tsd->quat_end.quatx=-tsd->quat_end.quatx;
			tsd->quat_end.quaty=-tsd->quat_end.quaty;
			tsd->quat_end.quatz=-tsd->quat_end.quatz;
			tsd->quat_end.quatw=-tsd->quat_end.quatw;
		}
		MakeControlQuat
		(
			&(tsd->quat_start_control),
			&(tsd->quat_prev),
			&(tsd->quat_start),
			&(tsd->quat_end)
		);
	}
	for(i=0;i<tc->num_sections-1;i++)
	{
		tc->sections[i].quat_end_control =  tc->sections[i+1].quat_start_control;
	}
	tc->sections[tc->num_sections-1].quat_end_control =  tc->sections[tc->num_sections-1].quat_end;
}

static void SmoothTrackPosition(TRACK_SECTION_DATA* trackPtr, int u, VECTORCH *outputPositionPtr)
{
	int u2 = MUL_FIXED(u,u);
	int u3 = MUL_FIXED(u2,u);

 	{
		int a = (-trackPtr->pivot_0.vx+3*trackPtr->pivot_1.vx-3*trackPtr->pivot_2.vx+trackPtr->pivot_3.vx);
		int b = (2*trackPtr->pivot_0.vx-5*trackPtr->pivot_1.vx+4*trackPtr->pivot_2.vx-trackPtr->pivot_3.vx);
		int c = (-1*trackPtr->pivot_0.vx+trackPtr->pivot_2.vx);
		outputPositionPtr->vx = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u))/2+trackPtr->pivot_1.vx;
	}
 	{
		int a = (-trackPtr->pivot_0.vy+3*trackPtr->pivot_1.vy-3*trackPtr->pivot_2.vy+trackPtr->pivot_3.vy);
		int b = (2*trackPtr->pivot_0.vy-5*trackPtr->pivot_1.vy+4*trackPtr->pivot_2.vy-trackPtr->pivot_3.vy);
		int c = (-1*trackPtr->pivot_0.vy+trackPtr->pivot_2.vy);
		outputPositionPtr->vy = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u))/2+trackPtr->pivot_1.vy;
	}
 	{
		int a = (-trackPtr->pivot_0.vz+3*trackPtr->pivot_1.vz-3*trackPtr->pivot_2.vz+trackPtr->pivot_3.vz);
		int b = (2*trackPtr->pivot_0.vz-5*trackPtr->pivot_1.vz+4*trackPtr->pivot_2.vz-trackPtr->pivot_3.vz);
		int c = (-1*trackPtr->pivot_0.vz+trackPtr->pivot_2.vz);
		outputPositionPtr->vz = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u))/2+trackPtr->pivot_1.vz;
	}
}
#if 0

static void SmoothTrackPositionBSpline(TRACK_SECTION_DATA* trackPtr, int u, VECTORCH *outputPositionPtr)
{
	int u2 = MUL_FIXED(u,u);
	int u3 = MUL_FIXED(u2,u);

 	{
		int a = (-trackPtr->pivot_0.vx		+3*trackPtr->pivot_1.vx	-3*trackPtr->pivot_2.vx	+1*trackPtr->pivot_3.vx);
		int b = (3*trackPtr->pivot_0.vx		-6*trackPtr->pivot_1.vx	+3*trackPtr->pivot_2.vx	+0*trackPtr->pivot_3.vx);
		int c = (-3*trackPtr->pivot_0.vx	+0*trackPtr->pivot_1.vx	+3*trackPtr->pivot_2.vx	+0*trackPtr->pivot_3.vx);
		int d = (trackPtr->pivot_0.vx		+4*trackPtr->pivot_1.vx	+1*trackPtr->pivot_2.vx +0*trackPtr->pivot_3.vx);

		outputPositionPtr->vx = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u)+d)/6;
	}
 	{
		int a = (-trackPtr->pivot_0.vy		+3*trackPtr->pivot_1.vy	-3*trackPtr->pivot_2.vy	+1*trackPtr->pivot_3.vy);
		int b = (3*trackPtr->pivot_0.vy		-6*trackPtr->pivot_1.vy	+3*trackPtr->pivot_2.vy	+0*trackPtr->pivot_3.vy);
		int c = (-3*trackPtr->pivot_0.vy	+0*trackPtr->pivot_1.vy	+3*trackPtr->pivot_2.vy	+0*trackPtr->pivot_3.vy);
		int d = (trackPtr->pivot_0.vy		+4*trackPtr->pivot_1.vy	+1*trackPtr->pivot_2.vy +0*trackPtr->pivot_3.vy);

		outputPositionPtr->vy = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u)+d)/6;
	}
 	{
		int a = (-trackPtr->pivot_0.vz		+3*trackPtr->pivot_1.vz	-3*trackPtr->pivot_2.vz	+1*trackPtr->pivot_3.vz);
		int b = (3*trackPtr->pivot_0.vz		-6*trackPtr->pivot_1.vz	+3*trackPtr->pivot_2.vz	+0*trackPtr->pivot_3.vz);
		int c = (-3*trackPtr->pivot_0.vz	+0*trackPtr->pivot_1.vz	+3*trackPtr->pivot_2.vz	+0*trackPtr->pivot_3.vz);
		int d = (trackPtr->pivot_0.vz		+4*trackPtr->pivot_1.vz	+1*trackPtr->pivot_2.vz +0*trackPtr->pivot_3.vz);

		outputPositionPtr->vz = (MUL_FIXED(a,u3)+MUL_FIXED(b,u2)+MUL_FIXED(c,u)+d)/6;
	}
}

#endif

static void SmoothTrackOrientation(TRACK_SECTION_DATA* trackPtr, int lerp, MATRIXCH* outputMatrixPtr)
{
	QUAT q1,q2,q3;

	if(lerp<0) lerp=0;
	if(lerp>65536)lerp=65536;
	
  	BasicSlerp(&trackPtr->quat_start,&trackPtr->quat_end, &q1, lerp);
 	BasicSlerp(&trackPtr->quat_start_control,&trackPtr->quat_end_control, &q2, lerp);
	
 	lerp = MUL_FIXED(ONE_FIXED-lerp,2*lerp);
	if(lerp<0) lerp=0;
	if(lerp>65536)lerp=65536;

  	BasicSlerp(&q1, &q2, &q3, lerp);

	QuatToMat(&q3,outputMatrixPtr);
}

static void BasicSlerp(QUAT *input1,QUAT *input2,QUAT *output,int lerp)
{
	int sclp,sclq;
	
	int cosom;
	int omega, sinom;

	cosom=QDot(input1,input2);
	if (cosom>0)
	{
		*output=*input2;
	}
	else
	{
		output->quatx=-input2->quatx;
		output->quaty=-input2->quaty;
		output->quatz=-input2->quatz;
		output->quatw=-input2->quatw;
		cosom =-cosom;
	}

	if(cosom<65500)
	{
		omega=ArcCos(cosom);
		sinom=GetSin(omega);

		/* healthy paranoia */
		LOCALASSERT(sinom!=0);
		
		sclp=DIV_FIXED
			 (
			 	GetSin
			 	(
			 		MUL_FIXED((ONE_FIXED-lerp),omega)
			 	),
			 	sinom
			 );
		sclq=DIV_FIXED
			 (
			 	GetSin
			 	(
			 		MUL_FIXED(lerp,omega)
			 	),
			 	sinom
			 );
	}
	else
	{
		sclp=ONE_FIXED-lerp;
		sclq=lerp;
	}


	output->quatx=(MUL_FIXED(input1->quatx,sclp))+(MUL_FIXED(output->quatx,sclq));
	output->quaty=(MUL_FIXED(input1->quaty,sclp))+(MUL_FIXED(output->quaty,sclq));
	output->quatz=(MUL_FIXED(input1->quatz,sclp))+(MUL_FIXED(output->quatz,sclq));
	output->quatw=(MUL_FIXED(input1->quatw,sclp))+(MUL_FIXED(output->quatw,sclq));
  	QNormalise(output);
}

static void MakeControlQuat(QUAT *control, QUAT *q0, QUAT *q1, QUAT *q2)
{
	QUAT a;
	QUAT b,c;

	a.quatx=-q1->quatx;
	a.quaty=-q1->quaty;
	a.quatz=-q1->quatz;
	a.quatw=q1->quatw;

	MulQuat(&a,q2, &b);
	MulQuat(&a,q0, &c);

	LnQuat(&b);
	LnQuat(&c);

	a.quatx = -(b.quatx+c.quatx)/4;
	a.quaty = -(b.quaty+c.quaty)/4;
	a.quatz = -(b.quatz+c.quatz)/4;

	ExpPurelyImaginaryQuat(&a);
	MulQuat(q1,&a, control);
}


#include <math.h>
static void LnQuat(QUAT *q)
{
	float theta;
   	float m,x,y,z;
		
   	x = q->quatx;
   	y = q->quaty;
   	z = q->quatz;

	if (x==0 && y==0 && z==0)
	{
		q->quatw = 0;
		q->quatx = 0;	
		q->quaty = 0;
		q->quatz = 0;
		return;
		
	}
   	{
	   	double cosine = (q->quatw/65536.0);

		if (cosine > 1.0) cosine = 1.0;
		else if (cosine < -1.0) cosine = -1.0;

		
		theta = (float)acos(cosine);
	}

	m = (65536.0/sqrt((x*x) + (y*y) + (z*z)) );

	x *= m;
	y *= m;
	z *= m;

	q->quatw = 0;
	q->quatx = (int)(x*theta);	
	q->quaty = (int)(y*theta);
	q->quatz = (int)(z*theta);
}
static void ExpPurelyImaginaryQuat(QUAT *q)
{
	float x,y,z;
	int theta;

	x = q->quatx;
	y = q->quaty;
   	z = q->quatz;

	if (x!=0||y!=0||z!=0)
	{
		float m = sqrt((x*x) + (y*y) + (z*z));
		x /= m;
		y /= m;
		z /= m;

		theta = (int)((m / 16.0) / 6.28318530718);
			
		q->quatx = (int)(x*(float)GetSin(theta));	
		q->quaty = (int)(y*(float)GetSin(theta));	
		q->quatz = (int)(z*(float)GetSin(theta));	
		q->quatw = GetCos(theta);
	}
	else
	{
		q->quatx = 0;	
		q->quaty = 0;	
		q->quatz = 0;	
		q->quatw = ONE_FIXED;
	}
	QNormalise(q);
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct track_save_block
{
	SAVE_BLOCK_HEADER header;
	
	int timer;
	int speed_mult;
	int current_section;

	unsigned int playing:1;
	unsigned int reverse:1;
	unsigned int no_rotation:1;
	unsigned int use_speed_mult:1;

	unsigned int start_sound_playing:1;
	unsigned int mid_sound_playing:1;
	unsigned int end_sound_playing:1;

	int start_sound_time_left;
	int mid_sound_time_left;
	int end_sound_time_left;
	
}TRACK_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV tc

void LoadTrackPosition(SAVE_BLOCK_HEADER* header,TRACK_CONTROLLER* tc)
{
	TRACK_SAVE_BLOCK* block = (TRACK_SAVE_BLOCK*)header;
	if(!header || !tc) return;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//start copying stuff

	COPYELEMENT_LOAD(timer)
	COPYELEMENT_LOAD(speed_mult)
	COPYELEMENT_LOAD(current_section)

	COPYELEMENT_LOAD(playing)
	COPYELEMENT_LOAD(reverse)
	COPYELEMENT_LOAD(no_rotation)
	COPYELEMENT_LOAD(use_speed_mult)

	Update_Track_Position_Only(tc);

	if(tc->start_sound)
	{
		tc->start_sound->playing = block->start_sound_playing;
		tc->start_sound->time_left = block->start_sound_time_left;
	}
	if(tc->sound)
	{
		tc->sound->playing = block->mid_sound_playing;
		tc->sound->time_left = block->mid_sound_time_left;
	}
	if(tc->end_sound)
	{
		tc->end_sound->playing = block->end_sound_playing;
		tc->end_sound->time_left = block->end_sound_time_left;
	}

	if(tc->start_sound) Load_SoundState(&tc->start_sound->activ_no);
	if(tc->sound) Load_SoundState(&tc->sound->activ_no);
	if(tc->end_sound) Load_SoundState(&tc->end_sound->activ_no);


}
void SaveTrackPosition(TRACK_CONTROLLER* tc)
{
	TRACK_SAVE_BLOCK* block;
	if(!tc) return;

	GET_SAVE_BLOCK_POINTER(block);

	//fill in header
	block->header.type = SaveBlock_Track;
	block->header.size = sizeof(*block);

	//start copying stuff

	COPYELEMENT_SAVE(timer)
	COPYELEMENT_SAVE(speed_mult)
	COPYELEMENT_SAVE(current_section)

	COPYELEMENT_SAVE(playing)
	COPYELEMENT_SAVE(reverse)
	COPYELEMENT_SAVE(no_rotation)
	COPYELEMENT_SAVE(use_speed_mult)

	if(tc->start_sound)
	{
		block->start_sound_playing = tc->start_sound->playing;
		block->start_sound_time_left = tc->start_sound->time_left;
	}
	else
	{
		block->start_sound_playing = 0;
		block->start_sound_time_left = 0;
	}
	
	if(tc->sound)
	{
		block->mid_sound_playing = tc->sound->playing;
		block->mid_sound_time_left = tc->sound->time_left;
	}
	else
	{
		block->mid_sound_playing = 0;
		block->mid_sound_time_left = 0;
	}

	if(tc->end_sound)
	{
		block->end_sound_playing = tc->end_sound->playing;
		block->end_sound_time_left = tc->end_sound->time_left;
	}
	else
	{
		block->end_sound_playing = 0;
		block->end_sound_time_left = 0;
	}						   

	if(tc->start_sound) Save_SoundState(&tc->start_sound->activ_no);
	if(tc->sound) Save_SoundState(&tc->sound->activ_no);
	if(tc->end_sound) Save_SoundState(&tc->end_sound->activ_no);
	
}
