#include "3dc.h"

#define UseLocalAssert Yes
#include "ourasert.h"

extern int NormalFrameTime;
extern int NumActiveBlocks;
extern DISPLAYBLOCK * ActiveBlockList[];


void CopyAnimationFrameToShape (SHAPEANIMATIONCONTROLDATA *sacd, DISPLAYBLOCK * dptr)
{
	SHAPEHEADER * shp = dptr->ObShapeData;

	GLOBALASSERT (sacd->current_frame >= 0);
	GLOBALASSERT (sacd->current_frame < (signed)sacd->sequence->num_frames);

	shp->points[0] = sacd->sequence->anim_frames[sacd->current_frame].vertices;
	shp->sh_normals[0] = sacd->sequence->anim_frames[sacd->current_frame].item_normals;
}

static void CopyAnimationSequenceDataToObject (SHAPEANIMATIONSEQUENCE * sas, DISPLAYBLOCK * dptr)
{
	dptr->ObRadius = sas->radius;

	dptr->ObMaxX = sas->max_x;
	dptr->ObMinX = sas->min_x;

	dptr->ObMaxY = sas->max_y;
	dptr->ObMinY = sas->min_y;

	dptr->ObMaxZ = sas->max_z;
	dptr->ObMinZ = sas->min_z;
}

static void ChooseNextFrame (SHAPEANIMATIONCONTROLDATA *current)
{
	while (current->time_to_next_frame <= 0)
	{
		current->time_to_next_frame += current->seconds_per_frame;

		if (current->reversed)
			current->current_frame --;
		else
			current->current_frame ++;

		current->done_a_frame = 1;

		if (current->current_frame >= (signed)current->sequence->num_frames)
			current->current_frame = 0;
		else if (current->current_frame < 0)
			current->current_frame = current->sequence->num_frames-1;

		if (current->current_frame == (signed)current->end_frame 
				&& current->done_a_frame 
				&& (current->stop_at_end || current->pause_at_end))
		{
			break;
		}
	}
}

void DoShapeAnimation (DISPLAYBLOCK * dptr)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;
	SHAPEANIMATIONCONTROLDATA * active_sequence = &sac->current;

	GLOBALASSERT (sac);

	if (!sac->playing)
		return;

	if (active_sequence->empty)
		return;

	active_sequence->time_to_next_frame -= NormalFrameTime;

	if (active_sequence->time_to_next_frame > 0)
		return;

	// At this point we may have switched to the last frame
	// but still had a bit of time left on it
	
	if ((active_sequence->current_frame == (signed)active_sequence->end_frame 
			&& active_sequence->done_a_frame 
			&& active_sequence->stop_at_end) || active_sequence->stop_now)
	{
		// set to next, or finished
		if (sac->next.empty)
		{
			sac->finished = 1;
			sac->playing = 0;
			return;
		}
		else
		{
			sac->next.time_to_next_frame = sac->current.time_to_next_frame;
			sac->current = sac->next;
			sac->next.empty = 1;

			active_sequence->time_to_next_frame += active_sequence->seconds_per_frame;
			ChooseNextFrame (active_sequence);

			CopyAnimationSequenceDataToObject (active_sequence->sequence, dptr);
			return;
		}
	}
	else if ( active_sequence->current_frame == (signed)active_sequence->end_frame 
						&& active_sequence->done_a_frame
						&& active_sequence->pause_at_end)
	{
		active_sequence->pause_at_end = 0;
		sac->playing = 0;
		active_sequence->done_a_frame = 0;
		return;
	}

	ChooseNextFrame (active_sequence);

	// if we have reached the last frame and we still have time
	// continue else swap sequences

	if (active_sequence->time_to_next_frame <= 0)
	{
		if (active_sequence->current_frame == (signed)active_sequence->end_frame 
				&& active_sequence->done_a_frame 
				&& active_sequence->stop_at_end)
		{
			// set to next, or finished
			if (sac->next.empty)
			{
				sac->finished = 1;
				sac->playing = 0;
				return;
			}
			else
			{
				sac->next.time_to_next_frame = sac->current.time_to_next_frame;
				
				// this will change the active_sequence pointers contents
				sac->current = sac->next;
				CopyAnimationSequenceDataToObject (active_sequence->sequence, dptr);


				// If I had a linked list (or queue) of sequences
				// I may want to put the next bit of code differently

				active_sequence->time_to_next_frame += active_sequence->seconds_per_frame;

				ChooseNextFrame (active_sequence);
			
				return;
			}
		}
		else if ( active_sequence->current_frame == (signed)active_sequence->end_frame 
							&& active_sequence->done_a_frame
							&& active_sequence->pause_at_end)
		{
			active_sequence->pause_at_end = 0;
			sac->playing = 0;
			active_sequence->done_a_frame = 0;
			return;
		}
		else
		{
			// Shouldn't be here
			GLOBALASSERT (0);
		}
	}

}


void DoAllShapeAnimations ()
{
	int i;

	for (i=0; i<NumActiveBlocks; i++)
	{
		DISPLAYBLOCK * dptr = ActiveBlockList[i];
		
		if (dptr->ShapeAnimControlBlock)
		{
			DoShapeAnimation(dptr);
		}

	}

}


unsigned int SetShapeAnimationSequence (DISPLAYBLOCK * dptr, SHAPEANIMATIONCONTROLDATA * sacd)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	GLOBALASSERT(sac);
	GLOBALASSERT(sacd);
	GLOBALASSERT(sacd->sequence_no < sac->anim_header->num_sequences);

	sac->next.empty = 1;

	sac->playing = 1;
	sac->finished = 0;

	sac->current.sequence_no = sacd->sequence_no;

	sac->current.reversed = sacd->reversed;
	sac->current.stop_at_end = sacd->stop_at_end;


	sac->current.empty = 0;
	sac->current.done_a_frame = 0;
	sac->current.sequence = &sac->anim_header->anim_sequences[sacd->sequence_no];
	sac->current.stop_now = 0;
	sac->current.pause_at_end = 0;

	if (sacd->default_start_and_end_frames)
	{
		if (sacd->reversed)
		{
			sac->current.start_frame = sac->current.sequence->num_frames-1;
			sac->current.end_frame = 0;
		}
		else
		{
			sac->current.start_frame = 0;
			sac->current.end_frame = sac->current.sequence->num_frames-1;
		}
	}
	else
	{
		sac->current.start_frame = sacd->start_frame;
		sac->current.end_frame = sacd->end_frame;
	}

	sac->current.seconds_per_frame = sacd->seconds_per_frame;

	sac->current.current_frame = sac->current.start_frame;

	sac->current.time_to_next_frame = sac->current.seconds_per_frame;

	CopyAnimationSequenceDataToObject (sac->current.sequence, dptr);

	return(1);
	

}


unsigned int SetNextShapeAnimationSequence (DISPLAYBLOCK * dptr, SHAPEANIMATIONCONTROLDATA * sacd)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	GLOBALASSERT(sac);
	GLOBALASSERT(sacd);
	GLOBALASSERT(sacd->sequence_no < sac->anim_header->num_sequences);


	if (sac->current.empty)
	{
		return(SetShapeAnimationSequence (dptr,sacd));
	}

	if (sac->finished)
	{
		return(0);
	}

	
	sac->next.sequence_no = sacd->sequence_no;

	sac->next.reversed = sacd->reversed;
	sac->next.stop_at_end = sacd->stop_at_end;


	sac->next.empty = 0;
	sac->next.done_a_frame = 0;
	sac->next.sequence = &sac->anim_header->anim_sequences[sacd->sequence_no];
	sac->next.stop_now = 0;
	sac->next.pause_at_end = 0;

	if (sacd->default_start_and_end_frames)
	{
		if (sacd->reversed)
		{
			sac->next.start_frame = sac->next.sequence->num_frames-1;
			sac->next.end_frame = 0;
		}
		else
		{
			sac->next.start_frame = 0;
			sac->next.end_frame = sac->next.sequence->num_frames-1;
		}
	}
	else
	{
		sac->next.start_frame = sacd->start_frame;
		sac->next.end_frame = sacd->end_frame;
	}

	sac->next.seconds_per_frame = sacd->seconds_per_frame;

	sac->next.current_frame = sac->next.start_frame;

	sac->next.time_to_next_frame = sac->next.seconds_per_frame;

	return(1);
	

}

void InitShapeAnimationController (SHAPEANIMATIONCONTROLLER * sac, SHAPEHEADER * shd)
{
	GLOBALASSERT(shd);
	GLOBALASSERT(shd->animation_header);

	sac->current.empty = 1;
	sac->next.empty = 1;

	sac->anim_header = shd->animation_header;

	sac->playing = 0;

}

void SetCurrentShapeAnimationToStop (DISPLAYBLOCK * dptr, unsigned long stop_now, signed long end_frame)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	GLOBALASSERT(sac);

	if (stop_now)
	{
		sac->current.stop_now = 1;
		return;
	}

	if (end_frame != -1)
	{
		GLOBALASSERT (end_frame >= 0);
		GLOBALASSERT (end_frame < (signed)sac->current.sequence->num_frames);

		sac->current.end_frame = end_frame;
	}

	sac->current.stop_at_end = 1;

}


SHAPEANIMATIONCONTROLDATA const * GetCurrentShapeAnimationSequenceData (DISPLAYBLOCK * dptr)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	if (sac)
	{
		if (!sac->current.empty)
			return(&sac->current);
	}

	return(0);
}


SHAPEANIMATIONCONTROLDATA const * GetNextShapeAnimationSequenceData (DISPLAYBLOCK * dptr)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	if (sac)
	{
		if (!sac->next.empty)
			return(&sac->next);
	}

	return(0);

}


void PauseCurrentShapeAnimation (DISPLAYBLOCK * dptr, unsigned long pause_now, signed long end_frame)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	GLOBALASSERT(sac);

	if (pause_now)
	{
		sac->playing = 0;
		return;
	}

	if (end_frame != -1)
	{
		GLOBALASSERT (end_frame >= 0);
		GLOBALASSERT (end_frame < (signed)sac->current.sequence->num_frames);

		sac->current.end_frame = end_frame;
	}

	sac->current.pause_at_end = 1;

}

void RestartCurrentShapeAnimation (DISPLAYBLOCK * dptr)
{
	SHAPEANIMATIONCONTROLLER * sac = dptr->ShapeAnimControlBlock;

	GLOBALASSERT(sac);
	
	sac->playing = 1;

	sac->current.pause_at_end = 0;

}


void InitShapeAnimationControlData (SHAPEANIMATIONCONTROLDATA * sacd)
{
	GLOBALASSERT(sacd);
	
	sacd->seconds_per_frame = 8192;

	sacd->sequence_no = 0;

	sacd->default_start_and_end_frames = 1;
	sacd->reversed = 0;

	sacd->stop_at_end = 0;

}

unsigned int SetOrphanedShapeAnimationSequence (SHAPEANIMATIONCONTROLLER * sac, SHAPEANIMATIONCONTROLDATA * sacd)
{

	GLOBALASSERT(sac);
	GLOBALASSERT(sacd);
	GLOBALASSERT(sacd->sequence_no < sac->anim_header->num_sequences);

	sac->next.empty = 1;

	sac->playing = 1;
	sac->finished = 0;

	sac->current.sequence_no = sacd->sequence_no;

	sac->current.reversed = sacd->reversed;
	sac->current.stop_at_end = sacd->stop_at_end;


	sac->current.empty = 0;
	sac->current.done_a_frame = 0;
	sac->current.sequence = &sac->anim_header->anim_sequences[sacd->sequence_no];
	sac->current.stop_now = 0;
	sac->current.pause_at_end = 0;

	if (sacd->default_start_and_end_frames)
	{
		if (sacd->reversed)
		{
			sac->current.start_frame = sac->current.sequence->num_frames-1;
			sac->current.end_frame = 0;
		}
		else
		{
			sac->current.start_frame = 0;
			sac->current.end_frame = sac->current.sequence->num_frames-1;
		}
	}
	else
	{
		sac->current.start_frame = sacd->start_frame;
		sac->current.end_frame = sacd->end_frame;
	}

	sac->current.seconds_per_frame = sacd->seconds_per_frame;

	sac->current.current_frame = sac->current.start_frame;

	sac->current.time_to_next_frame = sac->current.seconds_per_frame;

	/* CopyAnimationSequenceDataToObject (sac->current.sequence, dptr); */

	return(1);
	

}
