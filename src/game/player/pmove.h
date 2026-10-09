/*--------------Patrick 15/10/96 ---------------------
  Header File for Player Movement Stuff
  ---------------------------------------------------*/


#ifndef _pmove_h_
#define _pmove_h_ 1



#ifdef __cplusplus

	extern "C" {

#endif




/*--------------Patrick 15/10/96 ---------------------
  Enumeration of player movement states
  Free movement indicates normal movement.
  Special Movement indicates that the player is
  executing a special move.
  Aborting indicates that a special move is 
  prematurely terminating.  
  ---------------------------------------------------*/

typedef enum player_mov_state
{
	PMov_FreeMovement,
	PMov_SpecialMovement,
	PMov_AbortingSpecialMovement,
}PLAYER_MOVEMENT_STATE;

/*--------------Patrick 15/10/96 ---------------------
  Enumeration of special move types
  Special moves are flagged as belonging to a 
  character type
  ---------------------------------------------------*/


/*--------------Patrick 31/10/96 ---------------------
  Enumeration of player's morphing states for crouching
  and lying down.
  ---------------------------------------------------*/
typedef enum player_morph_state
{
	PMph_Standing,
	PMph_Crouching,
	PMph_Lying,
	PMph_StoC,
	PMph_CtoS,
	PMph_StoL,
	PMph_LtoS,

}PLAYER_MORPH_STATE;




/*--------------Patrick 1/11/96 ---------------------
  this define determines how crouching and lying down 
  are implemented for the player.  It can either be
  done by changing the shape, or by morphing....
  Morphing is better, but doesn't work with the current
  collision system.
  ---------------------------------------------------*/
#define CrouchByMorphingPlayer	0


/* Prototypes */
extern void InitPlayerMovementData(STRATEGYBLOCK* sbPtr);
extern void PlayerBehaviour(STRATEGYBLOCK* sbptr);
extern void ExecuteFreeMovement(STRATEGYBLOCK* sbPtr);
void ThrowAFlare(void);
void StartPlayerTaunt(void);
void NetPlayerRespawn(STRATEGYBLOCK *sbPtr);


#ifdef __cplusplus

	}

#endif


#endif
