
/***** bh_weap.h *****/

#include "particle.h"

extern void FireProjectileAmmo(enum AMMO_ID AmmoID);
extern void FrisbeeBehaviour(STRATEGYBLOCK *sbPtr); 
extern void RocketBehaviour(STRATEGYBLOCK *sbPtr); 
extern void GrenadeBehaviour(STRATEGYBLOCK *sbPtr); 
extern void MolotovBehaviour(STRATEGYBLOCK *sbPtr); 
extern void PulseGrenadeBehaviour(STRATEGYBLOCK *sbPtr); 
extern void ProximityGrenadeBehaviour(STRATEGYBLOCK *sbPtr);
extern void FlareGrenadeBehaviour(STRATEGYBLOCK *sbPtr);
extern void ClusterGrenadeBehaviour(STRATEGYBLOCK *sbPtr); 
extern void XenoborgEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr);
extern void PredatorEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr);
extern void AlienSpitBehaviour(STRATEGYBLOCK *sbPtr);
extern void NPCDiscBehaviour(STRATEGYBLOCK *sbPtr);
extern void DiscBehaviour_SeekTrack(STRATEGYBLOCK *sbPtr);
extern void PPPlasmaBoltBehaviour(STRATEGYBLOCK *sbPtr); 
extern void SpeargunBoltBehaviour(STRATEGYBLOCK *sbPtr); 
extern void FireFlameThrower(VECTORCH *position,VECTORCH *base_offset,MATRIXCH *orientmat, int player, int *timer);
extern void FireNetGhostFlameThrower(VECTORCH *positionPtr, MATRIXCH *orientMatPtr);
extern DISPLAYBLOCK *SpawnMolotovCocktail(SECTION_DATA *root, MATRIXCH *master_orient);
void Convert_Disc_To_Pickup(STRATEGYBLOCK *sbPtr);
void FirePredPistolFlechettes(VECTORCH *base_position,VECTORCH *base_offset,MATRIXCH *orientmat,int player,int *timer,BOOL damaging);
extern void FrisbeeEnergyBoltBehaviour(STRATEGYBLOCK *sbPtr);

extern int SBIsEnvironment(STRATEGYBLOCK *sbPtr);
int ValidTargetForProxMine(STRATEGYBLOCK *obstaclePtr);

typedef struct OneShotBehaviourType
{
	int counter;
} ONE_SHOT_BEHAV_BLOCK;

typedef struct PredPistolBehaviourType
{
	int counter;
	int player;
} PREDPISTOL_BEHAV_BLOCK;

typedef struct GrenadeBehaviourType
{
	int counter;
	int bouncelastframe;
} GRENADE_BEHAV_BLOCK;

typedef struct
{
	int LifeTimeRemaining;
	int ParticleGenerationTimer;
	int SoundHandle;

	/*
	becomeStuck set when flare hits wall.
	gets reset once a network message about it has been sent
	*/
	unsigned int becomeStuck:1; 

} FLARE_BEHAV_BLOCK;

typedef struct
{
	int LifeTimeRemaining;
	int SoundGenerationTimer;
	int SoundHandle;

} PROX_GRENADE_BEHAV_BLOCK;

typedef struct CasterBoltBehaviourType
{
	int counter;
	DAMAGE_PROFILE damage;
	int blast_radius;
	int player;
	int soundHandle;
} CASTER_BOLT_BEHAV_BLOCK;

typedef struct MolotovBehaviourType {
	int counter;
	HMODELCONTROLLER HModelController;
} MOLOTOV_BEHAV_BLOCK;

typedef struct FrisbeeBehaviourType
{
	int counter;
	HMODELCONTROLLER HModelController;
	int soundHandle;
	int Bounced :1; 
	int bounces;

	LASER_BEAM_DESC Laser;

} FRISBEE_BEHAV_BLOCK;

typedef struct PCPredDiscBehaviourType
{
	int counter;
	STRATEGYBLOCK *Target;
	char Target_SBname[SB_NAME_LENGTH];
	char Prev_Target_SBname[SB_NAME_LENGTH];
	char Prev_Damaged_SBname[SB_NAME_LENGTH];
	HMODELCONTROLLER HModelController;
	int soundHandle;
	int Destruct:1;
	int Stuck	:1;
	int Bounced :1; 
	int bounces;
				

} PC_PRED_DISC_BEHAV_BLOCK;

typedef struct SpearBehaviourType
{
	int counter;
	MATRIXCH Orient;
	VECTORCH Position;
	HMODELCONTROLLER HierarchicalFragment;
	int Android;

	/* behaviour type of parent object, e.g. I_BehaviourAlien */
	AVP_BEHAVIOUR_TYPE Type;
	int SubType;

	unsigned int SpearThroughFragment;
	unsigned int Stuck :1;

} SPEAR_BEHAV_BLOCK;

#define FLARE_LIFETIME 16
#define FLARE_PARTICLE_GENERATION_TIME (ONE_FIXED/120)

#define FRAG_LIFETIME 65536

#define NO_OF_FRAGS_IN_CLUSTER_BOMB 6   

#define PROX_GRENADE_TRIGGER_TIME (ONE_FIXED/4)
#define PROX_GRENADE_RANGE (4000)
#define PROX_GRENADE_SOUND_GENERATION_TIME 65536
#define PROX_GRENADE_LIFETIME (20)

STRATEGYBLOCK* CreateGrenadeKernel(AVP_BEHAVIOUR_TYPE behaviourID, VECTORCH *position, MATRIXCH *orient,int fromplayer);


/* KJL 17:46:30 02/24/97 - below is some old stuff I'll leave for reference */
#if 0                       
extern void FlameProjectileFunction(STRATEGYBLOCK *sptr);
extern void GrenadeBehaviour(STRATEGYBLOCK *sptr);
extern void TOWMissileBehaviour(STRATEGYBLOCK *sptr);
extern void PredatorDiscBehaviour(STRATEGYBLOCK *sptr);

typedef struct OneShotBehaviourType {

        AVP_BEHAVIOUR_TYPE bhvr_type;
        VECTORCH ObWorld;
        int counter;

        } ONE_SHOT_BEHAV_BLOCK;

typedef struct FlameProjectileBehaviourType {

        AVP_BEHAVIOUR_TYPE bhvr_type;
        VECTORCH ObWorld;
        int counter;
#if SupportMorphing
	MORPHCTRL *FPmctrl;
#endif

} FLAME_PROJ_BEHAV_BLOCK;

typedef struct TowMissileBehaviourType {

        AVP_BEHAVIOUR_TYPE bhvr_type;
        VECTORCH ObWorld;
		VECTORCH Target;
        int counter;

} TOW_MISSILE_BEHAV_BLOCK;

typedef struct PredatorDiscBehaviourType {

        AVP_BEHAVIOUR_TYPE bhvr_type;
        VECTORCH ObWorld;
		VECTORCH Target;
		STRATEGYBLOCK *MovingTarget;
        int counter;
		int retargetcounter;
		int phase;

} PRED_DISC_BEHAV_BLOCK;

#endif
