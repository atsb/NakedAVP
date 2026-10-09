#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "gamedef.h"
#include "stratdef.h"
#include "dynblock.h"
#include "bh_types.h"
#include "avpview.h"
#include "opengl.h"

#include "kshape.h"
#include "kzsort.h"
#include "frustum.h"
#include "vision.h"
#include "lighting.h"
#include "weapons.h"
#include "sfx.h"
#include "fmv.h"
/* character extents data so you know where the player's eyes are */
#include "extents.h"
#include "avp_userprofile.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/* KJL 13:59:05 04/19/97 - avpview.c
 *
 *	This is intended to be an AvP-specific streamlined version of view.c. 
 */
																		
extern void AllNewModuleHandler(void);
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;

DISPLAYBLOCK *OnScreenBlockList[maxobjects];
int NumOnScreenBlocks;

extern DISPLAYBLOCK *ActiveBlockList[];
extern int NumActiveBlocks;

extern int ScanDrawMode;
/* JH 13/5/97 */
extern int DrawMode;
extern int ZBufferMode;

//extern DPID MultiplayerObservedPlayer;
extern int MultiplayerObservedPlayer;

#if SupportMorphing
MORPHDISPLAY MorphDisplay;
#endif

#if SupportModules
SCENEMODULE **Global_ModulePtr = 0;
MODULE *Global_MotherModule;
char *ModuleCurrVisArray = 0;
char *ModulePrevVisArray = 0;
char *ModuleTempArray = 0;
char *ModuleLocalVisArray = 0;
int ModuleArraySize = 0;
#endif

/* KJL 11:12:10 06/06/97 - orientation */
MATRIXCH LToVMat;
EULER LToVMat_Euler;
MATRIXCH WToLMat = {1,};
VECTORCH LocalView;

/* KJL 11:16:37 06/06/97 - lights */
VECTORCH LocalLightCH;
int NumLightSourcesForObject;
LIGHTBLOCK *LightSourcesForObject[MaxLightsPerObject];
int GlobalAmbience;
int LightScale=ONE_FIXED;
int DrawingAReflection;

int *Global_ShapePoints;
int **Global_ShapeItems;
int *Global_ShapeNormals;
int *Global_ShapeVNormals;
int **Global_ShapeTextures;
VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
DISPLAYBLOCK *Global_ODB_Ptr;
SHAPEHEADER *Global_ShapeHeaderPtr;
EXTRAITEMDATA *Global_EID_Ptr;
int *Global_EID_IPtr;


extern float CameraZoomScale;
extern int CameraZoomLevel;
int AlienBiteAttackInProgress;

/* phase for cloaked objects */
int CloakingPhase;
extern int NormalFrameTime;

int LeanScale;
EULER deathTargetOrientation={0,0,0};

extern int GetSingleColourForPrimary(int Colour);
extern void ColourFillBackBuffer(int FillColour);

static void ModifyHeadOrientation(void);
int AVPViewVolumePlaneTest(CLIPPLANEBLOCK *cpb, DISPLAYBLOCK *dblockptr, int obr);



void UpdateRunTimeLights(void)
{
	extern int NumActiveBlocks;
	extern DISPLAYBLOCK *ActiveBlockList[];
	int numberOfObjects = NumActiveBlocks;

	while (numberOfObjects--)
	{
		DISPLAYBLOCK *dispPtr = ActiveBlockList[numberOfObjects];

		if( (dispPtr->SpecialFXFlags & SFXFLAG_ONFIRE)
		  ||((dispPtr->ObStrategyBlock)&&(dispPtr->ObStrategyBlock->SBDamageBlock.IsOnFire)) )
			AddLightingEffectToObject(dispPtr,LFX_OBJECTONFIRE);

		UpdateObjectLights(dispPtr);
	}

	HandleLightElementSystem();
}																			
void LightSourcesInRangeOfObject(DISPLAYBLOCK *dptr)
{

	DISPLAYBLOCK **aptr;
	DISPLAYBLOCK *dptr2;
	LIGHTBLOCK *lptr;
	VECTORCH llocal;
	int i, j;


	aptr = ActiveBlockList;


	NumLightSourcesForObject = 0;


	/*

	Light Sources attached to other objects

	*/

	for(i = NumActiveBlocks;
		i!=0 && NumLightSourcesForObject < MaxLightsPerObject; i--) {

		dptr2 = *aptr++;

		if(dptr2->ObNumLights) {

			for(j = 0; j < dptr2->ObNumLights
				&& NumLightSourcesForObject < MaxLightsPerObject; j++) {

				lptr = dptr2->ObLights[j];

				if (!lptr->LightBright || !(lptr->RedScale||lptr->GreenScale||lptr->BlueScale))
				{
					 continue;
				}

				if ((CurrentVisionMode == VISION_MODE_IMAGEINTENSIFIER) && (lptr->LightFlags & LFlag_PreLitSource))
					 continue;
//				lptr->LightFlags |= LFlag_NoSpecular;

		   		if(!(dptr->ObFlags3 & ObFlag3_PreLit &&
					lptr->LightFlags & LFlag_PreLitSource))
				{
					{
						VECTORCH vertexToLight;
						int distanceToLight;

						if (DrawingAReflection)
						{
							vertexToLight.vx = (MirroringAxis - lptr->LightWorld.vx) - dptr->ObWorld.vx;
						}
						else
						{
							vertexToLight.vx = lptr->LightWorld.vx - dptr->ObWorld.vx;
						}
						vertexToLight.vy = lptr->LightWorld.vy - dptr->ObWorld.vy;
						vertexToLight.vz = lptr->LightWorld.vz - dptr->ObWorld.vz;

						distanceToLight = Approximate3dMagnitude(&vertexToLight);

						#if 0
						if (CurrentVisionMode == VISION_MODE_IMAGEINTENSIFIER)
							distanceToLight /= 2;
						#endif

						if(distanceToLight < (lptr->LightRange + dptr->ObRadius) )
						{

							LightSourcesForObject[NumLightSourcesForObject] = lptr;
							NumLightSourcesForObject++;

							/* Transform the light position to local space */

							llocal = vertexToLight;

							RotateAndCopyVector(&llocal, &lptr->LocalLP, &WToLMat);

						}


					}

				}

			}

		}

	}

	{
		extern LIGHTELEMENT LightElementStorage[];
		extern int NumActiveLightElements;
		int i = NumActiveLightElements;
		LIGHTELEMENT *lightElementPtr = LightElementStorage;
		while(i--)
		{
			LIGHTBLOCK *lptr = &(lightElementPtr->LightBlock);
			VECTORCH vertexToLight;
			int distanceToLight;

			vertexToLight.vx = lptr->LightWorld.vx - dptr->ObWorld.vx;
			vertexToLight.vy = lptr->LightWorld.vy - dptr->ObWorld.vy;
			vertexToLight.vz = lptr->LightWorld.vz - dptr->ObWorld.vz;

			distanceToLight = Approximate3dMagnitude(&vertexToLight);

			#if 0
			if (CurrentVisionMode == VISION_MODE_IMAGEINTENSIFIER)
				distanceToLight /= 2;
			#endif

			if(distanceToLight < (lptr->LightRange + dptr->ObRadius) )
			{

				LightSourcesForObject[NumLightSourcesForObject] = lptr;
				NumLightSourcesForObject++;

				/* Transform the light position to local space */
				llocal = vertexToLight;
				RotateAndCopyVector(&llocal, &lptr->LocalLP, &WToLMat);

			}

			lightElementPtr++;
		}
	}
}

int LightIntensityAtPoint(VECTORCH *pointPtr)
{
	int intensity = 0;
	int i, j;
	
	DISPLAYBLOCK **activeBlockListPtr = ActiveBlockList;
	for(i = NumActiveBlocks; i != 0; i--) {
		DISPLAYBLOCK *dispPtr = *activeBlockListPtr++;
		
		if (dispPtr->ObNumLights) {
			for(j = 0; j < dispPtr->ObNumLights; j++) {
				LIGHTBLOCK *lptr = dispPtr->ObLights[j];
				VECTORCH disp = lptr->LightWorld;
				int dist;
				
				disp.vx -= pointPtr->vx;
				disp.vy -= pointPtr->vy;
				disp.vz -= pointPtr->vz;
				
				dist = Approximate3dMagnitude(&disp);
				
				if (dist<lptr->LightRange) {
					intensity += WideMulNarrowDiv(lptr->LightBright,lptr->LightRange-dist,lptr->LightRange);
				}
			}
		}
	}
	if (intensity>ONE_FIXED) intensity=ONE_FIXED;
	else if (intensity<GlobalAmbience) intensity=GlobalAmbience;
	
	/* KJL 20:31:39 12/1/97 - limit how dark things can be so blood doesn't go green */
	if (intensity<10*256) intensity = 10*256;

	return intensity;
}

EULER HeadOrientation = {0,0,0};

static void ModifyHeadOrientation(void)
{
	extern int NormalFrameTime;
	#define TILT_THRESHOLD 128
	PLAYER_STATUS *playerStatusPtr;
    
	/* get the player status block ... */
	playerStatusPtr = (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
  
    if (!playerStatusPtr->IsAlive && !MultiplayerObservedPlayer)
	{
		int decay = NormalFrameTime>>6;
		
		HeadOrientation.EulerX &= 4095;
	   	HeadOrientation.EulerX -= decay;
		if(HeadOrientation.EulerX < 3072)
			HeadOrientation.EulerX = 3072;

	}
	else
	{
		int decay = NormalFrameTime>>8;
		if(HeadOrientation.EulerX > 2048)
		{
			if (HeadOrientation.EulerX < 4096 - TILT_THRESHOLD)
				HeadOrientation.EulerX = 4096 - TILT_THRESHOLD;

		   	HeadOrientation.EulerX += decay;
			if(HeadOrientation.EulerX > 4095)
				HeadOrientation.EulerX =0;
		}
		else
		{
			if (HeadOrientation.EulerX > TILT_THRESHOLD)
				HeadOrientation.EulerX = TILT_THRESHOLD;

		   	HeadOrientation.EulerX -= decay;
			if(HeadOrientation.EulerX < 0)
				HeadOrientation.EulerX =0;
		}

		if(HeadOrientation.EulerY > 2048)
		{
			if (HeadOrientation.EulerY < 4096 - TILT_THRESHOLD)
				HeadOrientation.EulerY = 4096 - TILT_THRESHOLD;

		   	HeadOrientation.EulerY += decay;
			if(HeadOrientation.EulerY > 4095)
				HeadOrientation.EulerY =0;
		}
		else
		{
			if (HeadOrientation.EulerY > TILT_THRESHOLD)
				HeadOrientation.EulerY = TILT_THRESHOLD;

		   	HeadOrientation.EulerY -= decay;
			if(HeadOrientation.EulerY < 0)
				HeadOrientation.EulerY =0;
		}
		
		if(HeadOrientation.EulerZ > 2048)
		{
			if (HeadOrientation.EulerZ < 4096 - TILT_THRESHOLD)
				HeadOrientation.EulerZ = 4096 - TILT_THRESHOLD;

		   	HeadOrientation.EulerZ += decay;
			if(HeadOrientation.EulerZ > 4095)
				HeadOrientation.EulerZ =0;
		}
		else
		{
			if (HeadOrientation.EulerZ > TILT_THRESHOLD)
				HeadOrientation.EulerZ = TILT_THRESHOLD;

		   	HeadOrientation.EulerZ -= decay;
			if(HeadOrientation.EulerZ < 0)
				HeadOrientation.EulerZ =0;
		}
	}
}

void InteriorType_Body()
{
	DISPLAYBLOCK *subjectPtr = Player;
	extern int NormalFrameTime;

	static int verticalSpeed = 0;
	static int zAxisTilt=0;
	STRATEGYBLOCK *sbPtr;
	DYNAMICSBLOCK *dynPtr;
	
	sbPtr = subjectPtr->ObStrategyBlock;
	LOCALASSERT(sbPtr);
	dynPtr = sbPtr->DynPtr;	
	LOCALASSERT(dynPtr);
    
	ModifyHeadOrientation();
	{
		/* eye offset */
		VECTORCH ioff;
		COLLISION_EXTENTS *extentsPtr = 0;
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

		switch(AvP.PlayerType)
		{
			case I_Marine:
				extentsPtr = &CollisionExtents[CE_MARINE];
				break;
				
			case I_Alien:
				extentsPtr = &CollisionExtents[CE_ALIEN];
				break;
			
			case I_Predator:
				extentsPtr = &CollisionExtents[CE_PREDATOR];
				break;
		}
		
		/* set player state */
		if (playerStatusPtr->ShapeState == PMph_Standing)
		{
			ioff.vy = extentsPtr->StandingTop;
		}
		else
		{
			ioff.vy = extentsPtr->CrouchingTop;
		}

		if (LANDOFTHEGIANTS_CHEATMODE)
		{
			ioff.vy/=4;
		}
		if (!playerStatusPtr->IsAlive && !MultiplayerObservedPlayer)
		{
			extern int deathFadeLevel;
			
			ioff.vy = MUL_FIXED(deathFadeLevel*4-3*ONE_FIXED,ioff.vy);

			if (ioff.vy>-100)
			{
				ioff.vy = -100;
			}
		}

				
		ioff.vx = 0;
		ioff.vz = 0;//-extentsPtr->CollisionRadius*2;
		ioff.vy += verticalSpeed/16+200;

		RotateVector(&ioff, &subjectPtr->ObMat);
		AddVector(&ioff, &Global_VDB_Ptr->VDB_World);
		
		#if 0
		{
			static int i=-10;
			i=-i;
			ioff.vx = MUL_FIXED(GetSin((CloakingPhase/5)&4095),i);
			ioff.vy = MUL_FIXED(GetCos((CloakingPhase/3)&4095),i);
			ioff.vz = 0;

			RotateVector(&ioff, &subjectPtr->ObMat);
			AddVector(&ioff, &Global_VDB_Ptr->VDB_World);


		}
		#endif
	}
	{
		EULER orientation;
		MATRIXCH matrix;

		orientation = HeadOrientation;

	  orientation.EulerZ += (zAxisTilt>>8);
	  orientation.EulerZ &= 4095;
		
		if (NAUSEA_CHEATMODE)
		{
			orientation.EulerZ = (orientation.EulerZ+GetSin((CloakingPhase/2)&4095)/256)&4095;
			orientation.EulerX = (orientation.EulerX+GetSin((CloakingPhase/2+500)&4095)/512)&4095;
			orientation.EulerY = (orientation.EulerY+GetSin((CloakingPhase/3+800)&4095)/512)&4095;
		}
		// The next test drops the matrix multiply if the orientation is close to zero
		// There is an inaccuracy problem with the Z angle at this point
					 
		if (orientation.EulerX != 0 || orientation.EulerY != 0 || 
					(orientation.EulerZ > 1 && orientation.EulerZ <	4095))
		{
			CreateEulerMatrix(&orientation, &matrix);
			MatrixMultiply(&Global_VDB_Ptr->VDB_Mat, &matrix, &Global_VDB_Ptr->VDB_Mat);
	 	}

	}
	
	{
		VECTORCH relativeVelocity;
		
		/* get subject's total velocity */
		{
			MATRIXCH worldToLocalMatrix;

			/* make world to local matrix */
			worldToLocalMatrix = subjectPtr->ObMat;
			TransposeMatrixCH(&worldToLocalMatrix);													   

			relativeVelocity.vx = dynPtr->Position.vx - dynPtr->PrevPosition.vx;		
			relativeVelocity.vy = dynPtr->Position.vy - dynPtr->PrevPosition.vy;
			relativeVelocity.vz = dynPtr->Position.vz - dynPtr->PrevPosition.vz;
			/* rotate into object space */

			RotateVector(&relativeVelocity,&worldToLocalMatrix);
		}	 
		
		{
			int targetingSpeed = 10*NormalFrameTime;
	
			/* KJL 14:08:50 09/20/96 - the targeting is FRI, but care has to be taken
			   at very low frame rates to ensure that you can't overshoot */
			if (targetingSpeed > 65536)	targetingSpeed=65536;
					
			zAxisTilt += MUL_FIXED
				(
					DIV_FIXED
					(
						MUL_FIXED(relativeVelocity.vx,LeanScale),
						NormalFrameTime
					)-zAxisTilt,
					targetingSpeed
				);

			{
				static int previousVerticalSpeed = 0;
				int difference;

				if (relativeVelocity.vy >= 0)
				{ 
					difference = DIV_FIXED
					(
						previousVerticalSpeed - relativeVelocity.vy,
						NormalFrameTime
					);
				}
				else difference = 0;

				if (verticalSpeed < difference) verticalSpeed = difference;
				
			 	if(verticalSpeed > 150*16) verticalSpeed = 150*16;
				
				verticalSpeed -= NormalFrameTime>>2;
				if (verticalSpeed < 0) verticalSpeed = 0;				
				
				previousVerticalSpeed = relativeVelocity.vy;
			}
	 	}
	}
}

void UpdateCamera(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	int cos = GetCos(playerStatusPtr->ViewPanX);
	int sin = GetSin(playerStatusPtr->ViewPanX);
	MATRIXCH mat;
	DISPLAYBLOCK *dptr_s = Player;

	Global_VDB_Ptr->VDB_World = dptr_s->ObWorld;
	Global_VDB_Ptr->VDB_Mat = dptr_s->ObMat;

	mat.mat11 = ONE_FIXED;		 
	mat.mat12 = 0;
	mat.mat13 = 0;
	mat.mat21 = 0;	  	
	mat.mat22 = cos;	  	
	mat.mat23 = -sin;	  	
	mat.mat31 = 0;	  	
	mat.mat32 = sin;	  	
	mat.mat33 = cos;	  	
 	MatrixMultiply(&Global_VDB_Ptr->VDB_Mat,&mat,&Global_VDB_Ptr->VDB_Mat);

		
	InteriorType_Body();
}

void AVPGetInViewVolumeList(VIEWDESCRIPTORBLOCK *VDB_Ptr)
{
	DISPLAYBLOCK **activeblocksptr;
	int t;
	#if (SupportModules && SupportMultiCamModules)
	int MVis;
	#endif

	/* Initialisation */
	NumOnScreenBlocks = 0;

	/* Scan the Active Blocks List */
	activeblocksptr = &ActiveBlockList[0];

	for(t = NumActiveBlocks; t!=0; t--)
	{
		DISPLAYBLOCK *dptr = *activeblocksptr++;
	
		if (dptr==Player) continue;
		MVis = Yes;
		if(dptr->ObMyModule)
		{
			MODULE *mptr = dptr->ObMyModule;
			if(ModuleCurrVisArray[mptr->m_index] != 2) MVis = No;
			else
			{
				extern int NumberOfLandscapePolygons;
				SHAPEHEADER *shapePtr = GetShapeData(dptr->ObShape);
				NumberOfLandscapePolygons+=shapePtr->numitems;
			}

		}
		if (!(dptr->ObFlags&ObFlag_NotVis) && MVis) 
		{
			MakeVector(&dptr->ObWorld, &VDB_Ptr->VDB_World, &dptr->ObView);
			RotateVector(&dptr->ObView, &VDB_Ptr->VDB_Mat);

			/* Screen Test */
			#if MIRRORING_ON
			if (MirroringActive || dptr->HModelControlBlock || dptr->SfxPtr)
			{
				OnScreenBlockList[NumOnScreenBlocks++] = dptr;
			}
			else if (ObjectWithinFrustrum(dptr))
			{
				OnScreenBlockList[NumOnScreenBlocks++] = dptr;
			}
			#else
			if(dptr->SfxPtr || dptr->HModelControlBlock || ObjectWithinFrustrum(dptr))
			{
				OnScreenBlockList[NumOnScreenBlocks++] = dptr;
			}
			else
			{
				if(dptr->HModelControlBlock)
				{
					DoHModelTimer(dptr->HModelControlBlock);
				}
			}
			#endif
		}
		
	}
}

void ReflectObject(DISPLAYBLOCK *dPtr)
{
	dPtr->ObWorld.vx = MirroringAxis - dPtr->ObWorld.vx;
	dPtr->ObMat.mat11 = -dPtr->ObMat.mat11;
	dPtr->ObMat.mat21 = -dPtr->ObMat.mat21;
	dPtr->ObMat.mat31 = -dPtr->ObMat.mat31;
}

void CheckIfMirroringIsRequired(void);
void AvpShowViews(void)
{
	FlushD3DZBuffer();

	UpdateAllFMVTextures();	


	/* Update attached object positions and orientations etc. */
	UpdateCamera();

	/* Initialise the global VMA */
//	GlobalAmbience=655;
//	textprint("Global Ambience: %d\n",GlobalAmbience);

	/* Prepare the View Descriptor Block for use in ShowView() */

	PrepareVDBForShowView(Global_VDB_Ptr);
	PlatformSpecificShowViewEntry(Global_VDB_Ptr, &ScreenDescriptorBlock);
	TranslationSetup();

	{
		extern void ThisFramesRenderingHasBegun(void);
		ThisFramesRenderingHasBegun();
		D3D_DrawBackdrop();
	}

	/* Now we know where the camera is, update the modules */

	#if SupportModules
	AllNewModuleHandler();
//	ModuleHandler(Global_VDB_Ptr);
	#endif

	#if MIRRORING_ON
	CheckIfMirroringIsRequired();
	#endif

	/* Do lights */
	UpdateRunTimeLights();
	if (AvP.PlayerType==I_Alien)
	{
		MakeLightElement(&Player->ObWorld,LIGHTELEMENT_ALIEN_TEETH);
		MakeLightElement(&Player->ObWorld,LIGHTELEMENT_ALIEN_TEETH2);
	}

//	GlobalAmbience=ONE_FIXED/4;
	/* Find out which objects are in the View Volume */
	AVPGetInViewVolumeList(Global_VDB_Ptr);

	if (AlienBiteAttackInProgress)
	{
		CameraZoomScale += (float)NormalFrameTime/65536.0f;
		if (CameraZoomScale > 1.0f)
		{
			AlienBiteAttackInProgress = 0;
			CameraZoomScale = 1.0f;
		}
	}

	/* update players weapon */
	UpdateWeaponStateMachine();
	/* lights associated with the player may have changed */
	UpdateObjectLights(Player);


	if(NumOnScreenBlocks)
	{
	 	/* KJL 12:13:26 02/05/97 - divert rendering for AvP */
		KRenderItems(Global_VDB_Ptr);
	}

	PlatformSpecificShowViewExit(Global_VDB_Ptr, &ScreenDescriptorBlock);

	#if SupportZBuffering
	if ((ScanDrawMode != ScanDrawDirectDraw) &&	(ZBufferMode != ZBufferOff))
	{
		/* KJL 10:25:44 7/23/97 - this offset is used to push back the normal game gfx,
		so that the HUD can be drawn over the top without sinking into walls, etc. */
		HeadUpDisplayZOffset = 0;
	}
	#endif
}


void InitCameraValues(void)
{
	extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
	Global_VDB_Ptr = ActiveVDBList[0];

	HeadOrientation.EulerX = 0;
	HeadOrientation.EulerY = 0;
	HeadOrientation.EulerZ = 0;

	CameraZoomScale = 1.0f;
	CameraZoomLevel=0;
}



/*

 Prepare the View Descriptor Block for use in ShowView() and others.

 If there is a display block attached to the view, update the view location
 and orientation.

*/

void PrepareVDBForShowView(VIEWDESCRIPTORBLOCK *VDB_Ptr)
{
	EULER e;

	
	/* Get the View Object Matrix, transposed */
 	TransposeMatrixCH(&VDB_Ptr->VDB_Mat);

	/* Get the Matrix Euler Angles */
	MatrixToEuler(&VDB_Ptr->VDB_Mat, &VDB_Ptr->VDB_MatrixEuler);
	
	/* Get the Matrix Euler Angles */
	MatrixToEuler(&VDB_Ptr->VDB_Mat, &e);

	/* Create the "sprite" matrix" */
	e.EulerX = 0;
	e.EulerY = 0;
	e.EulerZ = (-e.EulerZ) & wrap360;
	
	CreateEulerMatrix(&e, &VDB_Ptr->VDB_SpriteMat);
}

   
/*

 This function updates the position and orientation of the lights attached
 to an object.

 It must be called after the object has completed its movements in a frame,
 prior to the call to the renderer.

*/

void UpdateObjectLights(DISPLAYBLOCK *dptr)
{

	int i;
	LIGHTBLOCK *lptr;
	LIGHTBLOCK **larrayptr = &dptr->ObLights[0];


	for(i = dptr->ObNumLights; i!=0; i--)
	{
		/* Get a light */
		lptr = *larrayptr++;

		/* Calculate the light's location */
		if(!(lptr->LightFlags & LFlag_AbsPos))
		{
			CopyVector(&dptr->ObWorld, &lptr->LightWorld);
     	}
		LOCALASSERT(lptr->LightRange!=0);
		lptr->BrightnessOverRange = DIV_FIXED(MUL_FIXED(lptr->LightBright,LightScale),lptr->LightRange);
	}
	

}














/****************************************************************************/

/*

 Find out which light sources are in range of the object.

*/




/*

 Initialise the Renderer

*/

void InitialiseRenderer(void)
{
	InitialiseObjectBlocks();
	InitialiseStrategyBlocks();

	InitialiseTxAnimBlocks();

	InitialiseLightBlocks();
	InitialiseVDBs();

	/* KJL 14:46:42 09/09/98 */
	InitialiseLightIntensityStamps();
}





/*

 General View Volume Test for Objects and Sub-Object Trees

 This function returns returns "Yes" / "True" for an if()

*/

int AVPViewVolumeTest(VIEWDESCRIPTORBLOCK *VDB_Ptr, DISPLAYBLOCK *dblockptr)
{
	int obr = dblockptr->ObRadius;

	/* Perform the view volume plane tests */

	if(
	AVPViewVolumePlaneTest(&VDB_Ptr->VDB_ClipZPlane, dblockptr, obr) &&
	AVPViewVolumePlaneTest(&VDB_Ptr->VDB_ClipLeftPlane, dblockptr, obr) &&
	AVPViewVolumePlaneTest(&VDB_Ptr->VDB_ClipRightPlane, dblockptr, obr) &&
	AVPViewVolumePlaneTest(&VDB_Ptr->VDB_ClipUpPlane, dblockptr, obr) &&
	AVPViewVolumePlaneTest(&VDB_Ptr->VDB_ClipDownPlane, dblockptr, obr))
		return Yes;

	else
		return No;

}
/*

 View Volume Plane Test

 Make the ODB VSL relative to the VDB Clip Plane POP and dot the resultant
 vector with the Clip Plane Normal.

*/

int AVPViewVolumePlaneTest(CLIPPLANEBLOCK *cpb, DISPLAYBLOCK *dblockptr, int obr)
{
	VECTORCH POPRelObView;

	MakeVector(&dblockptr->ObView, &cpb->CPB_POP, &POPRelObView);

	if(DotProduct(&POPRelObView, &cpb->CPB_Normal) < obr) return Yes;
	else return No;
}


#if MIRRORING_ON
void CheckIfMirroringIsRequired(void)
{
	extern char LevelName[];
	extern MODULE * playerPherModule;

	MirroringActive = 0;
	#if 0
	if ( (!stricmp(LevelName,"e3demo")) || (!stricmp(LevelName,"e3demosp")) )
	{
		int numOfObjects = NumActiveBlocks;

		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = ActiveBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
				if(!stricmp(modulePtr->name,"marine01b"))
				{
					if(ModuleCurrVisArray[modulePtr->m_index] == 2)
					{
						MirroringActive = 1;
						MirroringAxis = -149*2;
						break;
					}
				}
			}
		}
	
		if (playerPherModule && playerPherModule->name)
		{
			textprint("<%s>\n",playerPherModule->name);
			if((!stricmp(playerPherModule->name,"predator"))
			 ||(!stricmp(playerPherModule->name,"predator01"))
			 ||(!stricmp(playerPherModule->name,"predator03"))
			 ||(!stricmp(playerPherModule->name,"predator02")) )
			{
				MirroringActive = 1;
				MirroringAxis = -7164*2;
			}
		}
	}
	else
	#endif 
	#if 1
	if (!stricmp(LevelName,"derelict"))
	{
		if (playerPherModule && playerPherModule->name)
		{
			if((!stricmp(playerPherModule->name,"start"))
			 ||(!stricmp(playerPherModule->name,"start-en01")) )
			{
				MirroringActive = 1;
				MirroringAxis = -5596*2;
			}
		}
	}
	#endif
}
#endif

#define MinChangeInXSize 8
void MakeViewingWindowSmaller(void)
{
	extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
	int MinChangeInYSize = (ScreenDescriptorBlock.SDB_Height*MinChangeInXSize)/ScreenDescriptorBlock.SDB_Width;
	
	if (Global_VDB_Ptr->VDB_ClipLeft<ScreenDescriptorBlock.SDB_Width/2-16)
	{
		Global_VDB_Ptr->VDB_ClipLeft +=MinChangeInXSize;
		Global_VDB_Ptr->VDB_ClipRight -=MinChangeInXSize;
		Global_VDB_Ptr->VDB_ClipUp +=MinChangeInYSize;
		Global_VDB_Ptr->VDB_ClipDown -=MinChangeInYSize;
	}
	if(AvP.PlayerType == I_Alien)
	{
		Global_VDB_Ptr->VDB_ProjX = (Global_VDB_Ptr->VDB_ClipRight - Global_VDB_Ptr->VDB_ClipLeft)/4;
		Global_VDB_Ptr->VDB_ProjY = (Global_VDB_Ptr->VDB_ClipDown - Global_VDB_Ptr->VDB_ClipUp)/4;
	}
	else
	{
		Global_VDB_Ptr->VDB_ProjX = (Global_VDB_Ptr->VDB_ClipRight - Global_VDB_Ptr->VDB_ClipLeft)/2;
		Global_VDB_Ptr->VDB_ProjY = (Global_VDB_Ptr->VDB_ClipDown - Global_VDB_Ptr->VDB_ClipUp)/2;
	}
	//BlankScreen(); 
}

void MakeViewingWindowLarger(void)
{
	extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
	int MinChangeInYSize = (ScreenDescriptorBlock.SDB_Height*MinChangeInXSize)/ScreenDescriptorBlock.SDB_Width;

	if (Global_VDB_Ptr->VDB_ClipLeft>0)
	{
		Global_VDB_Ptr->VDB_ClipLeft -=MinChangeInXSize;
		Global_VDB_Ptr->VDB_ClipRight +=MinChangeInXSize;
		Global_VDB_Ptr->VDB_ClipUp -=MinChangeInYSize;
		Global_VDB_Ptr->VDB_ClipDown +=MinChangeInYSize;
	}
	if(AvP.PlayerType == I_Alien)
	{
		Global_VDB_Ptr->VDB_ProjX = (Global_VDB_Ptr->VDB_ClipRight - Global_VDB_Ptr->VDB_ClipLeft)/4;
		Global_VDB_Ptr->VDB_ProjY = (Global_VDB_Ptr->VDB_ClipDown - Global_VDB_Ptr->VDB_ClipUp)/4;
	}
	else
	{
		Global_VDB_Ptr->VDB_ProjX = (Global_VDB_Ptr->VDB_ClipRight - Global_VDB_Ptr->VDB_ClipLeft)/2;
		Global_VDB_Ptr->VDB_ProjY = (Global_VDB_Ptr->VDB_ClipDown - Global_VDB_Ptr->VDB_ClipUp)/2;
	}
}


extern void AlienBiteAttackHasHappened(void)
{
	extern int AlienTongueOffset;
	extern int AlienTeethOffset;

	AlienBiteAttackInProgress = 1;

	CameraZoomScale = 0.25f;
	AlienTongueOffset = ONE_FIXED;
	AlienTeethOffset = 0;
}

