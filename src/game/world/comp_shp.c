#include "3dc.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "comp_shp.h"

#define UseLocalAssert Yes

#include "ourasert.h"

/*

There is a small problem in the destiction between
compiled in shapes and binary loaded ones. I am
changing chnkmaps.c maps.c and shapes.c into comp_shp.c
comp_shp.h, comp_map.c and comp_map.h.

We need to generate data for the game by

1. Setting up the initial Environment
2. Placing the player in the environment
3. Generateing Activer blocks at runtime.

To deal with these problems we can now do the following

1 Binary Load Shapes. Link these to MAPs and SBs

2  Have functions to modify a mapblock that describes
 the player depending on how he entered a level

3 Have a list of shapes that is either Compiled in
or loaded at game init to represent the shapes that
can be generated at run time. Have a set of functions
to gnertae MAPBLOCKS for the above.
 
 

this is the compiled in shape list for AVP. For the
Saturn and the Playstation, these shapes may end up 
being loaded during game init, (not level init) to
reduce the overall memory load

compiled in shapes are descriptons and strategies of
shapes AND functions to spawn mapblocks and attach 
strategy blocks

To generate stuff we have a list of compiled in shapes
that are appended to the start of the mainshapelist.

We then have functions that cen generate the maps and
strategyblocks for each. Texture maps for the shapes
are read in at level init along with the rest of the
texture maps for each env. 

procedure:

Generate map block using reference into 

*/

#define NUM_MAR_SHAPES 14

int maxshapes=0;
SHAPEHEADER ** mainshapelist=0;

/* compiled in shapes that do not exist as yet*/

SHAPEHEADER* MarineCompiledShapes[] = {
	&CUBE_header,
	&CUBE_header,
    &CUBE_header,			/*MarinePlayer*/
    &CUBE_header,            /*PredatorPlayer*/
	&CUBE_header,			/*AlienPlayer*/
	&CUBE_header, /* was &ALIEN_header, but the textures are no longer there. The old alien should be fully purged. << keywords: BUG FIXME OPTIMIZEME OPTIMISEME ERROR MISTAKE HACK >> */
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
    &CUBE_header,		/* player crouch shape */
    &CUBE_header,			/* player lying down shape */
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,
	&CUBE_header,			/* alien generator */
};


#define STARTOF_PRECOMPILEDSHAPES 0
int load_precompiled_shapes(void)
{
	static int done = 0;
	int i,j = 0;
	
	if(!mainshapelist)
	{
		maxshapes=750;
		/*I'm not using AllocateMem because I'll have to realloc this later*/
		mainshapelist=(SHAPEHEADER**)malloc(sizeof(SHAPEHEADER*)*maxshapes);
		LOCALASSERT(mainshapelist);
		if(!mainshapelist)
		{
	   		ReleaseDirect3D();
			exit(0x74363); 
		}
		for(i=0;i<maxshapes;i++)
		{
			mainshapelist[i]=0;
		}
	}
	
	i=STARTOF_PRECOMPILEDSHAPES;

	if (done) return i+NUM_MAR_SHAPES;

    /* KJL 11:43:36 09/24/96 - load some marine stuff hacked in 'cos the old way was even worse */
	while(j < NUM_MAR_SHAPES)
	{
		mainshapelist[i] = MarineCompiledShapes[j];
		i++;
		j++;
	}
	
	done = 1;
	
	return i;
}
