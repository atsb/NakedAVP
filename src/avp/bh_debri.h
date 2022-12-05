#ifndef BH_DEBRI_H
#define BH_DEBRI_H

typedef struct OneShotAnimBehaviourType
{
	int counter;
	TXACTRLBLK *tac_os;
} ONESHOT_ANIM_BEHAV_BLOCK;

typedef struct SmokeGenBehaviourType
{
	int counter;
	int smokes;
} SMOKEGEN_BEHAV_BLOCK;

typedef struct HierarchicalDebrisBehaviourType {
	int counter;
	int smokes;
	int GibbFactor;
	int Android;
	HMODELCONTROLLER HModelController;

	/* behaviour type of parent object, e.g. I_BehaviourAlien */
	AVP_BEHAVIOUR_TYPE Type;
	int SubType;

	/* Silly stuff for bouncing sounds. */
	int bouncelastframe;
	enum soundindex Bounce_Sound;
	
} HDEBRIS_BEHAV_BLOCK;

// extern functions

extern DISPLAYBLOCK *MakeDebris(AVP_BEHAVIOUR_TYPE bhvr, VECTORCH *positionPtr);
extern DISPLAYBLOCK *MakeHierarchicalDebris(STRATEGYBLOCK *parent_sbPtr,SECTION_DATA *root, VECTORCH *positionPtr, MATRIXCH *orientation, int *wounds, int speed);
extern void Pop_Section(STRATEGYBLOCK *sbPtr,SECTION_DATA *section_data, VECTORCH *blastcentre, int *wounds);
extern void CreateShapeInstance(MODULEMAPBLOCK *mmbptr, char *shapeNamePtr);
extern void OneShotBehaveFun(STRATEGYBLOCK* sptr);
extern void OneShot_Anim_BehaveFun(STRATEGYBLOCK* sptr);
extern void MakeFragments (STRATEGYBLOCK *sbptr);

#endif
