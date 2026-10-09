/* KJL 15:01:53 02/25/97 - bh_debri.c */
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "dynblock.h"
#include "dynamics.h"
#include "comp_shp.h"
#include "load_shp.h"
#include "bh_types.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "inventry.h"
#include "psnd.h"
#include "plat_shp.h"
#include "particle.h"
#include "jsndsup.h"
#include "bh_alien.h"
#include "bh_marin.h"
#include "bh_corpse.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "weapons.h"
#include "lighting.h"
#include "sfx.h"

/* for win95 net game support */
#include "pldghost.h"

#include "avp_userprofile.h"
#include "savegame.h"

#include <math.h>

#define HDEBRIS_BLEEDING_TIME	(ONE_FIXED*2)

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
void MakeFleshRippingNoises(VECTORCH *positionPtr);

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
extern int NormalFrameTime;
void SetupSimpleAnimation(int counter, STRATEGYBLOCK *sbPtr);
extern int NumActiveBlocks;
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern void MakeBloodExplosion(VECTORCH *originPtr, int creationRadius, VECTORCH *blastPositionPtr, int noOfParticles, enum PARTICLE_ID particleID);
extern enum PARTICLE_ID GetBloodType(STRATEGYBLOCK *sbPtr);
extern int SBIsEnvironment(STRATEGYBLOCK *sbPtr);
extern void DoAlienLimbLossSound(VECTORCH *position);



extern MATRIXCH Identity_RotMat; /* From HModel.c */

int NextAlienFragmentToProduce;

static char *ShapeNameOfAlienFragment[] = 
{
	"AlFrga",
	"AlFrgb",
	"AlFrgc",
	"AlFrgd",
	"AlFrge",
	"AlFrgf",
	"AlFrgg",
	"AlFrgh",
	"AlFrgi",
	"AlFrgj",
};
#define NO_OF_DIFFERENT_ALIEN_FRAGS 10

/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/
DISPLAYBLOCK *MakeDebris(AVP_BEHAVIOUR_TYPE bhvr, VECTORCH *positionPtr)
{

	DISPLAYBLOCK *dispPtr;
  STRATEGYBLOCK *sbPtr;
  MODULEMAPBLOCK *mmbptr;
  MODULE m_temp;

  if( (NumActiveBlocks > maxobjects-5) || (NumActiveStBlocks > maxstblocks-5)) return NULL;

  // 1. Set up shape data BEFORE making the displayblock,
  // since "AllocateModuleObject()" will fill in shapeheader
  // information and extent data

  mmbptr = &TempModuleMap;
               
  switch (bhvr)
  {
  		/* KJL 12:45:30 03/20/97 - fragments
		switch on object which is going to be destroyed and create the correct fragments */
    case I_BehaviourAlien:
		{
			if( (NextAlienFragmentToProduce<0) || (NextAlienFragmentToProduce>=NO_OF_DIFFERENT_ALIEN_FRAGS))
				NextAlienFragmentToProduce=0;
			
			/* cycle through the available body parts */
			CreateShapeInstance(mmbptr,ShapeNameOfAlienFragment[NextAlienFragmentToProduce++]);

			/* 50/50 chance that it's an acid generator */
			if (FastRandom()&256)
			{
				bhvr = I_BehaviourAlienFragment;
			}
			else
			{
				bhvr = I_BehaviourFragment;
			}
			break;
		}

    case I_BehaviourPredator:
		{
			int randomNumber = FastRandom()&65535;

			if (randomNumber>43691)
			{
				CreateShapeInstance(mmbptr,"Bodprt1");
			}
			else if (randomNumber>21845)
			{
				CreateShapeInstance(mmbptr,"Bodprt2");
			}
			else 
			{
				CreateShapeInstance(mmbptr,"Bodprt3");
			}
			bhvr = I_BehaviourFragment;
			break;
		}

  	default:
		{
      // Don't call this function for undefined types
      GLOBALASSERT (1 == 0);
			break;
    }            
	}
 
  // And allocate the modulemapblock object

	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
  m_temp.m_mapptr = mmbptr;
  m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
  m_temp.m_dptr = NULL;
  AllocateModuleObject(&m_temp);    
  dispPtr = m_temp.m_dptr;
	if(dispPtr==NULL) return (DISPLAYBLOCK *)0; /* patrick: cannot create displayblock, so just return 0 */

  dispPtr->ObMyModule = NULL;     /* Module that created us */
	dispPtr->ObWorld = *positionPtr;

  sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
  
	if (sbPtr == 0) return (DISPLAYBLOCK *)0; // Failed to allocate a strategy block

  // 2. NOW set up the strategyblock-specific fields for
  // the new displayblock. We won't go through the "AttachNew
  // StrategyBlock" and "AssignRunTimeBehaviours" pair, since
  // the first switches on ObShape and the second on bhvr;
  // but, in this case, there isn't a particular connection
  // between them.

  sbPtr->I_SBtype = bhvr;

  switch (bhvr)
  {
		case I_BehaviourAlienFragment:		
		{
			DYNAMICSBLOCK *dynPtr;

			sbPtr->SBdataptr = (SMOKEGEN_BEHAV_BLOCK *) AllocateMem(sizeof(SMOKEGEN_BEHAV_BLOCK));
			if (sbPtr->SBdataptr == 0) 
			{	
				// Failed to allocate a strategy block data pointer
				RemoveBehaviourStrategy(sbPtr);
				return (DISPLAYBLOCK*)NULL;
			}
			
			((SMOKEGEN_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ALIEN_DYINGTIME;
			((SMOKEGEN_BEHAV_BLOCK * ) sbPtr->SBdataptr)->smokes=0;

			dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_DEBRIS);
			
			if (dynPtr == 0)
			{
				// Failed to allocate a dynamics block
				RemoveBehaviourStrategy(sbPtr);
				return (DISPLAYBLOCK*)NULL;
			}
		
			dynPtr->Position = *positionPtr;

			// Give explosion fragments an angular velocity
			dynPtr->AngVelocity.EulerX = (FastRandom()&2047)-1024;
			dynPtr->AngVelocity.EulerY = (FastRandom()&2047)-1024;
			dynPtr->AngVelocity.EulerZ = (FastRandom()&2047)-1024;

    		{
				int random = (FastRandom()&1023) - 512;
				if (random>0) dynPtr->LinImpulse.vx=(random+100)<<4;
				else dynPtr->LinImpulse.vx=(random-100)<<4;
    		}
    		{
				int random = (FastRandom()&1023) - 768;
				if (random>0) dynPtr->LinImpulse.vy=(random+100)<<4;
				else dynPtr->LinImpulse.vy=(random-100)<<4;
			}
    		{
				int random = (FastRandom()&1023) - 512;
				if (random>0) dynPtr->LinImpulse.vz=(random+100)<<4;
				else dynPtr->LinImpulse.vz=(random-100)<<4;
			}
			break;
		}
		case I_BehaviourFragment:
		{
			DYNAMICSBLOCK *dynPtr;

			sbPtr->SBdataptr = (ONE_SHOT_BEHAV_BLOCK *) AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK ));
			if (sbPtr->SBdataptr == 0) 
			{	
				// Failed to allocate a strategy block data pointer
				RemoveBehaviourStrategy(sbPtr);
				return(DISPLAYBLOCK*)NULL;
			}


			((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ALIEN_DYINGTIME;

			dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_DEBRIS);

			if (dynPtr == 0)
			{
				// Failed to allocate a dynamics block
				RemoveBehaviourStrategy(sbPtr);
				return(DISPLAYBLOCK*)NULL;
			}

			dynPtr->Position = *positionPtr;

			// Give explosion fragments an angular velocity
			dynPtr->AngVelocity.EulerX = (FastRandom()&2047)-1024;
			dynPtr->AngVelocity.EulerY = (FastRandom()&2047)-1024;
			dynPtr->AngVelocity.EulerZ = (FastRandom()&2047)-1024;

    		{
				int random = (FastRandom()&1023) - 512;
				if (random>0) dynPtr->LinImpulse.vx=(random+100)<<4;
				else dynPtr->LinImpulse.vx=(random-100)<<4;
    		}
    		{
				int random = (FastRandom()&1023) - 768;
				if (random>0) dynPtr->LinImpulse.vy=(random+100)<<4;
				else dynPtr->LinImpulse.vy=(random-100)<<4;
			}
    		{
				int random = (FastRandom()&1023) - 512;
				if (random>0) dynPtr->LinImpulse.vz=(random+100)<<4;
				else dynPtr->LinImpulse.vz=(random-100)<<4;
			}
			break;
		}


	   		default:
		{
		    GLOBALASSERT (1 == 0);
		}
    }

                    
    return dispPtr;
	
}



void CreateShapeInstance(MODULEMAPBLOCK *mmbptr, char *shapeNamePtr)
{
	int shapenum;
	shapenum = GetLoadedShapeMSL(shapeNamePtr);					
	#if debug
	if (shapenum<=0)
	{
		textprint("Unable to display shape:%s\n",shapeNamePtr);
		LOCALASSERT(0);
	}
	#endif
			
    mmbptr->MapShape = shapenum;
	mmbptr->MapType = MapType_Default;
}




void OneShotBehaveFun(STRATEGYBLOCK *sptr)
{

    // This fn. will have been called from "Execute Behaviour",
    // as a consequence of "ObjectBehaviours" running each
    // frame. It simply decrements the counter of the specified
    // temporary object, and destroys it if that counter hits
    // zero
    

    ONE_SHOT_BEHAV_BLOCK *osbhv;    
    GLOBALASSERT(sptr);
    osbhv = (ONE_SHOT_BEHAV_BLOCK * ) sptr->SBdataptr;    
    GLOBALASSERT(sptr->SBdptr);

    if (osbhv->counter < 0)
    {            
    	DestroyAnyStrategyBlock(sptr);
		return;            
	}
    
	{
		DISPLAYBLOCK *dispPtr = sptr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = osbhv->counter/2;

		}
	}
    osbhv->counter -= NormalFrameTime;


	{
		DYNAMICSBLOCK *dynPtr = sptr->DynPtr;

		if (dynPtr)
		{
			if (dynPtr->IsInContactWithFloor==0)
			{
				DynamicallyRotateObject(dynPtr);
			}
		}
	}
}


void OneShot_Anim_BehaveFun(STRATEGYBLOCK *sptr)
{

    // This fn. will have been called from "Execute Behaviour",
    // as a consequence of "ObjectBehaviours" running each
    // frame. It simply decrements the counter of the specified
    // temporary object, and destroys it if that counter hits
    // zero
    

    ONESHOT_ANIM_BEHAV_BLOCK *osbhv;    
    DISPLAYBLOCK* dptr;

    GLOBALASSERT(sptr);
    GLOBALASSERT(sptr->SBdptr);

    osbhv = (ONESHOT_ANIM_BEHAV_BLOCK * ) sptr->SBdataptr;    
		
    if (osbhv->counter < 0)
    {            
    	DestroyAnyStrategyBlock(sptr);
		return;            
	}
    
    osbhv->counter -= NormalFrameTime;

	{
		DYNAMICSBLOCK *dynPtr = sptr->DynPtr;

		if (dynPtr)
		{
			if (dynPtr->IsInContactWithFloor==0)
			{
				DynamicallyRotateObject(dynPtr);
			}
		}
	}

	dptr = sptr->SBdptr;

	if(dptr)
	{
		if(!dptr->ObTxAnimCtrlBlks)
		{ 
			dptr->ObTxAnimCtrlBlks = osbhv->tac_os;
		}
	}
}



void SetupSimpleAnimation(int counter, STRATEGYBLOCK *sbPtr)
{
	TXACTRLBLK **pptxactrlblk;		
	int item_num;
	SHAPEHEADER* shptr;
	ONESHOT_ANIM_BEHAV_BLOCK* osab;
	DISPLAYBLOCK *dispPtr = sbPtr->SBdptr;
	int shape_num;

	sbPtr->SBdataptr = (ONESHOT_ANIM_BEHAV_BLOCK *) AllocateMem(sizeof(ONESHOT_ANIM_BEHAV_BLOCK ));
	
	if (sbPtr->SBdataptr == 0) return;	// Failed to allocate an sb data ptr

	osab =	((ONESHOT_ANIM_BEHAV_BLOCK * ) sbPtr->SBdataptr);

	osab->counter = counter;

	shape_num = dispPtr->ObShape;
	shptr = GetShapeData(shape_num);
	pptxactrlblk = &osab->tac_os;
	
	SetupPolygonFlagAccessForShape(shptr);

	/*
	the bhdata is a ptr to the SHAPEHEADER each 
	animating polygon has an array of sequences, in 
	this case thers is only onr sequence per array
	*/

	for(item_num = 0; item_num < shptr->numitems; item_num ++)
	{
		POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
		LOCALASSERT(poly);
			
		if((Request_PolyFlags((void *)poly)) & iflag_txanim)
		{
			TXACTRLBLK *pnew_txactrlblk;

			pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
			
			if (pnew_txactrlblk) 
			{
				// We have allocated the new tx anim control block so initialise it

				pnew_txactrlblk->tac_flags = 0;										
				pnew_txactrlblk->tac_item = item_num;										
				pnew_txactrlblk->tac_sequence = 0;										
				pnew_txactrlblk->tac_node = 0;										
				pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);										
				pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);
			
				pnew_txactrlblk->tac_txah.txa_currentframe = 0;
				pnew_txactrlblk->tac_txah.txa_flags = txa_flag_play|txa_flag_noloop;
		
				/* change the value held in pptxactrlblk
				 which point to the previous structures "next"
			 	pointer*/

				*pptxactrlblk = pnew_txactrlblk;
				pptxactrlblk = &pnew_txactrlblk->tac_next;
			}
	 	}
	}
	dispPtr->ObTxAnimCtrlBlks = osab->tac_os;
														  
	*pptxactrlblk=0;
}

void AlienFragFun(STRATEGYBLOCK *sptr)
{
	int a;
	DYNAMICSBLOCK *dynptr;
	COLLISIONREPORT *reportptr;
    SMOKEGEN_BEHAV_BLOCK *sgbhv;    
    GLOBALASSERT(sptr);
    sgbhv = (SMOKEGEN_BEHAV_BLOCK * ) sptr->SBdataptr;    
    GLOBALASSERT(sptr->SBdptr);

	dynptr=sptr->DynPtr;
	
	GLOBALASSERT(dynptr);

	reportptr=dynptr->CollisionReportPtr;

	if (sgbhv->counter < 0)
	{            
		DestroyAnyStrategyBlock(sptr);
		return;            
	}
    
	{
		DISPLAYBLOCK *dispPtr = sptr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = sgbhv->counter/2;

		}
	}
	sgbhv->counter -= NormalFrameTime;

	a=0;

	while (reportptr) {

		if (reportptr->ObstacleSBPtr==NULL) {
			#if 0
			if (a==0) {
				a=1;
				sgbhv->counter=1;
				sptr->I_SBtype=I_BehaviourSmokeGenerator;
			}
			#endif
		}	
		else if (reportptr->ObstacleSBPtr->SBdptr==Player)
		{

			CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_ALIEN_FRAG].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
		}
		reportptr=reportptr->NextCollisionReportPtr;
	}

	{
		DYNAMICSBLOCK *dynPtr = sptr->DynPtr;

		if (dynPtr) {
			if (dynPtr->IsInContactWithFloor==0) {
				DynamicallyRotateObject(dynPtr);
			} else {
				dynPtr->AngVelocity.EulerX=0;
				dynPtr->AngVelocity.EulerY=0;
				dynPtr->AngVelocity.EulerZ=0;
			}
		}
	}
	
}

void SmokeGeneratorBehaviour(STRATEGYBLOCK *sptr) {

    // This fn. will have been called from "Execute Behaviour",
    // as a consequence of "ObjectBehaviours" running each
    // frame. It simply decrements the counter of the specified
    // temporary object, and destroys it if that counter hits
    // zero
    
	DYNAMICSBLOCK *dynptr;
	COLLISIONREPORT *reportptr;
    SMOKEGEN_BEHAV_BLOCK *sgbhv;    
    GLOBALASSERT(sptr);
    sgbhv = (SMOKEGEN_BEHAV_BLOCK * ) sptr->SBdataptr;    
    GLOBALASSERT(sptr->SBdptr);

	dynptr=sptr->DynPtr;
	reportptr=dynptr->CollisionReportPtr;

	if (sgbhv->counter < 0) sgbhv->counter=ONE_FIXED<<2;

	if (sgbhv->counter > (ONE_FIXED<<1))
	{            
		DestroyAnyStrategyBlock(sptr);
		return;            
	}
    
	sgbhv->counter += NormalFrameTime;

	if ((sgbhv->counter>>15)>sgbhv->smokes) 
	{
		{
			VECTORCH velocity={0,0,0};
			MakeParticle(&(dynptr->Position),&(velocity),PARTICLE_BLACKSMOKE);
		}
		sgbhv->smokes++;
	}

	#if 0
	{
		DYNAMICSBLOCK *dynPtr = sptr->DynPtr;
		if (dynPtr) DynamicallyRotateObject(dynPtr);
	}
	#endif

	while (reportptr) {
		if (reportptr->ObstacleSBPtr)	{
			if (reportptr->ObstacleSBPtr->SBdptr==Player) {
				CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_ALIEN_FRAG].MaxDamage[AvP.Difficulty], NormalFrameTime,NULL);
			}
		}
		reportptr=reportptr->NextCollisionReportPtr;
	}

}

int generate_random_between (int first, int second)
{
	int diff = second - first;
	int absdiff;
	int rand_no;
	
	if (diff == 0)
	{
		return(first);
	}
	
	absdiff = abs (diff);
	rand_no = FastRandom () % absdiff;
	
	if (diff < 0)
	{
		return (second + rand_no);
	}
	else
	{
		return (first + rand_no);
	}
	
}

void MakeFragments (STRATEGYBLOCK * sbptr)
{
	extern int NumActiveBlocks;
	DISPLAYBLOCK *dispPtr;
	STRATEGYBLOCK *sbPtr;
	MODULEMAPBLOCK *mmbptr;
	MODULE m_temp;
	int i=0;
	VECTORCH * posPtr;
	VECTORCH diff;
	int massfact;

	int mslpos;
	SHAPEFRAGMENT * frags;
	SHAPEFRAGMENTDESC * fragdesc;

	if( (NumActiveBlocks > maxobjects-5)
			|| (NumActiveStBlocks > maxstblocks-5))
		return;
	
	if (!sbptr)
		return;

	if (!sbptr->DynPtr)
		return;

	if (!sbptr->SBdptr)
		return;

	memset(&m_temp,0,sizeof(MODULE));
	
	posPtr = &(sbptr->DynPtr->Position);
	
  	mmbptr = &TempModuleMap;
	
	mslpos = sbptr->shapeIndex;

	if (mslpos < 0)
		return;

	fragdesc = mainshapelist[mslpos]->sh_fragdesc;

	if(!fragdesc || !fragdesc->sh_fragsound)
	{
  		Sound_Play(SID_EXPLOSION,"d",posPtr);  
	}
	else
	{
		SHAPEFRAGMENTSOUND * fragsound=fragdesc->sh_fragsound;
		if(fragsound->sound_loaded)
		{
			SOUND3DDATA s3d;
			s3d.position = *posPtr;
			s3d.inner_range = fragsound->inner_range;
			s3d.outer_range = fragsound->outer_range;
			s3d.velocity.vx = 0;
			s3d.velocity.vy = 0;
			s3d.velocity.vz = 0;
			Sound_Play ((SOUNDINDEX)fragsound->sound_loaded->sound_num, "nvp", &s3d,fragsound->max_volume,fragsound->pitch);
  			//Sound_Play((SOUNDINDEX)fragsound->sound_loaded->sound_num,"d",posPtr);  
		}
	}

	if (!fragdesc)
	{
		return;
	}
	frags=fragdesc->sh_frags;
	massfact = ((ONE_FIXED / sbptr->DynPtr->Mass)>>2) + ((ONE_FIXED>>2)*3);


	while (frags->ShapeIndex > 0)
	{
		mmbptr->MapShape = frags->ShapeIndex;
		mmbptr->MapType = MapType_Default;

		for (i=0; i<frags->NumFrags; i++)
		{
			DYNAMICSBLOCK *dynPtr;
			VECTORCH offset;

			m_temp.m_numlights = 0;
			m_temp.m_lightarray = NULL;
			m_temp.m_mapptr = mmbptr;
			m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
			m_temp.m_dptr = NULL;
			AllocateModuleObject(&m_temp); 
			if(m_temp.m_dptr==NULL) return; /* patrick: cannot create displayblock, so just return (?) */

			dispPtr = m_temp.m_dptr;

			dispPtr->ObMyModule = NULL;     /* Module that created us */

			offset.vx = frags->x_offset;
			offset.vy = frags->y_offset;
			offset.vz = frags->z_offset;
			
			if (offset.vx == 0 && offset.vy == 0 && offset.vz == 0)
			{
				// place the fragment randomly within the bounding box of the parent
				offset.vx = generate_random_between (mainshapelist[mslpos]->shapemaxx,
						mainshapelist[mslpos]->shapeminx);

				offset.vy = generate_random_between (mainshapelist[mslpos]->shapemaxy,
						mainshapelist[mslpos]->shapeminy);
				
				offset.vz = generate_random_between (mainshapelist[mslpos]->shapemaxz,
						mainshapelist[mslpos]->shapeminz);
			}

			sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
	    if (sbPtr == 0) return; // Failed to allocate a strategy block
	    
	    sbPtr->I_SBtype = I_BehaviourFragment;

			sbPtr->SBdataptr = (ONE_SHOT_BEHAV_BLOCK *) AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK ));

			if (sbPtr->SBdataptr == 0) 
			{	
				// Failed to allocate a strategy block data pointer
				RemoveBehaviourStrategy(sbPtr);
				return;
			}
			
			((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ((FastRandom()&32768)<<2) + 65535;

			dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_DEBRIS);
	
			if (dynPtr == 0)
			{
				// Failed to allocate a dynamics block
				RemoveBehaviourStrategy(sbPtr);
				return;
			}

			RotateVector (&offset, &(sbptr->DynPtr->OrientMat));
			
			
			if (sbptr->containingModule)
			{

				if (frags->x_offset || frags->y_offset || frags->z_offset)
				{
					diff.vx = offset.vx;
					diff.vy = offset.vy;
					diff.vz = offset.vz;
				}
				else
				{
					diff.vx = sbptr->containingModule->m_mapptr->MapWorld.vx - posPtr->vx;
					diff.vy = sbptr->containingModule->m_mapptr->MapWorld.vy - posPtr->vy;
					diff.vz = sbptr->containingModule->m_mapptr->MapWorld.vz - posPtr->vz;
				}


				Normalise (&diff);
			}
			else
			{
				diff.vx = 0;
				diff.vy = ONE_FIXED;
				diff.vz = 0;
			}

			
			{
				/*give fragment an impulse roughly in the direction of its offset*/
				VECTORCH impulse=offset;
				
				if(impulse.vx || impulse.vy || impulse.vz)
				{
					Normalise(&impulse);
									
			 		impulse.vx/=generate_random_between(5,10);
			  //	impulse.vy/=generate_random_between(5,10);
			 		impulse.vz/=generate_random_between(5,10);
				}
			   	dynPtr->LinImpulse=impulse;
			}
			
			diff.vx = (diff.vx>>4);
			diff.vy = (diff.vy>>4);
			diff.vz = (diff.vz>>4);
			
			
			offset.vx += posPtr->vx;
			offset.vy += posPtr->vy;
			offset.vz += posPtr->vz;
			
			dispPtr->ObWorld = offset;
			dispPtr->ObMat = sbptr->DynPtr->OrientMat;
			dispPtr->ObEuler = sbptr->DynPtr->OrientEuler;

			dynPtr->Position = offset;
			
			dynPtr->OrientMat = sbptr->DynPtr->OrientMat;
			dynPtr->PrevOrientMat = sbptr->DynPtr->PrevOrientMat;

			dynPtr->OrientEuler = sbptr->DynPtr->OrientEuler;
			dynPtr->PrevOrientEuler = sbptr->DynPtr->PrevOrientEuler;
			


			{
				dynPtr->AngVelocity.EulerX = (((FastRandom()&2047)-1023))<<2;
				dynPtr->AngVelocity.EulerY = (((FastRandom()&2047)-1023))<<2;
				dynPtr->AngVelocity.EulerZ = (((FastRandom()&2047)-1023))<<2;
		
				dynPtr->LinImpulse.vy = - (FastRandom()&1023)<<2;

				/* Look to see if object has only one polygon in it;
				   if so it's probably a glass fragment, so don't bother
				   giving it collisions */
				if (dispPtr->ObShape)
				{	
					SHAPEHEADER *shapePtr = GetShapeData(dispPtr->ObShape);
					
					if (shapePtr)
					{
						if (shapePtr->numitems!=1)
						{
							dynPtr->DynamicsType = DYN_TYPE_NRBB_COLLISIONS;
						}
					}	
				}
			}

		}

		frags++;
	}
}

DISPLAYBLOCK *MakeHierarchicalDebris(STRATEGYBLOCK *parent_sbPtr,SECTION_DATA *root, VECTORCH *positionPtr, MATRIXCH *orientation, int *wounds, int speed)
{

	DISPLAYBLOCK *dispPtr;
	STRATEGYBLOCK *sbPtr;
	DYNAMICSBLOCK *dynPtr;
	MODULEMAPBLOCK *mmbptr;
	MODULE m_temp;
	AVP_BEHAVIOUR_TYPE bhvr;
	int woundflags;

	/* KJL 16:53:22 28/02/98 - this seems to happen a lot in multiplayer */
	if(positionPtr->vx>1000000 || positionPtr->vx<-1000000)
		return NULL;
	if(positionPtr->vy>1000000 || positionPtr->vy<-1000000)
		return NULL;
	if(positionPtr->vz>1000000 || positionPtr->vz<-1000000)
		return NULL;
	
	if( (NumActiveBlocks > maxobjects-5) || (NumActiveStBlocks > maxstblocks-5)) return NULL;

	/* Right away, try to intercept all molotov cocktails! */
	GLOBALASSERT(root);
	if ((root->flags&section_data_notreal)==0) {
		if (strcmp(root->sempai->Section_Name,"bottle")==0) {
			/* Got one. */
			dispPtr=SpawnMolotovCocktail(root,orientation);
			/* Correct speed. */
			if (dispPtr) {
				dynPtr=dispPtr->ObStrategyBlock->DynPtr;

				/* This code CnP'd from further down! */
				dynPtr->AngVelocity.EulerX = (FastRandom()&2047)-1024;
				dynPtr->AngVelocity.EulerY = (FastRandom()&2047)-1024;
				dynPtr->AngVelocity.EulerZ = (FastRandom()&2047)-1024;

		   		{
					int random = (FastRandom()&1023) - 512;
					if (random>0) dynPtr->LinImpulse.vx=(random+100)<<speed;
					else dynPtr->LinImpulse.vx=(random-100)<<speed;
		   		}
		    	{
					int random = (FastRandom()&1023) - 768;
					if (random>0) dynPtr->LinImpulse.vy=(random+100)<<speed;
					else dynPtr->LinImpulse.vy=(random-100)<<speed;
				}
		    	{
					int random = (FastRandom()&1023) - 512;
					if (random>0) dynPtr->LinImpulse.vz=(random+100)<<speed;
					else dynPtr->LinImpulse.vz=(random-100)<<speed;
				}
			}
			return(dispPtr);
		}
	}

	// 1. Set up shape data BEFORE making the displayblock,
	// since "AllocateModuleObject()" will fill in shapeheader
	// information and extent data

	mmbptr = &TempModuleMap;
               
	/* Doesn't really matter what shape gets generated... */
	//CreateShapeInstance(mmbptr,root->sempai->ShapeName);
	CreateShapeInstance(mmbptr,"Shell");

	bhvr = I_BehaviourHierarchicalFragment;

	// And allocate the modulemapblock object

	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
	m_temp.m_mapptr = mmbptr;
	m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
	m_temp.m_dptr = NULL;
	AllocateModuleObject(&m_temp);    
	dispPtr = m_temp.m_dptr;
	if(dispPtr==NULL) return (DISPLAYBLOCK *)0; /* patrick: cannot create displayblock, so just return 0 */

	dispPtr->ObMyModule = NULL;     /* Module that created us */
	dispPtr->ObWorld = *positionPtr;

	sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
  
	if (sbPtr == 0) return (DISPLAYBLOCK *)0; // Failed to allocate a strategy block

	// 2. NOW set up the strategyblock-specific fields for
	// the new displayblock. We won't go through the "AttachNew
	// StrategyBlock" and "AssignRunTimeBehaviours" pair, since
	// the first switches on ObShape and the second on bhvr;
	// but, in this case, there isn't a particular connection
	// between them.

	sbPtr->I_SBtype = bhvr;

	GLOBALASSERT(root);

	{
	
		sbPtr->SBdataptr = (HDEBRIS_BEHAV_BLOCK *) AllocateMem(sizeof(HDEBRIS_BEHAV_BLOCK));
		if (sbPtr->SBdataptr == 0) {	
			// Failed to allocate a strategy block data pointer
			RemoveBehaviourStrategy(sbPtr);
			return (DISPLAYBLOCK*)NULL;
		}
			
		((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = HDEBRIS_LIFETIME;
		((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->smokes=0;

		if (root->sempai->flags&section_flag_gibbwhenfragged) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->GibbFactor=(ONE_FIXED>>1);
		} else {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->GibbFactor=0;
		}

		/* Inheritance of android flag. */
		if (GetBloodType(parent_sbPtr)==PARTICLE_ANDROID_BLOOD) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Android=1;
		} else {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Android=0;
		}
				
		woundflags=Splice_HModels(&(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),root);
	

		if (parent_sbPtr)
		{
			/* Inherit fire! */
			if (parent_sbPtr->SBDamageBlock.IsOnFire) {
				sbPtr->SBDamageBlock.IsOnFire=1;
			}
			/* KJL 11:28:27 14/10/98 - this is set so we can later know what the debris was part of */
			/* CDF 3/3/99 put it in the switch statement, to deal with complex cases */
			/* Creature specific code! */
			switch (parent_sbPtr->I_SBtype) {
				case I_BehaviourAlien:
					{
						ALIEN_STATUS_BLOCK *alienStatusPointer;    
						/* See if we can strip to template. */
						alienStatusPointer = (ALIEN_STATUS_BLOCK *)(parent_sbPtr->SBdataptr);    
						LOCALASSERT(alienStatusPointer);	          		
						/* Just go to spasm. */
						InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_AlienStand,ASSS_Spasm,-1,1);
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourAlien;
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = alienStatusPointer->Type;
						root->SecMat=*orientation;

						DoAlienLimbLossSound(positionPtr);
						/* Last as long as a dead alien. */
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = ALIEN_DYINGTIME;
						break;
					}
				case I_BehaviourMarine:
					{
						SECTION *template_root;
						SECTION *template_sempai;
						MARINE_STATUS_BLOCK *marineStatusPointer;    
						/* See if we can strip to template. */
						marineStatusPointer = (MARINE_STATUS_BLOCK *)(parent_sbPtr->SBdataptr);    
						LOCALASSERT(marineStatusPointer);	          		
						template_root=GetNamedHierarchyFromLibrary(marineStatusPointer->My_Weapon->Riffname,marineStatusPointer->My_Weapon->TemplateName);
						/* Now, find the section that matches. */
						template_sempai=Get_Corresponding_Section_Recursive(template_root,(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController).Root_Section->Section_Name);

						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourMarine;
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = marineStatusPointer->My_Weapon->ARealMarine;

						if (template_sempai) {
							/* We have a match! */
							Transmogrify_HModels(sbPtr,&(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),
								template_sempai, 1, 0,0);
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_MarineStand,MSSS_Spasm,-1,1);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
							//((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootRotation=1;
							root->SecMat=*orientation;
							MakeFleshRippingNoises(positionPtr);
						} else {						
							/* Forget it.  Must be a disembodied weapon, or something. */
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
							//((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootRotation=1;
							
							//dispPtr->ObMat=*orientation;
							/* Below is an alternative... */
							root->SecMat=*orientation;
						}
						/* Since we're dealing with a marine, consider expressions. */
						{
							TXACTRLBLK *tacb;
							SECTION_DATA *head;

							head=GetThisSectionData(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.section_data,"head");
							if (head) {
								if ((head->flags&section_data_notreal)==0) {
							
									tacb=head->tac_ptr;
							
									while (tacb) {
										tacb->tac_sequence = 4;
										tacb->tac_txah_s = GetTxAnimHeaderFromShape(tacb, head->ShapeNum);
									
										tacb=tacb->tac_next;
									}
								}
							}
						}
						/* Last as long as a dead marine. */
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = MARINE_DYINGTIME;
					}
					break;
				case I_BehaviourPredator:
					{
						SECTION *template_root;
						SECTION *template_sempai;
						PREDATOR_STATUS_BLOCK *predatorStatusPointer;    
						/* See if we can strip to template. */
						predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(parent_sbPtr->SBdataptr);    
						LOCALASSERT(predatorStatusPointer);	          		
						template_root=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
						/* Now, find the section that matches. */
						template_sempai=Get_Corresponding_Section_Recursive(template_root,(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController).Root_Section->Section_Name);

						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourPredator;
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = 0;

						if (template_sempai) {
							/* We have a match! */
							Transmogrify_HModels(sbPtr,&(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),
								template_sempai, 1, 0,0);
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_PredatorStand,PSSS_Spasm,-1,1);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
							root->SecMat=*orientation;
						} else {						
							/* Forget it.  Must be a disembodied weapon, or something. */
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
							root->SecMat=*orientation;
						}
						/* Just freeze for now! */
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.Playing=0;
						/* Last as long as a dead predator. */
						((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = PRED_DIETIME;
					}
					break;

				case I_BehaviourNetCorpse:
				{
					NETCORPSEDATABLOCK *corpseDataPtr = (NETCORPSEDATABLOCK *)parent_sbPtr->SBdataptr;
					switch (corpseDataPtr->Type) {
						case I_BehaviourAlien:
						{
							DoAlienLimbLossSound(positionPtr);
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_AlienStand,ASSS_Spasm,-1,1);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = corpseDataPtr->subtype;
							break;
						}
						case I_BehaviourMarine:
						{
							SECTION *template_root;
							SECTION *template_sempai;
							template_root=corpseDataPtr->TemplateRoot;
							/* Now, find the section that matches. */
							template_sempai=Get_Corresponding_Section_Recursive(template_root,(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController).Root_Section->Section_Name);
							
							if (template_sempai) {
								MakeFleshRippingNoises(positionPtr);
							}
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_MarineStand,MSSS_Spasm,-1,1);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = corpseDataPtr->ARealMarine;
							break;
						}
						case I_BehaviourPredator:
						{
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_PredatorStand,PSSS_Spasm,-1,1);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = corpseDataPtr->subtype;
							break;
						}
						default:
						{
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
							((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = corpseDataPtr->subtype;
							break;
						}
					}
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
					root->SecMat=*orientation;

					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = corpseDataPtr->Type;
					/* Inherit counter from parent corpse. */
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = corpseDataPtr->timer;
					break;
				}
				case I_BehaviourHierarchicalFragment:
				{
					HDEBRIS_BEHAV_BLOCK *debrisDataPtr  = (HDEBRIS_BEHAV_BLOCK *)parent_sbPtr->SBdataptr;
					switch (debrisDataPtr->Type) {
						case I_BehaviourAlien:
						{
							DoAlienLimbLossSound(positionPtr);
							/* Sound should be the same for all types of alien! */
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_AlienStand,ASSS_Spasm,-1,1);
							break;
						}
						case I_BehaviourMarine:
						{
							MakeFleshRippingNoises(positionPtr);
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_MarineStand,MSSS_Spasm,-1,1);
							break;
						}
						case I_BehaviourPredator:
						{
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_PredatorStand,PSSS_Spasm,-1,1);
							break;
						}
						default:
						{
							InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
							break;
						}
					}
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
					root->SecMat=*orientation;
					
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = debrisDataPtr->Type;
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = debrisDataPtr->SubType;
					/* Inherit counter from parent debris. */
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = debrisDataPtr->counter;
					break;
				}

				case I_BehaviourNetGhost:
					{
						NETGHOSTDATABLOCK *dataptr;
						dataptr=parent_sbPtr->SBdataptr;
						switch (dataptr->type) {
							case I_BehaviourAlien:
							{
								DoAlienLimbLossSound(positionPtr);
								InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_AlienStand,ASSS_Spasm,-1,1);
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
								root->SecMat=*orientation;

								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourAlien;
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = dataptr->subtype;
								break;
							}
							case I_BehaviourNetCorpse:
							{
								switch (dataptr->subtype) {
									case I_BehaviourAlien:
									{
										DoAlienLimbLossSound(positionPtr);
										InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED>>2),HMSQT_AlienStand,ASSS_Spasm,-1,1);
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
										root->SecMat=*orientation;

										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourAlien;
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = dataptr->IOType;
										break;
									}
									default:
									{
										InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
										root->SecMat=*orientation;

										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = parent_sbPtr->I_SBtype;
										((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = 0;
										break;
									}
								}
								break;
							}
							default:
								InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
								root->SecMat=*orientation;

								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = parent_sbPtr->I_SBtype;
								((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = 0;
								break;
						}
					}
					break;

				default:
					InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
					root->SecMat=*orientation;

					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = parent_sbPtr->I_SBtype;
					((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = 0;
					break;
			}
		}
		else
		{
			/* KJL 11:27:54 14/10/98 - set behaviour type to null to avoid confusion */
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Type = I_BehaviourNull;
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->SubType = 0;
			InitHModelTweening( &(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController),(ONE_FIXED<<1),0,1,ONE_FIXED,0);
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.LockTopSection=1;
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootDisplacement=1;
			//((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootRotation=1;
			root->SecMat=*orientation;
		}

		if (wounds) {
			*wounds=woundflags;
		}

		dispPtr->HModelControlBlock=&(((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController);
		
		dispPtr->ObWorld=*positionPtr;
		//dispPtr->ObMat=*orientation;
		dispPtr->ObMat=Identity_RotMat;

		LOCALASSERT(dispPtr->ObWorld.vx<1000000 && dispPtr->ObWorld.vx>-1000000);
		LOCALASSERT(dispPtr->ObWorld.vy<1000000 && dispPtr->ObWorld.vy>-1000000);
		LOCALASSERT(dispPtr->ObWorld.vz<1000000 && dispPtr->ObWorld.vz>-1000000);

		dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_DEBRIS);
			
		if (dynPtr == 0) {
			// Failed to allocate a dynamics block
			RemoveBehaviourStrategy(sbPtr);
			return (DISPLAYBLOCK*)NULL;
		}
		
		dynPtr->Position = *positionPtr;

		dynPtr->OrientMat=*orientation;

		dynPtr->UseDisplacement=0;

		dynPtr->LinVelocity.vx=0;
		dynPtr->LinVelocity.vy=0;
		dynPtr->LinVelocity.vz=0;
		
		dynPtr->Mass=50;

   		#if 1
		// Give explosion fragments an angular velocity
		dynPtr->AngVelocity.EulerX = (FastRandom()&2047)-1024;
		dynPtr->AngVelocity.EulerY = (FastRandom()&2047)-1024;
		dynPtr->AngVelocity.EulerZ = (FastRandom()&2047)-1024;

   		{
			int random = (FastRandom()&1023) - 512;
			if (random>0) dynPtr->LinImpulse.vx=(random+100)<<speed;
			else dynPtr->LinImpulse.vx=(random-100)<<speed;
   		}
    	{
			int random = (FastRandom()&1023) - 768;
			if (random>0) dynPtr->LinImpulse.vy=(random+100)<<speed;
			else dynPtr->LinImpulse.vy=(random-100)<<speed;
		}
    	{
			int random = (FastRandom()&1023) - 512;
			if (random>0) dynPtr->LinImpulse.vz=(random+100)<<speed;
			else dynPtr->LinImpulse.vz=(random-100)<<speed;
		}
		#else
		dynPtr->AngVelocity.EulerX = 0;
		dynPtr->AngVelocity.EulerY = 0;
		dynPtr->AngVelocity.EulerZ = 0;

		dynPtr->LinImpulse.vx=0;
		dynPtr->LinImpulse.vy=0;
		dynPtr->LinImpulse.vz=0;
		#endif
		
		/* Set up default here for neatness. */
		((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_NOSOUND;
		/* Consider the bounce sound, by section name. */
		if (strcmp(root->sempai->Section_Name,"SADAR")==0) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
			dynPtr->Elasticity=(ONE_FIXED>>3);
		} else if (strcmp(root->sempai->Section_Name,"gren stock")==0) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
			/* Grenade launchers aren't very bouncy. */
		} else if (strcmp(root->sempai->Section_Name,"flamer")==0) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
		} else if (strcmp(root->sempai->Section_Name,"spring one")==0) {
			/* This is a smartgun! */
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
			/* Whilst we're here... */
			dispPtr->ObMat=*orientation;
			root->SecMat=Identity_RotMat;
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.ZeroRootRotation=1;
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->HModelController.Playing=0;
		} else if (strcmp(root->sempai->Section_Name,"mini gun")==0) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
		} else if (strcmp(root->sempai->Section_Name,"flame thrower")==0) {
			/* Civvie flamer... */
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
		} else if (strcmp(root->sempai->Section_Name,"pulse mag")==0) {
			((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_NOSOUND;
			/* Don't have a 'tink' yet... */
			dynPtr->Elasticity=(ONE_FIXED>>1);
		}

		if (parent_sbPtr) {
			if (parent_sbPtr->I_SBtype==I_BehaviourAutoGun) {
				/* Always make a thump. */
				((HDEBRIS_BEHAV_BLOCK * ) sbPtr->SBdataptr)->Bounce_Sound=SID_ED_LARGEWEAPONDROP;
			}
		}

		ProveHModel(dispPtr->HModelControlBlock,dispPtr);

	}

	LOCALASSERT(dispPtr->ObWorld.vx<1000000 && dispPtr->ObWorld.vx>-1000000);
	LOCALASSERT(dispPtr->ObWorld.vy<1000000 && dispPtr->ObWorld.vy>-1000000);
	LOCALASSERT(dispPtr->ObWorld.vz<1000000 && dispPtr->ObWorld.vz>-1000000);
                    
    return dispPtr;
	
}
/* KJL 16:35:13 08/01/99 - make body ripping noises */
void MakeFleshRippingNoises(VECTORCH *positionPtr)
{
	int s = FastRandom()%5;

	Sound_Play(SID_BODY_BEING_HACKED_UP_0+s,"d",positionPtr);
}


void Pop_Section(STRATEGYBLOCK *sbPtr,SECTION_DATA *section_data, VECTORCH *blastcentre, int *wounds) {

	int temp_wounds=0;
	enum PARTICLE_ID blood_type;
	/* 'Explode' this section, then frag off all it's children! */

	GLOBALASSERT(section_data);
	GLOBALASSERT(blastcentre);

	if ((section_data->sempai->flags&section_sprays_anything)==0) {
		blood_type=PARTICLE_NULL;
	} else {
		if (section_data->sempai->flags&section_sprays_blood) {
			blood_type=GetBloodType(sbPtr);
		} else if (section_data->sempai->flags&section_sprays_acid) {
			blood_type=PARTICLE_ALIEN_BLOOD;
		} else if (section_data->sempai->flags&section_sprays_predoblood) {
			blood_type=PARTICLE_PREDATOR_BLOOD;
		} else if (section_data->sempai->flags&section_sprays_sparks) {
			blood_type=PARTICLE_SPARK;
		} else {
			blood_type=PARTICLE_NULL;	
		}
	}
	/* Right, should have a blood type set.  Now, trim off the extra bits... */
	
	if ((section_data->First_Child!=NULL)
		&&( (section_data->flags&section_data_terminate_here)==0)) {

		SECTION_DATA *child_ptr;
	
		child_ptr=section_data->First_Child;
	
		while (child_ptr!=NULL) {

			LOCALASSERT(child_ptr->My_Parent==section_data);
			/* Please work! */			
			MakeHierarchicalDebris(sbPtr,child_ptr,&child_ptr->World_Offset,&child_ptr->SecMat,&temp_wounds,2);

			(*wounds)|=temp_wounds;

			child_ptr=child_ptr->Next_Sibling;
		}

	}
	/* Okay.  Now, call the explosion of blood. */
	
	if ((section_data->sempai->Shape)&&(blood_type!=PARTICLE_NULL)) {	
		if (SUPERGORE_MODE) {
			MakeBloodExplosion(&section_data->World_Offset,section_data->sempai->Shape->shaperadius,
				blastcentre,500,blood_type);
		} else {
			MakeBloodExplosion(&section_data->World_Offset,section_data->sempai->Shape->shaperadius,
				blastcentre,100,blood_type);
		}
	}

	/* Now trim off THIS bit, permanently. */
	
	(*wounds)|=section_data->sempai->flags&section_flags_wounding;

	section_data->flags|=section_data_notreal;
	section_data->flags|=section_data_terminate_here;
	/* ~Fin~ */
}

void HierarchicalFragmentBehaviour(STRATEGYBLOCK *sptr)
{
	/* CDF 5/3/99 A new function for all Hierarchical Fragments. */
	int a;
	COLLISIONREPORT *reportptr;
    HDEBRIS_BEHAV_BLOCK *hdbhv;
 	DYNAMICSBLOCK *dynPtr;
	int bounce=0;

    GLOBALASSERT(sptr);
    hdbhv = (HDEBRIS_BEHAV_BLOCK * ) sptr->SBdataptr;    
    GLOBALASSERT(sptr->SBdptr);

	dynPtr=sptr->DynPtr;
	
	GLOBALASSERT(dynPtr);

	reportptr=dynPtr->CollisionReportPtr;

	if (hdbhv->counter < 0)
	{            
		DestroyAnyStrategyBlock(sptr);
		return;            
	}
    
	{
		DISPLAYBLOCK *dispPtr = sptr->SBdptr;
		/* do we have a displayblock? */
		if (dispPtr)
		{
			dispPtr->SpecialFXFlags |= SFXFLAG_MELTINGINTOGROUND;
			dispPtr->ObFlags2 = hdbhv->counter/2;

		}
	}
	
	{
		/* CDF 8/3/99 Added this on request... */
		if (hdbhv->counter<(HDEBRIS_LIFETIME-HDEBRIS_BLEEDING_TIME)) {
			hdbhv->HModelController.DisableBleeding=1;
		}
	}

	hdbhv->counter -= NormalFrameTime;

	a=0;

	if (reportptr==NULL) {
		hdbhv->bouncelastframe=0;
	}

	while (reportptr) {

		if (SBIsEnvironment(reportptr->ObstacleSBPtr)) {
			/* Hit environment. */
			bounce=1;
		}	
		else if (reportptr->ObstacleSBPtr->SBdptr==Player)
		{
			/* Hurt the player on collision? */
			if (hdbhv->HModelController.Root_Section->flags&section_sprays_acid) {
				/* That's the test that used to be used... */
				CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_ALIEN_FRAG].MaxDamage[AvP.Difficulty], ONE_FIXED,NULL);
			}
		}
		reportptr=reportptr->NextCollisionReportPtr;
	}

	if (bounce&&(hdbhv->bouncelastframe==0)) {
		if (hdbhv->Bounce_Sound!=SID_NOSOUND) {
			Sound_Play(hdbhv->Bounce_Sound,"dp",&(dynPtr->Position),((FastRandom()&511)-255));
		}
		hdbhv->bouncelastframe=1;
	}

	{
		if (dynPtr->IsInContactWithFloor==0) {
			DynamicallyRotateObject(dynPtr);
		} else {
			dynPtr->AngVelocity.EulerX=0;
			dynPtr->AngVelocity.EulerY=0;
			dynPtr->AngVelocity.EulerZ=0;
		}
	}
	
}



/*-------------------------------**
** Load/Save hierarchical debris **
**-------------------------------*/

typedef struct hier_debris_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block stuff
	int counter;
	int smokes;
	int GibbFactor;
	int Android;

	AVP_BEHAVIOUR_TYPE Type;
	int SubType;

	int bouncelastframe;
	enum soundindex Bounce_Sound;

//strategy block stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;
}HIER_DEBRIS_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV hdebrisStatusPointer

void LoadStrategy_HierarchicalDebris(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	HIER_DEBRIS_SAVE_BLOCK* block = (HIER_DEBRIS_SAVE_BLOCK*) header;
	HDEBRIS_BEHAV_BLOCK* hdebrisStatusPointer;

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//We need to create the debris then.
	{
		MODULEMAPBLOCK *mmbptr;
		MODULE m_temp;
		DISPLAYBLOCK* dispPtr;

		// 1. Set up shape data BEFORE making the displayblock,
		// since "AllocateModuleObject()" will fill in shapeheader
		// information and extent data

		mmbptr = &TempModuleMap;
    	           
		/* Doesn't really matter what shape gets generated... */
		//CreateShapeInstance(mmbptr,root->sempai->ShapeName);
		CreateShapeInstance(mmbptr,"Shell");


		// And allocate the modulemapblock object

		m_temp.m_numlights = 0;
		m_temp.m_lightarray = NULL;
		m_temp.m_mapptr = mmbptr;
		m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
		m_temp.m_dptr = NULL;
		AllocateModuleObject(&m_temp);    
		dispPtr = m_temp.m_dptr;
		if(dispPtr==NULL) return ; /* patrick: cannot create displayblock, so just return 0 */

		dispPtr->ObMyModule = NULL;     /* Module that created us */

		sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
  		if (sbPtr == 0) return;  // Failed to allocate a strategy block
		sbPtr->I_SBtype = I_BehaviourHierarchicalFragment;

		sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_ALIEN_DEBRIS);

		//allocate behaviour block memory
		sbPtr->SBdataptr = (HDEBRIS_BEHAV_BLOCK *) AllocateMem(sizeof(HDEBRIS_BEHAV_BLOCK));
		hdebrisStatusPointer = (HDEBRIS_BEHAV_BLOCK *) sbPtr->SBdataptr;

		memset(hdebrisStatusPointer,0,sizeof(*hdebrisStatusPointer));
		dispPtr->HModelControlBlock=&hdebrisStatusPointer->HModelController;
	
	}
	

	//start copying stuff

	COPYELEMENT_LOAD(counter)
	COPYELEMENT_LOAD(smokes)
	COPYELEMENT_LOAD(GibbFactor)
	COPYELEMENT_LOAD(Android)
	COPYELEMENT_LOAD(Type)
	COPYELEMENT_LOAD(SubType)
	COPYELEMENT_LOAD(bouncelastframe)
	COPYELEMENT_LOAD(Bounce_Sound)

	//copy strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	//load hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&hdebrisStatusPointer->HModelController);
		}
	}

}

void SaveStrategy_HierarchicalDebris(STRATEGYBLOCK* sbPtr)
{
	HDEBRIS_BEHAV_BLOCK* hdebrisStatusPointer;
	HIER_DEBRIS_SAVE_BLOCK* block;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);
	hdebrisStatusPointer = (HDEBRIS_BEHAV_BLOCK*) sbPtr->SBdataptr;

	//start copying stuff

	COPYELEMENT_SAVE(counter)
	COPYELEMENT_SAVE(smokes)
	COPYELEMENT_SAVE(GibbFactor)
	COPYELEMENT_SAVE(Android)
	COPYELEMENT_SAVE(Type)
	COPYELEMENT_SAVE(SubType)
	COPYELEMENT_SAVE(bouncelastframe)
	COPYELEMENT_SAVE(Bounce_Sound)

	//save strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

	//save the hierarchy
	SaveHierarchy(&hdebrisStatusPointer->HModelController);
		
}



/*------------------**
** Load/Save Debris **
**------------------*/

typedef struct debris_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;
	
    int counter;

	BOOL dynamicModuleObject;

	int shapeIndex;
	int shapeNumPoints;
	int shapeNumItems;
	int shapeRadius;
	

//strategy block stuff
	int integrity;
	DAMAGEBLOCK SBDamageBlock;
	DYNAMICSBLOCK dynamics;

}DEBRIS_SAVE_BLOCK;


STRATEGYBLOCK*  MakeDebrisForLoad(int shapeIndex,int counter,BOOL dynamicModuleObject)
{
	DISPLAYBLOCK *dispPtr;
	STRATEGYBLOCK *sbPtr;
	MODULEMAPBLOCK *mmbptr;
	MODULE m_temp;
	
	DYNAMICSBLOCK *dynPtr;

	if( (NumActiveBlocks > maxobjects-5)
			|| (NumActiveStBlocks > maxstblocks-5))
		return NULL;
	

	memset(&m_temp,0,sizeof(MODULE));
		  
	mmbptr = &TempModuleMap;
	

	mmbptr->MapShape = shapeIndex;
	mmbptr->MapType = MapType_Default;

	
	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
	m_temp.m_mapptr = mmbptr;
	m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
	m_temp.m_dptr = NULL;
	AllocateModuleObject(&m_temp); 
	if(m_temp.m_dptr==NULL) return NULL; /* patrick: cannot create displayblock, so just return (?) */

	dispPtr = m_temp.m_dptr;

	dispPtr->ObMyModule = NULL;     /* Module that created us */
		

	sbPtr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dispPtr);
	if (sbPtr == 0) return NULL; // Failed to allocate a strategy block
	  
	sbPtr->I_SBtype = I_BehaviourFragment;

	sbPtr->SBdataptr = (ONE_SHOT_BEHAV_BLOCK *) AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK ));

	if (sbPtr->SBdataptr == 0) 
	{	
		// Failed to allocate a strategy block data pointer
		RemoveBehaviourStrategy(sbPtr);
		return NULL;
	}
	
	((ONE_SHOT_BEHAV_BLOCK * ) sbPtr->SBdataptr)->counter = counter;

	dynPtr = sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_DEBRIS);
	
	if (dynPtr == 0)
	{
		// Failed to allocate a dynamics block
		RemoveBehaviourStrategy(sbPtr);
		return NULL;
	}

	if(dynamicModuleObject)
	{
		dispPtr->ObFlags3 |= ObFlag3_DynamicModuleObject;
	}
	return sbPtr;
}


void LoadStrategy_Debris(SAVE_BLOCK_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	DEBRIS_SAVE_BLOCK* block = (DEBRIS_SAVE_BLOCK*)header;

	//check the size
	if(block->header.size != sizeof(*block)) return;

	{
		SHAPEHEADER* shp = GetShapeData(block->shapeIndex);

		//check that the shape at shapeIndex is correct
		if(!shp) return;

		if(shp->numitems!=block->shapeNumItems) return;
		if(shp->numpoints!=block->shapeNumPoints) return;
		if(shp->shaperadius!=block->shapeRadius) return;

		sbPtr = MakeDebrisForLoad(block->shapeIndex,block->counter,block->dynamicModuleObject);
		if(!sbPtr) return;
	}

//strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;
	

}

void SaveStrategy_Debris(STRATEGYBLOCK* sbPtr)
{
	SHAPEHEADER* shp;
	DEBRIS_SAVE_BLOCK* block;
	ONE_SHOT_BEHAV_BLOCK* behav = (ONE_SHOT_BEHAV_BLOCK*)sbPtr->SBdataptr;

	if(!sbPtr->SBdptr) return;
	if(!sbPtr->SBdptr->ObShapeData) return;
	shp = sbPtr->SBdptr->ObShapeData;

	

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	block->counter = behav->counter;
	block->shapeIndex = sbPtr->SBdptr->ObShape;
	block->dynamicModuleObject = (sbPtr->SBdptr->ObFlags3 & ObFlag3_DynamicModuleObject)!=0;

	block->shapeNumPoints = shp->numpoints;
	block->shapeNumItems  = shp->numitems;
	block->shapeRadius = shp->shaperadius;


//strategy block stuff
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;
	
	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;

}
