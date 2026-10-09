/* KJL 14:30:27 06/05/98 - weapon targeting code */
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"
#include "inventry.h"
#include "comp_shp.h"
#include "load_shp.h"
#include "huddefs.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "dynblock.h"
#include "dynamics.h"
#include "lighting.h"
#include "pvisible.h"
#include "bh_alien.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_fhug.h"
#include "bh_marin.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "bh_agun.h"
#include "weapons.h"
#include "avpview.h"

#include "psnd.h"
#include "vision.h"
#include "plat_shp.h"

#include "particle.h"
#include "psndproj.h"
#include "showcmds.h"
#include "los.h"
#include <math.h>
 

#include "paintball.h"
/* for win 95 net support */
#include "pldghost.h"
#include "pldnet.h"

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/

void SmartTarget_GetCofM(DISPLAYBLOCK *target,VECTORCH *viewSpaceOutput);
void GetTargetingPointOfObject(DISPLAYBLOCK *objectPtr, VECTORCH *targetPtr);

extern int NumOnScreenBlocks;
extern DISPLAYBLOCK *OnScreenBlockList[];
extern struct Target PlayersTarget;
extern VECTORCH GunMuzzleDirectionInVS;
extern VECTORCH GunMuzzleDirectionInWS;
extern int NormalFrameTime;
extern int Weapon_ThisBurst;

/* stuff to do with where a gun is pointing */
extern int GunMuzzleSightX, GunMuzzleSightY;
/* In 16.16 for smoothness. On-screen coords indicating to where the gun's muzzle is pointing */

int SmartTargetSightX, SmartTargetSightY;
char CurrentlySmartTargetingObject;
DISPLAYBLOCK *SmartTarget_Object;
DISPLAYBLOCK *Old_SmartTarget_Object;

void CalculateWhereGunIsPointing(TEMPLATE_WEAPON_DATA *twPtr, PLAYER_WEAPON_DATA *weaponPtr);
void CalculatePlayersTarget(TEMPLATE_WEAPON_DATA *twPtr, PLAYER_WEAPON_DATA *weaponPtr);
DISPLAYBLOCK *SmartTarget_GetNewTarget(void);
int SmartTarget_TargetFilter(STRATEGYBLOCK *candidate);

void CalculateWhereGunIsPointing(TEMPLATE_WEAPON_DATA *twPtr, PLAYER_WEAPON_DATA *weaponPtr)
{
	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];

	extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
	MATRIXCH matrix	= VDBPtr->VDB_Mat;

	TransposeMatrixCH(&matrix);

//	textprint("Calculating where gun is pointing...\n");

	/* unnormalised vector in the direction	which the gun's muzzle is pointing, IN VIEW SPACE */				
	/* very useful when considering sprites, which lie in a Z-plane in view space */
 	GunMuzzleDirectionInVS.vz = 65536;
    GunMuzzleDirectionInVS.vx = 
    	(GunMuzzleSightX-(ScreenDescriptorBlock.SDB_Width<<15))/(VDBPtr->VDB_ProjX);
	GunMuzzleDirectionInVS.vy = 
		(((GunMuzzleSightY-(ScreenDescriptorBlock.SDB_Height<<15))/(VDBPtr->VDB_ProjY))*3)/4;
    
	/* Now fudge for gun judder! */
 	if ((twPtr->UseStateMovement==0)||(weaponPtr->WeaponIDNumber == WEAPON_MINIGUN)) {
		if ((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY) 
			||( (weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY)&&(weaponPtr->WeaponIDNumber == WEAPON_TWO_PISTOLS) )){
			if ((twPtr->PrimaryIsRapidFire)||(weaponPtr->WeaponIDNumber == WEAPON_TWO_PISTOLS)) {

				EULER judder;
				MATRIXCH juddermat;

				if (twPtr->RecoilMaxRandomZ>0) {
					weaponPtr->PositionOffset.vz = (FastRandom()%twPtr->RecoilMaxRandomZ) - twPtr->RecoilMaxZ;
				}
								
				if ((Weapon_ThisBurst>0)||(weaponPtr->WeaponIDNumber == WEAPON_TWO_PISTOLS)) {

					/* jiggle the weapon around when you shoot */
					int speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);
					/* speed should be between ~0 and ~27000 (jumping alien). ~15000 is a moving marine. */
					
					if (twPtr->RecoilMaxXTilt>0) {
						judder.EulerX=(FastRandom()%twPtr->RecoilMaxXTilt)-twPtr->RecoilMaxXTilt/2;
					} else {
						judder.EulerX=0;
					}
					if (twPtr->RecoilMaxYTilt>0) {
						judder.EulerY=(FastRandom()%twPtr->RecoilMaxYTilt)-twPtr->RecoilMaxYTilt/2;
					} else {
						judder.EulerY=0;
					}
					judder.EulerZ=0;
	
					judder.EulerX=MUL_FIXED(judder.EulerX,(ONE_FIXED+(speed<<2)));
					judder.EulerY=MUL_FIXED(judder.EulerY,(ONE_FIXED+(speed<<2)));

					judder.EulerX&=wrap360;
					judder.EulerY&=wrap360;
	
					CreateEulerMatrix(&judder,&juddermat);
					RotateVector(&GunMuzzleDirectionInVS,&juddermat);
				}
			}
		} else {
			/* Recentre Z offset. */
			int linearCenteringSpeed = MUL_FIXED(300,NormalFrameTime);

			if (weaponPtr->PositionOffset.vz > 0 )
			{
				weaponPtr->PositionOffset.vz -= linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vz < 0) weaponPtr->PositionOffset.vz = 0;	
			}
			else if (weaponPtr->PositionOffset.vz < 0 )
			{
				weaponPtr->PositionOffset.vz += linearCenteringSpeed;
				if (weaponPtr->PositionOffset.vz > 0) weaponPtr->PositionOffset.vz = 0;	
			}

		}
	}

    GunMuzzleDirectionInWS = GunMuzzleDirectionInVS;
    /* rotate vector into world space and then normalise */
    RotateVector(&GunMuzzleDirectionInWS,&matrix);
	Normalise(&GunMuzzleDirectionInWS);

}

void CalculatePlayersTarget(TEMPLATE_WEAPON_DATA *twPtr, PLAYER_WEAPON_DATA *weaponPtr)
{

	CalculateWhereGunIsPointing(twPtr,weaponPtr);

	FindPolygonInLineOfSight(&GunMuzzleDirectionInWS, &Global_VDB_Ptr->VDB_World, 1,Player);

	PlayersTarget.DispPtr =  LOS_ObjectHitPtr;
	PlayersTarget.Distance = LOS_Lambda;
	PlayersTarget.HModelSection = LOS_HModel_Section;
	
	if (PaintBallMode.IsOn)
	{
		PaintBallMode.TargetDispPtr = LOS_ObjectHitPtr;
		PaintBallMode.TargetPosition = LOS_Point;
		PaintBallMode.TargetNormal = LOS_ObjectNormal;
	}

	//textprint("Exiting CPT - PT.DP is %x, PT.HMS is %x\n",PlayersTarget.DispPtr,PlayersTarget.HModelSection);

	if (PlayersTarget.DispPtr) {
		if (PlayersTarget.HModelSection) {
			if (PlayersTarget.HModelSection->my_controller!=PlayersTarget.DispPtr->HModelControlBlock) {
				PlayersTarget.HModelSection=NULL;
			}
		}
	}

	/* did we hit anything? */
	if (PlayersTarget.DispPtr)
	{
		PlayersTarget.Position = LOS_Point;
	}
	else
	{
		/* pretend the target is right in front of the player, but a very long way off */
		PlayersTarget.Position.vx = Global_VDB_Ptr->VDB_World.vx + (Global_VDB_Ptr->VDB_Mat.mat13<<7);
		PlayersTarget.Position.vy = Global_VDB_Ptr->VDB_World.vy + (Global_VDB_Ptr->VDB_Mat.mat23<<7);
		PlayersTarget.Position.vz = Global_VDB_Ptr->VDB_World.vz + (Global_VDB_Ptr->VDB_Mat.mat33<<7);
		PlayersTarget.HModelSection = NULL;
	}
	if (ShowDebuggingText.Target)
	{
		PrintDebuggingText("Target Position: %d %d %d\n",PlayersTarget.Position.vx,PlayersTarget.Position.vy,PlayersTarget.Position.vz);
	}

	if (PlayersTarget.HModelSection) {
		GLOBALASSERT(PlayersTarget.DispPtr->HModelControlBlock==PlayersTarget.HModelSection->my_controller);
	}
  	
	if(AvP.Network!=I_No_Network) 
	{
		AddNetMsg_PredatorLaserSights(&PlayersTarget.Position,&LOS_ObjectNormal,PlayersTarget.DispPtr);
	}

	
	/* find position/orientation of predator's targeting sights */
	if ( (AvP.PlayerType == I_Predator)
	   &&((weaponPtr->WeaponIDNumber == WEAPON_PRED_RIFLE)
	    ||(weaponPtr->WeaponIDNumber == WEAPON_PRED_SHOULDERCANNON)) )
	{
		int i=2;

		VECTORCH offset[3] =
		{
			{0,-50,0},
			{43,25,0},
			{-43,25,0},
		};

		MATRIXCH matrix = Global_VDB_Ptr->VDB_Mat;
		TransposeMatrixCH(&matrix);
			
		do
		{
			VECTORCH position = offset[i];

		  	RotateVector(&position,&matrix);
			position.vx += Global_VDB_Ptr->VDB_World.vx;
			position.vy += Global_VDB_Ptr->VDB_World.vy;
			position.vz += Global_VDB_Ptr->VDB_World.vz;
			FindPolygonInLineOfSight(&GunMuzzleDirectionInWS, &position, 1,Player);
			PredatorLaserTarget.Normal[i] = LOS_ObjectNormal;

			if (PlayersTarget.DispPtr)
			{
				PredatorLaserTarget.Position[i] = LOS_Point;	
			}
			else
			{
				/* pretend the target is right in front of the player, but a very long way off */
				PredatorLaserTarget.Position[i].vx = Global_VDB_Ptr->VDB_World.vx + (Global_VDB_Ptr->VDB_Mat.mat13<<7);
				PredatorLaserTarget.Position[i].vy = Global_VDB_Ptr->VDB_World.vy + (Global_VDB_Ptr->VDB_Mat.mat23<<7);
				PredatorLaserTarget.Position[i].vz = Global_VDB_Ptr->VDB_World.vz + (Global_VDB_Ptr->VDB_Mat.mat33<<7);
			}

		}
		while(i--);
		PredatorLaserTarget.ShouldBeDrawn=1;
  	}
	else
	{
		PredatorLaserTarget.ShouldBeDrawn=0;
	}


}

BOOL CalculateFiringSolution(VECTORCH* firing_pos,VECTORCH* target_pos,VECTORCH* target_vel,int projectile_speed,VECTORCH* solution)
{
	VECTORCH normal; //normal from firer to target
	VECTORCH rotated_vel; 
	VECTORCH rotated_solution;
	MATRIXCH mat;
	
	int distance_to_target;

	GLOBALASSERT(firing_pos);
	GLOBALASSERT(target_pos);
	GLOBALASSERT(target_vel);
	GLOBALASSERT(projectile_speed);
	GLOBALASSERT(solution);

	//get a normalised vector from start to destination
	normal=*target_pos;
	SubVector(firing_pos,&normal);

	if(!normal.vx && !normal.vy && !normal.vz)
		return FALSE;

	distance_to_target=Approximate3dMagnitude(&normal);
	Normalise(&normal);

	//calculate a matrix that will rotate the normal to the zaxis
	{
		//normal will be the third row
		VECTORCH row1,row2;
		

		if(normal.vx>30000 || normal.vx<-30000 || normal.vy>30000 || normal.vy<-30000)
		{
			row1.vx=-normal.vy;
			row1.vy=normal.vx;
			row1.vz=0;
		}
		else 
		{
			row1.vx=-normal.vz;
			row1.vy=0;
			row1.vz=normal.vx;
		}
		Normalise(&row1);

		CrossProduct(&normal,&row1,&row2);
	
		mat.mat11=row1.vx;
		mat.mat21=row1.vy;
		mat.mat31=row1.vz;

		mat.mat12=row2.vx;
		mat.mat22=row2.vy;
		mat.mat32=row2.vz;

		mat.mat13=normal.vx;
		mat.mat23=normal.vy;
		mat.mat33=normal.vz;

	}


	//apply the rotation to the velocity
	rotated_vel=*target_vel;
	RotateVector(&rotated_vel,&mat);

	//is the target moving too fast?
	if(rotated_vel.vz>=projectile_speed || -rotated_vel.vz>=projectile_speed)
	{
		return FALSE;
	}

	//the x and y components of the rotated solution should match the rotated velocity
	//(scale down by projectile speed , because we want a normalised direction)

	rotated_solution.vx=DIV_FIXED(rotated_vel.vx,projectile_speed);
	rotated_solution.vy=DIV_FIXED(rotated_vel.vy,projectile_speed);
	
	//z=1-(x*x+y*y)
	{
		//not sure we have a fixed point square root
		float x=(float)rotated_solution.vx;
		float y=(float)rotated_solution.vy;
		float z_squared=65536.0*65536.0-(x*x+y*y);
		if(z_squared<0)
		{
			//target moving too fast to hit
			return FALSE;
		}
		rotated_solution.vz=(int)sqrt(z_squared);
	}

	//finally need to rotated solution back
	*solution=rotated_solution;
	TransposeMatrixCH(&mat);
	RotateVector(solution,&mat);
	
	//normalise solution to be on the safe side
	Normalise(solution);
	
	return TRUE;

}

void SmartTarget(int speed,int projectile_speed)
{
	DISPLAYBLOCK *trackedObject;

	if (SmartgunMode==I_Track) {
		trackedObject=SmartTarget_GetNewTarget();	
	} else {
		trackedObject=NULL;
	}

//	textprint("Tracking object %x ",trackedObject);
	{
		int screenX;
		int screenY;
		CurrentlySmartTargetingObject=0;

		/* If there is a valid near object which isn't so close as to cause a division by zero */
		if (trackedObject && (trackedObject->ObView.vz!=0))
		{
			VECTORCH targetView;
			extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
			VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
			#if 0
			STRATEGYBLOCK *sbPtr = trackedObject->ObStrategyBlock;
		   	int offsetX,offsetY;
			
			{
			  	/* calculate offset required to aim at the middle torso rather
				than the sprite's bollocks */
				MATRIXCH mat;
				int offsetMag;
				{
					SHAPEHEADER	*shapePtr = GetShapeData(sbPtr->SBdptr->ObShape);
			   		offsetMag = shapePtr->shapeminy/2;
				}
				
				offsetX = MUL_FIXED(trackedObject->ObMat.mat21,offsetMag);
				offsetY = MUL_FIXED(trackedObject->ObMat.mat22,offsetMag);
			}
			#endif
			/* Set targetView. */
			SmartTarget_GetCofM(trackedObject,&targetView);

			if(projectile_speed)
			{
				//get a firing solution so that projectile should hit if target maintains curremt velocity
				if(trackedObject->ObStrategyBlock)
				{
					if(trackedObject->ObStrategyBlock->DynPtr)
					{
						DYNAMICSBLOCK *dynPtr = trackedObject->ObStrategyBlock->DynPtr;
						if(dynPtr->LinVelocity.vx || dynPtr->LinVelocity.vy || dynPtr->LinVelocity.vz)
						{
							
							VECTORCH velocity=dynPtr->LinVelocity;
							VECTORCH zero={0,0,0};
							VECTORCH solution;
							//rotate velocity into view space
							RotateVector(&velocity,&Global_VDB_Ptr->VDB_Mat);

							if(CalculateFiringSolution(&zero,&targetView,&velocity,projectile_speed,&solution))
							{
								targetView=solution;
							}
							

						}
					}
				}
			}
			
			if (targetView.vz>0)
			{
				screenX = WideMulNarrowDiv
								(				 			
									//trackedObject->ObView.vx,//+offsetX,
									targetView.vx,
									VDBPtr->VDB_ProjX,
									//trackedObject->ObView.vz
									targetView.vz
								);
			   	screenY = WideMulNarrowDiv
			   					(
			   						//trackedObject->ObView.vy,//+offsetY,  
									targetView.vy*4,
			   						VDBPtr->VDB_ProjY,	    	  
									//trackedObject->ObView.vz
									(targetView.vz*3)
								);
		  		CurrentlySmartTargetingObject=1;
			}
			else
			{
		   		screenX=0;
		   		screenY=0;	   
			}
	  	}
	   	else
	   	{
	   		screenX=0;
	   		screenY=0;	   
	   	}
	   	
		
		{
			extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
			int targetX,targetY,dx,dy;
			int targetingSpeed = NormalFrameTime*speed;
			
			/* KJL 14:08:50 09/20/96 - the targeting is FRI, but care has to be taken
			   at very low frame rates to ensure that the sight doesn't jump about
			   all over the place. */
			if (targetingSpeed > 65536)	targetingSpeed=65536;
							
			targetX = (ScreenDescriptorBlock.SDB_Width>>1);
			targetY = (ScreenDescriptorBlock.SDB_Height>>1);
					
			{
				int maxRangeX = (ScreenDescriptorBlock.SDB_Width*14)/32;  /* gives nearly 90% of screen */
				
				if ((screenX>-maxRangeX) && (screenX<maxRangeX))
				{
		  			int maxRangeY = (ScreenDescriptorBlock.SDB_Height*14)/32; /* gives nearly 90% of screen */
			
					if ((screenY>-maxRangeY) && (screenY<maxRangeY))
					{
						targetX += screenX;
						targetY += screenY;
					}
					else CurrentlySmartTargetingObject=0;
				}
				else CurrentlySmartTargetingObject=0;
			}
		  		
			if (speed<=0)
			{
    	        SmartTargetSightX = targetX<<16;
        	    SmartTargetSightY = targetY<<16;
			}
			else
			{
				dx = MUL_FIXED( ((targetX<<16)-SmartTargetSightX), targetingSpeed );
			  	dy = MUL_FIXED( ((targetY<<16)-SmartTargetSightY), targetingSpeed );
		
				/* If the x-coord difference between the sight and the target is small,
				   just move the sight so that it has the same x-coord as the target.
				   This stops the sight from hovering a pixel away from where it should be */
				if (dx>16384 || dx<-16384) SmartTargetSightX += dx;
    	        else SmartTargetSightX = targetX<<16;
            
	            /* Similarly for the y-coord */
				if (dy>16384 || dy<-16384) SmartTargetSightY += dy;
        	    else SmartTargetSightY = targetY<<16;
			}
	 	}
     	
    }
	
	Old_SmartTarget_Object=SmartTarget_Object;

   	if (CurrentlySmartTargetingObject) {
		SmartTarget_Object=trackedObject;
	} else {
		SmartTarget_Object=NULL;
	}
    
	return;
}

#define SMART_TRACKABLE_TARGETS 100

DISPLAYBLOCK *SmartTarget_GetNewTarget(void) {

	int a,b,c;
	int numberOfObjects = NumOnScreenBlocks;
	DISPLAYBLOCK *nearestObjectPtr, *target;
	int nearestObjectDist;
	
	DISPLAYBLOCK *track_array[SMART_TRACKABLE_TARGETS];

	target=NULL;
		
	nearestObjectPtr=NULL;

	for (a=0; a<SMART_TRACKABLE_TARGETS; a++) {
		track_array[a]=NULL;
	}
	a=0;

	while (numberOfObjects--)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[numberOfObjects];
		STRATEGYBLOCK* sbPtr = objectPtr->ObStrategyBlock;
				
		if (sbPtr && sbPtr->DynPtr)
		{
			VECTORCH viewPos;
			/* Arc reject. */
			SmartTarget_GetCofM(objectPtr,&viewPos);
			if (viewPos.vz>0) {
				if (SmartTarget_TargetFilter(sbPtr))	{
					if (a<SMART_TRACKABLE_TARGETS) {
						track_array[a]=objectPtr;
						a++;
					} else {
						/* Arse. Well, I said I didn't like this way. */
					}
				}
			}
		}
	}

	/* Now, filter the track array. */
	b=a;
	c=a;


	while (b--) {

		int notFar;

		nearestObjectDist=SMART_TARGETING_RANGE;
		nearestObjectPtr=NULL;
		notFar=-1;

		a=c;

		while (a) {

			DISPLAYBLOCK* objectPtr;
			STRATEGYBLOCK* sbPtr;

			a--;

			objectPtr = track_array[a];

			if (objectPtr) {
				/* calc distance to player - no parallel strat support yet */
				/* 2d vector from player to object */
				int dist = FandVD_Distance_3d(&objectPtr->ObWorld,&Player->ObWorld);
				
				sbPtr = objectPtr->ObStrategyBlock;

				if (dist<nearestObjectDist)
				{
					nearestObjectDist=dist;
					nearestObjectPtr=objectPtr;
					notFar=a;
				}
			}

		}
		
		if (nearestObjectPtr) {

			GLOBALASSERT(notFar!=-1);

			if (IsThisObjectVisibleFromThisPosition_WithIgnore(track_array[notFar],Player,&(Global_VDB_Ptr->VDB_World),nearestObjectDist) ) {
				/* Valid. */
				target=track_array[notFar];
				break; // Exit loop
			} else {
				/* Remove from list. */
				track_array[notFar]=NULL;
			}		
		}
	}

	return(target);

}

void SmartTarget_GetCofM(DISPLAYBLOCK *target,VECTORCH *viewSpaceOutput) {

	AVP_BEHAVIOUR_TYPE targetType;
	
	/* Get smartgun aiming point. */

	if (target->HModelControlBlock==NULL) {
		*viewSpaceOutput=target->ObView;
		return;
	} else if (target->HModelControlBlock->section_data==NULL) {
		/* Always consider extreme possibilities. */
		*viewSpaceOutput=target->ObView;
		return;
	}

	/* Must be a hierarchy. */

	if (target->ObStrategyBlock->I_SBtype==I_BehaviourNetGhost) {
		
		NETGHOSTDATABLOCK *dataptr;

		dataptr=target->ObStrategyBlock->SBdataptr;

		targetType=dataptr->type;

	} else if (
		(target->ObStrategyBlock->I_SBtype==I_BehaviourMarinePlayer)||
		(target->ObStrategyBlock->I_SBtype==I_BehaviourAlienPlayer)||
		(target->ObStrategyBlock->I_SBtype==I_BehaviourPredatorPlayer)) {
		
		switch(AvP.PlayerType)
		{
			case I_Alien:
				targetType=I_BehaviourAlienPlayer;
				break;
			case I_Predator:
				targetType=I_BehaviourPredatorPlayer;
				break;
			case I_Marine:
				targetType=I_BehaviourMarinePlayer;
				break;
			default:
				GLOBALASSERT(0);
				return;
				break;
		}
		
	} else {
		targetType=target->ObStrategyBlock->I_SBtype;
	}

	/* Now, switch case on targetType. */

	switch (targetType) {
		case I_BehaviourMarine:
		case I_BehaviourMarinePlayer:
			{
				SECTION_DATA *targetsection;

				targetsection=GetThisSectionData(target->HModelControlBlock->section_data,
					"chest");

				if (targetsection==NULL) {
					targetsection=target->HModelControlBlock->section_data;
				}

				if (targetsection->flags&section_data_view_init) {
					*viewSpaceOutput=targetsection->View_Offset;
					return;
				} else {
					/* Whoops. */
					*viewSpaceOutput=target->ObView;
					return;
				}
			}
			break;
		case I_BehaviourPredator:
		case I_BehaviourPredatorPlayer:
			{
				SECTION_DATA *targetsection;

				targetsection=GetThisSectionData(target->HModelControlBlock->section_data,
					"chest");

				if (targetsection==NULL) {
					targetsection=target->HModelControlBlock->section_data;
				}

				if (targetsection->flags&section_data_view_init) {
					*viewSpaceOutput=targetsection->View_Offset;
					return;
				} else {
					/* Whoops. */
					*viewSpaceOutput=target->ObView;
					return;
				}
			}
			break;
		case I_BehaviourAlien:
		case I_BehaviourAlienPlayer:
			{
				SECTION_DATA *targetsection;

				targetsection=GetThisSectionData(target->HModelControlBlock->section_data,
					"chest");

				if (targetsection==NULL) {
					targetsection=target->HModelControlBlock->section_data;
				}

				if (targetsection->flags&section_data_view_init) {
					*viewSpaceOutput=targetsection->View_Offset;
					return;
				} else {
					/* Whoops. */
					*viewSpaceOutput=target->ObView;
					return;
				}
			}
			break;
		default:
			{
				/* General case. */
				if (target->HModelControlBlock->section_data->flags&section_data_view_init) {
					*viewSpaceOutput=target->HModelControlBlock->section_data->View_Offset;
					return;
				} else {
					/* Whoops. */
					*viewSpaceOutput=target->ObView;
					return;
				}
			}
			break;
	}

}



int SmartTarget_TargetFilter(STRATEGYBLOCK *candidate)
{
	PLAYER_WEAPON_DATA *weaponPtr = &(PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot]);
	
	/* KJL 11:56:33 14/10/98 - the Predator's Shoulder cannon can only track objects when used in the correct
	vision mode, e.g. you can only target marines when in infrared mode */
	if ((weaponPtr->WeaponIDNumber == WEAPON_PRED_SHOULDERCANNON)
		||(weaponPtr->WeaponIDNumber == WEAPON_PRED_DISC))
	{
		switch (CurrentVisionMode)
		{
			case VISION_MODE_PRED_SEEALIENS:
			{
				if (candidate->I_SBtype==I_BehaviourAlien && !NPC_IsDead(candidate))
				{
					return 1;
				}
				if (candidate->I_SBtype==I_BehaviourXenoborg && !NPC_IsDead(candidate))
				{
					return 1;
				}
				if (candidate->I_SBtype==I_BehaviourMarine && !NPC_IsDead(candidate))
				{
					MARINE_STATUS_BLOCK *marineStatusPointer;    	
					LOCALASSERT(candidate);
					marineStatusPointer = (MARINE_STATUS_BLOCK *)(candidate->SBdataptr);
					LOCALASSERT(marineStatusPointer);	
					
					if (marineStatusPointer->My_Weapon->Android) {
						return(1);
					} else {
						return(0);
					}
				}
				if (candidate->I_SBtype==I_BehaviourNetGhost)
				{
			   		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)candidate->SBdataptr;

					if (ghostDataPtr->type==I_BehaviourAlienPlayer || ghostDataPtr->type==I_BehaviourAlien)
					{
						return 1;
					}
				}
				break;
			}
			case VISION_MODE_PRED_THERMAL:
			{
				if (candidate->I_SBtype==I_BehaviourMarine && !NPC_IsDead(candidate))
				{
					MARINE_STATUS_BLOCK *marineStatusPointer;    	
					LOCALASSERT(candidate);
					marineStatusPointer = (MARINE_STATUS_BLOCK *)(candidate->SBdataptr);
					LOCALASSERT(marineStatusPointer);	
					
					if (marineStatusPointer->My_Weapon->Android) {
						return(0);
					} else {
						return(1);
					}
				}
				if (candidate->I_SBtype==I_BehaviourNetGhost)
			   	{
			   		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)candidate->SBdataptr;

					if (ghostDataPtr->type==I_BehaviourMarinePlayer || ghostDataPtr->type==I_BehaviourMarine)
					{
						return 1;
					}
				}
				break;

			}
			case VISION_MODE_PRED_SEEPREDTECH:
			{
				if (candidate->I_SBtype==I_BehaviourPredator && !NPC_IsDead(candidate))
				{
					return 1;
				}
				if (candidate->I_SBtype==I_BehaviourNetGhost)
				{
			   		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)candidate->SBdataptr;

					if (ghostDataPtr->type==I_BehaviourPredatorPlayer || ghostDataPtr->type==I_BehaviourPredator)
					{
						/* Check for game type? */
						switch (netGameData.gameType) {
							case NGT_Individual:
								return(1);
								break;
							case NGT_CoopDeathmatch:
								return(0);
								break;
							case NGT_LastManStanding:
								return(0);
								break;
							case NGT_PredatorTag:
								return(1);
								break;
							case NGT_Coop:
								return(0);
								break;
							default:
								return(1);
								break;
						}
						return (1);
					}
				}
				break;
			}
			default:
				break;
		}
		return 0;
	} else if (weaponPtr->WeaponIDNumber == WEAPON_SMARTGUN) {
		/* Don't target marines if Friendly Fire is disabled. */
		if (netGameData.disableFriendlyFire && (netGameData.gameType==NGT_CoopDeathmatch || netGameData.gameType==NGT_Coop)) {
	   		NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)candidate->SBdataptr;
			if (ghostDataPtr->type==I_BehaviourMarinePlayer || ghostDataPtr->type==I_BehaviourMarine) {
				return(0);
			}
		}
	}

	switch (candidate->I_SBtype) {
		case I_BehaviourPredator:
			/* Valid. */
			if (NPC_IsDead(candidate)) {
				return(0);
			} else {
				#if 0
				if (NPCPredatorIsCloaked(candidate)) {
					return(0);
				} else {
					return(1);
				}
				#else
				return(1);
				#endif
			}
			break;
		case I_BehaviourAlien:
		case I_BehaviourQueenAlien:
		case I_BehaviourFaceHugger:
		case I_BehaviourXenoborg:
		case I_BehaviourMarine:
		case I_BehaviourSeal:
		case I_BehaviourPredatorAlien:
			/* Valid. */
			if (NPC_IsDead(candidate)) {
				return(0);
			} else {
				return(1);
			}
			break;
		case I_BehaviourNetGhost:
			{
				NETGHOSTDATABLOCK *dataptr;
				dataptr=candidate->SBdataptr;
				switch (dataptr->type) {
					case I_BehaviourPredatorPlayer:
						#if 0
						if (dataptr->CloakingEffectiveness) {
							return(0);
						} else {
							return(1);
						}
						#else
						return(1);
						#endif
						break;
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourRocket:
					case I_BehaviourFrisbee:
					case I_BehaviourGrenade:
					case I_BehaviourFlareGrenade:
					case I_BehaviourFragmentationGrenade:
					case I_BehaviourClusterGrenade:
					case I_BehaviourNPCPredatorDisc:
					case I_BehaviourPredatorDisc_SeekTrack:
					case I_BehaviourAlien:
						return(1);
						break;
					case I_BehaviourProximityGrenade:
						{
							DYNAMICSBLOCK *dynPtr=candidate->DynPtr;
	
							/* Only visible when moving. */
	
							if (!((dynPtr->Position.vx==dynPtr->PrevPosition.vx)
								&&(dynPtr->Position.vy==dynPtr->PrevPosition.vy)
								&&(dynPtr->Position.vz==dynPtr->PrevPosition.vz) )) {
						
								return(1);
							
							} else {
								return(0);
							}
							
						}
						break;
					default:
						return(0);
						break;
				}
			}
			break;
		default:
			return(0);
			break;
	}

}

void GetTargetingPointOfObject_Far(STRATEGYBLOCK *sbPtr, VECTORCH *targetPtr)
{
	/* Can we use the near one?  This is a more general shell. */
	if (sbPtr->SBdptr) {
		GetTargetingPointOfObject(sbPtr->SBdptr,targetPtr);
		return;
	} else {
		if (sbPtr->DynPtr) {
			*targetPtr=sbPtr->DynPtr->Position;
			targetPtr->vy-=500; /* Just to be on the safe side. */
			return;
		} else {
			/* Aw, gawd! I don't know! There's NO position for this! */
			GLOBALASSERT(0);
		}
	}
	
}

void GetTargetingPointOfObject(DISPLAYBLOCK *objectPtr, VECTORCH *targetPtr)
{
	/* try to look at the centre of the object */
	if (objectPtr->HModelControlBlock)
	{
		SECTION_DATA *targetSectionPtr;
		SECTION_DATA *firstSectionPtr;

	  	ProveHModel(objectPtr->HModelControlBlock,objectPtr);

	  	firstSectionPtr=objectPtr->HModelControlBlock->section_data;
	  	LOCALASSERT(firstSectionPtr);
		LOCALASSERT(firstSectionPtr->flags&section_data_initialised);

		/* look for the object's torso in preference */
		targetSectionPtr =GetThisSectionData(objectPtr->HModelControlBlock->section_data,"chest");

		if (targetSectionPtr)
		{
			LOCALASSERT(targetSectionPtr->flags&section_data_initialised);
			*targetPtr = targetSectionPtr->World_Offset;
		}
		else /* just use the top of the hierarchy then */
		{
			*targetPtr = firstSectionPtr->World_Offset;
		}
	}
	else
	{
		/* KJL 12:23:26 15/05/98 - need to consider the player differently */
		if (objectPtr == Player)
		{
			#if 0
			*targetPtr = Global_VDB_Ptr->VDB_World;
			#else
			int height;

			/* Let's try a little experiment. */
			if (PlayerStatusPtr->IsAlive==0) {
				*targetPtr = Global_VDB_Ptr->VDB_World;
				return;
			}

			if (PlayerStatusPtr->ShapeState!=PMph_Standing) {
				height=-800;
			} else {
				height=-1500;
			}

			targetPtr->vx=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat21,height);
			targetPtr->vy=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat22,height);
			targetPtr->vz=MUL_FIXED(Player->ObStrategyBlock->DynPtr->OrientMat.mat23,height);
			#if 0
			targetPtr->vx+=Player->ObStrategyBlock->DynPtr->Position.vx;
			targetPtr->vy+=Player->ObStrategyBlock->DynPtr->Position.vy;
			targetPtr->vz+=Player->ObStrategyBlock->DynPtr->Position.vz;
			#else
			targetPtr->vx+=Player->ObWorld.vx;
			targetPtr->vy+=Player->ObWorld.vy;
			targetPtr->vz+=Player->ObWorld.vz;
			#endif
			#endif
		}
		else
		{
			*targetPtr = objectPtr->ObWorld;
		}

	}
}
