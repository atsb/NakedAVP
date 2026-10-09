/*KJL***********************************************************************
* WEAPONS.H                                                                *
* 			- this file contains prototypes for the functions in weapons.c *
* 			which can be called	externally.                                *
***********************************************************************KJL*/
#ifndef _weapons_h_
	#define _weapons_h_ 1


	#ifdef __cplusplus

		extern "C" {

	#endif


/*KJL****************************************************************************************
* 										D E F I N E S 										*
****************************************************************************************KJL*/

typedef enum HitAreaMatrix {
	HAM_Centre = 0,
	HAM_Front,
	HAM_Back,
	HAM_Top,
	HAM_Base,
	HAM_Left,
	HAM_Right,
	HAM_TopLeft,
	HAM_TopRight,
	HAM_BaseLeft,
	HAM_BaseRight,
	HAM_end,
} HITAREAMATRIX;

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
extern void UpdateWeaponStateMachine(void);

extern int AreTwoPistolsInTertiaryFire(void);

extern void HandleEffectsOfExplosion(STRATEGYBLOCK *objectToIgnorePtr, VECTORCH *centrePtr, int maxRange, DAMAGE_PROFILE *maxDamage, int flat);
/*KJL*******************************************************************************
* centrePtr - is a pointer to the explosion's position vector in world space.      *
*                                                                                  *
* maxRange  - is the distance away from the explosion's centre an object has to be *
*             for no damage to be incurred.                                        *
*                                                                                  *
* maxDamage - is the damage an object would incur if it were zero distance away    *
*             from the explosion.                                                  *
*******************************************************************************KJL*/


extern void FireAutoGun(STRATEGYBLOCK *sbPtr);
/*KJL********
* bang bang *
********KJL*/

extern void MakeMatrixFromDirection(VECTORCH *directionPtr, MATRIXCH *matrixPtr);
/*KJL******************************************************************************
* directionPtr - is a pointer to a NORMALISED vector in world space (input)       *
* matrixPtr    - is a pointer to a matrix that will contain the function's output *
*                                                                                 *
*                                                                                 *
* The output matrix is the OrientMat an object would have if it were pointing in  * 
* in the given direction.                                                         *
******************************************************************************KJL*/



extern void UpdateWeaponShape(void);
/*KJL**********************************************
* Call UpdateWeaponShape on changing environment. *
**********************************************KJL*/

extern void GrabWeaponShape(PLAYER_WEAPON_DATA *weaponPtr);
void HandleWeaponImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data);
void HandleSpearImpact(VECTORCH *positionPtr, STRATEGYBLOCK *sbPtr, enum AMMO_ID AmmoID, VECTORCH *directionPtr, int multiple, SECTION_DATA *this_section_data);
extern void GrabMuzzleFlashShape(TEMPLATE_WEAPON_DATA *twPtr);
extern void FindEndOfShape(VECTORCH* endPositionPtr, int shapeIndex);
extern void InitThisWeapon(PLAYER_WEAPON_DATA *pwPtr);

extern void CauseDamageToObject(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple,VECTORCH *incoming);
extern DISPLAYBLOCK *CauseDamageToHModel(HMODELCONTROLLER *HMC_Ptr, STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, SECTION_DATA *this_section_data, VECTORCH *incoming, VECTORCH *position, int FromHost);
extern int TotalKineticDamage(DAMAGE_PROFILE *damage);
extern void GetDirectionOfAttack(STRATEGYBLOCK *sbPtr,VECTORCH *WorldVector,VECTORCH *Output);
extern int Staff_Manager(DAMAGE_PROFILE *damage,SECTION_DATA *section1,SECTION_DATA *section2,SECTION_DATA *section3,
	STRATEGYBLOCK *wielder);

/* exported varibles */
extern DISPLAYBLOCK PlayersWeapon;
extern DISPLAYBLOCK PlayersWeaponMuzzleFlash;
extern void PositionPlayersWeapon(void);
extern void PositionPlayersWeaponMuzzleFlash(void);
extern void AutoSwapToDisc(void);
extern void AutoSwapToDisc_OutOfSequence(void);
void CreateSpearPossiblyWithFragment(DISPLAYBLOCK *dispPtr, VECTORCH *spearPositionPtr, VECTORCH *spearDirectionPtr);


struct Target
{
	VECTORCH Position;
	int Distance;
	DISPLAYBLOCK *DispPtr;
	SECTION_DATA *HModelSection;
};

extern struct Target PlayersTarget;
extern SECTION_DATA *HitLocationRoll(STRATEGYBLOCK *sbPtr, STRATEGYBLOCK *source);

#define FORCE_MINIGUN_STOP 1

#define SPEARS_PER_PICKUP	30
#define MAX_SPEARS			99

	#ifdef __cplusplus

	}

	#endif


#endif
