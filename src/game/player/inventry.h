#ifndef INVENTRY_H
#define INVENTRY_H

/*KJL*****************************************************
* INVENTRY.H - contains externs to the fns in INVENTRY.C *
*****************************************************KJL*/

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void InitialisePlayersInventory(PLAYER_STATUS *playerStatusPtr);
extern void MaintainPlayersInventory(void);

/*-------------------------------Patrick 11/3/97--------------------------------
  Protoypes for a couple of little functions: see inventry.c for details...
  ------------------------------------------------------------------------------*/
extern void SetPlayerSecurityClearance(STRATEGYBLOCK *sbPtr, unsigned int securityLevel);
extern int ReturnPlayerSecurityClearance(STRATEGYBLOCK *sbPtr, unsigned int securityLevel);
int SlotForThisWeapon(enum WEAPON_ID weaponID);



//structure for starting equipment information loaded from rif files
typedef struct player_starting_equipment
{
	unsigned int marine_jetpack :1;
	
	unsigned int predator_pistol :1;
	unsigned int predator_plasmacaster :1;
	unsigned int predator_disc :1;
	unsigned int predator_medicomp :1;
	unsigned int predator_grappling_hook :1;
	int predator_num_spears;

	
}PLAYER_STARTING_EQUIPMENT;

extern PLAYER_STARTING_EQUIPMENT StartingEquipment;

#define PISTOL_INFINITE_AMMO	(netGameData.pistolInfiniteAmmo)

#endif
