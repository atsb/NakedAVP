/* KJL 16:20:20 19/05/98 - targeting externs */

extern void GetTargetingPointOfObject(DISPLAYBLOCK *objectPtr, VECTORCH *targetPtr);
extern void GetTargetingPointOfObject_Far(STRATEGYBLOCK *sbPtr, VECTORCH *targetPtr);
extern void CalculatePlayersTarget(TEMPLATE_WEAPON_DATA *twPtr, PLAYER_WEAPON_DATA *weaponPtr);
void SmartTarget_GetCofM(DISPLAYBLOCK *target,VECTORCH *viewSpaceOutput);
