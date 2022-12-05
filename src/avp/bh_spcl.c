#include "3dc.h"
#include "bh_spcl.h"
#include "dynblock.h"

#define UseLocalAssert Yes

#include "ourasert.h"
#include "vision.h"
#include "plat_shp.h"

extern int NormalFrameTime;

signed int RequestFadeToBlackLevel = 0;

extern DISPLAYBLOCK* Player;

void * InitXenoMorphRoom (void * bhdata, STRATEGYBLOCK * sbptr)
{
	XENO_MORPH_ROOM_DATA * xmrd;
	XENO_MORPH_ROOM_TOOLS_TEMPLATE * xmrtt;
	MORPHCTRL* morphctrl;	
	MORPHHEADER* morphheader;
	MORPHFRAME* morphframe;
	MODULE * my_mod;

 	GLOBALASSERT(sbptr);

	xmrd = (XENO_MORPH_ROOM_DATA *)AllocateMem(sizeof(XENO_MORPH_ROOM_DATA));
	if (!xmrd)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	xmrd->bhvr_type = I_BehaviourXenoborgMorphRoom;

	xmrtt = (XENO_MORPH_ROOM_TOOLS_TEMPLATE *)bhdata;

	xmrd->MainShape = xmrtt->MainShape;
	xmrd->ShutShape = xmrtt->ShutShape;
	xmrd->WallsOutShape = xmrtt->WallsOutShape;
	xmrd->ProbesInShape = xmrtt->ProbesInShape;

	morphctrl = (MORPHCTRL*)AllocateMem(sizeof(MORPHCTRL));
	if (!morphctrl)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphheader = (MORPHHEADER*)AllocateMem(sizeof(MORPHHEADER));
	if (!morphheader)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphframe = (MORPHFRAME*)AllocateMem(sizeof(MORPHFRAME));
	if (!morphframe)
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	morphframe->mf_shape1 = xmrd->MainShape;
	morphframe->mf_shape2 = xmrd->ShutShape;

	morphheader->mph_numframes = 1;
	morphheader->mph_maxframes = ONE_FIXED;
	morphheader->mph_frames = morphframe;

	morphctrl->ObMorphCurrFrame = 0;
	morphctrl->ObMorphFlags = 0;
	morphctrl->ObMorphSpeed = 0;
	morphctrl->ObMorphHeader = morphheader;

	// Copy the names over
	COPY_NAME (sbptr->SBname, xmrtt->nameID);
	COPY_NAME (xmrd->doorID, xmrtt->doorID);

	// Setup module ref
	ConvertModuleNameToPointer (&xmrtt->my_module, MainSceneArray[0]->sm_marray);
	my_mod = xmrtt->my_module.mref_ptr;

	GLOBALASSERT (my_mod);

	my_mod->m_sbptr = sbptr;
	sbptr->SBmoptr = my_mod;
	sbptr->SBmomptr = my_mod->m_mapptr;
	sbptr->SBflags.no_displayblock = 1;
	sbptr->shapeIndex = my_mod->m_mapptr->MapShape;


	xmrd->XMR_Mctrl = morphctrl;
	xmrd->XMR_State = XMRS_Idle;
	xmrd->DoorToRoom = 0;

	sbptr->SBmorphctrl = xmrd->XMR_Mctrl;
	sbptr->SBmorphctrl->ObMorphCurrFrame = 0;

	if(sbptr->SBmomptr)
	{
		sbptr->SBmomptr->MapMorphHeader = sbptr->SBmorphctrl->ObMorphHeader;
	}


	// set up the animation control
	{
		int item_num;
		TXACTRLBLK **pptxactrlblk;		
		int shape_num = my_mod->m_mapptr->MapShape;
		SHAPEHEADER *shptr = GetShapeData(shape_num);
 
		SetupPolygonFlagAccessForShape(shptr);

		pptxactrlblk = &xmrd->tacb;

		for(item_num = 0; item_num < shptr->numitems; item_num ++)
		{
			POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
			LOCALASSERT(poly);
				
			if((Request_PolyFlags((void *)poly)) & iflag_txanim)
			{
				TXACTRLBLK *pnew_txactrlblk;
				int num_seq = 0;

				pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
				if (pnew_txactrlblk)
				{
					pnew_txactrlblk->tac_flags = 0;
					pnew_txactrlblk->tac_item = item_num;
					pnew_txactrlblk->tac_sequence = 0;
					pnew_txactrlblk->tac_node = 0;
					pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);
					pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

					while(pnew_txactrlblk->tac_txarray[num_seq+1])num_seq++;

					/* set the flags in the animation header */
					// we only ever have one frame of animation per sequence -
					// nb this can change talk to richard - one sequence with two frames
					// or mutliple sequences???

		//				pnew_txactrlblk->tac_txah.txa_flags |= txa_flag_play;

					/* change the value held in pptxactrlblk
					 which point to the previous structures "next"
					 pointer*/

					*pptxactrlblk = pnew_txactrlblk;
					pptxactrlblk = &pnew_txactrlblk->tac_next;
				}
				else
				{
					memoryInitialisationFailure = 1;
				}
			}
		}
		*pptxactrlblk=0;
	}

	RequestFadeToBlackLevel = 0;

	return((void*)xmrd);
}

extern MODULE * playerPherModule;

void XenoMorphRoomBehaviour (STRATEGYBLOCK * sbptr)
{
	XENO_MORPH_ROOM_DATA * xmrd = (XENO_MORPH_ROOM_DATA *)sbptr->SBdataptr;
	MORPHCTRL * mctrl = xmrd->XMR_Mctrl;

	GLOBALASSERT (mctrl);
	GLOBALASSERT (mctrl->ObMorphHeader);
	GLOBALASSERT (mctrl->ObMorphHeader->mph_frames);



	switch (xmrd->XMR_State)
	{
		case XMRS_Idle :
		{
			if (sbptr->SBmoptr->m_dptr)
			{

				if (playerPherModule == sbptr->SBmoptr)
				{
					VECTORCH diff;
					diff.vx = Player->ObWorld.vx - sbptr->SBmoptr->m_dptr->ObWorld.vx;
					diff.vy = Player->ObWorld.vy - sbptr->SBmoptr->m_dptr->ObWorld.vy;
					diff.vz = Player->ObWorld.vz - sbptr->SBmoptr->m_dptr->ObWorld.vz;

					if (diff.vx * diff.vx + diff.vz * diff.vz < 200000)
					{
						xmrd->XMR_State = XMRS_SafetyChecks;
					}

				}
				
			}
			break;
		}

		case XMRS_SafetyChecks :
		{
			BOOL can_continue = 1;
			int i;
			// waits to make sure everything is OK before it shuts the walls

			if (xmrd->DoorToRoom)
			{
				PROXDOOR_BEHAV_BLOCK * doorbhv = (PROXDOOR_BEHAV_BLOCK*)xmrd->DoorToRoom->SBdataptr;
				
				doorbhv->door_locked = 1;

				if (doorbhv->door_state != I_door_closed)
				{
					can_continue = 0;
				}

			}
			
			for (i=0; i<NumActiveStBlocks; i++)
			{
				if (ActiveStBlockList[i]->I_SBtype == I_BehaviourMarine || ActiveStBlockList[i]->I_SBtype == I_BehaviourSeal)
				{
					if (ActiveStBlockList[i]->containingModule == sbptr->SBmoptr)
					{
						can_continue = 0;
						break;
					}
				}
			}

			if (sbptr->SBmoptr->m_dptr)
			{

				if (playerPherModule == sbptr->SBmoptr)
				{
					VECTORCH diff;
					diff.vx = Player->ObWorld.vx - sbptr->SBmoptr->m_dptr->ObWorld.vx;
					diff.vy = Player->ObWorld.vy - sbptr->SBmoptr->m_dptr->ObWorld.vy;
					diff.vz = Player->ObWorld.vz - sbptr->SBmoptr->m_dptr->ObWorld.vz;


					if (diff.vx * diff.vx + diff.vz * diff.vz > 200000)
					{
						can_continue = 0;
					}

					if (Player->ObStrategyBlock->DynPtr->Position.vx != Player->ObStrategyBlock->DynPtr->PrevPosition.vx)
					{
						can_continue = 0;
					}
					if (Player->ObStrategyBlock->DynPtr->Position.vy != Player->ObStrategyBlock->DynPtr->PrevPosition.vy)
					{
						can_continue = 0;
					}
					if (Player->ObStrategyBlock->DynPtr->Position.vz != Player->ObStrategyBlock->DynPtr->PrevPosition.vz)
					{
						can_continue = 0;
					}
				}
				else
				{
					can_continue = 0;
				}
				
			}
			else
			{
					can_continue = 0;
					xmrd->XMR_State = XMRS_Idle;
			}


			if (can_continue)
			{
				// closes in player
				mctrl->ObMorphHeader->mph_frames->mf_shape1 = xmrd->MainShape;
				mctrl->ObMorphHeader->mph_frames->mf_shape2 = xmrd->ShutShape;

				mctrl->ObMorphCurrFrame = 0;
				mctrl->ObMorphFlags = mph_flag_play;
				mctrl->ObMorphFlags |= mph_flag_noloop;
				mctrl->ObMorphFlags &= ~mph_flag_reverse;
				mctrl->ObMorphFlags &= ~mph_flag_finished;
				mctrl->ObMorphSpeed = 1 << 18;
				
				xmrd->XMR_State = XMRS_EnclosingPlayer;
			}
			break;
		}

		case XMRS_EnclosingPlayer :
		{
			UpdateMorphing(mctrl);

			if(mctrl->ObMorphFlags & mph_flag_finished)		
			{
				mctrl->ObMorphHeader->mph_frames->mf_shape1 = xmrd->ShutShape;
				mctrl->ObMorphHeader->mph_frames->mf_shape2 = xmrd->WallsOutShape;

				mctrl->ObMorphCurrFrame = 0;

				mctrl->ObMorphFlags = mph_flag_play;
				mctrl->ObMorphFlags |= mph_flag_noloop;
				mctrl->ObMorphFlags &= ~mph_flag_reverse;
				mctrl->ObMorphFlags &= ~mph_flag_finished;
				mctrl->ObMorphSpeed = 1 << 13;
			
				xmrd->XMR_State = XMRS_WallsOut;
			}

			break;
		}

		case XMRS_WallsOut :
		{
			UpdateMorphing(mctrl);

			if(mctrl->ObMorphFlags & mph_flag_finished)		
			{
				mctrl->ObMorphHeader->mph_frames->mf_shape1 = xmrd->WallsOutShape;
				mctrl->ObMorphHeader->mph_frames->mf_shape2 = xmrd->ProbesInShape;
				
				mctrl->ObMorphCurrFrame = 0;

				mctrl->ObMorphFlags = mph_flag_play;
				mctrl->ObMorphFlags |= mph_flag_noloop;
				mctrl->ObMorphFlags &= ~mph_flag_reverse;
				mctrl->ObMorphFlags &= ~mph_flag_finished;
				mctrl->ObMorphSpeed = 1 << 13;
			
				xmrd->XMR_State = XMRS_ProbesIn;
			}
			break;
		}

		case XMRS_ProbesIn :
		{
			UpdateMorphing(mctrl);
			if(mctrl->ObMorphFlags & mph_flag_finished)		
			{
				d3d_light_ctrl.ctrl = LCCM_CONSTCOLOUR;
				d3d_light_ctrl.r = ONE_FIXED - RequestFadeToBlackLevel;
				d3d_light_ctrl.g = ONE_FIXED - RequestFadeToBlackLevel;
				d3d_light_ctrl.b = ONE_FIXED - RequestFadeToBlackLevel;

				xmrd->XMR_State = XMRS_FadeToBlack;
			}
			break;
		}

		case XMRS_FadeToBlack :
		{
			RequestFadeToBlackLevel += (NormalFrameTime >> 1);


			if (RequestFadeToBlackLevel > (ONE_FIXED))
			{
				RequestFadeToBlackLevel = ONE_FIXED;
				xmrd->timer = 0;
				xmrd->XMR_State = XMRS_Process;
			}

				d3d_light_ctrl.r = ONE_FIXED - RequestFadeToBlackLevel;
				d3d_light_ctrl.g = ONE_FIXED - RequestFadeToBlackLevel;
				d3d_light_ctrl.b = ONE_FIXED - RequestFadeToBlackLevel;

			break;
		}

		case XMRS_Process :
		{
			xmrd->timer += NormalFrameTime;
			if (xmrd->timer > (ONE_FIXED << 3))
			{
				if(sbptr->SBdptr)
				{
					SHAPEHEADER * pis = GetShapeData(xmrd->ProbesInShape);
					SHAPEHEADER * ms = GetShapeData(xmrd->MainShape);

					sbptr->SBdptr->ObMorphCtrl = 0;

					xmrd->pis_items_str = pis->items;
					xmrd->pis_sht_str = pis->sh_textures;

					pis->items = ms->items;
					pis->sh_textures = ms->sh_textures;
					pis->sh_instruction[4].sh_instr_data = ms->sh_instruction[4].sh_instr_data;

					sbptr->SBdptr->ObTxAnimCtrlBlks = xmrd->tacb;

					sbptr->SBdptr->ObShape = xmrd->ProbesInShape;
					sbptr->SBdptr->ObShapeData = GetShapeData(xmrd->ProbesInShape);

					// This moves the player to the required location

					Player->ObStrategyBlock->DynPtr->Position = sbptr->SBdptr->ObWorld;

					Player->ObStrategyBlock->DynPtr->Position.vy += 1472;
					Player->ObStrategyBlock->DynPtr->Position.vx -= 900;
					Player->ObStrategyBlock->DynPtr->PrevPosition = Player->ObStrategyBlock->DynPtr->Position;

					Player->ObStrategyBlock->DynPtr->OrientEuler.EulerX = 0;
					Player->ObStrategyBlock->DynPtr->OrientEuler.EulerY = 1024;
					Player->ObStrategyBlock->DynPtr->OrientEuler.EulerZ = 0;

					Player->ObStrategyBlock->DynPtr->PrevOrientEuler = Player->ObStrategyBlock->DynPtr->OrientEuler;
					CreateEulerMatrix (&Player->ObStrategyBlock->DynPtr->OrientEuler, 
														&Player->ObStrategyBlock->DynPtr->OrientMat);

					TransposeMatrixCH(&Player->ObStrategyBlock->DynPtr->OrientMat);

					Player->ObStrategyBlock->DynPtr->PrevOrientMat = Player->ObStrategyBlock->DynPtr->OrientMat;

					xmrd->XMR_State = XMRS_Return;
				}
				
			}
			break;
		}

		case XMRS_Return :
		{
			RequestFadeToBlackLevel -= (NormalFrameTime >> 1);
			if (RequestFadeToBlackLevel <= 0)
			{
				RequestFadeToBlackLevel = 0;

				xmrd->timer = 0;
				xmrd->XMR_State = XMRS_ReleasePlayer;

				d3d_light_ctrl.ctrl = LCCM_NORMAL;
			}

			d3d_light_ctrl.r = ONE_FIXED - RequestFadeToBlackLevel;
			d3d_light_ctrl.g = ONE_FIXED - RequestFadeToBlackLevel;
			d3d_light_ctrl.b = ONE_FIXED - RequestFadeToBlackLevel;
			break;
		}

		case XMRS_ReleasePlayer :
		{
			xmrd->timer += NormalFrameTime;

			if (xmrd->timer > (ONE_FIXED << 2))
			{
				sbptr->SBdptr->ObShape = xmrd->MainShape;
				sbptr->SBdptr->ObShapeData = GetShapeData(xmrd->MainShape);

				sbptr->SBdptr->ObMorphCtrl = mctrl;
				mctrl->ObMorphHeader->mph_frames->mf_shape1 = xmrd->ProbesInShape;
				mctrl->ObMorphHeader->mph_frames->mf_shape2 = xmrd->MainShape;

				mctrl->ObMorphCurrFrame = 0;

				mctrl->ObMorphFlags = mph_flag_play;
				mctrl->ObMorphFlags |= mph_flag_noloop;
				mctrl->ObMorphFlags &= ~mph_flag_reverse;
				mctrl->ObMorphFlags &= ~mph_flag_finished;
				mctrl->ObMorphSpeed = 1 << 15;
				
				xmrd->XMR_State = XMRS_Finished;
			}
			break;
		}

		case XMRS_Finished :
		{
			if(!(mctrl->ObMorphFlags & mph_flag_finished))		
			{
				UpdateMorphing(mctrl);
			}
			else
			{
				SHAPEHEADER * pis = GetShapeData(xmrd->ProbesInShape);

				sbptr->SBdptr->ObTxAnimCtrlBlks = 0;

				pis->items = xmrd->pis_items_str;
				pis->sh_textures = xmrd->pis_sht_str;

				pis->sh_instruction[4].sh_instr_data = xmrd->pis_items_str;

				xmrd->XMR_State = XMRS_Idle;
			}
			break;
		}

		default :
			GLOBALASSERT (0 == "Shouldn't be here");
			break;

	}
	
}

