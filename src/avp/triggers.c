/*KJL***********************************************************************
* I know "triggers.c" is a strange name, but I did write this on a Monday. *
***********************************************************************KJL*/
#include "3dc.h"
#include "inline.h"
#include "module.h"					  

#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"
#include "bh_types.h"
#include "huddefs.h"
#include "triggers.h"
#include "pldnet.h"
#include "los.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/* in mm */
#define ACTIVATION_Z_RANGE 3000
#define ACTIVATION_X_RANGE 1000
#define ACTIVATION_Y_RANGE 1000


extern int NumOnScreenBlocks;
extern DISPLAYBLOCK *OnScreenBlockList[];
extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;


void OperateObjectInLineOfSight(void)
{
	int numberOfObjects = NumOnScreenBlocks;
	
	DISPLAYBLOCK *nearestObjectPtr=0;
	int nearestMagnitude=ACTIVATION_X_RANGE*ACTIVATION_X_RANGE + ACTIVATION_Y_RANGE*ACTIVATION_Y_RANGE;
	
	while (numberOfObjects)
	{
		DISPLAYBLOCK* objectPtr = OnScreenBlockList[--numberOfObjects];
		GLOBALASSERT(objectPtr);
		
		/* does object have a strategy block? */
		if (objectPtr->ObStrategyBlock)
		{		
			AVP_BEHAVIOUR_TYPE behaviour = objectPtr->ObStrategyBlock->I_SBtype;
			
			/* is it operable? */
			if( (I_BehaviourBinarySwitch == behaviour)
			  ||(I_BehaviourLinkSwitch == behaviour) 
			  ||(I_BehaviourAutoGun == behaviour) 
			  ||(I_BehaviourDatabase == behaviour) )
			{
				/* is it in range? */
				if (objectPtr->ObView.vz > 0 && objectPtr->ObView.vz < ACTIVATION_Z_RANGE)
				{
					int absX = objectPtr->ObView.vx;
				   	int absY = objectPtr->ObView.vy;

					if (absX<0) absX=-absX;
					if (absY<0) absY=-absY;
					
					if (absX < ACTIVATION_X_RANGE && absY < ACTIVATION_Y_RANGE)
					{
						int magnitude = (absX*absX + absY*absY);

						if (nearestMagnitude > magnitude)
						{
							nearestMagnitude = magnitude;
							nearestObjectPtr = objectPtr;
						}
					}
				}
			}
		}
	}

	/* if we found a suitable object, operate it */
	if (nearestObjectPtr)
	{
		//only allow activation if you have a line of sight to the switch
		//allow the switch to be activated anyway for the moment
		if(IsThisObjectVisibleFromThisPosition_WithIgnore(Player,nearestObjectPtr,&nearestObjectPtr->ObWorld,10000))
		{
			switch(nearestObjectPtr->ObStrategyBlock->I_SBtype)
			{
				case I_BehaviourBinarySwitch:
				{
					if(AvP.Network!=I_No_Network)
					{
						AddNetMsg_LOSRequestBinarySwitch(nearestObjectPtr->ObStrategyBlock);
					}
					RequestState(nearestObjectPtr->ObStrategyBlock,1, 0);
					break;
				}
				case I_BehaviourLinkSwitch:
				{
					if(AvP.Network!=I_No_Network)
					{
						AddNetMsg_LOSRequestBinarySwitch(nearestObjectPtr->ObStrategyBlock);
					}
					RequestState(nearestObjectPtr->ObStrategyBlock,1, 0);
					break;
				}
				case I_BehaviourAutoGun:
				{
					RequestState(nearestObjectPtr->ObStrategyBlock,1, 0);
					break;
				}
				case I_BehaviourDatabase:
				{
					AvP.GameMode = I_GM_Menus;
					AvP.DatabaseAccessNum=((DATABASE_BLOCK *)nearestObjectPtr->ObStrategyBlock->SBdataptr)->num;
					break;
				}
				default:
					break;
			}
		}
	}
    
	return;
}


BOOL AnythingInMyModule(MODULE* my_mod)
{
	
		// simple overlap test


	//	this used within level - find objects in module
	// all will have sbs

	int i;
	int max_x, min_x, max_y, min_y, max_z, min_z;	

	max_x = my_mod->m_maxx + my_mod->m_world.vx;
	min_x = my_mod->m_minx + my_mod->m_world.vx;
	max_y = my_mod->m_maxy + my_mod->m_world.vy;
	min_y = my_mod->m_miny + my_mod->m_world.vy;
	max_z = my_mod->m_maxz + my_mod->m_world.vz;
	min_z = my_mod->m_minz + my_mod->m_world.vz;


	for(i = 0; i < NumActiveStBlocks; i++)
		{
			VECTORCH obj_world;
			STRATEGYBLOCK	*sbptr;
			DYNAMICSBLOCK	*dynptr;			

			sbptr = ActiveStBlockList[i];

			if(!(dynptr = sbptr->DynPtr))
				continue;
			
			obj_world = dynptr->Position;

			if(obj_world.vx < max_x)
				if(obj_world.vx > min_x)
					if(obj_world.vz < max_z)
						if(obj_world.vz > min_z)
							if(obj_world.vy < max_y)
								if(obj_world.vy > min_y)
									{
										return(1);
									}
		}

	return(0);
}	
