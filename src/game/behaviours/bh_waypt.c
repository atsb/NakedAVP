/***** bh_waypt.c *****/

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
#include "bh_alien.h"
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "psnd.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/*This allows ai not to go for the shortest all the time*/
#define ALLOW_USE_OF_LONGER_PATHS Yes

int NPC_Waypoints;
int Num_Target_Waypoints;
int Global_Num_Waypoints;
int Global_NPC_Type;
WAYPOINT_ROUTE Global_Route;

static int GlobalLinkShift; //Allows different aliens to take different routes

WAYPOINT_VOLUME *Global_Target_Waypoint,*Global_NPC_Waypoint;
VECTORCH Global_Module_Offset;

/* forward declarations for this file */
int NewFindThisRoute(WAYPOINT_HEADER *waypoints, WAYPOINT_ROUTE *thisroute,WAYPOINT_VOLUME *startwaypoint, WAYPOINT_VOLUME *endwaypoint);
void SweepWaypoints(WAYPOINT_HEADER *waypoints, STRATEGYBLOCK *sbPtr, VECTORCH *targetPosition);
int FindBestRoute(WAYPOINT_ROUTE *bestroute,WAYPOINT_HEADER *waypoints);
int NPCContainsPoint(STRATEGYBLOCK *sbPtr,VECTORCH *point);
static int WaypointContainsPoint(WAYPOINT_VOLUME *waypoint, VECTORCH *point);
void GetTargetPositionInWaypoint(WAYPOINT_VOLUME *waypoint,VECTORCH *output);


void InitWaypointSystem(int npctype) {

	Global_Route.num_waypoints=ONE_FIXED;
	Global_Route.start=NULL;
	Global_Route.second=NULL;
	Global_Route.last=NULL;
	Global_NPC_Type=npctype;

}

void InitWaypointManager(WAYPOINT_MANAGER *manager) {
	
	manager->current_container=NULL;
	manager->current_target=NULL;
	manager->current_link=NULL;
	
	manager->current_target_point.vx=0;
	manager->current_target_point.vy=0;
	manager->current_target_point.vz=0;

}

int NPCGetWaypointDirection(WAYPOINT_HEADER *waypoints, STRATEGYBLOCK *sbPtr, VECTORCH *velocityDirection, VECTORCH *targetPosition,WAYPOINT_MANAGER *manager) {

	/* Revision of NPCGetMovementDirection, to accomodate the waypoint system. */

	VECTORCH targetDirection;
	WAYPOINT_ROUTE current_route;
	WAYPOINT_VOLUME *dest_waypoint;
	 	 		
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);
	LOCALASSERT(velocityDirection);
	LOCALASSERT(targetPosition);
	
	/* I don't know if we'll need this... */
	targetDirection = *targetPosition;
	targetDirection.vx -= sbPtr->DynPtr->Position.vx;
	targetDirection.vz -= sbPtr->DynPtr->Position.vz;
	targetDirection.vy = 0;
	Normalise(&targetDirection);

	/*in the unlikely case that the player or npc doesnt have a current containingg module, 
	just return the target direction */
	if((playerPherModule==NULL)||(sbPtr->containingModule==NULL))
	{
		*velocityDirection = targetDirection;
		textprint("Waypoint dropout: no modules\n");
		InitWaypointManager(manager);
		return(0);
	}

	Global_Module_Offset=sbPtr->containingModule->m_aimodule->m_world;

	Global_Route.num_waypoints=ONE_FIXED;
	Global_Route.start=NULL;
	Global_Route.second=NULL;
	Global_Route.last=NULL;
	Global_Route.first_link=NULL;

	NPC_Waypoints=0;
	Num_Target_Waypoints=0;

	/* This will establish the locations of the NPC and the target. */
	SweepWaypoints(waypoints, sbPtr, targetPosition);
	
	/* Return the target direction if the NPC or the target have no containing waypoint. */
	if ( (NPC_Waypoints==0) || (Num_Target_Waypoints==0) ) {
		*velocityDirection = targetDirection;
		textprint("Waypoint dropout: no containing waypoints (%d, %d)\n",NPC_Waypoints,Num_Target_Waypoints);
		InitWaypointManager(manager);
		return(0);
	}

	/* Now what?  Compute the best route to the target. */

	textprint("NPC is in %d waypoints.\n",NPC_Waypoints);

	//Base shift value on strategy block so that the aliens don't keep changing their minds
	//about which route to take
	GlobalLinkShift=(((intptr_t)sbPtr)&0xffff)>>4;
	if (FindBestRoute(&current_route,waypoints)==0) {
		/* Yuck! */
		textprint("Waypoint dropout: no continuous route!\n");
		InitWaypointManager(manager);
		return(0);
	}

	/* If the best route has start and end the same, go for it. */
	
	Global_Route=current_route;

	if ((Global_Route.start!=Global_Route.second)
		&&(Global_Route.second!=Global_Route.last)) {
		GLOBALASSERT(Global_Route.first_link);
	}

	if (current_route.start==current_route.last) {
		textprint("Waypoint dropout Success: start==end\n");
		*velocityDirection = targetDirection;
		return(1);
	}


	dest_waypoint=NULL;

	/* If the NPC is entirely inside the first waypoint, then head for the centre of
	the second one.  Else, head for the centre of the first one. */

	if (NPC_Waypoints==1) {
		/* Global_NPC_Waypoint must be correct. */
		if (Global_NPC_Waypoint==current_route.start) {
			dest_waypoint=current_route.second;
		} else {
			/* wtf? */
		}
	} else {
		if (NPCContainsPoint(sbPtr,&manager->current_target_point)) {
			/* Must have reached the 'centre'. */
			if (Global_NPC_Waypoint==current_route.start) {
				dest_waypoint=current_route.second;
			} else {
				/* wtf? */
			}
		}
	}
	
	if (dest_waypoint==NULL) {
		dest_waypoint=current_route.start;
	}

	LOCALASSERT(dest_waypoint);

	/* Right, at this point we have a route, a containing waypoint, a destination, etc. */
	if (manager->current_target==dest_waypoint) {
		/* Continue to head for the old target position, if it's valid. */
		if (!WaypointContainsPoint(manager->current_target,&manager->current_target_point)) {
			manager->current_target_point=dest_waypoint->centre;
		}
	} else {
		/* Set destination... */
		manager->current_target=dest_waypoint;
		manager->current_link=Global_Route.first_link;
		/* Get a new target position, based on the link we're now using. */
		if (manager->current_link==NULL) {
			manager->current_target_point=dest_waypoint->centre;
		} else if (manager->current_link->link_flags&linkflag_largetarget) {
			GetTargetPositionInWaypoint(dest_waypoint,&manager->current_target_point);
		} else {
			manager->current_target_point=dest_waypoint->centre;
		}
	}

	if (manager->current_container!=Global_Route.start) {
		manager->current_container=Global_Route.start;
		manager->current_link=Global_Route.first_link;
		/* Actually, no need to change the target... is there? */
		#if ALLOW_USE_OF_LONGER_PATHS
		//make it cost to enter this waypoint volume 
		//(encouraging ai to use other routes)
		Global_Route.start->weighting++;
		//if the weighting for a volume has got too high , reduce the weighting for all volumes
		if(Global_Route.start->weighting>25)
		{
			int i;
			for(i=0;i<waypoints->num_waypoints;i++)
			{
				//don't want to reduce weighting below the original weighting
				if(waypoints->first_waypoint[i].weighting>5)
				{
					waypoints->first_waypoint[i].weighting--;
				}
			}
		}
		#endif
	} else {
		/* Er, well done. */
	}

	//targetDirection=dest_waypoint->centre;
	targetDirection=manager->current_target_point;

	targetDirection.vx+=Global_Module_Offset.vx;
	targetDirection.vy+=Global_Module_Offset.vy;
	targetDirection.vz+=Global_Module_Offset.vz;
	
	targetDirection.vx -= sbPtr->DynPtr->Position.vx;
	targetDirection.vz -= sbPtr->DynPtr->Position.vz;
	targetDirection.vy -= sbPtr->DynPtr->Position.vy;
	Normalise(&targetDirection);

	{
		int a,b;
	
		b=-1;

		for (a=0; a<waypoints->num_waypoints; a++) {
		
			WAYPOINT_VOLUME *this_waypoint;

			this_waypoint=waypoints->first_waypoint;
			this_waypoint+=a;

			if (this_waypoint==dest_waypoint) {
				b=a;
			}
		}

		textprint("Destination waypoint: %d\n",b);
	}
	textprint("Waypoint Generated Velocity = %d %d %d\n",targetDirection.vx,targetDirection.vy,targetDirection.vz);

	*velocityDirection = targetDirection;

	/* That's all folks. */

	return(1);	

}

static int WaypointContainsPoint(WAYPOINT_VOLUME *waypoint, VECTORCH *point) {

	/* 'point' should be in module space. */

	int minx,maxx,miny,maxy,minz,maxz;

	minx=waypoint->centre.vx+waypoint->min_extents.vx;
	miny=waypoint->centre.vy+waypoint->min_extents.vy;
	minz=waypoint->centre.vz+waypoint->min_extents.vz;

	maxx=waypoint->centre.vx+waypoint->max_extents.vx;
	maxy=waypoint->centre.vy+waypoint->max_extents.vy;
	maxz=waypoint->centre.vz+waypoint->max_extents.vz;

	if (point->vx>=minx) {
		if (point->vx<=maxx) {
			if (point->vy>=miny) {
				if (point->vy<=maxy) {
					if (point->vz>=minz) {
						if (point->vz<=maxz) {
							return(1);
						}
					}
				}
			}
		}
	}

	return(0);

}

int WaypointContainsPoint_2d(WAYPOINT_VOLUME *waypoint, VECTORCH *point) {

	/* 'point' should be in module space. */

	int minx,maxx,minz,maxz;

	minx=waypoint->centre.vx+waypoint->min_extents.vx;
	minz=waypoint->centre.vz+waypoint->min_extents.vz;

	maxx=waypoint->centre.vx+waypoint->max_extents.vx;
	maxz=waypoint->centre.vz+waypoint->max_extents.vz;

	if (point->vx>=minx) {
		if (point->vx<=maxx) {
			if (point->vz>=minz) {
				if (point->vz<=maxz) {
					return(1);
				}
			}
		}
	}

	return(0);

}

int WaypointContainsNPC(WAYPOINT_VOLUME *volume,STRATEGYBLOCK *sbPtr) {

	VECTORCH testpoint;

	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);
	LOCALASSERT(sbPtr->SBdptr);

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMaxX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMaxY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMaxZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMinX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMaxY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMaxZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMaxX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMinY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMaxZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMinX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMinY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMaxZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMaxX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMaxY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMinZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMinX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMaxY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMinZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMaxX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMinY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMinZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	testpoint.vx=sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMinX-Global_Module_Offset.vx;
	testpoint.vy=sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMinY-Global_Module_Offset.vy;
	testpoint.vz=sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMinZ-Global_Module_Offset.vz;
	if (WaypointContainsPoint(volume,&testpoint)) return(1);	

	/* Failed! */

	return(0);

}

int NPCContainsPoint(STRATEGYBLOCK *sbPtr,VECTORCH *point) {

	/* Point is in MODULESPACE! */
	LOCALASSERT(point);
	LOCALASSERT(sbPtr);
	LOCALASSERT(sbPtr->DynPtr);
	LOCALASSERT(sbPtr->SBdptr);

	if (point->vx<(sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMaxX-Global_Module_Offset.vx)) {
		if (point->vx>(sbPtr->DynPtr->Position.vx+sbPtr->SBdptr->ObMinX-Global_Module_Offset.vx)) {
			if (point->vy<(sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMaxY-Global_Module_Offset.vy)) {
				if (point->vy>(sbPtr->DynPtr->Position.vy+sbPtr->SBdptr->ObMinY-Global_Module_Offset.vy)) {
					if (point->vz<(sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMaxZ-Global_Module_Offset.vz)) {
						if (point->vz>(sbPtr->DynPtr->Position.vz+sbPtr->SBdptr->ObMinZ-Global_Module_Offset.vz)) {
							return(1);
						}
					}
				}
			}	
		}	
	}
	
	return(0);

}

void SweepWaypoints(WAYPOINT_HEADER *waypoints, STRATEGYBLOCK *sbPtr, VECTORCH *targetPosition) {

	WAYPOINT_VOLUME *this_waypoint;
	int a;

	Global_Num_Waypoints=waypoints->num_waypoints;

	for (a=0; a<waypoints->num_waypoints; a++) {

		VECTORCH modtargetpos;
		
		this_waypoint=waypoints->first_waypoint;
		this_waypoint+=a;

		this_waypoint->contains_npc=0;
		this_waypoint->contains_target=0;
				
		modtargetpos.vx=targetPosition->vx-Global_Module_Offset.vx;
		modtargetpos.vy=targetPosition->vy-Global_Module_Offset.vy;
		modtargetpos.vz=targetPosition->vz-Global_Module_Offset.vz;

		if (WaypointContainsPoint(this_waypoint,&modtargetpos)) {
			Num_Target_Waypoints++;
			Global_Target_Waypoint=this_waypoint;
			this_waypoint->contains_target=1;
		}
		if (WaypointContainsNPC(this_waypoint,sbPtr)) {
			NPC_Waypoints++;
			Global_NPC_Waypoint=this_waypoint;
			this_waypoint->contains_npc=1;
		}
	}

}

void Trickle_Down(WAYPOINT_HEADER *waypoints, WAYPOINT_VOLUME *this_waypoint, int trickle_value) {

	int a;

	LOCALASSERT(this_waypoint->workspace<=trickle_value);

	this_waypoint->workspace=trickle_value;

	for (a=0; a<this_waypoint->num_links; a++) {
		
		WAYPOINT_LINK *link_to_next;
		WAYPOINT_VOLUME *next_waypoint;

		link_to_next=this_waypoint->first_link;
		link_to_next+=a;

		if ((link_to_next->link_flags&linkflag_oneway)==0) {
		
			next_waypoint=waypoints->first_waypoint;
			next_waypoint+=link_to_next->link_target_index;
	
			if (next_waypoint->workspace<trickle_value) {
	
				LOCALASSERT(trickle_value>0);
	
				Trickle_Down(waypoints,next_waypoint,trickle_value-1);
	
			}

		}

	}

}

void FindThisRoute(WAYPOINT_HEADER *waypoints, WAYPOINT_ROUTE *thisroute,WAYPOINT_VOLUME *startwaypoint, WAYPOINT_VOLUME *endwaypoint) {

	int a,b;
	int current_trickle_value;
	WAYPOINT_VOLUME *thiswaypoint;

	/* Trivial case... */
	if ( (startwaypoint->contains_npc) && (startwaypoint->contains_target) ) {
		thisroute->num_waypoints=0;
		thisroute->start=startwaypoint;
		thisroute->second=startwaypoint;
		thisroute->last=startwaypoint;
		return;
	}

	/* Setup... */

	thisroute->num_waypoints=0;
	thisroute->start=startwaypoint;
	thisroute->second=NULL;
	thisroute->last=endwaypoint;

	for (a=0; a<waypoints->num_waypoints; a++) {
		/* Preparation... */
		WAYPOINT_VOLUME *temp_waypoint;

		temp_waypoint=waypoints->first_waypoint;
		temp_waypoint+=a;
		temp_waypoint->workspace=0;
	}

	current_trickle_value=waypoints->num_waypoints;

	/* Not so trivial cases: cheap 'pheromone' system. */

	Trickle_Down(waypoints, endwaypoint, current_trickle_value);

	/* Now extract best route. */

	b=0; /* Number of waypoints stepped. */
	thiswaypoint=startwaypoint;
	
	while (thiswaypoint!=endwaypoint) {
		
		/* Look through links for highest increment. */

		int nextstep=0;
		WAYPOINT_VOLUME *bestnextwaypoint=NULL;

		for (a=0; a<thiswaypoint->num_links; a++) {

			WAYPOINT_LINK *link_to_next;
			WAYPOINT_VOLUME *next_waypoint;
	
			link_to_next=thiswaypoint->first_link;
			link_to_next+=a;
	
			if ((link_to_next->link_flags&linkflag_reversed_oneway)==0) {
			
				next_waypoint=waypoints->first_waypoint;
				next_waypoint+=link_to_next->link_target_index;
				
				if (next_waypoint->workspace>thiswaypoint->workspace) {				
					if (next_waypoint->workspace>nextstep) {
		
						/* Found a better step. */
		

						nextstep=next_waypoint->workspace;
						bestnextwaypoint=next_waypoint;

					}
				}
			}
			
		}

		LOCALASSERT(nextstep);
		LOCALASSERT(bestnextwaypoint);

		if (b==0) {
			thisroute->second=bestnextwaypoint;
		}

		b++;

		thiswaypoint=bestnextwaypoint;

		/* Go round again... */

	}
	
	thisroute->num_waypoints=b;

	/* That may be it... */

}

int FindBestRoute(WAYPOINT_ROUTE *bestroute,WAYPOINT_HEADER *waypoints) {

	int a;
	WAYPOINT_ROUTE thisroute;

	/* Wipe the best route... */
	bestroute->num_waypoints=ONE_FIXED;
	bestroute->start=NULL;
	bestroute->second=NULL;
	bestroute->last=NULL;
	bestroute->first_link=NULL;

	for (a=0; a<waypoints->num_waypoints; a++) {
	
		WAYPOINT_VOLUME *start;

		start=waypoints->first_waypoint;
		start+=a;

		if (start->contains_npc) {
			if (NewFindThisRoute(waypoints,&thisroute,start,Global_Target_Waypoint)==0) {
				/* Yuck. */
				return(0);
			}

			/* We now have a route from this waypoint to the target. */

			if (thisroute.num_waypoints<bestroute->num_waypoints) {
				/* New best route. */
				bestroute->num_waypoints=thisroute.num_waypoints;
				bestroute->start=thisroute.start;
				bestroute->second=thisroute.second;
				bestroute->last=thisroute.last;
				bestroute->first_link=thisroute.first_link;
			}

		}

	}

	/* We now have a best route. */
	{
		int start, second, last;

		for (a=0; a<waypoints->num_waypoints; a++) {
		
			WAYPOINT_VOLUME *this_waypoint;

			this_waypoint=waypoints->first_waypoint;
			this_waypoint+=a;

			if (this_waypoint==bestroute->start) {
				start=a;
			}
			if (this_waypoint==bestroute->second) {
				second=a;
			}
			if (this_waypoint==bestroute->last) {
				last=a;
			}

		}

		textprint("Waypoint route: %d %d %d\n",start,second,last);

	}
	return(1);
}

int AlienIsAllowedToAttack(STRATEGYBLOCK *sbPtr) {

	ALIEN_STATUS_BLOCK *alienStatusPtr;

	/* Define rules: alien may not attack if the containing module has
	waypoints, AND the target is more than one waypoint away. 
	Note that the waypoint system MUST have been called just a bit earlier... */

	LOCALASSERT(sbPtr);
	alienStatusPtr=(ALIEN_STATUS_BLOCK *)(sbPtr->SBdataptr);    
	LOCALASSERT(alienStatusPtr);

	if (alienStatusPtr->EnableWaypoints==0) {
		/* I see no waypoints. */
		return(1);
	}

	if (sbPtr->containingModule->m_aimodule->m_waypoints!=NULL) {

		if ((Global_Route.num_waypoints<2)
			||(Global_Route.num_waypoints==ONE_FIXED)) {
			/* ONE_FIXED = no containing waypoints dropout. */
			return(1);
		} else {
			return(0);
		}

	} else {	
		return(1);
	}
}

#define WAYVOLUME_QUEUE_LENGTH 100

typedef struct route_queue {
	int depth;
	WAYPOINT_VOLUME *wayvolume;
} ROUTE_QUEUE;

ROUTE_QUEUE Waypoint_Route_Queue[WAYVOLUME_QUEUE_LENGTH];

int Queue_End,Queue_Exec;

int NewFindThisRoute(WAYPOINT_HEADER *waypoints, WAYPOINT_ROUTE *thisroute,WAYPOINT_VOLUME *startwaypoint, WAYPOINT_VOLUME *endwaypoint) {

	int a;
	int terminate;

	/* Trivial case... */
	if ( (startwaypoint->contains_npc) && (startwaypoint->contains_target) ) {
		thisroute->num_waypoints=0;
		thisroute->start=startwaypoint;
		thisroute->second=startwaypoint;
		thisroute->last=startwaypoint;
		thisroute->first_link=NULL;
		return(1);
	}

	/* Setup... */

	thisroute->num_waypoints=0;
	thisroute->start=startwaypoint;
	thisroute->second=NULL;
	thisroute->last=endwaypoint;
	thisroute->first_link=NULL;

	for (a=0; a<waypoints->num_waypoints; a++) {
		/* Preparation... */
		WAYPOINT_VOLUME *temp_waypoint;

		temp_waypoint=waypoints->first_waypoint;
		temp_waypoint+=a;
		temp_waypoint->workspace=0;
	}

	/* Start with the end point. Look down all the links,
	add all the waypoints there to the queue. */
	#if ALLOW_USE_OF_LONGER_PATHS
		#define STARTING_DEPTH 5000
	#else
		#define STARTING_DEPTH waypoints->num_waypoints
	#endif

	Waypoint_Route_Queue[0].depth=STARTING_DEPTH;
	Waypoint_Route_Queue[0].wayvolume=endwaypoint;
	endwaypoint->workspace=Waypoint_Route_Queue[0].depth;
	Waypoint_Route_Queue[1].wayvolume=NULL; /* To set a standard. */

	Queue_End=1;
	Queue_Exec=0;
	terminate=0;

	while ( (Waypoint_Route_Queue[Queue_Exec].wayvolume!=NULL)&&(terminate==0) ) {
		
		WAYPOINT_VOLUME *this_waypoint;
		int current_link=0;

		this_waypoint=Waypoint_Route_Queue[Queue_Exec].wayvolume;

		//don't always start with the first link so that the ai can arbitrarily
		//choose between routes of equal length
		if(this_waypoint->num_links)
		{
			current_link=GlobalLinkShift % this_waypoint->num_links;
		}
		for (a=0; a<this_waypoint->num_links; a++) {
			
			WAYPOINT_LINK *link_to_next;
			WAYPOINT_VOLUME *next_waypoint;
			int exec_this_link;
			
			link_to_next=this_waypoint->first_link;
			link_to_next+=current_link;
	
			current_link++;
			if(current_link==this_waypoint->num_links)current_link=0;

			if ( (Global_NPC_Type!=1) && (link_to_next->link_flags&linkflag_alienonly) ) {
				exec_this_link=0;
			} else {
				exec_this_link=1;
			}

			if ( ((link_to_next->link_flags&linkflag_oneway)==0)&&(exec_this_link) ) {
			
				next_waypoint=waypoints->first_waypoint;
				next_waypoint+=link_to_next->link_target_index;
		
				/* Got the next waypoint. */

				if (next_waypoint==startwaypoint) {
					/* Found the route. */
					thisroute->start=next_waypoint;
					thisroute->second=this_waypoint;
					thisroute->last=endwaypoint;
					thisroute->num_waypoints=1+STARTING_DEPTH-this_waypoint->workspace;
					thisroute->first_link=link_to_next;
					terminate=1;
					textprint("Got a route.\n");
				#if ALLOW_USE_OF_LONGER_PATHS
				}else if (next_waypoint->workspace+next_waypoint->weighting<this_waypoint->workspace) {
					/* Add to queue. */
					Waypoint_Route_Queue[Queue_End].wayvolume=next_waypoint;
					Waypoint_Route_Queue[Queue_End].depth=Waypoint_Route_Queue[Queue_Exec].depth-next_waypoint->weighting;
					next_waypoint->workspace=Waypoint_Route_Queue[Queue_End].depth;
					{
						/*Need to move this new entry down the queue until the entries are sorted
						by depth*/
						int this_index=Queue_End;
						int prev_index;
						while(1)
						{
							prev_index=this_index-1;
							if(prev_index==-1)prev_index=WAYVOLUME_QUEUE_LENGTH-1;

							if(Waypoint_Route_Queue[this_index].depth>Waypoint_Route_Queue[prev_index].depth)
							{
								/*Swap two entries in the queue*/
								int temp_depth=Waypoint_Route_Queue[this_index].depth;
								WAYPOINT_VOLUME * temp_wayvolume=Waypoint_Route_Queue[this_index].wayvolume; 
								Waypoint_Route_Queue[this_index].depth=Waypoint_Route_Queue[prev_index].depth;
								Waypoint_Route_Queue[this_index].wayvolume=Waypoint_Route_Queue[prev_index].wayvolume;
								Waypoint_Route_Queue[prev_index].depth=temp_depth;
								Waypoint_Route_Queue[prev_index].wayvolume=temp_wayvolume;
							}
							else
							{
								break;
							}

							this_index=prev_index;

						}
					}
					Queue_End++;
					if (Queue_End>=WAYVOLUME_QUEUE_LENGTH) {
						Queue_End=0;
						textprint("Wrapping Waypoint Queue!\n");
					}
					Waypoint_Route_Queue[Queue_End].wayvolume=NULL;
					LOCALASSERT(Queue_End!=Queue_Exec); //if this happens the queue probably needs to be longer

				}
				#else
				}else if (next_waypoint->workspace<this_waypoint->workspace) {
					/* Add to queue. */
					Waypoint_Route_Queue[Queue_End].wayvolume=next_waypoint;
					Waypoint_Route_Queue[Queue_End].depth=Waypoint_Route_Queue[Queue_Exec].depth-1;
					next_waypoint->workspace=Waypoint_Route_Queue[Queue_End].depth;
					
					Queue_End++;
					if (Queue_End>=WAYVOLUME_QUEUE_LENGTH) {
						Queue_End=0;
						textprint("Wrapping Waypoint Queue!\n");
					}
					Waypoint_Route_Queue[Queue_End].wayvolume=NULL;
					LOCALASSERT(Queue_End!=Queue_Exec); //if this happens the queue probably needs to be longer

				}
				#endif
	
			}
	
		}

		/* Done all the links. */

		Queue_Exec++;
		if (Queue_Exec>=WAYVOLUME_QUEUE_LENGTH) Queue_Exec=0;

	}

	/* Still here? */

	if (terminate) {
		/* We found a route! */
		return(1);
	} else {
		/* We didn't find a route... oh dear. */
		thisroute->start=startwaypoint;
		thisroute->second=endwaypoint;
		thisroute->last=endwaypoint;
		thisroute->first_link=NULL;
		/* |(&(*$*&"^)(*!*&%&%!!!!! */
		return(0);
	}

}


extern int AlienIsEncouragedToCrawl(void) {

	if (Global_Route.first_link==NULL) {
		#if 0
		if (Global_Route.start!=NULL) {
			if (Global_Route.second!=NULL) {
				if (Global_Route.start->flags&wayflag_cancrawl) {
					if (Global_Route.second->flags&wayflag_cancrawl) {
						return(1);
					}
				}
			}
		}
		#endif
		return(0);
	} else {
		/* Look at this link. */
		if (Global_Route.first_link->link_flags&linkflag_alienonly) {
			return(1);
		} else {
			return(0);
		}
	}
}

WAYPOINT_VOLUME *GetPositionValidity(MODULE *conmod, VECTORCH *position, VECTORCH *suggestion) {

	WAYPOINT_HEADER *waypoints;
	WAYPOINT_VOLUME *this_waypoint,*retval;
	int a;

	/* Added for near bimble state. */

	waypoints=conmod->m_waypoints;
	retval=NULL;

	if (waypoints==NULL) {
		return(NULL);
	}
	
	suggestion->vx=position->vx;
	suggestion->vy=position->vy;
	suggestion->vz=position->vz;

	for (a=0; a<waypoints->num_waypoints; a++) {

		VECTORCH modtargetpos;
		
		this_waypoint=waypoints->first_waypoint;
		this_waypoint+=a;

		this_waypoint->contains_npc=0;
		this_waypoint->contains_target=0;
				
		modtargetpos.vx=position->vx-conmod->m_world.vx;
		modtargetpos.vy=position->vy-conmod->m_world.vy;
		modtargetpos.vz=position->vz-conmod->m_world.vz;

		if (WaypointContainsPoint(this_waypoint,&modtargetpos)) {
			retval=this_waypoint;
		}
	}

	if (retval) {
		return(retval);
	}

	/* Try again, sneakily. */

	for (a=0; a<waypoints->num_waypoints; a++) {

		VECTORCH modtargetpos;
		
		this_waypoint=waypoints->first_waypoint;
		this_waypoint+=a;

		this_waypoint->contains_npc=0;
		this_waypoint->contains_target=0;
				
		modtargetpos.vx=position->vx-conmod->m_world.vx;
		modtargetpos.vy=position->vy-conmod->m_world.vy;
		modtargetpos.vz=position->vz-conmod->m_world.vz;

		if (WaypointContainsPoint_2d(this_waypoint,&modtargetpos)) {
			retval=this_waypoint;
		}
	}

	if (retval) {
		suggestion->vy=(retval->max_extents.vy+retval->min_extents.vy)>>1;
		suggestion->vy+=conmod->m_world.vy;
		return(retval);
	}
	
	return(NULL);
}

void GetTargetPositionInWaypoint(WAYPOINT_VOLUME *waypoint,VECTORCH *output) {

	int minx,maxx,miny,maxy,minz,maxz;
	int rangex,rangey,rangez;

	minx=waypoint->centre.vx+((waypoint->min_extents.vx*3)>>2);
	miny=waypoint->centre.vy+((waypoint->min_extents.vy*3)>>2);
	minz=waypoint->centre.vz+((waypoint->min_extents.vz*3)>>2);
							 
	maxx=waypoint->centre.vx+((waypoint->max_extents.vx*3)>>2);
	maxy=waypoint->centre.vy+((waypoint->max_extents.vy*3)>>2);
	maxz=waypoint->centre.vz+((waypoint->max_extents.vz*3)>>2);

	rangex=maxx-minx;			
	rangey=maxy-miny;			
	rangez=maxz-minz;			

	output->vx=minx+(FastRandom()%rangex);
	output->vy=miny+(FastRandom()%rangey);
	output->vz=minz+(FastRandom()%rangez);

	GLOBALASSERT(WaypointContainsPoint(waypoint,output));

}
