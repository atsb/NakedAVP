/*KJL*****************************************
* AI_Sight.c handles the NPC's visual senses *
*****************************************KJL*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"
#include "dynamics.h"
#include "los.h"
#include "showcmds.h"
#include "equipmnt.h"
#include "bh_marin.h"
#include "bh_xeno.h"
#include "targeting.h"
#include "bh_weap.h"
#include "bh_agun.h"

#include "ai_sight.h"

#define UseLocalAssert Yes
#include "ourasert.h"

extern int FrisbeeSight_FrustrumReject(STRATEGYBLOCK *sbPtr,VECTORCH *localOffset,STRATEGYBLOCK *target);

int NPCCanSeeTarget(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *target, int viewRange)
{
	int frustrum_test;
	/* connect eyeposition to head */
	VECTORCH eyePosition = {0,-1500,0};

	LOCALASSERT(target);
	LOCALASSERT(sbPtr);

	if (target->containingModule==NULL) {
		return(0);
	}

	if (sbPtr->containingModule==NULL) {
		return(0);
	}
	
	if ((target->SBdptr==NULL)||(sbPtr->SBdptr==NULL)) {
		if ((IsModuleVisibleFromModule(target->containingModule,sbPtr->containingModule))) {
			return(1);
		} else {
			return(0);
		}
	} else {

		switch (sbPtr->I_SBtype) {
			case I_BehaviourFrisbee:
				{
					MATRIXCH WtoL;
					VECTORCH offset, sourcepos, targetpos;
					FRISBEE_BEHAV_BLOCK *frisbeeStatusPointer;
	    			SECTION_DATA *disc_sec;

					LOCALASSERT(sbPtr);
					LOCALASSERT(sbPtr->containingModule); 
					frisbeeStatusPointer = (FRISBEE_BEHAV_BLOCK *)(sbPtr->SBdataptr);    
				    LOCALASSERT(frisbeeStatusPointer);	          		
					/* Arc reject. */
			
					disc_sec=GetThisSectionData(frisbeeStatusPointer->HModelController.section_data,"Mdisk");

					if (disc_sec) {
						WtoL=disc_sec->SecMat;
						sourcepos=disc_sec->World_Offset;
					} else {
						WtoL=sbPtr->DynPtr->OrientMat;
						GetTargetingPointOfObject_Far(sbPtr,&sourcepos);
					}

					GetTargetingPointOfObject_Far(target,&targetpos);
 					
					offset.vx=sourcepos.vx-targetpos.vx;
					offset.vy=sourcepos.vy-targetpos.vy;
					offset.vz=sourcepos.vz-targetpos.vz;
					
					TransposeMatrixCH(&WtoL);
					RotateVector(&offset,&WtoL);

					frustrum_test=FrisbeeSight_FrustrumReject(sbPtr,&offset,target);
				}
				break;
			case I_BehaviourMarine:
			case I_BehaviourSeal:
				{
					MATRIXCH WtoL;
					VECTORCH offset, sourcepos, targetpos;
					MARINE_STATUS_BLOCK *marineStatusPointer;
	    			SECTION_DATA *head_sec;

					LOCALASSERT(sbPtr);
					LOCALASSERT(sbPtr->containingModule); 
					marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				    LOCALASSERT(marineStatusPointer);	          		
					/* Arc reject. */
			
					head_sec=GetThisSectionData(marineStatusPointer->HModelController.section_data,"head");

					if (head_sec) {
						WtoL=head_sec->SecMat;
						sourcepos=head_sec->World_Offset;
					} else {
						WtoL=sbPtr->DynPtr->OrientMat;
						GetTargetingPointOfObject_Far(sbPtr,&sourcepos);
					}

					GetTargetingPointOfObject_Far(target,&targetpos);
 					
					offset.vx=sourcepos.vx-targetpos.vx;
					offset.vy=sourcepos.vy-targetpos.vy;
					offset.vz=sourcepos.vz-targetpos.vz;
					
					TransposeMatrixCH(&WtoL);
					RotateVector(&offset,&WtoL);

					frustrum_test=MarineSight_FrustrumReject(sbPtr,&offset,target);
				}
				break;
			case I_BehaviourXenoborg:
				{
					MATRIXCH WtoL;
					VECTORCH offset, sourcepos, targetpos;
					XENO_STATUS_BLOCK *xenoStatusPointer;
	    			SECTION_DATA *head_sec;

					LOCALASSERT(sbPtr);
					LOCALASSERT(sbPtr->containingModule); 
					xenoStatusPointer = (XENO_STATUS_BLOCK *)(sbPtr->SBdataptr);    
				    LOCALASSERT(xenoStatusPointer);	          		
					/* Arc reject. */
			
					head_sec=GetThisSectionData(xenoStatusPointer->HModelController.section_data,"head");

					if (head_sec) {
						WtoL=head_sec->SecMat;
						sourcepos=head_sec->World_Offset;
					} else {
						WtoL=sbPtr->DynPtr->OrientMat;
						GetTargetingPointOfObject_Far(sbPtr,&sourcepos);
					}

					GetTargetingPointOfObject_Far(target,&targetpos);
 					
					offset.vx=sourcepos.vx-targetpos.vx;
					offset.vy=sourcepos.vy-targetpos.vy;
					offset.vz=sourcepos.vz-targetpos.vz;
					
					TransposeMatrixCH(&WtoL);
					RotateVector(&offset,&WtoL);

					frustrum_test=XenoSight_FrustrumReject(sbPtr,&offset);
				}
				break;
			case I_BehaviourAutoGun:
				{
					/* Less pretentious, based on the SB. */
					MATRIXCH WtoL;
					VECTORCH offset, sourcepos, targetpos;
					/* Arc reject. */
			
					WtoL=sbPtr->DynPtr->OrientMat;
					GetTargetingPointOfObject_Far(sbPtr,&sourcepos);
					GetTargetingPointOfObject_Far(target,&targetpos);
 					
					offset.vx=sourcepos.vx-targetpos.vx;
					offset.vy=sourcepos.vy-targetpos.vy;
					offset.vz=sourcepos.vz-targetpos.vz;
					
					TransposeMatrixCH(&WtoL);
					RotateVector(&offset,&WtoL);

					frustrum_test=AGunSight_FrustrumReject(&offset);
				}
				break;
			default:
				frustrum_test=1;
				break;
		}
		
		if (frustrum_test) {

			RotateVector(&eyePosition,&(sbPtr->DynPtr->OrientMat));

			eyePosition.vx += sbPtr->DynPtr->Position.vx;
			eyePosition.vy += sbPtr->DynPtr->Position.vy;
			eyePosition.vz += sbPtr->DynPtr->Position.vz;

			return IsThisObjectVisibleFromThisPosition_WithIgnore(target->SBdptr,sbPtr->SBdptr,&eyePosition,NPC_MAX_VIEWRANGE);
		}
	}

	return(0);
}
