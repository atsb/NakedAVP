/* KJL 12:15:10 8/23/97 - extents.c
 *
 * This file contains the collision extents
 * data for the game characters.
 *
 */
#include "extents.h"

COLLISION_EXTENTS CollisionExtents[MAX_NO_OF_COLLISION_EXTENTS] =
{
	/* CE_MARINE */
	{450,0, -1950, -1200},

	/* CE_PREDATOR */
	{450,0, -789-1309, -1309},

	/* CE_ALIEN */
	{450,0, -900-1133, -900},

	/* CE_XENOBORG */
	{450,0, -900-1133, -900},

	/* CE_PREDATORALIEN */
	{400,0, -851-1338, -851},
						 
	/* CE_FACEHUGGER */
	{300,0, 27-329, 27},

	/* CE_QUEEN */
	{1300,0, -2867-1133, -867},

	/* CE_CORPSE */
	{700,0, -200, -200},
};

