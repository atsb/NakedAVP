/* KJL 12:15:10 8/23/97 - extents.h
 *
 * This file contains the collision extents
 * structures for the game characters.
 *
 */

enum COLLISION_EXTENTS_ID
{
	CE_MARINE,

	CE_PREDATOR,
	
	CE_ALIEN,

	CE_XENOBORG,
	
	CE_PREDATORALIEN,
	
	CE_FACEHUGGER,

	CE_QUEEN,

	CE_CORPSE,

	MAX_NO_OF_COLLISION_EXTENTS
};


typedef struct
{
	/* radius of shape in XZ plane */
	int CollisionRadius;

	/* height extents */
	int Bottom; /* ie. max Y of the shape */

	int StandingTop;
	int CrouchingTop;

} COLLISION_EXTENTS;


extern COLLISION_EXTENTS CollisionExtents[];
