#ifndef _shp_anim_h
#define _shp_anim_h 1


#ifdef __cplusplus

	extern "C" {

#endif


/*

	Structures for animating shapes (not morphing)

	I am using standard types to organise the
	sequences as an enum would force a complete
	recompile each time it changed

*/

typedef struct shapeanimationframe
{

	// Public variables

	
	
	// Private variables
	
	int * vertices;
	int * item_normals;
	

} SHAPEANIMATIONFRAME;


typedef struct shapeanimationsequence
{

	// Public variables

	int radius;

	int max_x;
	int max_y;
	int max_z;

	int min_x;
	int min_y;
	int min_z;
	
	
	// Private variables
	
	unsigned long num_frames;
	SHAPEANIMATIONFRAME * anim_frames;

//	unsigned long num_interpolated_frames_per_frame;
	// 0 for none

	int * vertex_normals;
	
} SHAPEANIMATIONSEQUENCE;


typedef struct shapeanimationheader
{

	// Public variables


	// Private variables
	
	int num_sequences;
	
	SHAPEANIMATIONSEQUENCE * anim_sequences;
	
	int num_shapes_using_this;  //number of shapes sharing the same shapeanimationheader

} SHAPEANIMATIONHEADER;



typedef struct shapeanimationcontroldata
{

	// Public variables

	// 16.16 fixed point
	unsigned long seconds_per_frame;
	// default is ONE_FIXED / 8
	
	unsigned long sequence_no;
	// default is 0

	// if you're not interested in setting a start and an end frame
	// then set the flag 'default_start_and_end_frames' below
	unsigned long start_frame;
	unsigned long end_frame;
	// no default values

	unsigned long default_start_and_end_frames : 1;
	// default is on
	
	unsigned long reversed : 1;
	// default is off	
	
	unsigned long stop_at_end : 1;
	// default is off	
	

	
	// Private variables

	unsigned long empty : 1;
	unsigned long stop_now : 1;
	unsigned long pause_at_end : 1;
	
	// if start_frame == end_frame
	// then do not stop unless it has done at least one frame
	unsigned long done_a_frame : 1;

	SHAPEANIMATIONSEQUENCE * sequence;

	signed long current_frame;
	signed long time_to_next_frame;

} SHAPEANIMATIONCONTROLDATA;


typedef struct shapeanimationcontroller
{

	// Public variables
	
	SHAPEANIMATIONCONTROLDATA current;
	
	SHAPEANIMATIONCONTROLDATA next;
	
	unsigned long finished : 1;
	
	
	// Private variables
	
	unsigned long playing : 1;
	
	SHAPEANIMATIONHEADER * anim_header;
	
} SHAPEANIMATIONCONTROLLER;
	
	
	
	
//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////      Funtions      /////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

struct displayblock;
struct shapeheader;

// This sets the current animation (and the next to empty)
unsigned int SetShapeAnimationSequence (struct displayblock *, SHAPEANIMATIONCONTROLDATA *);
unsigned int SetOrphanedShapeAnimationSequence (SHAPEANIMATIONCONTROLLER * sac, SHAPEANIMATIONCONTROLDATA * sacd);

// This sets the next animation (if the current is empty it will set the current)
unsigned int SetNextShapeAnimationSequence (struct displayblock *, SHAPEANIMATIONCONTROLDATA *);

// stop_now == 1 will cause the function to ignore the end_frame value
// set end_frame to -1 for no change to the ending frame of the sequence
void SetCurrentShapeAnimationToStop (struct displayblock *, unsigned long stop_now, signed long end_frame);

SHAPEANIMATIONCONTROLDATA const * GetCurrentShapeAnimationSequenceData (struct displayblock *);
SHAPEANIMATIONCONTROLDATA const * GetNextShapeAnimationSequenceData (struct displayblock *);

// pause_now == 1 will cause the function to ignore the end_frame value
// set end_frame to -1 for no change to the ending frame of the sequence
void PauseCurrentShapeAnimation (struct displayblock *, unsigned long pause_now, signed long end_frame);
void RestartCurrentShapeAnimation (struct displayblock *);

// Please use these functions, whenever you create a block of each type
void InitShapeAnimationController (SHAPEANIMATIONCONTROLLER *, struct shapeheader *);
void InitShapeAnimationControlData (SHAPEANIMATIONCONTROLDATA *);




// These are for the system

void DoAllShapeAnimations ();
	
void CopyAnimationFrameToShape (SHAPEANIMATIONCONTROLDATA *sacd, struct displayblock * dptr);
	
#ifdef __cplusplus

	};

#endif


#endif
