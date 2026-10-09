/* KJL 11:24:31 03/21/97 - quick cheat mode stuff */
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"
#include "huddefs.h"
#include "language.h"
#include "cheat.h"
#include "weapons.h"
#include "equipmnt.h"

/* Extern for global keyboard buffer */
extern unsigned char KeyboardInput[];
extern void LoadAllWeapons(PLAYER_STATUS *playerStatusPtr);


void HandleCheatModes(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	
	#if 1
	playerStatusPtr->securityClearances = 0;
	#else
	/* KJL 16:28:54 05/11/97 - reveal map */
	if (KeyboardInput[KEY_F9])
	{
		extern SCENE Global_Scene;
		extern int ModuleArraySize;
		MODULE **moduleList;
   		int i;
		
		moduleList = MainSceneArray[Global_Scene]->sm_marray;

		for(i = 0; i < ModuleArraySize; i++)
		{
			MODULE *modulePtr = moduleList[i];

			if (modulePtr && modulePtr->m_mapptr)
			{
				modulePtr->m_flags |= m_flag_visible_on_map;
			}
		}    					       
		NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_CHEATACTIVATED));
	}
		
			
//#if !GAMEFLOW_ON
#if 0

	/* give all security levels */	
	if (KeyboardInput[KEY_F10])
	{
		playerStatusPtr->securityClearances = 0xffffffff;
		NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_CHEATACTIVATED));
	}
	if (KeyboardInput[KEY_F5])
	{
		playerStatusPtr->securityClearances = 0x3f;
		NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_CHEATACTIVATED));
	}
#endif

	#if 1
	/* give all weapons & ammo */
	if((KeyboardInput[KEY_F11]))
    {
 	}
	#endif
	/* give immortality */
	if (KeyboardInput[KEY_F12])
	{
		if(AvP.Network==I_No_Network)
		{
			NewOnScreenMessage(GetTextString(TEXTSTRING_INGAME_CHEATACTIVATED));
			playerStatusPtr->IsImmortal = 1;
		}
		else
		{
			/* commit suicide */
			CauseDamageToObject(Player->ObStrategyBlock, &certainDeath, ONE_FIXED,NULL);
		}
	}
	#endif
}

void GiveAllWeaponsCheat(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

	if(AvP.PlayerType == I_Marine)
   	{
    	int slot = MAX_NO_OF_WEAPON_SLOTS;
        while(slot--)
        {
    		PLAYER_WEAPON_DATA *wdPtr = &playerStatusPtr->WeaponSlot[slot];
   	
   	 	 	//if (slot == WEAPON_PLASMAGUN || slot == WEAPON_SONICCANNON) continue;
			  
			if (wdPtr->WeaponIDNumber==NULL_WEAPON) continue;
			
			if (wdPtr->Possessed==-1) continue; /* This weapon not allowed! */

	 		wdPtr->Possessed=1;

			/* KJL 21:49:47 05/15/97 - give ammo for weapons, apart from
			experimental weapons */
			if (slot<=WEAPON_SLOT_10)	{
				if (wdPtr->WeaponIDNumber==WEAPON_CUDGEL) {
					wdPtr->PrimaryMagazinesRemaining=0;
				} else {
					wdPtr->PrimaryMagazinesRemaining=50;
				}
			} else wdPtr->PrimaryMagazinesRemaining=0;

			if (wdPtr->WeaponIDNumber==WEAPON_PULSERIFLE) {
				wdPtr->SecondaryRoundsRemaining=(ONE_FIXED*99);
			}
			if (wdPtr->WeaponIDNumber==WEAPON_TWO_PISTOLS) {
				wdPtr->SecondaryMagazinesRemaining=50;
			}


        }
		GrenadeLauncherData.StandardMagazinesRemaining=50;
		GrenadeLauncherData.FlareMagazinesRemaining=50;
		GrenadeLauncherData.ProximityMagazinesRemaining=50;
		GrenadeLauncherData.FragmentationMagazinesRemaining=50;

	} else if (AvP.PlayerType==I_Predator) {
    	
    	int slot = MAX_NO_OF_WEAPON_SLOTS;
        while(slot--)
        {
    		PLAYER_WEAPON_DATA *wdPtr = &playerStatusPtr->WeaponSlot[slot];
   	
   	 	 	//if (slot == WEAPON_PLASMAGUN || slot == WEAPON_SONICCANNON) continue;
			  
			if (wdPtr->WeaponIDNumber==NULL_WEAPON) continue;
		
			if (wdPtr->Possessed==-1) continue; /* This weapon not allowed! */

			if (wdPtr->WeaponIDNumber==WEAPON_PRED_RIFLE) {
				wdPtr->PrimaryRoundsRemaining+=(ONE_FIXED*SPEARS_PER_PICKUP);
				if (wdPtr->PrimaryRoundsRemaining>(ONE_FIXED*MAX_SPEARS)) {
					wdPtr->PrimaryRoundsRemaining=(ONE_FIXED*MAX_SPEARS);
				}
		 		wdPtr->Possessed=1;
				continue;
			}

			if (wdPtr->WeaponIDNumber==WEAPON_PRED_DISC) {
				wdPtr->PrimaryRoundsRemaining+=(ONE_FIXED);
				if (wdPtr->PrimaryRoundsRemaining>(ONE_FIXED*99)) {
					wdPtr->PrimaryRoundsRemaining=(ONE_FIXED*99);
				}
		 		wdPtr->Possessed=1;
				continue;
			}
	
			if (wdPtr->WeaponIDNumber==WEAPON_PRED_STAFF) {
				continue;
			}

	 		wdPtr->Possessed=1;

        }
	}
	LoadAllWeapons(playerStatusPtr);
}
