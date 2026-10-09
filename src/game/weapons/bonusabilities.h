/* KJL 12:09:35 10/09/98 - BonusAbilities.h */

/* KJL 12:09:43 10/09/98 - Grappling Hook */
extern void InitialiseGrapplingHook(void);
extern void ActivateGrapplingHook(void);
extern void HandleGrapplingHookForces(void);
extern void RenderGrapplingHook(void);
extern void DisengageGrapplingHook(void);
void GrapplingHookBehaviour(STRATEGYBLOCK *sbPtr);
