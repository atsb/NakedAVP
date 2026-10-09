/***** bh_waypt.h *****/

/***** Waypoint system code... *****/

/***** CDF 27/10/97 *****/

struct strategyblock;

typedef struct waypoint_link {
	
	int link_target_index;
	int link_flags;

} WAYPOINT_LINK;

#define linkflag_oneway				0x00000001	/* link is... get this... one way! */
#define linkflag_reversed_oneway	0x00000002	/* link is one way, only NOT this way. */
#define linkflag_alienonly		 	0x00000004
#define linkflag_largetarget		0x00000008

typedef struct waypoint_volume {

	VECTORCH centre;	/* in MODULE space! */
	VECTORCH max_extents;
	VECTORCH min_extents;
	int flags;
	int workspace:14;
	unsigned int contains_npc:1;
	unsigned int contains_target:1;
	int num_links;
	WAYPOINT_LINK *first_link;
	unsigned char weighting;

} WAYPOINT_VOLUME;

#define wayflag_cancrawl	0x00000001

typedef struct waypoint_header {

	int num_waypoints;
	WAYPOINT_VOLUME *first_waypoint;
	
} WAYPOINT_HEADER;

typedef struct waypoint_route {
	
	int num_waypoints;
	WAYPOINT_VOLUME *start;
	WAYPOINT_VOLUME *second;
	WAYPOINT_VOLUME *last;
	WAYPOINT_LINK *first_link;

} WAYPOINT_ROUTE;

typedef struct waypoint_manager {
	
	WAYPOINT_VOLUME *current_container;
	WAYPOINT_VOLUME *current_target;
	WAYPOINT_LINK *current_link;
	VECTORCH current_target_point;

} WAYPOINT_MANAGER;

extern void InitWaypointSystem(int npctype);
extern int NPCGetWaypointDirection(WAYPOINT_HEADER *waypoints, struct strategyblock *sbPtr, VECTORCH *velocityDirection, VECTORCH *targetPosition, WAYPOINT_MANAGER *manager);
extern int AlienIsAllowedToAttack(struct strategyblock *sbPtr);
extern int AlienIsEncouragedToCrawl(void);
extern WAYPOINT_VOLUME *GetPositionValidity(struct module *conmod, VECTORCH *position, VECTORCH *suggestion);
extern void InitWaypointManager(WAYPOINT_MANAGER *manager);
