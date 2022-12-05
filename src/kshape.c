/*KJL************************************************************************************
* kshape.c - replacement for all the pipeline stuff previously done in shape.c & clip.c *
************************************************************************************KJL*/
#include "3dc.h"
#include <math.h>
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "maths.h"

#include "kshape.h"
#include "kzsort.h"
#include "frustum.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "equipmnt.h"
#include "bh_pred.h"
#include "bh_marin.h"
#include "bh_corpse.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "bh_types.h"
#include "pldghost.h"
#include "particle.h"
#include "vision.h"
#include "sfx.h"
#include "d3d_render.h"
#include "opengl.h"
#include "avpview.h"
#include "sphere.h"
#include "detaillevels.h"
#include "avp_userprofile.h"
#include "hud.h"
#include "weapons.h"

#define ALIENS_LIFEFORCE_GLOW_COLOUR 0x20ff8080
#define MARINES_LIFEFORCE_GLOW_COLOUR 0x208080ff
#define PREDATORS_LIFEFORCE_GLOW_COLOUR 0x2080ff80

/* KJL 15:02:50 05/14/97 - new max lighting intensity */
#define MAX_INTENSITY (65536*4-1)

extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
extern DISPLAYBLOCK *Global_ODB_Ptr;
extern EXTRAITEMDATA *Global_EID_Ptr;
extern int *Global_EID_IPtr;
extern int ScanDrawMode;
extern int ZBufferMode;
extern int NormalFrameTime;

extern SHAPEHEADER *Global_ShapeHeaderPtr;
extern int *Global_ShapePoints;
extern int **Global_ShapeItems;
extern int *Global_ShapeNormals;
extern int *Global_ShapeVNormals;
extern int **Global_ShapeTextures;

extern MATRIXCH LToVMat;
extern EULER LToVMat_Euler;
extern MATRIXCH WToLMat;
extern VECTORCH LocalView;
extern VECTORCH LocalLightCH;

extern int NumLightSourcesForObject;
extern LIGHTBLOCK *LightSourcesForObject[];

#if SupportMorphing
extern MORPHDISPLAY MorphDisplay;
#endif

extern int VideoModeType;
extern int GlobalAmbience;
extern int NumActiveBlocks;

extern DISPLAYBLOCK *ActiveBlockList[];
extern SHAPEHEADER **mainshapelist;

int MirroringActive=0;
int MirroringAxis=-149*2;

VECTORCHF FogPosition;
float FogMagnitude;
#define VOLUMETRIC_FOG 0
#define UNDERWATER 0 
#define SPATIAL_SHOCKWAVE 0
float CameraZoomScale;

int DrawFullBright;

int TripTasticPhase;

void SetupShapePipeline(void);
void ShapePipeline(SHAPEHEADER *shapePtr);

static void GouraudPolygon_Construct(POLYHEADER *polyPtr);

static void GouraudTexturedPolygon_Construct(POLYHEADER *polyPtr);

static void (*VertexIntensity)(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_Pred_Thermal(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_Pred_SeeAliens(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_Pred_SeePredatorTech(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_ImageIntensifier(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_Alien_Sense(RENDERVERTEX *renderVertexPtr);

static void VertexIntensity_Standard_Opt(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_FullBright(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_DiscoInferno(RENDERVERTEX *renderVertexPtr);
static void VertexIntensity_Underwater(RENDERVERTEX *renderVertexPtr);


extern void CreateTxAnimUVArray(int *txa_data, int *uv_array, int *shapeitemptr);

void PredatorThermalVision_ShapePipeline(SHAPEHEADER *shapePtr);
void PredatorSeeAliensVision_ShapePipeline(SHAPEHEADER *shapePtr);
static void CloakedPolygon_Construct(POLYHEADER *polyPtr);
static void PredatorThermalVisionPolygon_Construct(POLYHEADER *polyPtr);
static void PredatorSeeAliensVisionPolygon_Construct(POLYHEADER *polyPtr);
void DoAlienEnergyView(DISPLAYBLOCK *dispPtr);
static void FindAlienEnergySource_Recursion(HMODELCONTROLLER *controllerPtr, SECTION_DATA *sectionDataPtr, unsigned int colour);
void SquishPoints(SHAPEINSTR *shapeinstrptr);
void MorphPoints(SHAPEINSTR *shapeinstrptr);
void TranslateShapeVertices(SHAPEINSTR *shapeinstrptr);
static void ParticlePolygon_Construct(PARTICLE *particlePtr);
void RenderMirroredDecal(DECAL *decalPtr);
static void DecalPolygon_Construct(DECAL *decalPtr);
void RenderShaftOfLight2(MODULE *modulePtr);
void FindIntersectionWithYPlane(VECTORCH *startPtr, VECTORCH *directionPtr, VECTORCH *intersectionPtr);
void FindZFromXYIntersection(VECTORCH *startPtr, VECTORCH *directionPtr, VECTORCH *intersectionPtr);
void AddToTranslucentPolyList(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr);
void DrawWaterFallPoly(VECTORCH *v);


/*KJL************************************************************************************
* N.B. All the following global variables have their first elements initialised so that *
* they will end up in high memory on the Saturn.                                        *
************************************************************************************KJL*/

/*
 Global variables and arrays
*/

VECTORCH RotatedPts[maxrotpts]={{1,}};
int ItemColour=1;



#if SupportMorphing

#if (LazyEvaluationForMorphing == No)
VECTORCH MorphedPts[maxmorphPts];
#endif


#endif	/* SupportMorphing */

					
static COLOURINTENSITIES ColourIntensityArray[maxrotpts];
														


RENDERPOLYGON RenderPolygon;
RENDERVERTEX VerticesBuffer[9]={{1,}};
static RENDERVERTEX TriangleVerticesBuffer[3]={{1,}};

static int *VertexNumberPtr=(int*)1;

extern struct KItem KItemList[maxpolyptrs];
extern int *MorphedObjectPointsPtr;

#define MAX_NO_OF_TRANSLUCENT_POLYGONS 1000
RENDERPOLYGON TranslucentPolygons[MAX_NO_OF_TRANSLUCENT_POLYGONS];
POLYHEADER TranslucentPolygonHeaders[MAX_NO_OF_TRANSLUCENT_POLYGONS];
int CurrentNumberOfTranslucentPolygons;

/* KJL 10:25:44 7/23/97 - this offset is used to push back the normal game gfx,
so that the HUD can be drawn over the top without sinking into walls, etc. */
int HeadUpDisplayZOffset=0;

extern int CloakingPhase;
static VECTORCH ObjectCentre;
static int HierarchicalObjectsLowestYValue;

HEATSOURCE HeatSourceList[MAX_NUMBER_OF_HEAT_SOURCES];
int NumberOfHeatSources;
int CloakingMode;
char CloakedPredatorIsMoving;
#if 0
static VECTORCH LocalCameraZAxis;
#endif

static int ObjectCounter;

extern void InitialiseLightIntensityStamps(void)
{
	int i = maxrotpts;
	do
	{
		i--;
		ColourIntensityArray[i].Stamp=0;
	}
	while(i);
	ObjectCounter = 0;

}


void SetupShapePipeline(void)
{
	#if VOLUMETRIC_FOG
	{
//		VECTORCH v = {-30399, -1792, 1050}; // genshd1
//		VECTORCH v = {49937,-4000,-37709};		// hangar
//		VECTORCH v = {-185,0,642};
//		VECTORCH v = {6894,469,-13203};
		VECTORCH v = {73608,3582,56211};
	   	TranslatePointIntoViewspace(&v);
		FogPosition.vx = v.vx;
		FogPosition.vy = v.vy;
		FogPosition.vz = v.vz;
		FogMagnitude = FogPosition.vx*FogPosition.vx+FogPosition.vy*FogPosition.vy+FogPosition.vz*FogPosition.vz;
	}
	#endif
	
	/* Set up these global pointers */
	Global_ShapePoints    = *(Global_ShapeHeaderPtr->points);
	Global_ShapeTextures  = Global_ShapeHeaderPtr->sh_textures;

	if(Global_ODB_Ptr->ObEIDPtr)
	{
		Global_EID_Ptr  = Global_ODB_Ptr->ObEIDPtr;
		Global_EID_IPtr = (int *) Global_ODB_Ptr->ObEIDPtr;
	}
	else
	{
		Global_EID_Ptr  = Global_ShapeHeaderPtr->sh_extraitemdata;
		Global_EID_IPtr = (int *) Global_ShapeHeaderPtr->sh_extraitemdata;
	}

	if(Global_ShapeHeaderPtr->sh_normals)
	{
		Global_ShapeNormals = *(Global_ShapeHeaderPtr->sh_normals);
	}
	else
	{
		Global_ShapeNormals = 0;
	}
	
	if(Global_ShapeHeaderPtr->sh_vnormals)
	{
		Global_ShapeVNormals = *(Global_ShapeHeaderPtr->sh_vnormals);
	}
	else
	{
		Global_ShapeVNormals = 0;
	}


 //	if((Global_ODB_Ptr->ObStrategyBlock)&&(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourQueenAlien))
//		Global_ODB_Ptr->ObFlags3 &= ObFlag3_NoLightDot;

	ObjectCounter++;

}

void ChooseLightingModel(DISPLAYBLOCK *dispPtr)
{
	LOCALASSERT(dispPtr);
	LOCALASSERT(dispPtr->ObShapeData);

	if (DrawFullBright)
	{
		VertexIntensity = VertexIntensity_FullBright;
	}
	else if (DISCOINFERNO_CHEATMODE || TRIPTASTIC_CHEATMODE)
	{
		VertexIntensity = VertexIntensity_DiscoInferno;
	}
	else if (UNDERWATER_CHEATMODE)
	{
		VertexIntensity = VertexIntensity_Underwater;
	}
	else
	{
		switch (CurrentVisionMode)
		{
			default:
			case VISION_MODE_NORMAL:
			{
				VertexIntensity = VertexIntensity_Standard_Opt;
				break;
			}
			case VISION_MODE_ALIEN_SENSE:
			{
				VertexIntensity = VertexIntensity_Alien_Sense;
				break;
			}
			case VISION_MODE_IMAGEINTENSIFIER:
			{
				VertexIntensity = VertexIntensity_ImageIntensifier;
				break;
			}
			case VISION_MODE_PRED_THERMAL:
			{
			  	VertexIntensity = VertexIntensity_Pred_Thermal;
			  	break;
			}
			case VISION_MODE_PRED_SEEALIENS:
			{
				VertexIntensity = VertexIntensity_Pred_SeeAliens;
				break;
			}
			case VISION_MODE_PRED_SEEPREDTECH:
			{
				VertexIntensity = VertexIntensity_Pred_SeePredatorTech;
				break;
			}
		}
	}
}



/*KJL**********************************************************************************
* ShapePipeline() - this function processes a shape for rendering by considering each *
* polygon (item) in turn.                                                             *
**********************************************************************************KJL*/
void ShapePipeline(SHAPEHEADER *shapePtr)
{
	int numitems= shapePtr->numitems;
	int **itemArrayPtr = shapePtr->items;
	#if 0
	char objectCompletelyInView;
	#endif
	LOCALASSERT(numitems);
	
	switch(CurrentVisionMode)
	{
		case VISION_MODE_PRED_THERMAL:
		{
			/* if we have an object with heat sources, draw it as such */
			if (NumberOfHeatSources)//||((Global_ODB_Ptr->ObStrategyBlock)&&(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourAlien)))
			{
				PredatorThermalVision_ShapePipeline(shapePtr);
				return;
			}
			break;
		}
		case VISION_MODE_PRED_SEEALIENS:
		{
			STRATEGYBLOCK *sbPtr = Global_ODB_Ptr->ObStrategyBlock;
		 	if(sbPtr)
		 	{
				int useVision=0;
		 		switch (sbPtr->I_SBtype)
				{
					case I_BehaviourAutoGun:
			 		case I_BehaviourAlien:
					case I_BehaviourQueenAlien:
					case I_BehaviourFaceHugger:
					case I_BehaviourPredatorAlien:
					case I_BehaviourXenoborg:
					{
						useVision=1;
						break;
					}
					case I_BehaviourMarine:
					{
						MARINE_STATUS_BLOCK *marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
						GLOBALASSERT(marineStatusPointer);
						
						if (marineStatusPointer->Android)
						{
							useVision=1;
						}
						break;
					}

					case I_BehaviourNetGhost:
					{
			   			NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;

						if (ghostDataPtr->type==I_BehaviourAlienPlayer || ghostDataPtr->type==I_BehaviourAlien
						 	|| (ghostDataPtr->type==I_BehaviourNetCorpse&&ghostDataPtr->subtype==I_BehaviourAlienPlayer) )
						{	
							useVision=1;
						}
						break;
					}

					case I_BehaviourNetCorpse:
					{
						NETCORPSEDATABLOCK *corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
						if (corpseDataPtr->Android || corpseDataPtr->Type==I_BehaviourAlienPlayer || corpseDataPtr->Type==I_BehaviourAlien)
						{
							useVision=1;
						}
						break;
					}
					case I_BehaviourHierarchicalFragment:
					{
						HDEBRIS_BEHAV_BLOCK *debrisDataPtr  = (HDEBRIS_BEHAV_BLOCK *)sbPtr->SBdataptr;
						if (debrisDataPtr->Type==I_BehaviourAlien
						  ||debrisDataPtr->Type==I_BehaviourQueenAlien
						  ||debrisDataPtr->Type==I_BehaviourPredatorAlien
						  ||debrisDataPtr->Type==I_BehaviourAutoGun
						  ||debrisDataPtr->Android)
						{
							useVision=1;
						}
						break;
					}
					case I_BehaviourSpeargunBolt:
					{
						SPEAR_BEHAV_BLOCK *spearDataPtr  = (SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr;
						if (spearDataPtr->SpearThroughFragment) // more flags required!
						if (spearDataPtr->Type==I_BehaviourAlien
						  ||spearDataPtr->Type==I_BehaviourPredatorAlien
						  ||spearDataPtr->Type==I_BehaviourAutoGun)
						{
							useVision=1;
						}	
						break;
					}
					default:
						break;
				}
	
				if (useVision)
				{
					PredatorSeeAliensVision_ShapePipeline(shapePtr);
					return;
				}
			}
			break;
		}
		case VISION_MODE_PRED_SEEPREDTECH:
		{
			STRATEGYBLOCK *sbPtr = Global_ODB_Ptr->ObStrategyBlock;
		 	if(sbPtr)
		 	{
				int useVision=0;
		 		switch (sbPtr->I_SBtype)
				{
			 		case I_BehaviourPredator:
					{
			   			PREDATOR_STATUS_BLOCK *predData = (PREDATOR_STATUS_BLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;
					
						if (!predData->CloakingEffectiveness)
						{
							useVision=1;
						}
						break;
					}
					case I_BehaviourNPCPredatorDisc:
					case I_BehaviourPredatorDisc_SeekTrack:
					{
						useVision=1;
						break;
					}
					case I_BehaviourNetGhost:
					{
			   			NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;
	
						if((ghostDataPtr->CloakingEffectiveness == 0)
						&& (ghostDataPtr->type==I_BehaviourPredatorPlayer || ghostDataPtr->type==I_BehaviourPredator
					 	|| (ghostDataPtr->type==I_BehaviourInanimateObject&&ghostDataPtr->IOType==IOT_Ammo&&ghostDataPtr->subtype==AMMO_PRED_DISC) 
					 	|| (ghostDataPtr->type==I_BehaviourPredatorDisc_SeekTrack) 
					 	|| (ghostDataPtr->type==I_BehaviourNetCorpse&&ghostDataPtr->subtype==I_BehaviourPredatorPlayer) ))
						{	
							useVision=1;
						}
						break;
					}

					case I_BehaviourNetCorpse:
					{
						NETCORPSEDATABLOCK *corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
						if (corpseDataPtr->Type==I_BehaviourPredatorPlayer || corpseDataPtr->Type==I_BehaviourPredator)
						{
							useVision=1;
						}
						break;
					}
					case I_BehaviourHierarchicalFragment:
					{
						HDEBRIS_BEHAV_BLOCK *debrisDataPtr  = (HDEBRIS_BEHAV_BLOCK *)sbPtr->SBdataptr;
						if (debrisDataPtr->Type==I_BehaviourPredator)
						{
							useVision=1;
						}
						break;
					}
					case I_BehaviourInanimateObject:
					{
						INANIMATEOBJECT_STATUSBLOCK* objStatPtr = (INANIMATEOBJECT_STATUSBLOCK*) sbPtr->SBdataptr;

						switch(objStatPtr->typeId)
						{
							case IOT_FieldCharge:
							{
								useVision = 1;
								break;
							}
							case IOT_Ammo:
							{
								if (objStatPtr->subType == AMMO_PRED_RIFLE || objStatPtr->subType == AMMO_PRED_DISC)
								{
									useVision = 1;
								}
								break;
							}
							default:
								break;
						}
						break;
					}

					default:
						break;
				}
	
				if (useVision)
				{
					PredatorSeeAliensVision_ShapePipeline(shapePtr);
					return;
				}
			}
			else if (!Global_ODB_Ptr->ObMyModule)
			{
				PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
				if (!(playerStatusPtr->cloakOn||playerStatusPtr->CloakingEffectiveness!=0))
				{
					PredatorSeeAliensVision_ShapePipeline(shapePtr);
					return;
				}
			}
			break;
		}
		default:
			break;
	}

// 	if((Global_ODB_Ptr->ObStrategyBlock)&&(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourAlien))
	//textprint("shape alien\n");
	#if 0
	objectCompletelyInView = ObjectCompletelyWithinFrustrum(Global_ODB_Ptr);
	if(!objectCompletelyInView)	TestVerticesWithFrustrum();
	#else
	TestVerticesWithFrustrum();
	#endif

	#if 1
	/* interesting hack for predator cloaking */
  	if(Global_ODB_Ptr->ObStrategyBlock)
	{
		PRED_CLOAKSTATE cloakingStatus =  PCLOAK_Off;

	 	if(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourNetGhost)
		{
   			NETGHOSTDATABLOCK *ghostData = (NETGHOSTDATABLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;
		
			if(ghostData->CloakingEffectiveness)
			{
				cloakingStatus = PCLOAK_On;
				CloakingMode = ONE_FIXED*5/4-ghostData->CloakingEffectiveness;
			}
		}
	 	if(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourPredator)
		{
   			PREDATOR_STATUS_BLOCK *predData = (PREDATOR_STATUS_BLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;
		
			if (predData->CloakingEffectiveness)
			{
				cloakingStatus = PCLOAK_On;
				CloakingMode = ONE_FIXED*5/4-predData->CloakingEffectiveness;//32768;
			}
		}

		if (cloakingStatus == PCLOAK_On)
		{
			do
			{
				POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
				int pif;
				#if 0
				if (objectCompletelyInView)
				{
					pif = PolygonShouldBeDrawn(polyPtr);
				}
				else
				{
					pif = PolygonWithinFrustrum(polyPtr);
				}
				#else
				pif = PolygonWithinFrustrum(polyPtr);
				#endif
				if(pif)
				{		 

					#if 1
					switch(polyPtr->PolyItemType)
					{
						case I_ZB_Gouraud3dTexturedPolygon:
						case I_ZB_Gouraud2dTexturedPolygon:
						CloakedPolygon_Construct(polyPtr);
						if (pif!=2)
						{
							GouraudTexturedPolygon_ClipWithZ();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							D3D_ZBufferedCloakedPolygon_Output(polyPtr,RenderPolygon.Vertices);
						}
						else D3D_ZBufferedCloakedPolygon_Output(polyPtr,VerticesBuffer);
						break;
						default:
							textprint("found polygon of type %d\n",polyPtr->PolyItemType);
							break;
					}
					#else
					{
						CloakedTexturedPolygon_Construct(polyPtr);
						if (pif!=2)
						{
							TexturedPolygon_ClipWithZ();
							if(RenderPolygon.NumberOfVertices<3) continue;
							TexturedPolygon_ClipWithNegativeX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							TexturedPolygon_ClipWithPositiveY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							TexturedPolygon_ClipWithNegativeY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							TexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							D3D_CloakedPredatorPolygon_Output(polyPtr,RenderPolygon.Vertices);
						}
						else D3D_CloakedPredatorPolygon_Output(polyPtr,VerticesBuffer);
					}
					#endif
				}
			}
			while(--numitems);
			return;
		}
	}
	else if (!Global_ODB_Ptr->ObMyModule)
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
		if (playerStatusPtr->cloakOn||playerStatusPtr->CloakingEffectiveness!=0)
		{
			int a = GetSin(CloakingPhase&4095);
			a = MUL_FIXED(a,a);
			CloakingMode = ONE_FIXED*5/4-playerStatusPtr->CloakingEffectiveness;//32768;
			do
			{
				POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
				int pif;
				pif = PolygonWithinFrustrum(polyPtr);
				if(pif)
				{		 
					switch(polyPtr->PolyItemType)
					{
						case I_ZB_Gouraud3dTexturedPolygon:
						case I_ZB_Gouraud2dTexturedPolygon:
						CloakedPolygon_Construct(polyPtr);
						if (pif!=2)
						{
							GouraudTexturedPolygon_ClipWithZ();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							D3D_ZBufferedCloakedPolygon_Output(polyPtr,RenderPolygon.Vertices);
						}
						else D3D_ZBufferedCloakedPolygon_Output(polyPtr,VerticesBuffer);
						break;
						default:
							textprint("found polygon of type %d\n",polyPtr->PolyItemType);
							break;
					}
				}
			}
			while(--numitems);
			return;
		}
	
	}
	#endif
	#if 0
//	if (Global_ODB_Ptr->ObStrategyBlock && !Global_ODB_Ptr->ObMyModule)
	{
		do
		{
			POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
			int pif;	

			pif = PolygonWithinFrustrum(polyPtr);
					 
			if (pif)
			{		 
				GouraudPolygon_Construct(polyPtr);

				if (pif!=2)
				{
					GouraudPolygon_ClipWithZ();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					D3D_ZBufferedGouraudPolygon_Output(polyPtr,RenderPolygon.Vertices);
	  			
	  			}
				else D3D_ZBufferedGouraudPolygon_Output(polyPtr,VerticesBuffer);
	 		}
		}
		while(--numitems);
		return;
	}
	#endif
	do
	{
		POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
		int pif;	
		#if 0
		if (objectCompletelyInView)
		{
			pif = PolygonShouldBeDrawn(polyPtr);
		}
		else
		{
			pif = PolygonWithinFrustrum(polyPtr);
		}
		#else
		pif = PolygonWithinFrustrum(polyPtr);
		#endif
				 
		if (pif)
		{		 
			switch(polyPtr->PolyItemType)
			{
				#if debug
				case I_Polyline:
				case I_FilledPolyline:
				case I_Wireframe:

				/* NB This is intended to fall through to the GouraudPolygon case */
				#endif
//				case I_Gouraud3dTexturedPolygon:
				case I_GouraudPolygon:
				case I_Gouraud2dTexturedPolygon:
				case I_Gouraud3dTexturedPolygon:
				case I_2dTexturedPolygon:
				case I_3dTexturedPolygon:
				case I_ZB_2dTexturedPolygon:
				case I_ZB_3dTexturedPolygon:
				{

				//	LOCALASSERT(0);
					break;
				}
				case I_ZB_GouraudPolygon:
				{
//					break;
  //				LOCALASSERT(0);
					GouraudPolygon_Construct(polyPtr);

					if (pif!=2)
					{
						GouraudPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						D3D_ZBufferedGouraudPolygon_Output(polyPtr,RenderPolygon.Vertices);
		  			
		  			}
					else D3D_ZBufferedGouraudPolygon_Output(polyPtr,VerticesBuffer);
					break;
				}
	 			case I_ZB_Gouraud3dTexturedPolygon:
				case I_ZB_Gouraud2dTexturedPolygon:
				{
					GouraudTexturedPolygon_Construct(polyPtr);
				   	if (pif!=2)
					{
						/* if this polygon is a quad, split it into two */
						if(RenderPolygon.NumberOfVertices==4)
						{
							RenderPolygon.NumberOfVertices=3;
							TriangleVerticesBuffer[0] = VerticesBuffer[0];
							TriangleVerticesBuffer[1] = VerticesBuffer[2];
							TriangleVerticesBuffer[2] = VerticesBuffer[3];

							GouraudTexturedPolygon_ClipWithZ();
							if(RenderPolygon.NumberOfVertices<3) goto SecondTriangle;
							GouraudTexturedPolygon_ClipWithNegativeX();
							if(RenderPolygon.NumberOfVertices<3) goto SecondTriangle;
							GouraudTexturedPolygon_ClipWithPositiveY();
							if(RenderPolygon.NumberOfVertices<3) goto SecondTriangle;
							GouraudTexturedPolygon_ClipWithNegativeY();
							if(RenderPolygon.NumberOfVertices<3) goto SecondTriangle;
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices<3) goto SecondTriangle;

							if (polyPtr->PolyFlags & iflag_transparent)
							{
								AddToTranslucentPolyList(polyPtr,RenderPolygon.Vertices);
							}
							else 
							{
								D3D_ZBufferedGouraudTexturedPolygon_Output(polyPtr,RenderPolygon.Vertices);
							}
							
							SecondTriangle:
							RenderPolygon.NumberOfVertices=3;
							VerticesBuffer[0] = TriangleVerticesBuffer[0];
							VerticesBuffer[1] = TriangleVerticesBuffer[1];
							VerticesBuffer[2] = TriangleVerticesBuffer[2];
						}
						GouraudTexturedPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) continue;

//						polyPtr->PolyFlags |= iflag_transparent;
						if (polyPtr->PolyFlags & iflag_transparent)
						{
							AddToTranslucentPolyList(polyPtr,RenderPolygon.Vertices);
						}
						else 
						{
							D3D_ZBufferedGouraudTexturedPolygon_Output(polyPtr,RenderPolygon.Vertices);
						}
		  			
		  			}
					else
					{
//						polyPtr->PolyFlags |= iflag_transparent;
						if (polyPtr->PolyFlags & iflag_transparent)
						{
							AddToTranslucentPolyList(polyPtr,VerticesBuffer);
						}
						else 
						{
							D3D_ZBufferedGouraudTexturedPolygon_Output(polyPtr,VerticesBuffer);
						}
					}
					break;
				}
   				default:
					break;
			}
 		}
	}
	while(--numitems);
}

void PredatorThermalVision_ShapePipeline(SHAPEHEADER *shapePtr)
{
	int numitems= shapePtr->numitems;
	int **itemArrayPtr = shapePtr->items;

	LOCALASSERT(numitems);
   
	TestVerticesWithFrustrum();
	do
	{
		POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
	
		int pif = PolygonWithinFrustrum(polyPtr);
				 
		if (pif)
		{		 
			PredatorThermalVisionPolygon_Construct(polyPtr);

			if (pif!=2)
			{
				GouraudPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) continue;
			
  				D3D_PredatorThermalVisionPolygon_Output(polyPtr,RenderPolygon.Vertices);
  			}
  			else D3D_PredatorThermalVisionPolygon_Output(polyPtr,VerticesBuffer);
		}
	}
	while(--numitems);
}
void PredatorSeeAliensVision_ShapePipeline(SHAPEHEADER *shapePtr)
{
	int numitems= shapePtr->numitems;
	int **itemArrayPtr = shapePtr->items;

	LOCALASSERT(numitems);
   
	TestVerticesWithFrustrum();
	do
	{
		POLYHEADER *polyPtr = (POLYHEADER*) (*itemArrayPtr++);
	
		switch (polyPtr->PolyItemType)
		{
	 		case I_ZB_Gouraud3dTexturedPolygon:
			case I_ZB_Gouraud2dTexturedPolygon:
			{
				int pif = PolygonWithinFrustrum(polyPtr);
						 
				if (pif)
				{		 
					PredatorSeeAliensVisionPolygon_Construct(polyPtr);

					#if 0
					if (pif!=2)
					{
						GouraudPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) continue;
					
		  				D3D_PredatorSeeAliensVisionPolygon_Output(polyPtr,RenderPolygon.Vertices);
		  			}
		  			else D3D_PredatorSeeAliensVisionPolygon_Output(polyPtr,VerticesBuffer);
					#else
					if (pif!=2)
					{
						GouraudTexturedPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) continue;
						GouraudTexturedPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) continue;
						D3D_ZBufferedGouraudTexturedPolygon_Output(polyPtr,RenderPolygon.Vertices);
					}
					else D3D_ZBufferedGouraudTexturedPolygon_Output(polyPtr,VerticesBuffer);
					#endif
				}
				break;
			}
			default:
				break;
		}
	}
	while(--numitems);
}


/* CLOAKED POLYGONS */
static void CloakedPolygon_Construct(POLYHEADER *polyPtr)
{
	int *texture_defn_ptr;
	RENDERVERTEX *renderVerticesPtr = VerticesBuffer;
	int i = RenderPolygon.NumberOfVertices;

	/* get ptr to uv coords for this polygon */
	{
		int texture_defn_index = (polyPtr->PolyColour >> TxDefn);
		texture_defn_ptr = Global_ShapeTextures[texture_defn_index];
	}
	
	VertexNumberPtr = &polyPtr->Poly1stPt;

	/* If this texture is animated the UV array must be calculated */
	if(polyPtr->PolyFlags & iflag_txanim)
	{
		/* Create the UV array */
		int uv_array[maxpolypts * 2];
		CreateTxAnimUVArray(texture_defn_ptr, uv_array, (int*)polyPtr);
		texture_defn_ptr = uv_array;

		do
		{
			VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
			renderVerticesPtr->X = vertexPtr->vx;
			renderVerticesPtr->Y = vertexPtr->vy;
			renderVerticesPtr->Z = vertexPtr->vz;
			renderVerticesPtr->U = texture_defn_ptr[0];
			renderVerticesPtr->V = texture_defn_ptr[1];

			VertexIntensity(renderVerticesPtr);
			{
				VECTORCH mag;
				int alpha;
				mag.vx = vertexPtr->vx - Global_ODB_Ptr->ObView.vx;
				mag.vy = vertexPtr->vy - Global_ODB_Ptr->ObView.vy;
				mag.vz = vertexPtr->vz - Global_ODB_Ptr->ObView.vz;
				

				if (mag.vx<0) mag.vx = -mag.vx;
				if (mag.vy<0) mag.vy = -mag.vy;
				if (mag.vz<0) mag.vz = -mag.vz;
				alpha = GetSin(((mag.vx+mag.vy+mag.vz)*3+CloakingPhase)&4095);
				
				renderVerticesPtr->A = MUL_FIXED(alpha,alpha)>>10;
				
				if(renderVerticesPtr->A==255)
				{
					renderVerticesPtr->R = 255;
					renderVerticesPtr->G = 255;
					renderVerticesPtr->B = 255;
				}	

			}
			renderVerticesPtr++;
			VertexNumberPtr++;

			texture_defn_ptr += 2;
		}
	    while(--i);
	}
	else
	{
		do
		{
			VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
			renderVerticesPtr->X = vertexPtr->vx;
			renderVerticesPtr->Y = vertexPtr->vy;
			renderVerticesPtr->Z = vertexPtr->vz;
			renderVerticesPtr->U = texture_defn_ptr[0] << 16;
			renderVerticesPtr->V = texture_defn_ptr[1] << 16;

			VertexIntensity(renderVerticesPtr);
			{
				VECTORCH mag;
				int alpha;

				mag.vx = vertexPtr->vx - ObjectCentre.vx;
				mag.vy = vertexPtr->vy - MUL_FIXED(ObjectCentre.vy,87381);
				mag.vz = vertexPtr->vz - ObjectCentre.vz;

				if (mag.vx<0) mag.vx = -mag.vx;
				if (mag.vy<0) mag.vy = -mag.vy;
				if (mag.vz<0) mag.vz = -mag.vz;
				alpha = GetSin(((mag.vx+mag.vy+mag.vz)*8+CloakingPhase)&4095);
	
				alpha=MUL_FIXED(alpha,alpha);
				if (alpha>CloakingMode) 
				{
					alpha=CloakingMode;
				}
				alpha/=256;
				if (alpha>255) alpha = 255;
				renderVerticesPtr->A = alpha;

				if(CloakingMode>ONE_FIXED)
				{
					alpha = GetSin(((mag.vx+mag.vy+mag.vz)+CloakingPhase)&4095);
					alpha = MUL_FIXED(alpha,alpha)>>8;
					if(alpha==255)
					{
						renderVerticesPtr->A = 255;
						renderVerticesPtr->G = 128;
						renderVerticesPtr->B = 255;
					}
				}	
			}
			renderVerticesPtr++;
			VertexNumberPtr++;

			texture_defn_ptr += 2;
		}
	    while(--i);
	}

}

static void PredatorThermalVisionPolygon_Construct(POLYHEADER *polyPtr)
{
	RENDERVERTEX *renderVerticesPtr = VerticesBuffer;
	int i = RenderPolygon.NumberOfVertices;

	VertexNumberPtr = &polyPtr->Poly1stPt;

	do
	{
		VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
		renderVerticesPtr->X = vertexPtr->vx;
		renderVerticesPtr->Y = vertexPtr->vy;
		renderVerticesPtr->Z = vertexPtr->vz;

		{
			int alpha;
			if (Global_ODB_Ptr->SpecialFXFlags&SFXFLAG_ISAFFECTEDBYHEAT)
			{
				int distanceFromHeatSource = 100000;
				int sourceNumber=NumberOfHeatSources;
				while(sourceNumber--)
				{
					VECTORCH mag;
					int m;
					mag.vx = vertexPtr->vx - HeatSourceList[sourceNumber].Position.vx;
					mag.vy = vertexPtr->vy - HeatSourceList[sourceNumber].Position.vy;
					mag.vz = vertexPtr->vz - HeatSourceList[sourceNumber].Position.vz;

					m = Approximate3dMagnitude(&mag)*64;
					
					if(m<distanceFromHeatSource) distanceFromHeatSource = m;
				}
				
				alpha = distanceFromHeatSource+(GetSin(CloakingPhase&4095)>>3);
				if (alpha>65536) alpha = 65536;
			}
			else
			{
				alpha = 65536;
			}

			{
				int brightness = MUL_FIXED(MUL_FIXED(alpha,alpha),1275);

				if (brightness<256)
				{
					renderVerticesPtr->R=255;
					renderVerticesPtr->G=brightness;
					renderVerticesPtr->B=0;
				}
				else if (brightness<255+256)
				{
					int b=brightness-255;
					renderVerticesPtr->R=(255-b);
					renderVerticesPtr->G=255;
					renderVerticesPtr->B=0;
				}
				else if (brightness<255*2+256)
				{
					int b=brightness-255*2;
					renderVerticesPtr->R=0;
					renderVerticesPtr->G=255;
					renderVerticesPtr->B=b;
				}
				else if (brightness<255*3+256)
				{
					int b=brightness-255*3;
					renderVerticesPtr->R=0;
					renderVerticesPtr->G=255-b;
					renderVerticesPtr->B=255;
				}
				else
				{
					int b=brightness-255*4;
					renderVerticesPtr->R=0;
					renderVerticesPtr->G=0;
					renderVerticesPtr->B=255-b/2;
				}
		 	}
		}
		renderVerticesPtr++;
		VertexNumberPtr++;
	}
    while(--i);
}

static void PredatorSeeAliensVisionPolygon_Construct(POLYHEADER *polyPtr)
{
	int *texture_defn_ptr;
	RENDERVERTEX *renderVerticesPtr = VerticesBuffer;
	int i = RenderPolygon.NumberOfVertices;
	int alpha;

	VertexNumberPtr = &polyPtr->Poly1stPt;

	{
		{
			int texture_defn_index = (polyPtr->PolyColour >> TxDefn);
			texture_defn_ptr = Global_ShapeTextures[texture_defn_index];
		}

		/* get ptr to uv coords for this polygon */
		if(polyPtr->PolyFlags & iflag_txanim)
		{
			/* Create the UV array */
			int uv_array[maxpolypts * 2];
			CreateTxAnimUVArray(texture_defn_ptr, uv_array, (int*)polyPtr);
			texture_defn_ptr = uv_array;
		}

		if( (Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)
		  &&(Global_ODB_Ptr->ObFlags2 < ONE_FIXED) )	
		{
			alpha = Global_ODB_Ptr->ObFlags2 >> 8;
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
		}
		else
		{
			alpha = 255;
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;
		}
		
		do
		{
			VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
		
			if(polyPtr->PolyFlags & iflag_txanim)
			{
				renderVerticesPtr->U = texture_defn_ptr[0];
				renderVerticesPtr->V = texture_defn_ptr[1];
			}
			else
			{
				renderVerticesPtr->U = texture_defn_ptr[0] << 16;
				renderVerticesPtr->V = texture_defn_ptr[1] << 16;
			}
			
		
			renderVerticesPtr->X = vertexPtr->vx;
			renderVerticesPtr->Y = vertexPtr->vy;
			renderVerticesPtr->Z = vertexPtr->vz;
			{
				VECTORCH mag = RotatedPts[*VertexNumberPtr];//*(((VECTORCH *)Global_ShapeVNormals) + *VertexNumberPtr);
				int colour;
				mag.vx = vertexPtr->vx - Global_ODB_Ptr->ObView.vx;
				mag.vy = vertexPtr->vy - Global_ODB_Ptr->ObView.vy;
				mag.vz = vertexPtr->vz - Global_ODB_Ptr->ObView.vz;
				
				colour = GetSin(((mag.vx+mag.vy+mag.vz)*8+CloakingPhase)&4095);
				colour = MUL_FIXED(colour,colour);
				renderVerticesPtr->B = MUL_FIXED(colour,255);
				renderVerticesPtr->R = renderVerticesPtr->B/2;
				renderVerticesPtr->G = renderVerticesPtr->B/2;

				colour = MUL_FIXED(colour,colour);
				colour = MUL_FIXED(colour,colour);

				renderVerticesPtr->SpecularR = colour/1024;
				renderVerticesPtr->SpecularG = colour/1024;
				renderVerticesPtr->SpecularB = colour/1024;
				renderVerticesPtr->A = alpha;
			}

			texture_defn_ptr += 2;
			renderVerticesPtr++;
			VertexNumberPtr++;
		}
	    while(--i);
	}
}

/* GOURAUD POLYGONS */
static void GouraudPolygon_Construct(POLYHEADER *polyPtr)
{
	RENDERVERTEX *renderVerticesPtr = VerticesBuffer;
	int i = RenderPolygon.NumberOfVertices;
	VertexNumberPtr = &polyPtr->Poly1stPt;
	RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;

	do
	{
		int i;
		renderVerticesPtr->X = RotatedPts[*VertexNumberPtr].vx;
		renderVerticesPtr->Y = RotatedPts[*VertexNumberPtr].vy;
		renderVerticesPtr->Z = RotatedPts[*VertexNumberPtr].vz;
		VertexIntensity(renderVerticesPtr);
		i = (renderVerticesPtr->B+renderVerticesPtr->R+renderVerticesPtr->G)/3;
		renderVerticesPtr->R = i;
		renderVerticesPtr->G = i;
		renderVerticesPtr->B = 0;
		renderVerticesPtr++;
		VertexNumberPtr++;
	}
    while(--i);

}






/* GOURAUD TEXTURED POLYGONS */
static void GouraudTexturedPolygon_Construct(POLYHEADER *polyPtr)
{
	int *texture_defn_ptr;
	RENDERVERTEX *renderVerticesPtr = VerticesBuffer;
	int i = RenderPolygon.NumberOfVertices;


	/* get ptr to uv coords for this polygon */
	{
		int texture_defn_index = (polyPtr->PolyColour >> TxDefn);
		texture_defn_ptr = Global_ShapeTextures[texture_defn_index];
	}
		
	VertexNumberPtr = &polyPtr->Poly1stPt;

	/* If this texture is animated the UV array must be calculated */
	if(polyPtr->PolyFlags & iflag_txanim)
	{
		/* Create the UV array */
		int uv_array[maxpolypts * 2];
		CreateTxAnimUVArray(texture_defn_ptr, uv_array, (int*)polyPtr);
		texture_defn_ptr = uv_array;

		do
		{
			VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
			if (TRIPTASTIC_CHEATMODE)
			{
				renderVerticesPtr->X = vertexPtr->vx+GetSin((CloakingPhase*2	+vertexPtr->vz)&4095)/1024;
				renderVerticesPtr->Y = vertexPtr->vy+GetSin((CloakingPhase-3000	+vertexPtr->vx)&4095)/1024;
				renderVerticesPtr->Z = vertexPtr->vz+GetSin((CloakingPhase*3+239+vertexPtr->vy)&4095)/1024;
			}
			else if (UNDERWATER_CHEATMODE)
			{
				renderVerticesPtr->X = vertexPtr->vx+(GetSin((CloakingPhase/2	+vertexPtr->vz)&4095))/1024;
				renderVerticesPtr->Y = vertexPtr->vy+(GetSin((CloakingPhase-3000+vertexPtr->vx)&4095))/1024;
				renderVerticesPtr->Z = vertexPtr->vz+(GetSin((CloakingPhase/3+239+vertexPtr->vy)&4095))/1024;
			}
			else
			{
				renderVerticesPtr->X = vertexPtr->vx;
				renderVerticesPtr->Y = vertexPtr->vy;
				renderVerticesPtr->Z = vertexPtr->vz;
			}
			renderVerticesPtr->U = texture_defn_ptr[0];
			renderVerticesPtr->V = texture_defn_ptr[1];

			if( (Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)
			  &&(Global_ODB_Ptr->ObFlags2 < ONE_FIXED) )	
			{
				renderVerticesPtr->A = Global_ODB_Ptr->ObFlags2 >> 8;
				RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;

			}
			else if (polyPtr->PolyFlags & iflag_transparent)
			{
				renderVerticesPtr->A = 128;
				RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
			}
			else
			{
				if (TRIPTASTIC_CHEATMODE)
				{
					renderVerticesPtr->A = TripTasticPhase;
				}
				else if (MOTIONBLUR_CHEATMODE)
				{
					renderVerticesPtr->A = 128;
				}
				else
				{
					renderVerticesPtr->A = 255;
				}
				RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;
			}

	 		if (polyPtr->PolyFlags & iflag_nolight)
			{
				switch (CurrentVisionMode)
				{
					default:
					case VISION_MODE_NORMAL:
					{
						renderVerticesPtr->R = 255;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 255;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
						break;
					}
					case VISION_MODE_IMAGEINTENSIFIER:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
						break;
					}
					case VISION_MODE_PRED_THERMAL:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 0;
						renderVerticesPtr->B = 255;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
					  	break;
					}
					case VISION_MODE_PRED_SEEALIENS:
					{
						renderVerticesPtr->R = 255;
						renderVerticesPtr->G = 0;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
					  	break;
					}
					case VISION_MODE_PRED_SEEPREDTECH:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 255;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 255;
					  	break;
					}
				}
				
			}
			else
			{
				VertexIntensity(renderVerticesPtr);
			}
			renderVerticesPtr++;
			VertexNumberPtr++;

			texture_defn_ptr += 2;
		}
	    while(--i);
	}
	else
	{
		do
		{
			VECTORCH *vertexPtr = &(RotatedPts[*VertexNumberPtr]);
			#if UNDERWATER
			renderVerticesPtr->X = vertexPtr->vx+GetSin((CloakingPhase*2	+vertexPtr->vz)&4095)/1024;
			renderVerticesPtr->Y = vertexPtr->vy+GetSin((CloakingPhase-3000	+vertexPtr->vx)&4095)/1024;
			renderVerticesPtr->Z = vertexPtr->vz+GetSin((CloakingPhase*3+239+vertexPtr->vy)&4095)/1024;
			#elif SPATIAL_SHOCKWAVE
			{
				int d = Magnitude(vertexPtr);
				int a = (CloakingPhase&16383)+4000;
				int u = d-a;
				int offset;
				
				if (u>0 && u<8192)
				{
					VECTORCH n = *vertexPtr;
					Normalise(&n);
					u<<=3;
					offset = MUL_FIXED(MUL_FIXED(2*u,ONE_FIXED-u),8000) + MUL_FIXED(MUL_FIXED(u,u),8192 );
					LOCALASSERT(offset>=0 && offset<=8192);
					renderVerticesPtr->X = MUL_FIXED(n.vx,d);//a+offset*2);
					renderVerticesPtr->Y = MUL_FIXED(n.vy,d);//a+offset*2);
					renderVerticesPtr->Z = MUL_FIXED(n.vz,a+offset);

				}
				else
				{
					renderVerticesPtr->X = vertexPtr->vx;
					renderVerticesPtr->Y = vertexPtr->vy;
					renderVerticesPtr->Z = vertexPtr->vz;
				}
				
			}
			#else
			if (TRIPTASTIC_CHEATMODE)
			{
				renderVerticesPtr->X = vertexPtr->vx+GetSin((CloakingPhase*2	+vertexPtr->vz)&4095)/1024;
				renderVerticesPtr->Y = vertexPtr->vy+GetSin((CloakingPhase-3000	+vertexPtr->vx)&4095)/1024;
				renderVerticesPtr->Z = vertexPtr->vz+GetSin((CloakingPhase*3+239+vertexPtr->vy)&4095)/1024;
			}
			else if (UNDERWATER_CHEATMODE)
			{
				renderVerticesPtr->X = vertexPtr->vx+(GetSin((CloakingPhase/2	+vertexPtr->vz)&4095))/1024;
				renderVerticesPtr->Y = vertexPtr->vy+(GetSin((CloakingPhase-3000	+vertexPtr->vx)&4095))/1024;
				renderVerticesPtr->Z = vertexPtr->vz+(GetSin((CloakingPhase/3+239+vertexPtr->vy)&4095))/1024;
			}
			else
			{
				renderVerticesPtr->X = vertexPtr->vx;
				renderVerticesPtr->Y = vertexPtr->vy;
				renderVerticesPtr->Z = vertexPtr->vz;
			}
			#endif															
			renderVerticesPtr->U = texture_defn_ptr[0] << 16;
			renderVerticesPtr->V = texture_defn_ptr[1] << 16;

			if( (Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)
			  &&(Global_ODB_Ptr->ObFlags2 < ONE_FIXED) )	
			{
				renderVerticesPtr->A = Global_ODB_Ptr->ObFlags2 >> 8;
				RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;

			}
			else if (polyPtr->PolyFlags & iflag_transparent)
			{
				renderVerticesPtr->A = 128;
				RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
			}
			else
			{
				#if 0
				VECTORCH velocity;
				int a;
				velocity.vx = Player->ObStrategyBlock->DynPtr->Position.vx - Player->ObStrategyBlock->DynPtr->PrevPosition.vx;
				velocity.vy = Player->ObStrategyBlock->DynPtr->Position.vy - Player->ObStrategyBlock->DynPtr->PrevPosition.vy;
				velocity.vz = Player->ObStrategyBlock->DynPtr->Position.vz - Player->ObStrategyBlock->DynPtr->PrevPosition.vz;
				a = DIV_FIXED(Magnitude(&velocity)*4,NormalFrameTime)/256;
				if (a>192) a = 192;
				renderVerticesPtr->A = a;
				
				#elif 1
				if (TRIPTASTIC_CHEATMODE)
				{
					renderVerticesPtr->A = TripTasticPhase;
				}
				else if (MOTIONBLUR_CHEATMODE)
				{
					renderVerticesPtr->A = 128;
				}
				else
				{
					renderVerticesPtr->A = 255;
				}
				#endif

				RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;
			}

	
	 		if (polyPtr->PolyFlags & iflag_nolight)
			{
				switch (CurrentVisionMode)
				{
					default:
					case VISION_MODE_NORMAL:
					{
						renderVerticesPtr->R = 255;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 255;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
						break;
					}
					case VISION_MODE_IMAGEINTENSIFIER:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
						break;
					}
					case VISION_MODE_PRED_THERMAL:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 0;
						renderVerticesPtr->B = 255;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
					  	break;
					}
					case VISION_MODE_PRED_SEEALIENS:
					{
						renderVerticesPtr->R = 255;
						renderVerticesPtr->G = 0;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 0;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 0;
					  	break;
					}
					case VISION_MODE_PRED_SEEPREDTECH:
					{
						renderVerticesPtr->R = 0;
						renderVerticesPtr->G = 255;
						renderVerticesPtr->B = 0;
						renderVerticesPtr->SpecularR = 255;
						renderVerticesPtr->SpecularG = 0;
						renderVerticesPtr->SpecularB = 255;
					  	break;
					}
				}
				
			}
			else
			{
		 		VertexIntensity(renderVerticesPtr);
			}
			renderVerticesPtr++;
			VertexNumberPtr++;

			texture_defn_ptr += 2;
		}
	    while(--i);
	}

}





static void VertexIntensity_Pred_Thermal(RENDERVERTEX *renderVertexPtr)
{
	int redI,blueI,specular=0;

	int vertexNumber = *VertexNumberPtr;


	
	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	{
		VECTORCH *vertexNormalPtr = ((VECTORCH *)Global_ShapeVNormals) + vertexNumber;
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;
		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		redI = 0;

		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			if (lptr->LightFlags & LFlag_PreLitSource) continue;

			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;

			#if 0
			distanceToLight = Approximate3dMagnitude(&vertexToLight)/2;
			#else
			distanceToLight = Approximate3dMagnitude(&vertexToLight);
			#endif														  				
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);

				if( (distanceToLight>0) && (!(Global_ODB_Ptr->ObFlags3 & ObFlag3_NoLightDot)) )
				{
				 	int dotproduct = MUL_FIXED(vertexNormalPtr->vx,vertexToLight.vx)
					     + MUL_FIXED(vertexNormalPtr->vy,vertexToLight.vy)
					     + MUL_FIXED(vertexNormalPtr->vz,vertexToLight.vz);
					if(dotproduct>0)
					{ 
						idot = WideMulNarrowDiv(idot,dotproduct,distanceToLight);
					}
					else
					{
						idot = 0;
					}
	
					idot = WideMulNarrowDiv(idot,dotproduct,distanceToLight);
				}

		  				
		  		redI += idot;			
				if (lptr->LightFlags&LFlag_Thermal)
				{
					specular += idot;
				}
			}
	  	}
	}
	blueI = ONE_FIXED/2;
	if (renderVertexPtr->Z>5000)
	{
		int a = (renderVertexPtr->Z-5000);	
		if (a>4096) blueI = (blueI*4096)/a;
	}

	blueI >>= 8;

 	if(redI >= ONE_FIXED)	redI = (ONE_FIXED - 1);
	redI >>=8;

	specular>>=6;
	if (specular >= 255) specular = 255;

	/* KJL 12:41:54 05/10/98 - red/green swapped, whilst testing colours */
	renderVertexPtr->R = 0;
	ColourIntensityArray[vertexNumber].R = 0;
	renderVertexPtr->B = blueI;
	ColourIntensityArray[vertexNumber].B = blueI;
	renderVertexPtr->G = redI/2;
	ColourIntensityArray[vertexNumber].G = redI/2;
	

	renderVertexPtr->SpecularR = specular;//specularR;
	ColourIntensityArray[vertexNumber].SpecularR = specular;//specularR;
	
	renderVertexPtr->SpecularG = specular;
	ColourIntensityArray[vertexNumber].SpecularG = specular;
		
	renderVertexPtr->SpecularB = specular;//specularB;
	ColourIntensityArray[vertexNumber].SpecularB = specular;//specularB;
	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
}
static void VertexIntensity_Pred_SeeAliens(RENDERVERTEX *renderVertexPtr)
{
	int redI,blueI,specular=0;

	int vertexNumber = *VertexNumberPtr;


	
	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
	
	{
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;
		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		redI = 0;

		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			if (lptr->LightFlags & LFlag_PreLitSource) continue;

			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;

			distanceToLight = Approximate3dMagnitude(&vertexToLight);
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);

		  		redI += idot;			
			 	if(lptr->LightFlags&LFlag_Electrical)
				{
					specular += idot;
			 	}
			}
	  	}
	}
	redI >>=11;
 	if(redI > 255) redI = 255;
	renderVertexPtr->G = redI;
	ColourIntensityArray[vertexNumber].G = redI;
	
	
	blueI = ONE_FIXED/2;
	if (renderVertexPtr->Z>5000)
	{
		int a = (renderVertexPtr->Z-5000);	
		if (a>4096) blueI = (blueI*4096)/a;
	}
	/* KJL 12:41:54 05/10/98 - red/green swapped, whilst testing colours */
	blueI >>= 9;
	renderVertexPtr->R = blueI;
	ColourIntensityArray[vertexNumber].R = blueI;
	renderVertexPtr->B = 0;
	ColourIntensityArray[vertexNumber].B = 0;

	specular >>=10;
 	if(specular>255) specular = 255;
	renderVertexPtr->SpecularR = specular;//specularR;
	ColourIntensityArray[vertexNumber].SpecularR = specular;//specularR;
	renderVertexPtr->SpecularG = specular;
	ColourIntensityArray[vertexNumber].SpecularG = specular;
	renderVertexPtr->SpecularB = specular;//specularB;
	ColourIntensityArray[vertexNumber].SpecularB = specular;//specularB;
}
static void VertexIntensity_Pred_SeePredatorTech(RENDERVERTEX *renderVertexPtr)
{
	int redI,blueI;

	int vertexNumber = *VertexNumberPtr;


	
	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
	{
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;
		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		redI = 0;

		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			if (lptr->LightFlags & LFlag_PreLitSource) continue;

			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;

			distanceToLight = Approximate3dMagnitude(&vertexToLight);
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);

		  		redI += idot;			
			}
	  	}
	}
	blueI = ONE_FIXED-1;
	if (renderVertexPtr->Z>5000)
	{
		int a = (renderVertexPtr->Z-5000);	
		if (a>4096) blueI = (blueI*4096)/a;
	}

	blueI >>=8;

	redI >>=9;
	if (redI>255) redI=255;

	/* KJL 12:41:54 05/10/98 - red/green swapped, whilst testing colours */
	renderVertexPtr->R = 255;
	ColourIntensityArray[vertexNumber].R = 255;
	renderVertexPtr->B = 255;
	ColourIntensityArray[vertexNumber].B = 255;
	renderVertexPtr->G = blueI;
	ColourIntensityArray[vertexNumber].G = blueI;
	

	renderVertexPtr->SpecularR = 255;//specularR;
	ColourIntensityArray[vertexNumber].SpecularR = 255;//specularR;
	
	renderVertexPtr->SpecularG = redI;
	ColourIntensityArray[vertexNumber].SpecularG = redI;
		
	renderVertexPtr->SpecularB = 255;//specularB;
	ColourIntensityArray[vertexNumber].SpecularB = 255;//specularB;
	
}
static void VertexIntensity_ImageIntensifier(RENDERVERTEX *renderVertexPtr)
{
	int greenI;
	int specular;

	int vertexNumber = *VertexNumberPtr;

	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;

	{
		VECTORCH *vertexNormalPtr = ((VECTORCH *)Global_ShapeVNormals) + vertexNumber;
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;

		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		greenI = 0;			
		specular = 0;

  		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;
			{
				int dx,dy,dz;

				dx = vertexToLight.vx;
				if (dx<0) dx = -dx;
					 
				dy = vertexToLight.vy;
				if (dy<0) dy = -dy;
				
				dz = vertexToLight.vz;	 	 
				if (dz<0) dz = -dz;
									 
				
				if (dx>dy)
				{
					if (dx>dz)
					{
						distanceToLight = dx + ((dy+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dy+dx)>>2);
					}
				}
				else
				{
					if (dy>dz)
					{
						distanceToLight = dy + ((dx+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dx+dy)>>2);
					}
				}
			}
																	  				
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);

				if(distanceToLight>0)
				{
				 	int dotproduct = MUL_FIXED(vertexNormalPtr->vx,vertexToLight.vx)
					     + MUL_FIXED(vertexNormalPtr->vy,vertexToLight.vy)
					     + MUL_FIXED(vertexNormalPtr->vz,vertexToLight.vz);

					if(dotproduct>0)
					{ 
						idot = (WideMulNarrowDiv(idot,dotproduct,distanceToLight)+idot/4);
					}
					else
					{
						idot /= 4;
					}
				}
				if(idot<0)
				{
					LOCALASSERT(idot>=0);
				}
				specular += idot;			
			}
	  	}
	}

	greenI = 255;
	if (renderVertexPtr->Z>5000)
	{
		int a = (renderVertexPtr->Z-5000);	
		if (a>4096) greenI = (greenI*4096)/a;
	}
	renderVertexPtr->G = greenI;
	ColourIntensityArray[vertexNumber].G = greenI;

	renderVertexPtr->R = 0;
	ColourIntensityArray[vertexNumber].R = 0;
	renderVertexPtr->B = 0;
	ColourIntensityArray[vertexNumber].B = 0;

 	specular>>=7;
 	if (specular>254) specular=254;
	LOCALASSERT(specular>=0 && specular<=254); 
	renderVertexPtr->SpecularR = specular;
	ColourIntensityArray[vertexNumber].SpecularR = specular;
	renderVertexPtr->SpecularG = specular;
	ColourIntensityArray[vertexNumber].SpecularG = specular;
	renderVertexPtr->SpecularB = specular;
	ColourIntensityArray[vertexNumber].SpecularB = specular;

}

static void VertexIntensity_Alien_Sense(RENDERVERTEX *renderVertexPtr)
{
	int intensity;
	int vertexNumber = *VertexNumberPtr;

	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = 0;
		renderVertexPtr->SpecularG = 0;
		renderVertexPtr->SpecularB = 0;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;

	intensity = 255;
	if (renderVertexPtr->Z>5000)
	{
		int a = (renderVertexPtr->Z-5000);	
		if (a>1024) intensity = (intensity*1024)/a;
	}

	renderVertexPtr->R = intensity;
	ColourIntensityArray[vertexNumber].R = intensity;
	
	renderVertexPtr->G = intensity;
	ColourIntensityArray[vertexNumber].G = intensity;
	
	renderVertexPtr->B = intensity;
	ColourIntensityArray[vertexNumber].B = intensity;

	renderVertexPtr->SpecularR = 0;
	renderVertexPtr->SpecularG = 0;
	renderVertexPtr->SpecularB = 0; 
	
}



static void VertexIntensity_Standard_Opt(RENDERVERTEX *renderVertexPtr)
{
	int redI,greenI,blueI;
	int specularR,specularG,specularB;

	int vertexNumber = *VertexNumberPtr;

	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
	{
		VECTORCH *vertexNormalPtr = ((VECTORCH *)Global_ShapeVNormals) + vertexNumber;
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;

		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_PreLit) 
		{
			unsigned int packedI = Global_EID_IPtr[vertexNumber];
			blueI = (packedI&255)*257;

			packedI >>=8;
			greenI = (packedI&255)*257;

			packedI >>=8;
			redI = (packedI&255)*257;
		}
		else 
		{
			redI = 0;			
			greenI = 0;			
			blueI = 0;
		}

		specularR = 0;
		specularG = 0;
		specularB = 0;



		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;
			{
				int dx,dy,dz;

				dx = vertexToLight.vx;
				if (dx<0) dx = -dx;
					 
				dy = vertexToLight.vy;
				if (dy<0) dy = -dy;
				
				dz = vertexToLight.vz;	 	 
				if (dz<0) dz = -dz;
									 
				
				if (dx>dy)
				{
					if (dx>dz)
					{
						distanceToLight = dx + ((dy+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dy+dx)>>2);
					}
				}
				else
				{
					if (dy>dz)
					{
						distanceToLight = dy + ((dx+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dx+dy)>>2);
					}
				}
			}
																	  				
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);
				int r,g,b;

				if(distanceToLight>0)
				{
				 	int dotproduct = MUL_FIXED(vertexNormalPtr->vx,vertexToLight.vx)
					     + MUL_FIXED(vertexNormalPtr->vy,vertexToLight.vy)
					     + MUL_FIXED(vertexNormalPtr->vz,vertexToLight.vz);

					if(dotproduct>0)
					{ 
						idot = (WideMulNarrowDiv(idot,dotproduct,distanceToLight)+idot/4)/2;
					}
					else
					{
						idot /= 8;
					}
				}
			
				r = MUL_FIXED(idot,lptr->RedScale);			
				g = MUL_FIXED(idot,lptr->GreenScale);			
				b = MUL_FIXED(idot,lptr->BlueScale);			

				redI += r;			
				greenI += g;			
				blueI += b;			
			
				if( !(lptr->LightFlags & LFlag_PreLitSource)
				 && !(lptr->LightFlags & LFlag_NoSpecular) )
				{
					specularR += r;			
					specularG += g;			
					specularB += b;			
				}
			}
	  	}
	}

	if(Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_ONFIRE)
	{
		specularR>>=2;
		specularG>>=2;
		specularB>>=2;

		redI>>=1;
		greenI>>=1;
		blueI>>=1;
	}

	/* Intensity for Textures */
	redI >>= 8;
	if(redI > 255) redI = 255;
	renderVertexPtr->R = redI;
	ColourIntensityArray[vertexNumber].R = redI;
	
	greenI >>= 8;
	if(greenI > 255) greenI = 255;
	renderVertexPtr->G = greenI;
	ColourIntensityArray[vertexNumber].G = greenI;
	
	blueI >>= 8;
	if(blueI > 255) blueI = 255;
	renderVertexPtr->B = blueI;
	ColourIntensityArray[vertexNumber].B = blueI;

	specularR >>= 10;
	if(specularR > 255) specularR = 255;		 
	renderVertexPtr->SpecularR = specularR;
	ColourIntensityArray[vertexNumber].SpecularR = specularR;
	
	specularG >>= 10;
	if(specularG > 255) specularG = 255;
	renderVertexPtr->SpecularG = specularG;
	ColourIntensityArray[vertexNumber].SpecularG = specularG;
		
	specularB >>= 10;
	if(specularB > 255) specularB = 255;
	renderVertexPtr->SpecularB = specularB;
	ColourIntensityArray[vertexNumber].SpecularB = specularB;
	
}
static void VertexIntensity_FullBright(RENDERVERTEX *renderVertexPtr)
{
	int vertexNumber = *VertexNumberPtr;
	renderVertexPtr->R = 255;
	ColourIntensityArray[vertexNumber].R = 255;
	renderVertexPtr->G = 255;
	ColourIntensityArray[vertexNumber].G = 255;
	renderVertexPtr->B = 255;
	ColourIntensityArray[vertexNumber].B = 255;

	renderVertexPtr->SpecularR = 0;
	ColourIntensityArray[vertexNumber].SpecularR = 0;
	renderVertexPtr->SpecularG = 0;
	ColourIntensityArray[vertexNumber].SpecularG = 0;
	renderVertexPtr->SpecularB = 0;
	ColourIntensityArray[vertexNumber].SpecularB = 0;
}

static void VertexIntensity_DiscoInferno(RENDERVERTEX *renderVertexPtr)
{
	int redI,greenI,blueI;
	int specularR,specularG,specularB;

	int vertexNumber = *VertexNumberPtr;

	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
	{
		VECTORCH *vertexNormalPtr = ((VECTORCH *)Global_ShapeVNormals) + vertexNumber;
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;

		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_PreLit) 
		{
			unsigned int packedI = Global_EID_IPtr[vertexNumber];
			blueI = (packedI&255)*257;

			packedI >>=8;
			greenI = (packedI&255)*257;

			packedI >>=8;
			redI = (packedI&255)*257;
		}
		else 
		{
			redI = 0;			
			greenI = 0;			
			blueI = 0;
		}

		specularR = 0;
		specularG = 0;
		specularB = 0;



		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;
			{
				int dx,dy,dz;

				dx = vertexToLight.vx;
				if (dx<0) dx = -dx;
					 
				dy = vertexToLight.vy;
				if (dy<0) dy = -dy;
				
				dz = vertexToLight.vz;	 	 
				if (dz<0) dz = -dz;
									 
				
				if (dx>dy)
				{
					if (dx>dz)
					{
						distanceToLight = dx + ((dy+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dy+dx)>>2);
					}
				}
				else
				{
					if (dy>dz)
					{
						distanceToLight = dy + ((dx+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dx+dy)>>2);
					}
				}
			}
																	  				
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);
				int r,g,b;

				if(distanceToLight>0)
				{
				 	int dotproduct = MUL_FIXED(vertexNormalPtr->vx,vertexToLight.vx)
					     + MUL_FIXED(vertexNormalPtr->vy,vertexToLight.vy)
					     + MUL_FIXED(vertexNormalPtr->vz,vertexToLight.vz);

					if(dotproduct>0)
					{ 
						idot = (WideMulNarrowDiv(idot,dotproduct,distanceToLight)+idot/4)/2;
					}
					else
					{
						idot /= 8;
					}
				}
			
				r = MUL_FIXED(idot,lptr->RedScale);			
				g = MUL_FIXED(idot,lptr->GreenScale);			
				b = MUL_FIXED(idot,lptr->BlueScale);			

				redI += r;			
				greenI += g;			
				blueI += b;			
			
				if( !(lptr->LightFlags & LFlag_PreLitSource)
				 && !(lptr->LightFlags & LFlag_NoSpecular) )
				{
					specularR += r;			
					specularG += g;			
					specularB += b;			
				}
			}
	  	}
	}

	if(Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_ONFIRE)
	{
		specularR>>=2;
		specularG>>=2;
		specularB>>=2;

		redI>>=1;
		greenI>>=1;
		blueI>>=1;
	}

	{
		int i = (redI+greenI+blueI);
		int si = (specularR+specularG+specularB);

		VECTORCH vertex = *(((VECTORCH *)Global_ShapePoints)+vertexNumber);
		int r,g,b;
		vertex.vx += Global_ODB_Ptr->ObWorld.vx;
		vertex.vy += Global_ODB_Ptr->ObWorld.vy;
		vertex.vz += Global_ODB_Ptr->ObWorld.vz;

		r = GetSin((vertex.vx+CloakingPhase)&4095);
		r = MUL_FIXED(r,r);
		redI = MUL_FIXED(r,i);
		specularR = MUL_FIXED(r,si);

		g = GetSin((vertex.vy+CloakingPhase/2)&4095);
		g = MUL_FIXED(g,g);
		greenI = MUL_FIXED(g,i);
		specularG = MUL_FIXED(g,si);

		b = GetSin((vertex.vz+CloakingPhase*3)&4095);
		b = MUL_FIXED(b,b);
		blueI = MUL_FIXED(b,i);
		specularB = MUL_FIXED(b,si);

	}



	/* Intensity for Textures */
	redI >>= 8;
	if(redI > 255) redI = 255;
	renderVertexPtr->R = redI;
	ColourIntensityArray[vertexNumber].R = redI;
	
	greenI >>= 8;
	if(greenI > 255) greenI = 255;
	renderVertexPtr->G = greenI;
	ColourIntensityArray[vertexNumber].G = greenI;
	
	blueI >>= 8;
	if(blueI > 255) blueI = 255;
	renderVertexPtr->B = blueI;
	ColourIntensityArray[vertexNumber].B = blueI;

	specularR >>= 10;
	if(specularR > 255) specularR = 255;		 
	renderVertexPtr->SpecularR = specularR;
	ColourIntensityArray[vertexNumber].SpecularR = specularR;
	
	specularG >>= 10;
	if(specularG > 255) specularG = 255;
	renderVertexPtr->SpecularG = specularG;
	ColourIntensityArray[vertexNumber].SpecularG = specularG;
		
	specularB >>= 10;
	if(specularB > 255) specularB = 255;
	renderVertexPtr->SpecularB = specularB;
	ColourIntensityArray[vertexNumber].SpecularB = specularB;


}
static void VertexIntensity_Underwater(RENDERVERTEX *renderVertexPtr)
{
	int redI,greenI,blueI;
	int specularR,specularG,specularB;

	int vertexNumber = *VertexNumberPtr;

	if(ColourIntensityArray[vertexNumber].Stamp==ObjectCounter)
	{
		renderVertexPtr->R = ColourIntensityArray[vertexNumber].R;
		renderVertexPtr->G = ColourIntensityArray[vertexNumber].G;
		renderVertexPtr->B = ColourIntensityArray[vertexNumber].B;	
		renderVertexPtr->SpecularR = ColourIntensityArray[vertexNumber].SpecularR;
		renderVertexPtr->SpecularG = ColourIntensityArray[vertexNumber].SpecularG;
		renderVertexPtr->SpecularB = ColourIntensityArray[vertexNumber].SpecularB;	
		return;
	}	
	ColourIntensityArray[vertexNumber].Stamp=ObjectCounter;
	{
		VECTORCH *vertexNormalPtr = ((VECTORCH *)Global_ShapeVNormals) + vertexNumber;
		VECTORCH *vertexPtr = ((VECTORCH *)Global_ShapePoints)+vertexNumber;

		LIGHTBLOCK **larrayptr;
		LIGHTBLOCK *lptr;
		int i;

		if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_PreLit) 
		{
			unsigned int packedI = Global_EID_IPtr[vertexNumber];
			blueI = (packedI&255)*257;

			packedI >>=8;
			greenI = (packedI&255)*257;

			packedI >>=8;
			redI = (packedI&255)*257;
		}
		else 
		{
			redI = 0;			
			greenI = 0;			
			blueI = 0;
		}

		specularR = 0;
		specularG = 0;
		specularB = 0;



		larrayptr = LightSourcesForObject;

		for(i = NumLightSourcesForObject; i!=0; i--) 
		{

			VECTORCH vertexToLight;
			int distanceToLight;
	
			lptr = *larrayptr++;
			
			vertexToLight.vx = lptr->LocalLP.vx - vertexPtr->vx;
			vertexToLight.vy = lptr->LocalLP.vy - vertexPtr->vy;
			vertexToLight.vz = lptr->LocalLP.vz - vertexPtr->vz;
			{
				int dx,dy,dz;

				dx = vertexToLight.vx;
				if (dx<0) dx = -dx;
					 
				dy = vertexToLight.vy;
				if (dy<0) dy = -dy;
				
				dz = vertexToLight.vz;	 	 
				if (dz<0) dz = -dz;
									 
				
				if (dx>dy)
				{
					if (dx>dz)
					{
						distanceToLight = dx + ((dy+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dy+dx)>>2);
					}
				}
				else
				{
					if (dy>dz)
					{
						distanceToLight = dy + ((dx+dz)>>2);
					}
					else
					{
						distanceToLight = dz + ((dx+dy)>>2);
					}
				}
			}
																	  				
			if(distanceToLight < lptr->LightRange) 
			{
				int idot = MUL_FIXED(lptr->LightRange-distanceToLight,lptr->BrightnessOverRange);
				int r,g,b;

				if(distanceToLight>0)
				{
				 	int dotproduct = MUL_FIXED(vertexNormalPtr->vx,vertexToLight.vx)
					     + MUL_FIXED(vertexNormalPtr->vy,vertexToLight.vy)
					     + MUL_FIXED(vertexNormalPtr->vz,vertexToLight.vz);

					if(dotproduct>0)
					{ 
						idot = (WideMulNarrowDiv(idot,dotproduct,distanceToLight)+idot/4)/2;
					}
					else
					{
						idot /= 8;
					}
				}
			
				r = MUL_FIXED(idot,lptr->RedScale);			
				g = MUL_FIXED(idot,lptr->GreenScale);			
				b = MUL_FIXED(idot,lptr->BlueScale);			

				redI += r;			
				greenI += g;			
				blueI += b;			
			
				if( !(lptr->LightFlags & LFlag_PreLitSource)
				 && !(lptr->LightFlags & LFlag_NoSpecular) )
				{
					specularR += r;			
					specularG += g;			
					specularB += b;			
				}
			}
	  	}
	}

	if(Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_ONFIRE)
	{
		specularR>>=2;
		specularG>>=2;
		specularB>>=2;

		redI>>=1;
		greenI>>=1;
		blueI>>=1;
	}

	{
		if (specularB<renderVertexPtr->Z*4)
			specularB = renderVertexPtr->Z*4;

	}



	/* Intensity for Textures */
	redI >>= 8;
	if(redI > 255) redI = 255;
	renderVertexPtr->R = redI;
	ColourIntensityArray[vertexNumber].R = redI;
	
	greenI >>= 8;
	if(greenI > 255) greenI = 255;
	renderVertexPtr->G = greenI;
	ColourIntensityArray[vertexNumber].G = greenI;
	
	blueI >>= 8;
	if(blueI > 255) blueI = 255;
	renderVertexPtr->B = blueI;
	ColourIntensityArray[vertexNumber].B = blueI;

	specularR >>= 10;
	if(specularR > 255) specularR = 255;		 
	renderVertexPtr->SpecularR = specularR;
	ColourIntensityArray[vertexNumber].SpecularR = specularR;
	
	specularG >>= 10;
	if(specularG > 255) specularG = 255;
	renderVertexPtr->SpecularG = specularG;
	ColourIntensityArray[vertexNumber].SpecularG = specularG;
		
	specularB >>= 10;
	if(specularB > 255) specularB = 255;
	renderVertexPtr->SpecularB = specularB;
	ColourIntensityArray[vertexNumber].SpecularB = specularB;


 }

/*KJL***********************************************************************
* The following functions have been transplanted from the old shape.c, and *
* will probably be found a new home at some point in the future.           *
***********************************************************************KJL*/
	
/*

 Texture Animation

*/

int* GetTxAnimArrayZ(int shape, int item)

{

	SHAPEHEADER *sptr;
	int **item_array_ptr;
	int **shape_textures;
	int *item_ptr;
	POLYHEADER *pheader;
	int texture_defn_index;


	sptr = GetShapeData(shape);

	if(sptr && sptr->sh_textures && sptr->items) {

		item_array_ptr = sptr->items;
		shape_textures = sptr->sh_textures;

		item_ptr = item_array_ptr[item];
		pheader  = (POLYHEADER *) item_ptr;

		texture_defn_index = (pheader->PolyColour >> TxDefn);

		if(pheader->PolyFlags & iflag_txanim) {

			return (int*) shape_textures[texture_defn_index];

		}

		else return 0;

	}

	else return 0;

}


TXANIMHEADER* GetTxAnimDataZ(int shape, int item, int sequence)

{

	SHAPEHEADER *sptr;
	TXANIMHEADER **txah_ptr;
	TXANIMHEADER *txah;
	int **item_array_ptr;
	int **shape_textures;
	int *item_ptr;
	POLYHEADER *pheader;
	int texture_defn_index;


	sptr = GetShapeData(shape);

	if(sptr && sptr->sh_textures && sptr->items) {

		item_array_ptr = sptr->items;
		shape_textures = sptr->sh_textures;

		item_ptr = item_array_ptr[item];
		pheader  = (POLYHEADER *) item_ptr;

		texture_defn_index = (pheader->PolyColour >> TxDefn);

		if(pheader->PolyFlags & iflag_txanim) {

			txah_ptr = (TXANIMHEADER **) shape_textures[texture_defn_index];
			txah_ptr++;		/* Skip sequence shadow */

			txah = txah_ptr[sequence];

			return txah;

		}

		else return 0;

	}

	else return 0;

}




/*

 For some animated textures each sequence will represent a different view
 of a sprite. When each sequence has the same number of frames there is no
 problem transferring the value from one "txa_currentframe" to the other.
 However if the new sequence has a different number of frames a scaling must
 be done.

*/

void ChangeSequence(TXANIMHEADER *txah_old, TXANIMHEADER *txah_new)

{

	if(txah_new->txa_numframes == txah_old->txa_numframes) {

		txah_new->txa_currentframe = txah_old->txa_currentframe;

	}

	else {

		txah_new->txa_currentframe =

		WideMulNarrowDiv(txah_old->txa_currentframe,
								txah_new->txa_maxframe,
								txah_old->txa_maxframe);

	}
}


/*

 This function copies the TXANIMHEADER from the shape data item sequence
 selected by the TXACTRLBLK to the TXANIMHEADER in the TXACTRLBLK

*/

TXANIMHEADER* GetTxAnimHeaderFromShape(TXACTRLBLK *taptr, int shape)

{

	TXANIMHEADER *txah = 0;


	{

		txah = GetTxAnimDataZ(shape, taptr->tac_item, taptr->tac_sequence);

	}

	if(txah) {

		taptr->tac_txah.txa_flags        = txah->txa_flags;
		taptr->tac_txah.txa_state        = txah->txa_state;
		taptr->tac_txah.txa_numframes    = txah->txa_numframes;
		taptr->tac_txah.txa_framedata    = txah->txa_framedata;
		taptr->tac_txah.txa_currentframe = txah->txa_currentframe;
		taptr->tac_txah.txa_maxframe     = txah->txa_maxframe;
		taptr->tac_txah.txa_speed        = txah->txa_speed;

	}

	return txah;

}


/*

 Texture Animation Control Blocks are used to update animation. At the start
 of "AddShape()" the relevant control block values are copied across to the
 item TXANIMHEADER.

*/

void UpdateTxAnim(TXANIMHEADER *txah)

{

	int UpdateRate;


	if(txah->txa_flags & txa_flag_play) {

		/* How fast do we go? */

		if(txah->txa_flags & txa_flag_quantiseframetime) {

			/* This option is still being designed and tested */

			UpdateRate = txah->txa_speed & (~4096);		/* 1/16th */
			if(UpdateRate < 4096) UpdateRate = 4096;
			UpdateRate = MUL_FIXED(NormalFrameTime, txah->txa_speed);

		}

		else UpdateRate = MUL_FIXED(NormalFrameTime, txah->txa_speed);


		/* Update the current frame */

		if(txah->txa_flags & txa_flag_reverse) {

			txah->txa_currentframe -= UpdateRate;

			if(txah->txa_currentframe < 0) {

				if(txah->txa_flags & txa_flag_noloop) {

					txah->txa_currentframe = 0;

				}

				else {

					txah->txa_currentframe += txah->txa_maxframe;

				}

			}

		}

		else {

			txah->txa_currentframe += UpdateRate;

			if(txah->txa_currentframe >= txah->txa_maxframe) {

				if(txah->txa_flags & txa_flag_noloop) {

					txah->txa_currentframe = txah->txa_maxframe - 1;

				}

				else {

					txah->txa_currentframe -= txah->txa_maxframe;

				}

			}

		}

	}

}


/*

 Display block TXACTRLBLKS pass their data on to shape TXANIMHEADERs

*/

void ControlTextureAnimation(DISPLAYBLOCK *dptr)
{

	TXACTRLBLK *taptr;
	TXANIMHEADER *txah;
	int *iptr;


	taptr = dptr->ObTxAnimCtrlBlks;

	while(taptr)
	{
		/* Update animation for the display block TXACTRLBLK */
		LOCALASSERT(&(taptr->tac_txah));
		UpdateTxAnim(&taptr->tac_txah);

		/* Get the TXANIMHEADER from the shape data */

		txah = taptr->tac_txah_s;

		/* Copy across the current frame */
		LOCALASSERT(txah);
		txah->txa_currentframe = taptr->tac_txah.txa_currentframe;

		iptr = taptr->tac_txarray;
		LOCALASSERT(iptr);
		*iptr = taptr->tac_sequence;

		taptr = taptr->tac_next;
	}
}

void CreateTxAnimUVArray(int *txa_data, int *uv_array, int *shapeitemptr)
{
	TXANIMHEADER **txah_ptr;
	TXANIMHEADER *txah;
	TXANIMFRAME *txaf;
	TXANIMFRAME *txaf0;
	TXANIMFRAME *txaf1;
	int *txaf0_uv;
	int *txaf1_uv;
	int CurrentFrame, NextFrame, Alpha, OneMinusAlpha;
	int i;
	int *iptr;
	int Orient, Scale;
	int OrientX, OrientY;
	int ScaleX, ScaleY;
	int sin, cos;
	int x, y;
	int x1, y1;
	int o1, o2, od;
	POLYHEADER *pheader = (POLYHEADER*) shapeitemptr;
	int sequence;
	int *txf_imageptr;


	/* The sequence # will have been copied across by the control block */

	sequence = *txa_data;
        
        // SBF: 64HACK - skip over the rest of the int*
        txa_data = (int *)((intptr_t) txa_data + sizeof(int *));
        
	#if 0
	textprint("sequence = %d\n", sequence);
	#endif

	txah_ptr = (TXANIMHEADER **) txa_data;
	txah = txah_ptr[sequence];
	txaf = txah->txa_framedata;


	/* Because the current frame can be set from outside, clamp it first */

	if(txah->txa_currentframe < 0) {

		txah->txa_currentframe = 0;

	}

	if(txah->txa_currentframe >= txah->txa_maxframe) {

		txah->txa_currentframe = txah->txa_maxframe - 1;

	}


	/* Frame # */

	CurrentFrame  = txah->txa_currentframe >> 16;
	Alpha         = txah->txa_currentframe - (CurrentFrame << 16);
	OneMinusAlpha = ONE_FIXED - Alpha;


	/* Start and End Frame */

	NextFrame = CurrentFrame + 1;
	if(NextFrame >= txah->txa_numframes) NextFrame = 0;

	txaf0 = &txaf[CurrentFrame];
	txaf1 = &txaf[NextFrame];


	/*

	Write the image index back to the item by overwriting the shape data.
	This is not elegant but it is one of the kind of things you expect to
	have happen when a major new feature is retro-fitted to a system.

	*/

	pheader->PolyColour &= ClrTxIndex;


	/* Multi-View Sprites need to select an image from the array */

	if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_MultiViewSprite) {

		int **txf_uvarrayptr0 = (int **) txaf0->txf_uvdata;
		int **txf_uvarrayptr1 = (int **) txaf1->txf_uvdata;
		int index;


		index = GetMVSIndex(txah, &LToVMat_Euler);

		/*textprint("index = %d\n", index);*/


		txf_imageptr = (int *) txaf0->txf_image;
		pheader->PolyColour |= txf_imageptr[index];


		/* Get the uv data */

		txaf0_uv = txf_uvarrayptr0[index];
		txaf1_uv = txf_uvarrayptr1[index];

	}


	/* Single-View Sprites have just one image per frame */

	else {

		pheader->PolyColour |= txaf0->txf_image;

		txaf0_uv = txaf0->txf_uvdata;
		txaf1_uv = txaf1->txf_uvdata;

	}


	/* Calculate UVs */

	iptr = uv_array;

	if(txah->txa_flags & txa_flag_interpolate_uvs) {

		for(i = txaf0->txf_numuvs; i!=0; i--) {

			iptr[0] = MUL_FIXED(txaf0_uv[0], OneMinusAlpha)
							+ MUL_FIXED(txaf1_uv[0], Alpha);

			iptr[1] = MUL_FIXED(txaf0_uv[1], OneMinusAlpha)
							+ MUL_FIXED(txaf1_uv[1], Alpha);

			/*textprint("%d, %d\n", iptr[0] >> 16, iptr[1] >> 16);*/

			txaf0_uv += 2;
			txaf1_uv += 2;
			iptr += 2;

		}

	}

	else {

		for(i = txaf0->txf_numuvs; i!=0; i--) {

			iptr[0] = txaf0_uv[0];
			iptr[1] = txaf0_uv[1];

			/*textprint("%d, %d\n", iptr[0] >> 16, iptr[1] >> 16);*/

			txaf0_uv += 2;
			iptr += 2;

		}

	}


	/* Interpolate Orient and Scale */

	o1 = txaf0->txf_orient;
	o2 = txaf1->txf_orient;

	if(o1 == o2) {

		Orient = o1;

	}

	else {

		od = o1 - o2;
		if(od < 0) od = -od;

		if(od >= deg180) {

			o1 <<= (32 - 12);
			o1 >>= (32 - 12);
			o2 <<= (32 - 12);
			o2 >>= (32 - 12);

		}

		Orient = MUL_FIXED(o1, OneMinusAlpha) + MUL_FIXED(o2, Alpha);
		Orient &= wrap360;

	}


	if(txaf0->txf_scale == txaf1->txf_scale) {

		Scale = txaf0->txf_scale;

	}

	else {

		Scale = WideMul2NarrowDiv(txaf0->txf_scale, OneMinusAlpha,
										txaf1->txf_scale, Alpha, ONE_FIXED);

	}


	/* Interpolate Orient and Scale Origins */

	if(txaf0->txf_orientx == txaf1->txf_orientx) {

		OrientX = txaf0->txf_orientx;

	}

	else {

		OrientX = MUL_FIXED(txaf0->txf_orientx, OneMinusAlpha)
					+ MUL_FIXED(txaf1->txf_orientx, Alpha);

	}


	if(txaf0->txf_orienty == txaf1->txf_orienty) {

		OrientY = txaf0->txf_orienty;

	}

	else {

		OrientY = MUL_FIXED(txaf0->txf_orienty, OneMinusAlpha)
					+ MUL_FIXED(txaf1->txf_orienty, Alpha);

	}


	if(txaf0->txf_scalex == txaf1->txf_scalex) {

		ScaleX = txaf0->txf_scalex;

	}

	else {

		ScaleX = MUL_FIXED(txaf0->txf_scalex, OneMinusAlpha)
					+ MUL_FIXED(txaf1->txf_scalex, Alpha);

	}


	if(txaf0->txf_scaley == txaf1->txf_scaley) {

		ScaleY = txaf0->txf_scaley;

	}

	else {

		ScaleY = MUL_FIXED(txaf0->txf_scaley, OneMinusAlpha)
					+ MUL_FIXED(txaf1->txf_scaley, Alpha);

	}



	#if 0
	textprint("Alpha         = %d\n", Alpha);
	textprint("OneMinusAlpha = %d\n", OneMinusAlpha);
	textprint("Orient = %d\n", Orient);
	textprint("txaf0->txf_scale = %d\n", txaf0->txf_scale);
	textprint("txaf1->txf_scale = %d\n", txaf1->txf_scale);
	textprint("Scale  = %d\n", Scale);
	#endif

	/*WaitForReturn();*/



#if 1


	/* Rotate UV Array */

	if(Orient) {

		sin = GetSin(Orient);
		cos = GetCos(Orient);

		iptr = uv_array;

		for(i = txaf0->txf_numuvs; i!=0; i--) {

			x = iptr[0] - OrientX;
			y = iptr[1] - OrientY;

			x1 = MUL_FIXED(x, cos) - MUL_FIXED(y, sin);
			y1 = MUL_FIXED(x, sin) + MUL_FIXED(y, cos);

			iptr[0] = x1 + OrientX;
			iptr[1] = y1 + OrientY;

			iptr += 2;

		}

	}


	/* Scale UV Array */

	if(Scale != ONE_FIXED) {

		iptr = uv_array;

		for(i = txaf0->txf_numuvs; i!=0; i--) {

			x = iptr[0] - ScaleX;
			y = iptr[1] - ScaleY;

			x = MUL_FIXED(x, Scale);
			y = MUL_FIXED(y, Scale);

			iptr[0] = x + ScaleX;
			iptr[1] = y + ScaleY;

			iptr += 2;

		}

	}


#endif






	#if 0
	textprint("Current Frame = %d\n", txah->txa_currentframe);
	textprint("Current Frame = %d\n", CurrentFrame);
	textprint("Next Frame    = %d\n", NextFrame);
	textprint("Alpha         = %d\n", Alpha);
	#endif


	/*textprint("Leaving CreateTxAnimUVArray\n");*/
	/*WaitForReturn();*/

}













/*

 Shape Points for Unrotated Sprites

*/

void ShapeSpritePointsInstr(SHAPEINSTR *shapeinstrptr)

{

	int **shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	int *shapeitemptr       = *shapeitemarrayptr;
	VECTORCH *rotptsptr       = RotatedPts;
	int numitems;


	for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--) {

		rotptsptr->vx =  shapeitemptr[ix];
		rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

		rotptsptr->vy =  shapeitemptr[iy];
		rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

		rotptsptr->vz =  shapeitemptr[iz];
		rotptsptr->vz += Global_ODB_Ptr->ObView.vz;

		shapeitemptr += vsize;
		rotptsptr++;

	}

}


/*

 Shape Points for Rotated Sprites

*/


//I've put my alterations to the sprite rotation
//in the #else part.Richard
#define UseKevinsModifiedSSRPI No


#if UseKevinsModifiedSSRPI



#define ssrpi_kill_py Yes

void ShapeSpriteRPointsInstr(SHAPEINSTR *shapeinstrptr)

{

	int **shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	int *shapeitemptr       = *shapeitemarrayptr;
	VECTORCH *rotptsptr       = RotatedPts;
	int numitems;
	int x,y,z;
	MATRIXCH m;
	#if ssrpi_kill_py
	EULER e;
	char flipX=0;
	#endif 
	/*

	Sprite Resizing

	If this shape is a sprite and is using sprite resizing, there will be a transformed
	copy of the polygon points array (XY only) to copy back to the shape.

	WARNING!

	This function and data structure ASSUME that the sprite shape is using an item array,
	that there is just the one item, and that the world and UV space coordinates are in the
	form of "TL, BL, BR, TR". It also assumes that the sprite polygon is in the XY plane.

	If ANY of these is not true for your sprite, DON'T attempt to use resizing!

	*/

	if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_Sprite &&
		Global_ShapeHeaderPtr->shapeflags & ShapeFlag_SpriteResizing) {

		int *ShapePoints;
		int **item_array_ptr;
		int *item_ptr;
		POLYHEADER *pheader;
		int *mypolystart;
		int texture_defn_index;
		int *texture_defn_ptr;
		TXANIMHEADER **txah_ptr;
		TXANIMHEADER *txah;
		TXANIMFRAME *txaf;
		TXANIMFRAME *txaf0;
		int CurrentFrame, sequence;
		int *iptr;

		ShapePoints = *(Global_ShapeHeaderPtr->points);

		/* Item */

		item_array_ptr = Global_ShapeHeaderPtr->items;	/* Assume item array */
		item_ptr = item_array_ptr[0];					/* Assume only one polygon */
		pheader = (POLYHEADER *) item_ptr;

		/* Texture Animation */

		texture_defn_index = (pheader->PolyColour >> TxDefn);
		texture_defn_ptr = Global_ShapeTextures[texture_defn_index];

		sequence = *texture_defn_ptr++;

		txah_ptr = (TXANIMHEADER **) texture_defn_ptr;
		txah = txah_ptr[sequence];
		txaf = txah->txa_framedata;

		/* Because the current frame can be set from outside, clamp it first */

		if(txah->txa_currentframe < 0)
			txah->txa_currentframe = 0;
		if(txah->txa_currentframe >= txah->txa_maxframe)
			txah->txa_currentframe = txah->txa_maxframe - 1;

		CurrentFrame  = txah->txa_currentframe >> 16;

		txaf0 = &txaf[CurrentFrame];

		/* UV array */

		if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_Sprite) {

			int **uvarrayptr = (int **) txaf0->txf_uvdata;
			iptr = uvarrayptr[GetMVSIndex(txah, &LToVMat_Euler)];

		}

		else {

			iptr = txaf0->txf_uvdata;

		}
		
		iptr += (txaf0->txf_numuvs * 2);

		/* Redefine the Shape Points */

		mypolystart = &pheader->Poly1stPt;

		while(*mypolystart != Term) {

			/*textprint("copying point %d\n", *mypolystart / vsize);*/

			*(ShapePoints + *mypolystart + ix) = iptr[0];
			*(ShapePoints + *mypolystart + iy) = iptr[1];

			mypolystart++;
			iptr += 2;

		}

	}


	#if ssrpi_kill_py


	/* Make a copy of the object matrix */

	CopyMatrix(&Global_ODB_Ptr->ObMat, &m);



	/* Combine it with the view matrix */

	MatrixMultiply(&Global_VDB_Ptr->VDB_SpriteMat, &m, &m);

	/* Extract the Euler Angles */
	MatrixToEuler(&m, &e);

	#if 0
	textprint("X: %d\n", e.EulerX);
	textprint("Y: %d\n", e.EulerY);
	textprint("Z: %d\n", e.EulerZ);
	#endif


	/* Knock out the pitch and yaw */

	/* KJL 17:23:22 01/09/97 - If the sprite is turned away from you, flip along the
	x-axis so that you get the mirror image of the sprite */
 	if (e.EulerY<1024 || e.EulerY>3072)	flipX = 1;
   	e.EulerY=0;
	e.EulerX=0;


	/* Turn it back into a matrix */
	CreateEulerMatrix(&e, &m);
	TransposeMatrixCH(&m);

	#else	/* ssrpi_kill_py */


	MatrixMultiply(&Global_VDB_Ptr->VDB_SpriteMat, &Global_ODB_Ptr->ObMat, &m);


	#endif	/* ssrpi_kill_py */


	for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--) {

		x = shapeitemptr[ix];
		y = shapeitemptr[iy];
		z = shapeitemptr[iz];

		rotptsptr->vx =  MUL_FIXED(m.mat11, x);
		rotptsptr->vx += MUL_FIXED(m.mat21, y);
		rotptsptr->vx += MUL_FIXED(m.mat31, z);
		#if ssrpi_kill_py
		if (flipX) rotptsptr->vx = - rotptsptr->vx;
		#endif
		rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

		rotptsptr->vy =  MUL_FIXED(m.mat12, x);
		rotptsptr->vy += MUL_FIXED(m.mat22, y);
		rotptsptr->vy += MUL_FIXED(m.mat32, z);
		rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

		rotptsptr->vz =  MUL_FIXED(m.mat13, x);
		rotptsptr->vz += MUL_FIXED(m.mat23, y);
		rotptsptr->vz += MUL_FIXED(m.mat33, z);
		rotptsptr->vz += Global_ODB_Ptr->ObView.vz;

		shapeitemptr += vsize;
		rotptsptr++;

	}

}




#else		/* UseKevinsModifiedSSRPI */




#define ssrpi_kill_py No

void ShapeSpriteRPointsInstr(SHAPEINSTR *shapeinstrptr)

{

	int **shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	int *shapeitemptr       = *shapeitemarrayptr;
	VECTORCH *rotptsptr       = RotatedPts;
	int numitems;
	int x,y;
	VECTORCH vectx,vecty;
	#if ssrpi_kill_py
	MATRIXCH m2;
	EULER e;
	#endif


	/*

	Sprite Resizing

	If this shape is a sprite and is using sprite resizing, there will be a transformed
	copy of the polygon points array (XY only) to copy back to the shape.

	WARNING!

	This function and data structure ASSUME that the sprite shape is using an item array,
	that there is just the one item, and that the world and UV space coordinates are in the
	form of "TL, BL, BR, TR". It also assumes that the sprite polygon is in the XY plane.

	If ANY of these is not true for your sprite, DON'T attempt to use resizing!

	*/

	if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_Sprite &&
		Global_ShapeHeaderPtr->shapeflags & ShapeFlag_SpriteResizing) {

		int *ShapePoints;
		int **item_array_ptr;
		int *item_ptr;
		POLYHEADER *pheader;
		int *mypolystart;
		int texture_defn_index;
		int *texture_defn_ptr;
		TXANIMHEADER **txah_ptr;
		TXANIMHEADER *txah;
		TXANIMFRAME *txaf;
		TXANIMFRAME *txaf0;
		int CurrentFrame, sequence;
		int *iptr;

		ShapePoints = *(Global_ShapeHeaderPtr->points);

		/* Item */

		item_array_ptr = Global_ShapeHeaderPtr->items;	/* Assume item array */
		item_ptr = item_array_ptr[0];					/* Assume only one polygon */
		pheader = (POLYHEADER *) item_ptr;

		/* Texture Animation */

		texture_defn_index = (pheader->PolyColour >> TxDefn);
		texture_defn_ptr = Global_ShapeTextures[texture_defn_index];

		sequence = *texture_defn_ptr++;

		txah_ptr = (TXANIMHEADER **) texture_defn_ptr;
		txah = txah_ptr[sequence];
		txaf = txah->txa_framedata;

		/* Because the current frame can be set from outside, clamp it first */

		if(txah->txa_currentframe < 0)
			txah->txa_currentframe = 0;
		if(txah->txa_currentframe >= txah->txa_maxframe)
			txah->txa_currentframe = txah->txa_maxframe - 1;

		CurrentFrame  = txah->txa_currentframe >> 16;

		txaf0 = &txaf[CurrentFrame];


		/* UV array */

		if(Global_ShapeHeaderPtr->shapeflags & ShapeFlag_Sprite) {

			int **uvarrayptr = (int **) txaf0->txf_uvdata;
			iptr = uvarrayptr[GetMVSIndex(txah, &LToVMat_Euler)];

		}

		else {

			iptr = txaf0->txf_uvdata;

		}

		iptr += (txaf0->txf_numuvs * 2);


		/* Redefine the Shape Points */

		mypolystart = &pheader->Poly1stPt;

		while(*mypolystart != Term) {

			/*textprint("copying point %d\n", *mypolystart / vsize);*/

			((VECTORCH *)ShapePoints)[*mypolystart].vx = iptr[0];
			((VECTORCH *)ShapePoints)[*mypolystart].vy = iptr[1];

			/*textprint("x, y = %d, %d\n", iptr[0], iptr[1]);*/

			mypolystart++;
			iptr += 2;

		}

	}

   	//project the object's y vector onto the screen.
	//then rotate the sprite's points according to the vector's orintation
	//relative to the screen's y vector.
   	vecty=*(VECTORCH*)&Global_ODB_Ptr->ObMat.mat21;
		
	RotateVector(&vecty,&Global_VDB_Ptr->VDB_Mat);
   	if(Global_VDB_Ptr->VDB_ProjX!=Global_VDB_Ptr->VDB_ProjY)
   	{
   		vecty.vx=MUL_FIXED(vecty.vx,Global_VDB_Ptr->VDB_ProjX);
		vecty.vy=MUL_FIXED(vecty.vy,Global_VDB_Ptr->VDB_ProjY);
	}
	vecty.vz=0;
	if(!vecty.vx && !vecty.vy)vecty.vy=ONE_FIXED;
	Normalise(&vecty);
	vectx.vx=-vecty.vy;
	vectx.vy=vecty.vx;
	vectx.vz=0;

	for(numitems = shapeinstrptr->sh_numitems; numitems!=0; numitems--) {

		x = -shapeitemptr[ix];
		y = shapeitemptr[iy];

		rotptsptr->vx =  MUL_FIXED(vectx.vx, x);
		rotptsptr->vx += MUL_FIXED(vectx.vy, y);
		rotptsptr->vx += Global_ODB_Ptr->ObView.vx;

		rotptsptr->vy =  MUL_FIXED(vecty.vx, x);
		rotptsptr->vy += MUL_FIXED(vecty.vy, y);
		rotptsptr->vy += Global_ODB_Ptr->ObView.vy;

		rotptsptr->vz = Global_ODB_Ptr->ObView.vz;

		shapeitemptr += vsize;
		rotptsptr++;

	}


}



#endif		/* UseKevinsModifiedSSRPI */

int GetMVSIndex(TXANIMHEADER
 *txah, EULER* e )
{
	int EulerXIndex, EulerYIndex;
	int theta,phi; //angles in spherical polar coordinates
				   //phi goes from 0 (top) to deg180 (bottom)	
	VECTORCH v;
	
	MakeVectorLocal(&Global_ODB_Ptr->ObWorld,&v,&Global_VDB_Ptr->VDB_World,&Global_ODB_Ptr->ObMat);
	Normalise(&v);
	phi=-ArcSin(v.vy);
	phi+=deg90;
	phi&=wrap360;
	if(phi==deg180)phi--;
	if(!v.vx && !v.vz)
	{
		theta=0;
	}
	else
	{
		v.vy=0;
		Normalise(&v);
		if(v.vz > Cosine45 || -v.vz>Cosine45) {

			theta = ArcSin(-v.vx);

			if(v.vz < 0) {
				theta &= wrap360;
			}
			else
			{
				theta+=deg180;
				theta =-theta;
				theta &= wrap360;
			}
		}

		else {

			theta = ArcCos(v.vz);

			if(v.vx < 0) {
				theta =  -theta;
			}
			theta+=deg180;
			theta &= wrap360;

		}
	}

	EulerYIndex = theta;
	EulerYIndex >>= txah->txa_euleryshift;


	EulerYIndex <<= (11 - txah->txa_eulerxshift);

	EulerXIndex = phi;
	EulerXIndex >>= txah->txa_eulerxshift;

    GLOBALASSERT((EulerXIndex+EulerYIndex)<txah->txa_num_mvs_images);

	return (EulerXIndex + EulerYIndex);

}


void AddShape(DISPLAYBLOCK *dptr, VIEWDESCRIPTORBLOCK *VDB_Ptr)
{
	SHAPEHEADER *shapeheaderptr;

	if (!dptr->ObShape && dptr->SfxPtr)
	{
//		DrawSfxObject(dptr);
		return;
	}
	/* KJL 12:42:38 18/05/98 - check to see if object is on fire */
	if (dptr->ObStrategyBlock)
	{
		if(dptr->ObStrategyBlock->SBDamageBlock.IsOnFire)
		{
			dptr->SpecialFXFlags |= SFXFLAG_ONFIRE;
		}
		else
		{
			dptr->SpecialFXFlags &= ~SFXFLAG_ONFIRE;
		}

	}
	
	/* is object a morphing one? */	
 	if(dptr->ObMorphCtrl)
	{
		LOCALASSERT(dptr->ObMorphCtrl->ObMorphHeader);

		/* SBF - commented out */
		/* if(dptr->ObMorphCtrl->ObMorphHeader) */
	 	{
			GetMorphDisplay(&MorphDisplay, dptr);
			dptr->ObShape     = MorphDisplay.md_shape1;
			dptr->ObShapeData = MorphDisplay.md_sptr1;
			shapeheaderptr    = MorphDisplay.md_sptr1;
		}
	}
	else
	{
		shapeheaderptr = GetShapeData(dptr->ObShape);
		
		/* It is important to pass this SHAPEHEADER* on to the display block */
		dptr->ObShapeData = shapeheaderptr;

		// I've put this inside the else so that it does
		// not conflict with morphing !!!
		// make sure dptr->ObShapeData is up to date before
		// doing CopyAnimationFrameToShape

		if (dptr->ShapeAnimControlBlock)
		{
			if (!(dptr->ShapeAnimControlBlock->current.empty))
			{
				CopyAnimationFrameToShape (&dptr->ShapeAnimControlBlock->current, dptr);
			}
		}
	}
  	
  	
	ChooseLightingModel(dptr);
	/* hierarchical object? */
	#if 0
	if (dptr->HModelControlBlock && !dptr->ObStrategyBlock)
	{
   		DoHModel(dptr->HModelControlBlock,dptr);
		return;
	}
	#endif



	/* Texture Animation Control */

	if(dptr->ObTxAnimCtrlBlks) ControlTextureAnimation(dptr);

	/* Global Variables */
	Global_VDB_Ptr        = VDB_Ptr;
	Global_ODB_Ptr        = dptr;
	Global_ShapeHeaderPtr = shapeheaderptr;

	/* Shape Language Specific Setup */
	SetupShapePipeline();

		/*
																		   
	Create the Local -> View Matrix
	
	LToVMat = VDB_Mat * ObMat

	"Get the points into View Space, then apply the Local Transformation"

	*/

	MatrixMultiply(&VDB_Ptr->VDB_Mat, &dptr->ObMat, &LToVMat);
	MatrixToEuler(&LToVMat, &LToVMat_Euler);

	/*

	Create the World -> Local Matrix

	WToLMat = Transposed Local Matrix

	*/

	CopyMatrix(&dptr->ObMat, &WToLMat);
	TransposeMatrixCH(&WToLMat);


	/*

	Transform the View World Location to Local Space

	-> Make the View Loc. relative to the Object View Space Centre
	-> Rotate this vector using WToLMat

	*/


	MakeVector(&VDB_Ptr->VDB_World, &dptr->ObWorld, &LocalView);
	RotateVector(&LocalView, &WToLMat);

	#if 0
	{
		LocalCameraZAxis.vx = - dptr->ObWorld.vx;
		LocalCameraZAxis.vy = - dptr->ObWorld.vy;
		LocalCameraZAxis.vz = - dptr->ObWorld.vz;

		RotateVector(&LocalCameraZAxis, &WToLMat);
	}
	#endif
	
	NumberOfHeatSources=0;
	if (dptr->HModelControlBlock)
	{
		ObjectCentre = dptr->ObView;

		if (dptr->ObStrategyBlock)
		{
			HierarchicalObjectsLowestYValue = dptr->ObStrategyBlock->DynPtr->ObjectVertices[0].vy;
		 	if (CurrentVisionMode == VISION_MODE_NORMAL && AvP.PlayerType==I_Alien)
			{
				DoAlienEnergyView(dptr);
			}
			/*
			else if (CurrentVisionMode == VISION_MODE_PRED_SEEALIENS && dptr->ObStrategyBlock->I_SBtype == I_BehaviourAlien)
			{	
				DoAlienEnergyView(dptr);
			}
			*/
		}

		if (CurrentVisionMode == VISION_MODE_PRED_THERMAL)
		{
			FindHeatSourcesInHModel(dptr);
		}
		DoHModel(dptr->HModelControlBlock,dptr);
		return;
	}
  //	return;
	

  	/* Find out which light sources are in range of of the object */
	LightSourcesInRangeOfObject(dptr);

	/* Shape Language Execution Shell */
	{
		SHAPEINSTR *shapeinstrptr = shapeheaderptr->sh_instruction;
		
		/* 	setup the rotated points array */
		switch (shapeinstrptr->sh_instr)
		{
		 	default:
		 	case I_ShapePoints:
			{
				if(Global_ODB_Ptr->ObMorphCtrl)
				{
					MorphPoints(shapeinstrptr);
				}
				else
				{
					TranslateShapeVertices(shapeinstrptr);
				}
				break;
	 		}
		}
	}
	/* call polygon pipeline */
	ShapePipeline(shapeheaderptr);
	/* call sfx code */
	HandleSfxForObject(dptr);
	if (dptr->ObStrategyBlock)
	{
		if (dptr->ObStrategyBlock->I_SBtype==I_BehaviourInanimateObject)
		{
			INANIMATEOBJECT_STATUSBLOCK* objStatPtr = (INANIMATEOBJECT_STATUSBLOCK*) dptr->ObStrategyBlock->SBdataptr;
			if(objStatPtr->typeId==IOT_FieldCharge)
			{
	
			   	int i;
			  	D3D_DecalSystem_Setup();
				for(i=0; i<63; i++)
				{
					PARTICLE particle;

					// SBF - 20080518 - commented out the undefined usage of particle.Position.vz
					particle.Position.vy = -280+i-GetCos((CloakingPhase/16*i + i*64/*+particle.Position.vz*/)&4095)/1024;

					particle.Position.vx = GetCos((CloakingPhase +i*64+particle.Position.vy)&4095)/512;
					particle.Position.vz = GetSin((CloakingPhase +i*64+particle.Position.vy)&4095)/512;
					RotateVector(&particle.Position,&dptr->ObMat);
					particle.Position.vx += dptr->ObWorld.vx;
					particle.Position.vy += dptr->ObWorld.vy;
					particle.Position.vz += dptr->ObWorld.vz;

					particle.ParticleID=PARTICLE_MUZZLEFLASH;
					particle.Colour = 0xff00007f+(FastRandom()&0x7f7f7f);
					particle.Size = 40;
					RenderParticle(&particle);
				}
			 	D3D_DecalSystem_End();
				
			}

		}
	}
}
void DoAlienEnergyView(DISPLAYBLOCK *dispPtr)
{
	HMODELCONTROLLER *controllerPtr = dispPtr->HModelControlBlock;
	unsigned int colour = MARINES_LIFEFORCE_GLOW_COLOUR;
	
	LOCALASSERT(controllerPtr);
	


	/* KJL 16:36:25 10/02/98 - process model */
	{
		STRATEGYBLOCK *sbPtr = Global_ODB_Ptr->ObStrategyBlock;
	 	if(sbPtr)
	 	{
	 		switch (sbPtr->I_SBtype)
			{
		 		case I_BehaviourAlien:
				{
					colour = ALIENS_LIFEFORCE_GLOW_COLOUR;
					break;
				}
		 		case I_BehaviourPredator:
				{
					colour = PREDATORS_LIFEFORCE_GLOW_COLOUR;
					break;
				}
				case I_BehaviourMarine:
				case I_BehaviourSeal:
				{
					MARINE_STATUS_BLOCK *marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
					GLOBALASSERT(marineStatusPointer);
					
					if (marineStatusPointer->Android)
					{
						return;
					}
					colour = MARINES_LIFEFORCE_GLOW_COLOUR;
				}

				case I_BehaviourNetGhost:
				{
		   			NETGHOSTDATABLOCK *ghostDataPtr = (NETGHOSTDATABLOCK *)Global_ODB_Ptr->ObStrategyBlock->SBdataptr;

					if (ghostDataPtr->type==I_BehaviourAlienPlayer || ghostDataPtr->type==I_BehaviourAlien
					 	|| (ghostDataPtr->type==I_BehaviourNetCorpse&&ghostDataPtr->subtype==I_BehaviourAlienPlayer) )
					{	
						colour = ALIENS_LIFEFORCE_GLOW_COLOUR;
					}
					else if (ghostDataPtr->type==I_BehaviourPredatorPlayer || ghostDataPtr->type==I_BehaviourPredator
					 	|| (ghostDataPtr->type==I_BehaviourNetCorpse&&ghostDataPtr->subtype==I_BehaviourPredatorPlayer) )
					{	
						colour = PREDATORS_LIFEFORCE_GLOW_COLOUR;
					}

					break;
				}

				case I_BehaviourNetCorpse:
				{
					NETCORPSEDATABLOCK *corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;

					if (corpseDataPtr->Android)
					{
						return;
					}

					if (corpseDataPtr->Type==I_BehaviourAlienPlayer || corpseDataPtr->Type==I_BehaviourAlien)
					{
						colour = ALIENS_LIFEFORCE_GLOW_COLOUR;
					}
					else if (corpseDataPtr->Type==I_BehaviourPredatorPlayer || corpseDataPtr->Type==I_BehaviourPredator)
					{
						colour = PREDATORS_LIFEFORCE_GLOW_COLOUR;
					}
					break;
				}
				case I_BehaviourHierarchicalFragment:
				{
					HDEBRIS_BEHAV_BLOCK *debrisDataPtr  = (HDEBRIS_BEHAV_BLOCK *)sbPtr->SBdataptr;
					if(debrisDataPtr->Type==I_BehaviourAutoGun || debrisDataPtr->Android)
					{
						return;
					}
					else if(debrisDataPtr->Type==I_BehaviourAlien)
					{
						colour = ALIENS_LIFEFORCE_GLOW_COLOUR;
					}
					else if (debrisDataPtr->Type==I_BehaviourPredator)
					{
						colour = PREDATORS_LIFEFORCE_GLOW_COLOUR;
					}
					else if ((debrisDataPtr->Type==I_BehaviourMarine)||(debrisDataPtr->Type==I_BehaviourSeal))
					{
						colour = MARINES_LIFEFORCE_GLOW_COLOUR;
					}
					else return;
					break;
				}

				case I_BehaviourAutoGun:
				{
					/* KJL 19:31:53 25/01/99 - organics only, please */
					return;
					break;
				}
				default:
					break;
			}
		}
	}
	if( (Global_ODB_Ptr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)
	  &&(Global_ODB_Ptr->ObFlags2 < ONE_FIXED) )	
	{
		unsigned int alpha = MUL_FIXED(Global_ODB_Ptr->ObFlags2,colour >> 24);
		colour = (colour&0xffffff)+(alpha<<24);
	}

	/* KJL 16:36:12 10/02/98 - check positions are up to date */
	ProveHModel(controllerPtr,dispPtr);

	D3D_DecalSystem_Setup();

	FindAlienEnergySource_Recursion(controllerPtr,controllerPtr->section_data,colour);

	D3D_DecalSystem_End();
}
																	  
static void FindAlienEnergySource_Recursion(HMODELCONTROLLER *controllerPtr, SECTION_DATA *sectionDataPtr, unsigned int colour)
{
	/* KJL 16:29:40 10/02/98 - Recurse through hmodel */
	if ((sectionDataPtr->First_Child!=NULL)&&(!(sectionDataPtr->flags&section_data_terminate_here)))
	{
		SECTION_DATA *childSectionPtr = sectionDataPtr->First_Child;
	
		while (childSectionPtr!=NULL)
		{
			LOCALASSERT(childSectionPtr->My_Parent==sectionDataPtr);

			FindAlienEnergySource_Recursion(controllerPtr,childSectionPtr,colour);
			childSectionPtr=childSectionPtr->Next_Sibling;
		}
	}
	if(sectionDataPtr->Shape && sectionDataPtr->Shape->shaperadius>LocalDetailLevels.AlienEnergyViewThreshold)
	{
		PARTICLE particle;

		particle.Position = sectionDataPtr->World_Offset;
		particle.ParticleID=PARTICLE_MUZZLEFLASH;
		particle.Colour = colour;//0x208080ff;
//		particle.Colour = 0x20ff8080;
//		particle.Size = sectionDataPtr->Shape->shaperadius*3;
//		particle.Colour = 0x20ffffff;
		particle.Size = sectionDataPtr->Shape->shaperadius*2;
		RenderParticle(&particle);
	}
}

void AddHierarchicalShape(DISPLAYBLOCK *dptr, VIEWDESCRIPTORBLOCK *VDB_Ptr)
{

	SHAPEHEADER *shapeheaderptr;
	SHAPEINSTR *shapeinstrptr;

	GLOBALASSERT(!dptr->HModelControlBlock);
	if(!ObjectWithinFrustrum(dptr)) return;


	#if 0
	shapeheaderptr = GetShapeData(dptr->ObShape);
	
	/* It is important to pass this SHAPEHEADER* on to the display block */

	dptr->ObShapeData = shapeheaderptr;
	#else
	shapeheaderptr = dptr->ObShapeData;
	#endif


	/* Texture Animation Control */
	if(dptr->ObTxAnimCtrlBlks) ControlTextureAnimation(dptr);

	/* Global Variables */
	Global_VDB_Ptr        = VDB_Ptr;
	Global_ODB_Ptr        = dptr;
	Global_ShapeHeaderPtr = shapeheaderptr;
	
//	if((Global_ODB_Ptr->ObStrategyBlock)&&(Global_ODB_Ptr->ObStrategyBlock->I_SBtype == I_BehaviourAlien))
 //	textprint("hier alien part\n");

	/* Shape Language Specific Setup */
	SetupShapePipeline();

		/*
																		   
	Create the Local -> View Matrix
	
	LToVMat = VDB_Mat * ObMat

	"Get the points into View Space, then apply the Local Transformation"

	*/

	MatrixMultiply(&VDB_Ptr->VDB_Mat, &dptr->ObMat, &LToVMat);
	MatrixToEuler(&LToVMat, &LToVMat_Euler);

	/*

	Create the World -> Local Matrix

	WToLMat = Transposed Local Matrix

	*/

	CopyMatrix(&dptr->ObMat, &WToLMat);
	TransposeMatrixCH(&WToLMat);


	/*

	Transform the View World Location to Local Space

	-> Make the View Loc. relative to the Object View Space Centre
	-> Rotate this vector using WToLMat

	*/


	MakeVector(&VDB_Ptr->VDB_World, &dptr->ObWorld, &LocalView);
	RotateVector(&LocalView, &WToLMat);

	if (!(PIPECLEANER_CHEATMODE||BALLSOFFIRE_CHEATMODE) || !dptr->ObStrategyBlock)
	{
	  	/* Find out which light sources are in range of of the object */
		LightSourcesInRangeOfObject(dptr);

		/* Shape Language Execution Shell */
		shapeinstrptr = shapeheaderptr->sh_instruction;

		/* 	setup the rotated points array */
		if( (dptr->SpecialFXFlags & SFXFLAG_MELTINGINTOGROUND)
		  &&(dptr->ObFlags2 <= ONE_FIXED) )	
		{
			SquishPoints(shapeinstrptr);
		}
		else
		{
			TranslateShapeVertices(shapeinstrptr);
		}
		
		/* call polygon pipeline */
	  	ShapePipeline(shapeheaderptr);
	}

	if (BALLSOFFIRE_CHEATMODE && dptr->ObStrategyBlock)
	{
		HandleObjectOnFire(dptr);
	}

	/* call sfx code */
	HandleSfxForObject(dptr);

}


float ViewMatrix[12];
float ObjectViewMatrix[12];
float Source[3];
float Dest[3];

extern void TranslationSetup(void)
{
	VECTORCH v = Global_VDB_Ptr->VDB_World;
	extern int PredatorVisionChangeCounter;
	float p = PredatorVisionChangeCounter/65536.0f;
	float o = 1.0f;
	p = 1.0f+p;
	
	if (NAUSEA_CHEATMODE)
	{
		p = (GetSin((CloakingPhase/3)&4095))/65536.0f;
		p = 1.0f + p*p;

		o = (GetCos((CloakingPhase/5)&4095))/65536.0f;
		o = 1.0f + o*o;
	}

	#if 1
	ViewMatrix[0+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat11)/65536.0f*o;
	ViewMatrix[1+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat21)/65536.0f*o;
	ViewMatrix[2+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat31)/65536.0f*o;
	#else
	ViewMatrix[0+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat11)/65536.0f;
	ViewMatrix[1+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat21)/65536.0f;
	ViewMatrix[2+0*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat31)/65536.0f;
	#endif

	#if 1
	ViewMatrix[0+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat12)*4.0f/(65536.0f*3.0f)*p;
	ViewMatrix[1+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat22)*4.0f/(65536.0f*3.0f)*p;
	ViewMatrix[2+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat32)*4.0f/(65536.0f*3.0f)*p;
	#else
	ViewMatrix[0+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat12)/(65536.0f);
	ViewMatrix[1+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat22)/(65536.0f);
	ViewMatrix[2+1*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat32)/(65536.0f);
	#endif
	ViewMatrix[0+2*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat13)/65536.0f*CameraZoomScale;
	ViewMatrix[1+2*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat23)/65536.0f*CameraZoomScale;
	ViewMatrix[2+2*4] = (float)(Global_VDB_Ptr->VDB_Mat.mat33)/65536.0f*CameraZoomScale;

	RotateVector(&v,&Global_VDB_Ptr->VDB_Mat);

	ViewMatrix[3+0*4] = ((float)-v.vx)*o;
	ViewMatrix[3+1*4] = ((float)-v.vy)*4.0f/3.0f*p;
	ViewMatrix[3+2*4] = ((float)-v.vz)*CameraZoomScale;

	if (MIRROR_CHEATMODE)
	{
		ViewMatrix[0+0*4] = -ViewMatrix[0+0*4];
		ViewMatrix[1+0*4] =	-ViewMatrix[1+0*4];
		ViewMatrix[2+0*4] =	-ViewMatrix[2+0*4];
		
		ViewMatrix[3+0*4] =	-ViewMatrix[3+0*4];
	}
}


#if defined(_MSC_VER) && 0
void TranslatePoint(int *source, int *dest, int *matrix)
{
	__asm
	{
		mov esi,source
		mov ebx,dest
		mov edi,matrix 
		fld	DWORD PTR [esi]
		fmul	DWORD PTR [edi]
		fld	DWORD PTR [esi+4]
		fmul	DWORD PTR [edi+4]
		fld	DWORD PTR [esi+8]
		fmul	DWORD PTR [edi+8]
		fxch	st(1)
		faddp	st(2),st
		fld	DWORD PTR [esi]
		fmul	DWORD PTR [edi+16]
		fxch	st(1)
		faddp	st(2),st
		fld	DWORD PTR [esi+4]
		fmul	DWORD PTR [edi+20]
		fld	DWORD PTR [esi+8]
		fmul	DWORD PTR [edi+24]
		fxch	st(1)
		faddp	st(2),st
		fld	DWORD PTR [esi]
		fmul	DWORD PTR [edi+32]
		fxch	st(1)
		faddp	st(2),st
		fld	DWORD PTR [esi+4]
		fmul	DWORD PTR [edi+36]
		fld	DWORD PTR [esi+8]
		fmul	DWORD PTR [edi+40]
		fxch	st(1)
		faddp	st(2),st
		fxch	st(3)
		fadd	DWORD PTR [edi+12]
		fxch	st(1)
		faddp	st(3),st
		fxch	st(1)
		fadd	DWORD PTR [edi+28]
		fxch	st(2)
		fadd	DWORD PTR [edi+44]
		fxch	st(1)
		fstp	DWORD PTR [ebx]
		fxch	st(1)
		fstp	DWORD PTR [ebx+4]
		fstp	DWORD PTR [ebx+8]
	}
}
#else
static void TranslatePoint(const float *source, float *dest, const float *matrix)
{
	dest[0] = matrix[ 0] * source[0] + matrix[ 1] * source[1] + matrix[ 2] * source[2] + matrix[ 3];
	dest[1] = matrix[ 4] * source[0] + matrix[ 5] * source[1] + matrix[ 6] * source[2] + matrix[ 7];
	dest[2] = matrix[ 8] * source[0] + matrix[ 9] * source[1] + matrix[10] * source[2] + matrix[11];
}
#endif

void TranslatePointIntoViewspace(VECTORCH *pointPtr)
{
	Source[0] = pointPtr->vx;
	Source[1] = pointPtr->vy;
	Source[2] = pointPtr->vz;

	TranslatePoint(Source, Dest, ViewMatrix);

	f2i(pointPtr->vx,Dest[0]);
	f2i(pointPtr->vy,Dest[1]);
	f2i(pointPtr->vz,Dest[2]);
}

void SquishPoints(SHAPEINSTR *shapeinstrptr)
{
	int **shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	VECTORCH *shapePts      = (VECTORCH*)*shapeitemarrayptr;
	{
		int i;
		int scale = Global_ODB_Ptr->ObFlags2;
		
		for (i=0; i<Global_ShapeHeaderPtr->numpoints; i++)
		{
			VECTORCH point = shapePts[i];

			RotateVector(&point,&Global_ODB_Ptr->ObMat);

			point.vx = MUL_FIXED(point.vx,ONE_FIXED*3/2 - scale/2);
			point.vx += Global_ODB_Ptr->ObWorld.vx;

			point.vz = MUL_FIXED(point.vz,ONE_FIXED*3/2 - scale/2);
			point.vz += Global_ODB_Ptr->ObWorld.vz;

			point.vy += Global_ODB_Ptr->ObWorld.vy;
			point.vy = HierarchicalObjectsLowestYValue + MUL_FIXED(point.vy-HierarchicalObjectsLowestYValue, scale);

			Source[0] = point.vx;
			Source[1] = point.vy;
			Source[2] = point.vz;

			TranslatePoint(Source, Dest, ViewMatrix);

			f2i(RotatedPts[i].vx,Dest[0]);
			f2i(RotatedPts[i].vy,Dest[1]);
			f2i(RotatedPts[i].vz,Dest[2]);
		}	
	}
}

void MorphPoints(SHAPEINSTR *shapeinstrptr)
{
	VECTORCH *srcPtr;
	{
		SHAPEHEADER *shape1Ptr;
		VECTORCH *shape1PointsPtr;
		VECTORCH *shape2PointsPtr;
						
		/* Set up the morph data */
		GetMorphDisplay(&MorphDisplay, Global_ODB_Ptr);

		shape1Ptr = MorphDisplay.md_sptr1;

		if(MorphDisplay.md_lerp == 0x0000)
		{
			
			srcPtr = (VECTORCH *)*shape1Ptr->points;
		}
		else if(MorphDisplay.md_lerp == 0xffff)
		{
			SHAPEHEADER *shape2Ptr;
			shape2Ptr = MorphDisplay.md_sptr2;
			
			srcPtr = (VECTORCH *)*shape2Ptr->points;
			Global_ShapePoints    = *(shape2Ptr->points);
				
		}
		else
		{
			SHAPEHEADER *shape2Ptr;
			shape2Ptr = MorphDisplay.md_sptr2;
		    
			shape1PointsPtr = (VECTORCH *)(*shape1Ptr->points);
			shape2PointsPtr = (VECTORCH *)(*shape2Ptr->points);

			{
		    	int numberOfPoints = shape1Ptr->numpoints;
				VECTORCH *morphedPointsPtr = (VECTORCH *) MorphedPts;
		    
				while(numberOfPoints--)
				{
				   	VECTORCH vertex1 = *shape1PointsPtr;
				   	VECTORCH vertex2 = *shape2PointsPtr;
				
					if( (vertex1.vx == vertex2.vx && vertex1.vy == vertex2.vy && vertex1.vz == vertex2.vz) )
					{
						*morphedPointsPtr = vertex1;
					}
					else
					{
						/* KJL 15:27:20 05/22/97 - I've changed this to speed things up, If a vertex
						component has a magnitude greater than 32768 things will go wrong. */
						morphedPointsPtr->vx = vertex1.vx + (((vertex2.vx-vertex1.vx)*MorphDisplay.md_lerp)>>16);
						morphedPointsPtr->vy = vertex1.vy + (((vertex2.vy-vertex1.vy)*MorphDisplay.md_lerp)>>16);
						morphedPointsPtr->vz = vertex1.vz + (((vertex2.vz-vertex1.vz)*MorphDisplay.md_lerp)>>16);
					}

		            shape1PointsPtr++;
		            shape2PointsPtr++;
					morphedPointsPtr++;
				}
			}

			Global_ShapePoints = (int*)MorphedPts;
		    srcPtr = (VECTORCH *)MorphedPts;
		}
	}
	{
		VECTORCH *destPtr = RotatedPts;
		int i;
		for(i = shapeinstrptr->sh_numitems; i!=0; i--)
		{
			Source[0] = srcPtr->vx+Global_ODB_Ptr->ObWorld.vx;
			Source[1] = srcPtr->vy+Global_ODB_Ptr->ObWorld.vy;
			Source[2] = srcPtr->vz+Global_ODB_Ptr->ObWorld.vz;

			TranslatePoint(Source, Dest, ViewMatrix);
			
			f2i(destPtr->vx,Dest[0]);
			f2i(destPtr->vy,Dest[1]);
			f2i(destPtr->vz,Dest[2]);
			srcPtr++;
			destPtr++;
		}
	}

}

void TranslateShapeVertices(SHAPEINSTR *shapeinstrptr)
{
	VECTORCH *destPtr = RotatedPts;
	int **shapeitemarrayptr;
	VECTORCH *srcPtr;
	int i;
//	MNormalise(&LToVMat);
	shapeitemarrayptr = shapeinstrptr->sh_instr_data;
	srcPtr = (VECTORCH*)*shapeitemarrayptr;
	if (Global_ODB_Ptr->ObFlags & ObFlag_ArbRot)
	{
		for(i = shapeinstrptr->sh_numitems; i!=0; i--)
		{
			destPtr->vx = (srcPtr->vx+Global_ODB_Ptr->ObView.vx);
			destPtr->vy = ((srcPtr->vy+Global_ODB_Ptr->ObView.vy)*4)/3;
			destPtr->vz = (srcPtr->vz+Global_ODB_Ptr->ObView.vz);

			srcPtr++;
			destPtr++;

		}
	}
	else
	{
		ObjectViewMatrix[0+0*4] = (float)(Global_ODB_Ptr->ObMat.mat11)/65536.0f;
		ObjectViewMatrix[1+0*4] = (float)(Global_ODB_Ptr->ObMat.mat21)/65536.0f;
		ObjectViewMatrix[2+0*4] = (float)(Global_ODB_Ptr->ObMat.mat31)/65536.0f;

		ObjectViewMatrix[0+1*4] = (float)(Global_ODB_Ptr->ObMat.mat12)/(65536.0f);
		ObjectViewMatrix[1+1*4] = (float)(Global_ODB_Ptr->ObMat.mat22)/(65536.0f);
		ObjectViewMatrix[2+1*4] = (float)(Global_ODB_Ptr->ObMat.mat32)/(65536.0f);

		ObjectViewMatrix[0+2*4] = (float)(Global_ODB_Ptr->ObMat.mat13)/65536.0f;
		ObjectViewMatrix[1+2*4] = (float)(Global_ODB_Ptr->ObMat.mat23)/65536.0f;
		ObjectViewMatrix[2+2*4] = (float)(Global_ODB_Ptr->ObMat.mat33)/65536.0f;

		ObjectViewMatrix[3+0*4] = Global_ODB_Ptr->ObWorld.vx;
		ObjectViewMatrix[3+1*4] = Global_ODB_Ptr->ObWorld.vy;
		ObjectViewMatrix[3+2*4] = Global_ODB_Ptr->ObWorld.vz;
		for(i = shapeinstrptr->sh_numitems; i!=0; i--)
		{
			Source[0] = srcPtr->vx;
			Source[1] = srcPtr->vy;
			Source[2] = srcPtr->vz;

			TranslatePoint(Source, Dest, ObjectViewMatrix);
			TranslatePoint(Dest, Source, ViewMatrix);
			
			f2i(destPtr->vx,Source[0]);
			f2i(destPtr->vy,Source[1]);
			f2i(destPtr->vz,Source[2]);
			srcPtr++;
			destPtr++;
		}
	}
}

void RenderDecal(DECAL *decalPtr)
{
	/* translate decal into view space */
	{
		VECTORCH translatedPosition = decalPtr->Vertices[0];
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[0].X = translatedPosition.vx;
		VerticesBuffer[0].Y = translatedPosition.vy;
		VerticesBuffer[0].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[1];
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[1].X = translatedPosition.vx;
		VerticesBuffer[1].Y = translatedPosition.vy;
		VerticesBuffer[1].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[2];
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[2].X = translatedPosition.vx;
		VerticesBuffer[2].Y = translatedPosition.vy;
		VerticesBuffer[2].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[3];
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[3].X = translatedPosition.vx;
		VerticesBuffer[3].Y = translatedPosition.vy;
		VerticesBuffer[3].Z = translatedPosition.vz;
	}
	{
		int outcode = DecalWithinFrustrum(decalPtr);
										  
		if (outcode)
		{		 
			switch(decalPtr->DecalID)
			{
				default:
				case DECAL_SCORCHED:
				{
					DecalPolygon_Construct(decalPtr);

					if (outcode!=2)
					{
						TexturedPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) return;
						D3D_Decal_Output(decalPtr,RenderPolygon.Vertices);
		  			
		  			}
					else D3D_Decal_Output(decalPtr,VerticesBuffer);
					break;
				}
			}
		}
	}	
	#if MIRRORING_ON
	if (MirroringActive) RenderMirroredDecal(decalPtr);
	#endif
}
void RenderParticle(PARTICLE *particlePtr)
{
//	PARTICLE_DESC *particleDescPtr = &ParticleDescription[particlePtr->ParticleID];
	int particleSize = particlePtr->Size;
	
	{
		VECTORCH translatedPosition = particlePtr->Position;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[0].X = translatedPosition.vx;
		VerticesBuffer[3].X = translatedPosition.vx;
		VerticesBuffer[0].Y = translatedPosition.vy;
		VerticesBuffer[3].Y = translatedPosition.vy;
		VerticesBuffer[0].Z = translatedPosition.vz;
		VerticesBuffer[3].Z = translatedPosition.vz;
	}

	if ((particlePtr->ParticleID == PARTICLE_EXPLOSIONFIRE)
	  ||(particlePtr->ParticleID == PARTICLE_RICOCHET_SPARK)
	  ||(particlePtr->ParticleID == PARTICLE_SPARK)
	  ||(particlePtr->ParticleID == PARTICLE_ORANGE_SPARK)
	  ||(particlePtr->ParticleID == PARTICLE_ORANGE_PLASMA)
	  ||(particlePtr->ParticleID == PARTICLE_ALIEN_BLOOD)
	  ||(particlePtr->ParticleID == PARTICLE_PREDATOR_BLOOD)
	  ||(particlePtr->ParticleID == PARTICLE_HUMAN_BLOOD)
	  ||(particlePtr->ParticleID == PARTICLE_WATERFALLSPRAY)
	  ||(particlePtr->ParticleID == PARTICLE_LASERBEAM)
	  ||(particlePtr->ParticleID == PARTICLE_PLASMABEAM)
	  ||(particlePtr->ParticleID == PARTICLE_TRACER)
	  ||(particlePtr->ParticleID == PARTICLE_PREDPISTOL_FLECHETTE)
	  ||(particlePtr->ParticleID == PARTICLE_PREDPISTOL_FLECHETTE_NONDAMAGING)
	  )
	{
		VECTORCH translatedPosition = particlePtr->Offset;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[1].X = translatedPosition.vx;
		VerticesBuffer[2].X = translatedPosition.vx;
		VerticesBuffer[1].Y = translatedPosition.vy;
		VerticesBuffer[2].Y = translatedPosition.vy;
		VerticesBuffer[1].Z = translatedPosition.vz;
		VerticesBuffer[2].Z = translatedPosition.vz;
	
		{
			int deltaX = VerticesBuffer[1].X - VerticesBuffer[0].X;
			int deltaY = VerticesBuffer[1].Y - VerticesBuffer[0].Y;
			int splitY = 0;

			if (deltaX>=0)
			{	
				if (deltaY>=0)
				{
					if (deltaX>deltaY)
					{
						splitY = 1;
					}
				}
				else if (deltaX>-deltaY)
				{
					splitY = 1;
				}
			}
			else
			{	
				if (deltaY>=0)
				{
					if (-deltaX>deltaY)
					{
						splitY = 1;
					}
				}
				else if (-deltaX>-deltaY)
				{
					splitY = 1;
				}
			}
			if (splitY)
			{
				if (deltaX>0)
				{
					/* 1 & 2 are more +ve in X */
					VerticesBuffer[0].X -= particleSize;
					VerticesBuffer[0].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[1].X += particleSize;
					VerticesBuffer[1].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[2].X += particleSize;
					VerticesBuffer[2].Y += MUL_FIXED(particleSize,87381);
					VerticesBuffer[3].X -= particleSize;
					VerticesBuffer[3].Y += MUL_FIXED(particleSize,87381);
				}
				else
				{
					/* 1 & 2 are more -ve in X */
					VerticesBuffer[0].X += particleSize;
					VerticesBuffer[0].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[1].X -= particleSize;
					VerticesBuffer[1].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[2].X -= particleSize;
					VerticesBuffer[2].Y += MUL_FIXED(particleSize,87381);
					VerticesBuffer[3].X += particleSize;
					VerticesBuffer[3].Y += MUL_FIXED(particleSize,87381);
				}

			}
			else
			{
				if (deltaY>0)
				{
					/* 1 & 2 are more +ve in Y */
					VerticesBuffer[0].X -= particleSize;
					VerticesBuffer[0].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[1].X -= particleSize;
					VerticesBuffer[1].Y += MUL_FIXED(particleSize,87381);
					VerticesBuffer[2].X += particleSize;
					VerticesBuffer[2].Y += MUL_FIXED(particleSize,87381);
					VerticesBuffer[3].X += particleSize;
					VerticesBuffer[3].Y -= MUL_FIXED(particleSize,87381);
				}
				else
				{
					/* 1 & 2 are more -ve in Y */
					VerticesBuffer[0].X -= particleSize;
					VerticesBuffer[0].Y += MUL_FIXED(particleSize,87381);
					VerticesBuffer[1].X -= particleSize;
					VerticesBuffer[1].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[2].X += particleSize;
					VerticesBuffer[2].Y -= MUL_FIXED(particleSize,87381);
					VerticesBuffer[3].X += particleSize;
					VerticesBuffer[3].Y += MUL_FIXED(particleSize,87381);
				}

			}
		}
	}
	else
	{
		VECTOR2D offset[4];
		VerticesBuffer[1].X = VerticesBuffer[0].X;
		VerticesBuffer[2].X = VerticesBuffer[0].X;
		VerticesBuffer[1].Y = VerticesBuffer[0].Y;
		VerticesBuffer[2].Y = VerticesBuffer[0].Y;
		VerticesBuffer[1].Z = VerticesBuffer[0].Z;
		VerticesBuffer[2].Z = VerticesBuffer[0].Z;

		offset[0].vx = -particleSize;
		offset[0].vy = -particleSize;

		offset[1].vx = +particleSize;
		offset[1].vy = -particleSize;

		offset[2].vx = +particleSize;
		offset[2].vy = +particleSize;

		offset[3].vx = -particleSize;
		offset[3].vy = +particleSize;

		if ((particlePtr->ParticleID == PARTICLE_MUZZLEFLASH) )
		{
			int theta = FastRandom()&4095;
			RotateVertex(&offset[0],theta);
			RotateVertex(&offset[1],theta);
			RotateVertex(&offset[2],theta);
			RotateVertex(&offset[3],theta);
		}
		else if ((particlePtr->ParticleID == PARTICLE_SMOKECLOUD)
			||(particlePtr->ParticleID == PARTICLE_GUNMUZZLE_SMOKE)
			||(particlePtr->ParticleID == PARTICLE_PARGEN_FLAME) 
			||(particlePtr->ParticleID == PARTICLE_FLAME)) 
		{
			int theta = (particlePtr->Offset.vx+MUL_FIXED(CloakingPhase,particlePtr->Offset.vy))&4095;
			RotateVertex(&offset[0],theta);
			RotateVertex(&offset[1],theta);
			RotateVertex(&offset[2],theta);
			RotateVertex(&offset[3],theta);
		}
		VerticesBuffer[0].X += offset[0].vx;
		VerticesBuffer[0].Y += MUL_FIXED(offset[0].vy,87381);
		
		VerticesBuffer[1].X += offset[1].vx;
		VerticesBuffer[1].Y += MUL_FIXED(offset[1].vy,87381);

		VerticesBuffer[2].X += offset[2].vx;
		VerticesBuffer[2].Y += MUL_FIXED(offset[2].vy,87381);
		
		VerticesBuffer[3].X += offset[3].vx;
		VerticesBuffer[3].Y += MUL_FIXED(offset[3].vy,87381);
	
	}
	
	{
		int outcode = QuadWithinFrustrum();
										  
		if (outcode)
		{		 
			ParticlePolygon_Construct(particlePtr);

			if (outcode!=2)
			{
				TexturedPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) return;
				D3D_Particle_Output(particlePtr,RenderPolygon.Vertices);
  			
  			}
			else D3D_Particle_Output(particlePtr,VerticesBuffer);
		}
	}	
}

extern void RenderFlechetteParticle(PARTICLE *particlePtr)
{
	VECTORCH vertices[5];
	MATRIXCH mat;
	
	MakeMatrixFromDirection(&particlePtr->Velocity,&mat);

	mat.mat11 >>= 12;
	mat.mat12 >>= 12;
	mat.mat13 >>= 12;
	mat.mat21 >>= 12;
	mat.mat22 >>= 12;
	mat.mat23 >>= 12;
	mat.mat31 >>= 9;
	mat.mat32 >>= 9;
	mat.mat33 >>= 9;

	
	vertices[0].vx = particlePtr->Position.vx-mat.mat31+mat.mat11;
	vertices[0].vy = particlePtr->Position.vy-mat.mat32+mat.mat12;
	vertices[0].vz = particlePtr->Position.vz-mat.mat33+mat.mat13;
	
	vertices[1].vx = particlePtr->Position.vx-mat.mat31-mat.mat11;
	vertices[1].vy = particlePtr->Position.vy-mat.mat32-mat.mat12;
	vertices[1].vz = particlePtr->Position.vz-mat.mat33-mat.mat13;

	vertices[2] = particlePtr->Position;

	vertices[3].vx = particlePtr->Position.vx-mat.mat31+mat.mat21;
	vertices[3].vy = particlePtr->Position.vy-mat.mat32+mat.mat22;
	vertices[3].vz = particlePtr->Position.vz-mat.mat33+mat.mat23;
	
	vertices[4].vx = particlePtr->Position.vx-mat.mat31-mat.mat21;
	vertices[4].vy = particlePtr->Position.vy-mat.mat32-mat.mat22;
	vertices[4].vz = particlePtr->Position.vz-mat.mat33-mat.mat23;

	TranslatePointIntoViewspace(&vertices[0]);
	TranslatePointIntoViewspace(&vertices[1]);
	TranslatePointIntoViewspace(&vertices[2]);
	TranslatePointIntoViewspace(&vertices[3]);
	TranslatePointIntoViewspace(&vertices[4]);

	{
		int i;
		for (i=0; i<3; i++) 
		{
			VerticesBuffer[i].X	= vertices[i].vx;
			VerticesBuffer[i].Y	= vertices[i].vy;
			VerticesBuffer[i].Z	= vertices[i].vz;

			VerticesBuffer[i].A = (particlePtr->Colour>>24)&255;
			VerticesBuffer[i].R = (particlePtr->Colour>>16)&255;
			VerticesBuffer[i].G	= (particlePtr->Colour>>8)&255;
			VerticesBuffer[i].B = (particlePtr->Colour)&255;
		}
		RenderPolygon.NumberOfVertices=3;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
	}	
	{
		int outcode = TriangleWithinFrustrum();
		POLYHEADER fakeHeader;
		fakeHeader.PolyFlags  = iflag_transparent;

		do
		{
		if (outcode)
		{		 
			if (outcode!=2)
			{
				GouraudPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) continue;
				D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
  			}
			else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
		}
		}
		while(0);
	}
	{
		int i;
		for (i=0; i<3; i++) 
		{
			VerticesBuffer[i].X	= vertices[i+2].vx;
			VerticesBuffer[i].Y	= vertices[i+2].vy;
			VerticesBuffer[i].Z	= vertices[i+2].vz;

			VerticesBuffer[i].A = (particlePtr->Colour>>24)&255;
			VerticesBuffer[i].R = (particlePtr->Colour>>16)&255;
			VerticesBuffer[i].G	= (particlePtr->Colour>>8)&255;
			VerticesBuffer[i].B = (particlePtr->Colour)&255;
		}
		RenderPolygon.NumberOfVertices=3;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
	}	
	{
		int outcode = TriangleWithinFrustrum();
		POLYHEADER fakeHeader;
		fakeHeader.PolyFlags  = iflag_transparent;

		do
		{
		if (outcode)
		{		 
			if (outcode!=2)
			{
				GouraudPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) continue;
				GouraudPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) continue;
				D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
  			}
			else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
		}
		}
		while(0);
	}

}

static void ParticlePolygon_Construct(PARTICLE *particlePtr)
{
	PARTICLE_DESC *particleDescPtr = &ParticleDescription[particlePtr->ParticleID];
	RenderPolygon.NumberOfVertices=4;
	
	VerticesBuffer[0].U = particleDescPtr->StartU;
	VerticesBuffer[0].V = particleDescPtr->StartV;

	VerticesBuffer[1].U = particleDescPtr->EndU;
	VerticesBuffer[1].V = particleDescPtr->StartV;

	VerticesBuffer[2].U = particleDescPtr->EndU;
	VerticesBuffer[2].V = particleDescPtr->EndV;

	VerticesBuffer[3].U = particleDescPtr->StartU;
	VerticesBuffer[3].V = particleDescPtr->EndV;

}

void RenderMirroredDecal(DECAL *decalPtr)
{
	/* translate decal into view space */
	{
		VECTORCH translatedPosition = decalPtr->Vertices[0];
		translatedPosition.vx = MirroringAxis - translatedPosition.vx;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[0].X = translatedPosition.vx;
		VerticesBuffer[0].Y = translatedPosition.vy;
		VerticesBuffer[0].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[1];
		translatedPosition.vx = MirroringAxis - translatedPosition.vx;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[1].X = translatedPosition.vx;
		VerticesBuffer[1].Y = translatedPosition.vy;
		VerticesBuffer[1].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[2];
		translatedPosition.vx = MirroringAxis - translatedPosition.vx;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[2].X = translatedPosition.vx;
		VerticesBuffer[2].Y = translatedPosition.vy;
		VerticesBuffer[2].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = decalPtr->Vertices[3];
		translatedPosition.vx = MirroringAxis - translatedPosition.vx;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[3].X = translatedPosition.vx;
		VerticesBuffer[3].Y = translatedPosition.vy;
		VerticesBuffer[3].Z = translatedPosition.vz;
	}
	{
		int outcode = DecalWithinFrustrum(decalPtr);
										  
		if (outcode)
		{		 
			switch(decalPtr->DecalID)
			{
				default:
				case DECAL_SCORCHED:
				{
					DecalPolygon_Construct(decalPtr);

					if (outcode!=2)
					{
						TexturedPolygon_ClipWithZ();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithNegativeX();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithPositiveY();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices<3) return;
						TexturedPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices<3) return;
						D3D_Decal_Output(decalPtr,RenderPolygon.Vertices);
		  			
		  			}
					else D3D_Decal_Output(decalPtr,VerticesBuffer);
					break;
				}
			}
		}
	}	
}


static void DecalPolygon_Construct(DECAL *decalPtr)
{
	DECAL_DESC *decalDescPtr = &DecalDescription[decalPtr->DecalID];
	RenderPolygon.NumberOfVertices=4;
	
	VerticesBuffer[0].U = decalDescPtr->StartU+decalPtr->UOffset;
	VerticesBuffer[0].V = decalDescPtr->StartV;

	VerticesBuffer[1].U = decalDescPtr->EndU+decalPtr->UOffset;
	VerticesBuffer[1].V = decalDescPtr->StartV;

	VerticesBuffer[2].U = decalDescPtr->EndU+decalPtr->UOffset;
	VerticesBuffer[2].V = decalDescPtr->EndV;

	VerticesBuffer[3].U = decalDescPtr->StartU+decalPtr->UOffset;
	VerticesBuffer[3].V = decalDescPtr->EndV;

}

#if 0
	int polys[][4] =
	{
		{1,3,5,7},
		{1,3,9,11},
		{2,3,5,4},
		{2,3,9,8},

		{0,2,4,6},
		{0,2,8,10},
		{0,1,7,6},
		{0,1,11,10},
		
		{12,13,15,14},
 		{4,5,7,6},

		{8,9,5,4},
		{6,7,11,10},			   
		{8,4,6,10},
		{5,9,11,7},
		
//		{8,2,4,4},
//		{9,3,5,5},
//		{10,0,6,6},
//		{7,1,11,11}
	};
	int alphaValue[]={64,64,64,64, 16,16,16,16, 0,0,0,0, 128,128,128,128};

//void RenderShaftOfLight(SHAFTOFLIGHT *shaftPtr)
void RenderShaftOfLight(MODULE *modulePtr)
{
	/* translate shaft into view space */
 
//	suitable for invasion2
	VECTORCH shaftVertices[]=
		{
			{-5500,9000,9822},
			{-4500,9000,9822},
			{-5500,10000,9822},
			{-4500,10000,9822},

			{-6000,15900,13000},
			{-4000,15900,13000},
			{-6000,15900,15500},
			{-4000,15900,15500},
		
			{-6500,15900,12500},
			{-3500,15900,12500},
			{-6500,15900,16000},
			{-3500,15900,16000},
		};
   /*
	VECTORCH shaftVertices[]=
		{
			{0,		0,		0},
			{0,		0,		1139},
			{-1338,	7113,	0},
			{-1338,	7113,	1139},


			{,1948,}

			{26002+ -1700,	7781 ,			-25328+ -800},
			{26002+ -900,	7781 ,			-25328+ +800},
			{26002+ 1700,	7781 ,			-25328+ -800},
			{26002+ 1700,	7781 ,			-25328+ +800},
		
			{26002+ -1100,	7781 ,			-25328+ -1000},
			{26002+ -1100,	7781 ,			-25328+ +1000},
			{26002+ 2000,	7781 ,			-25328+ -1000},
			{26002+ 2000,	7781 ,			-25328+ +1000},

		};
  */
	VECTORCH translatedPts[12];
	
	POLYHEADER fakeHeader;
	DECAL fakeDecal;
	int polyNumber;

	fakeDecal.ModuleIndex=148;
//	fakeDecal.ModuleIndex=17;
	fakeHeader.PolyFlags = iflag_transparent;

	{
		int i = 11;
		do
		{
			translatedPts[i] = shaftVertices[i];
//			translatedPts[i].vx+=10712; 
//			translatedPts[i].vy+=-6480; 
//			translatedPts[i].vz+=-25898; 
			TranslatePointIntoViewspace(&translatedPts[i]);
		}
	   	while(i--);
   	}
	
	for(polyNumber=0; polyNumber<10; polyNumber++)
	{
		{
			int i;
			for (i=0; i<4; i++) 
			{
				int v = polys[polyNumber][i];
				VerticesBuffer[i].A = alphaValue[v];
				if (v>11)
				{
					VerticesBuffer[i].X	= translatedPts[v-12].vx;
					VerticesBuffer[i].Y	= translatedPts[v-12].vy;
					VerticesBuffer[i].Z	= translatedPts[v-12].vz;
				}
				else
				{
					VerticesBuffer[i].X	= translatedPts[v].vx;
					VerticesBuffer[i].Y	= translatedPts[v].vy;
					VerticesBuffer[i].Z	= translatedPts[v].vz;
				}
				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 255;
				VerticesBuffer[i].B = 192;
			}
			RenderPolygon.NumberOfVertices=4;
		}
		{
			int outcode = DecalWithinFrustrum(&fakeDecal);
											  
			if (outcode)
			{		 
				if (outcode!=2)
				{
					GouraudPolygon_ClipWithZ();
					if(RenderPolygon.NumberOfVertices<3) return;
					GouraudPolygon_ClipWithNegativeX();
					if(RenderPolygon.NumberOfVertices<3) return;
					GouraudPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices<3) return;
					GouraudPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices<3) return;
					GouraudPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices<3) return;
					D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
	  			}
				else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
			}
		}
	}	
}
#else
	int polys[][4] =
	{
		{0,1,3,2},
		{2,3,5,4},
		{4,5,7,6},

		{1,3,5,7},
		{0,2,4,6},
		{6,8,10,0},
		{7,9,11,1},

		{0,1,11,10},
		{6,7,9,8},


	};
	int alphaValue[]={32,32,32,32, 28,28,28,28, 4,4,4,4, 128,128,128,128};
//	int alphaValue[]={16,16,16,16, 14,14,14,14, 2,2,2,2, 128,128,128,128};
	VECTORCH shaftVertices[]=
	{
			{0,		0,		0},
			{0,		0,		1956},
			{-1492,	7969,	0},
			{-1492,	7969,	1956},

//			{0,		0,		0},
//			{0,		0,		1139},
//			{-1338,	7113,	0},
//			{-1338,	7113,	1139},

			{0,		8840,	0},
			{0,		8840,	0},
			{3138, 	8850,	0},
			{3138, 	8850,	0},

			{0,		14500,	0},
			{0,		14500,	0},
			{0, 	14500,	0},
			{0, 	14500,	0},


	};

void RenderShaftOfLight(MODULE *modulePtr)
{
	/* translate shaft into view space */
	#define NUM_OF_SHAFT_VERTICES 12
	VECTORCH translatedPts[NUM_OF_SHAFT_VERTICES];
	
	POLYHEADER fakeHeader;
	DECAL fakeDecal;
	int polyNumber;
	VECTORCH lightDirection1 = {29309,29309,-2000};
	VECTORCH lightDirection2 = {29309,29309,2000};

//	return;
	{
		int offset = GetSin((CloakingPhase/16)&4095);
		if (offset<0) offset=-offset;
		offset = MUL_FIXED(offset,offset);
		offset = MUL_FIXED(offset,offset);
		offset=MUL_FIXED(offset,4000);
		lightDirection1.vz += offset;
		lightDirection2.vz += offset;
	}
	Normalise(&lightDirection1);
	Normalise(&lightDirection2);
//	textprint("light shaft active");
	fakeDecal.ModuleIndex=modulePtr->m_index;
	fakeHeader.PolyFlags = iflag_transparent;
	

	FindIntersectionWithYPlane(&shaftVertices[2],&lightDirection1,&shaftVertices[4]);
	FindIntersectionWithYPlane(&shaftVertices[3],&lightDirection2,&shaftVertices[5]);
	FindZFromXYIntersection(&shaftVertices[2],&lightDirection1,&shaftVertices[6]);
	FindZFromXYIntersection(&shaftVertices[3],&lightDirection2,&shaftVertices[7]);
	FindIntersectionWithYPlane(&shaftVertices[0],&lightDirection1,&shaftVertices[10]);
	FindIntersectionWithYPlane(&shaftVertices[1],&lightDirection2,&shaftVertices[11]);
	FindIntersectionWithYPlane(&shaftVertices[6],&lightDirection1,&shaftVertices[8]);
	FindIntersectionWithYPlane(&shaftVertices[7],&lightDirection2,&shaftVertices[9]);
	
	{

		int i = NUM_OF_SHAFT_VERTICES-1;
		do
		{
			translatedPts[i] = shaftVertices[i];
			translatedPts[i].vx+=11762; 
			translatedPts[i].vy+=-6919; 
			translatedPts[i].vz+=-26312; 
//			translatedPts[i].vx+=10712; 
//			translatedPts[i].vy+=-6480; 
//			translatedPts[i].vz+=-25898; 
			TranslatePointIntoViewspace(&translatedPts[i]);
		}
	   	while(i--);
   	}
	

	for(polyNumber=0; polyNumber<9; polyNumber++)
	{
		{
			int i;
			for (i=0; i<4; i++) 
			{
				int v = polys[polyNumber][i];
				VerticesBuffer[i].A = alphaValue[v];
				VerticesBuffer[i].X	= translatedPts[v].vx;
				VerticesBuffer[i].Y	= translatedPts[v].vy;
				VerticesBuffer[i].Z	= translatedPts[v].vz;
				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 255;
				VerticesBuffer[i].B = 192;
			}
			RenderPolygon.NumberOfVertices=4;
		}
		{
			int outcode = DecalWithinFrustrum(&fakeDecal);
											  
			if (outcode)
			{		 
				if (outcode!=2)
				{
					GouraudPolygon_ClipWithZ();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
	  			}
				else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
			}
		}
	}	
	RenderShaftOfLight2(modulePtr);
}
void RenderShaftOfLight2(MODULE *modulePtr)
{
	/* translate shaft into view space */
	#define NUM_OF_SHAFT_VERTICES 12
	VECTORCH translatedPts[NUM_OF_SHAFT_VERTICES];
	
	POLYHEADER fakeHeader;
	DECAL fakeDecal;
	int polyNumber;
	VECTORCH lightDirection1 = {29309+1000,29309,-3000};
	VECTORCH lightDirection2 = {29309+1000,29309,3000};
	VECTORCH lightDirection3 = {29309-1000,29309,-3000};
	VECTORCH lightDirection4 = {29309-1000,29309,3000};
	{
		int offset = GetSin((CloakingPhase/16)&4095);
		if (offset<0) offset=-offset;
		offset = MUL_FIXED(offset,offset);
		offset = MUL_FIXED(offset,offset);
		offset=MUL_FIXED(offset,4000);
		lightDirection1.vz += offset;
		lightDirection2.vz += offset;
		lightDirection3.vz += offset;
		lightDirection4.vz += offset;
	}
	Normalise(&lightDirection1);
	Normalise(&lightDirection2);
	Normalise(&lightDirection3);
	Normalise(&lightDirection4);

	fakeDecal.ModuleIndex=modulePtr->m_index;
	fakeHeader.PolyFlags = iflag_transparent;
	

	FindIntersectionWithYPlane(&shaftVertices[2],&lightDirection1,&shaftVertices[4]);
	FindIntersectionWithYPlane(&shaftVertices[3],&lightDirection2,&shaftVertices[5]);
	FindZFromXYIntersection(&shaftVertices[2],&lightDirection3,&shaftVertices[6]);
	FindZFromXYIntersection(&shaftVertices[3],&lightDirection4,&shaftVertices[7]);
	FindIntersectionWithYPlane(&shaftVertices[0],&lightDirection1,&shaftVertices[10]);
	FindIntersectionWithYPlane(&shaftVertices[1],&lightDirection2,&shaftVertices[11]);
	FindIntersectionWithYPlane(&shaftVertices[6],&lightDirection3,&shaftVertices[8]);
	FindIntersectionWithYPlane(&shaftVertices[7],&lightDirection4,&shaftVertices[9]);

	
	{

		int i = NUM_OF_SHAFT_VERTICES-1;
		do
		{
			translatedPts[i] = shaftVertices[i];
			translatedPts[i].vx+=11762; 
			translatedPts[i].vy+=-6919; 
			translatedPts[i].vz+=-26312; 
//			translatedPts[i].vx+=10712; 
//			translatedPts[i].vy+=-6480; 
//			translatedPts[i].vz+=-25898; 
			TranslatePointIntoViewspace(&translatedPts[i]);
		}
	   	while(i--);
   	}
	

	for(polyNumber=0; polyNumber<9; polyNumber++)
	{
		{
			int i;
			for (i=0; i<4; i++) 
			{
				int v = polys[polyNumber][i];
				VerticesBuffer[i].A = alphaValue[v]/2;
				VerticesBuffer[i].X	= translatedPts[v].vx;
				VerticesBuffer[i].Y	= translatedPts[v].vy;
				VerticesBuffer[i].Z	= translatedPts[v].vz;
				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 255;
				VerticesBuffer[i].B = 192;
			}
			RenderPolygon.NumberOfVertices=4;
		}
		{
			int outcode = DecalWithinFrustrum(&fakeDecal);
											  
			if (outcode)
			{		 
				if (outcode!=2)
				{
					GouraudPolygon_ClipWithZ();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices<3) continue;
					GouraudPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices<3) continue;
					D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
	  			}
				else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
			}
		}
	}	
}

void FindIntersectionWithYPlane(VECTORCH *startPtr, VECTORCH *directionPtr, VECTORCH *intersectionPtr)
{
	int lambda = DIV_FIXED(intersectionPtr->vy - startPtr->vy,directionPtr->vy);

	intersectionPtr->vx = startPtr->vx + MUL_FIXED(directionPtr->vx,lambda);
	intersectionPtr->vz = startPtr->vz + MUL_FIXED(directionPtr->vz,lambda);
//	textprint("%d %d %d\n",intersectionPtr->vx,intersectionPtr->vy,intersectionPtr->vz);
}
void FindZFromXYIntersection(VECTORCH *startPtr, VECTORCH *directionPtr, VECTORCH *intersectionPtr)
{
	float a = intersectionPtr->vx - startPtr->vx;
	
	a/=directionPtr->vx;
	
	intersectionPtr->vz = startPtr->vz + (directionPtr->vz*a);
//	textprint("%d %d %d\n",intersectionPtr->vx,intersectionPtr->vy,intersectionPtr->vz);
}

#endif












void ClearTranslucentPolyList(void)
{
	CurrentNumberOfTranslucentPolygons=0;
}

void AddToTranslucentPolyList(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	/* copy the data to the list for processing later */
	int i = RenderPolygon.NumberOfVertices;
	int maxZ = 0;
	RENDERVERTEX *vertexPtr = TranslucentPolygons[CurrentNumberOfTranslucentPolygons].Vertices;

	TranslucentPolygons[CurrentNumberOfTranslucentPolygons].NumberOfVertices = i;
	
	do
	{
		if (maxZ<renderVerticesPtr->Z)
			maxZ = renderVerticesPtr->Z;
		*vertexPtr++ = *renderVerticesPtr++;
	}
	while(--i);
	TranslucentPolygons[CurrentNumberOfTranslucentPolygons].MaxZ = maxZ;
	TranslucentPolygonHeaders[CurrentNumberOfTranslucentPolygons] = *inputPolyPtr;

	/* increment counter */
	CurrentNumberOfTranslucentPolygons++;
	LOCALASSERT(CurrentNumberOfTranslucentPolygons<MAX_NO_OF_TRANSLUCENT_POLYGONS);
}

void OutputTranslucentPolyList(void)
{
	int i = CurrentNumberOfTranslucentPolygons;
	while(i--)
	{
		int k = CurrentNumberOfTranslucentPolygons;
		int maxFound = 0;
		while(k--)
		{
			if (TranslucentPolygons[k].MaxZ>TranslucentPolygons[maxFound].MaxZ)
			{
				maxFound = k;
			}
		}
		
		RenderAllParticlesFurtherAwayThan(TranslucentPolygons[maxFound].MaxZ);

		RenderPolygon.NumberOfVertices = TranslucentPolygons[maxFound].NumberOfVertices;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
		D3D_ZBufferedGouraudTexturedPolygon_Output(&TranslucentPolygonHeaders[maxFound],TranslucentPolygons[maxFound].Vertices);
		TranslucentPolygons[maxFound].MaxZ=0;
	}
	
	RenderAllParticlesFurtherAwayThan(-0x7fffffff);
	
}


static const int CuboidPolyVertexU[][4] =
{
	{1,1,1,1},
	
	{127,127,0,0},	 
	{128,128,255,255},	 

	{127,127,0,0},	 
	{128,128,255,255},	 
};
static const int CuboidPolyVertexV[][4] =
{
	{1,1,1,1},	
	
	{127,0,0,127},	 	
	{127,0,0,127},	
	{128,255,255,128},	
	{128,255,255,128},	
};


void RenderMirrorSurface(void)
{
	VECTORCH translatedPts[4] =
	{
		{-5596,-932,-1872},
		{-5596,-932,-702},
		{-5596,1212,-702},
		{-5596,1212,-1872},
			
	};
 	int mirrorUV[]=
	{ 0,0, 127<<16,0, 127<<16,127<<16, 0,127<<16};
 	POLYHEADER fakeHeader;


	{
		extern int CloudyImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = CloudyImageNumber;
	}

 	{
		int i;
		for (i=0; i<4; i++) 
		{
			VerticesBuffer[i].A = 128;

			TranslatePointIntoViewspace(&translatedPts[i]);
			VerticesBuffer[i].X	= translatedPts[i].vx;
			VerticesBuffer[i].Y	= translatedPts[i].vy;
			VerticesBuffer[i].Z	= translatedPts[i].vz;
			VerticesBuffer[i].U = mirrorUV[i*2];
			VerticesBuffer[i].V = mirrorUV[i*2+1];


			VerticesBuffer[i].R = 255;
			VerticesBuffer[i].G	= 255;
			VerticesBuffer[i].B = 255;
			VerticesBuffer[i].SpecularR = 0;
			VerticesBuffer[i].SpecularG = 0;
			VerticesBuffer[i].SpecularB = 0;
			
		}
		RenderPolygon.NumberOfVertices=4;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_COLOUR;
	}
			
	GouraudTexturedPolygon_ClipWithZ();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeX();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveX();
	if(RenderPolygon.NumberOfVertices<3) return;
	D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
}
void RenderMirrorSurface2(void)
{
	VECTORCH translatedPts[4] =
	{
		{-5596,-592,562},
		{-5596,-592,1344},
		{-5596,140,1344},
		{-5596,140,562},
			
	};
 	int mirrorUV[]=
	{ 0,0, 127<<16,0, 127<<16,127<<16, 0,127<<16};
 	POLYHEADER fakeHeader;


	{
		extern int CloudyImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = CloudyImageNumber;
	}

 	{
		int i;
		for (i=0; i<4; i++) 
		{
			VerticesBuffer[i].A = 128;

			TranslatePointIntoViewspace(&translatedPts[i]);
			VerticesBuffer[i].X	= translatedPts[i].vx;
			VerticesBuffer[i].Y	= translatedPts[i].vy;
			VerticesBuffer[i].Z	= translatedPts[i].vz;
			VerticesBuffer[i].U = mirrorUV[i*2];
			VerticesBuffer[i].V = mirrorUV[i*2+1];


			VerticesBuffer[i].R = 255;
			VerticesBuffer[i].G	= 255;
			VerticesBuffer[i].B = 255;
			VerticesBuffer[i].SpecularR = 0;
			VerticesBuffer[i].SpecularG = 0;
			VerticesBuffer[i].SpecularB = 0;

		}
		RenderPolygon.NumberOfVertices=4;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_COLOUR;
	}
			
	GouraudTexturedPolygon_ClipWithZ();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeX();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithNegativeY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudTexturedPolygon_ClipWithPositiveX();
	if(RenderPolygon.NumberOfVertices<3) return;
	D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
}
void RenderSmokeTest(void)
{
 	int mirrorUV[]=
	{
		64<<16, 0,
		64<<16, 31<<16,
		95<<16, 31<<16,
		95<<16, 0
	};
 	POLYHEADER fakeHeader;
	int a = GetSin(CloakingPhase&4095);
	int image;

	a = MUL_FIXED(MUL_FIXED(a,a),255);
	{
		extern int SpecialFXImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = SpecialFXImageNumber;
	}

 	for (image = 0; image<=1; image++)
 	{
	 	{
			VECTORCH translatedPts[4] =
			{
				{45300,0+-1000, 26000+-1000},
				{45300,0+-1000, 26000+ 1000},
				{45300,0+ 1000, 26000+ 1000},
				{45300,0+ 1000, 26000+-1000},
					
			};

			int i;

			if (image) a = 255-a;
			for (i=0; i<4; i++) 
			{
				VerticesBuffer[i].A = a/2;

				TranslatePointIntoViewspace(&translatedPts[i]);
				VerticesBuffer[i].X	= translatedPts[i].vx;
				VerticesBuffer[i].Y	= translatedPts[i].vy;
				VerticesBuffer[i].Z	= translatedPts[i].vz;
				VerticesBuffer[i].U = mirrorUV[i*2];
				VerticesBuffer[i].V = mirrorUV[i*2+1]+image*(32<<16);


				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 255;
				VerticesBuffer[i].B = 255;
				VerticesBuffer[i].SpecularR = 0;
				VerticesBuffer[i].SpecularG = 0;
				VerticesBuffer[i].SpecularB = 0;

			}
			RenderPolygon.NumberOfVertices=4;
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;
		}
				
		GouraudTexturedPolygon_ClipWithZ();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudTexturedPolygon_ClipWithNegativeX();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudTexturedPolygon_ClipWithPositiveY();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudTexturedPolygon_ClipWithNegativeY();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudTexturedPolygon_ClipWithPositiveX();
		if(RenderPolygon.NumberOfVertices<3) return;
		D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
	}
}
#if 1
#define OCTAVES 3
int u[OCTAVES];
int v[OCTAVES];
int du[OCTAVES];
int dv[OCTAVES];
int setup=0;

int SkyColour_R=200;
int SkyColour_G=200;
int SkyColour_B=200;
void RenderSky(void)
{
   	POLYHEADER fakeHeader;
	int x,z,o;
	if(!setup)
	{
		int i;
		setup=1;
		for(i=0;i<OCTAVES;i++)
		{
			u[i] = (FastRandom()&65535)*128;
			v[i] = (FastRandom()&65535)*128;
			du[i] = ( ((FastRandom()&65535)-32768)*(i+1) )*8;
			dv[i] = ( ((FastRandom()&65535)-32768)*(i+1) )*8;
		}											   
	}
	{
		extern int CloudyImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = CloudyImageNumber;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;
	}
	for (o=0; o<OCTAVES; o++)
	{
		u[o]+=MUL_FIXED(du[o],NormalFrameTime);
		v[o]+=MUL_FIXED(dv[o],NormalFrameTime);
	}
	for(x=-10; x<=10; x++)
	{
	for(z=-10; z<=10; z++)
	{
		int t = 255;
		int size = 65536*128;
	for (o=0; o<OCTAVES; o++)
	{
	 	{
		   VECTORCH translatedPts[4] =
			{
				{-1024,-1000,-1024},
				{-1024,-1000, 1024},
				{ 1024,-1000, 1024},
				{ 1024,-1000,-1024},
					
			};			
			int i;
			for (i=0; i<4; i++) 
			{
				VerticesBuffer[i].A = t;
				translatedPts[i].vx += 2048*x;//+(Global_VDB_Ptr->VDB_World.vx*7)/8;
				translatedPts[i].vz += 2048*z;//+(Global_VDB_Ptr->VDB_World.vz*7)/8;
//				RotateVector(&translatedPts[i],&(Global_VDB_Ptr->VDB_Mat));
//				translatedPts[i].vy = MUL_FIXED(translatedPts[i].vy,87381);
				translatedPts[i].vx += Global_VDB_Ptr->VDB_World.vx;
				translatedPts[i].vy += Global_VDB_Ptr->VDB_World.vy;
				translatedPts[i].vz += Global_VDB_Ptr->VDB_World.vz;
				TranslatePointIntoViewspace(&translatedPts[i]);
				VerticesBuffer[i].X	= translatedPts[i].vx;
				VerticesBuffer[i].Y	= translatedPts[i].vy;
				VerticesBuffer[i].Z	= translatedPts[i].vz;

				switch (CurrentVisionMode)
				{
					default:
					case VISION_MODE_NORMAL:
					{
						VerticesBuffer[i].R = SkyColour_R;
						VerticesBuffer[i].G	= SkyColour_G;
						VerticesBuffer[i].B = SkyColour_B;
						break;
					}
					case VISION_MODE_IMAGEINTENSIFIER:
					{
						VerticesBuffer[i].R = 0;
						VerticesBuffer[i].G	= 255;
						VerticesBuffer[i].B = 0;
						break;
					}
					case VISION_MODE_PRED_THERMAL:
					case VISION_MODE_PRED_SEEALIENS:
					case VISION_MODE_PRED_SEEPREDTECH:
					{
						VerticesBuffer[i].R = 0;
						VerticesBuffer[i].G	= 0;
						VerticesBuffer[i].B = 255;
					  	break;
					}
				}

			}	
			VerticesBuffer[0].U = (u[o]+size*x);
			VerticesBuffer[0].V = (v[o]+size*z);
			VerticesBuffer[1].U = (u[o]+size*x);
			VerticesBuffer[1].V = (v[o]+size*(z+1));
			VerticesBuffer[2].U = (u[o]+size*(x+1));
			VerticesBuffer[2].V = (v[o]+size*(z+1));
			VerticesBuffer[3].U = (u[o]+size*(x+1));
			VerticesBuffer[3].V = (v[o]+size*z);
																

			RenderPolygon.NumberOfVertices=4;
		}
				
		GouraudTexturedPolygon_ClipWithZ();
		if(RenderPolygon.NumberOfVertices>=3)
		{
			GouraudTexturedPolygon_ClipWithNegativeX();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudTexturedPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudTexturedPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						GouraudTexturedPolygon_ClipWithPositiveX();
						if(RenderPolygon.NumberOfVertices>=3)
						{
							D3D_SkyPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
						}
					}
				}
			}
		}
		t/=2;
		size*=2;
	}
	}
	}
}
#endif
void RenderWaterFall(int xOrigin, int yOrigin, int zOrigin)
{
	int i,z;
	VECTORCH v[4];

	{
		int waterfallX[9];
		int waterfallY[9];
		int waterfallZ[9];
		int waterfallZScale[9];
		for (i=0; i<9; i++)
		{

			int u = (i*65536)/8;

			int b = MUL_FIXED(2*u,(65536-u));
			int c = MUL_FIXED(u,u);
			int y3 = (4742-yOrigin);
			int x3 = 2000;
			int y2 = 2000;
			int x2 = 1500;
			
			waterfallX[i] = MUL_FIXED(b,x2)+MUL_FIXED(c,x3);
			waterfallY[i] = yOrigin+MUL_FIXED(b,y2)+MUL_FIXED(c,y3);
		 	waterfallZ[i] = zOrigin+MUL_FIXED((66572-zOrigin),u);
			waterfallZScale[i] = ONE_FIXED+b/2-c;
			if (i!=8)
			{
				waterfallZScale[i]+=(FastRandom()&8191);
				waterfallY[i]-=(FastRandom()&127);
			}

		}
		for (z=0; z<8; z++)
		for (i=0; i<8; i++)
		{
			v[0].vx = xOrigin+MUL_FIXED(waterfallX[i],waterfallZScale[z]);
			v[1].vx = xOrigin+MUL_FIXED(waterfallX[i],waterfallZScale[z+1]);
			v[2].vx = xOrigin+MUL_FIXED(waterfallX[i+1],waterfallZScale[z+1]);
			v[3].vx = xOrigin+MUL_FIXED(waterfallX[i+1],waterfallZScale[z]);
			v[0].vy = waterfallY[i];
			v[1].vy = waterfallY[i];
			v[2].vy = waterfallY[i+1];
			v[3].vy = waterfallY[i+1];
		 

		 	v[0].vz = waterfallZ[z];
			v[1].vz = waterfallZ[z+1];
			v[2].vz = v[1].vz;
			v[3].vz = v[0].vz;
			
			DrawWaterFallPoly(v);
		}
		for (z=0; z<3; z++)
		{
			v[0].vx = xOrigin+MUL_FIXED(waterfallX[8],waterfallZScale[z+1]);
			v[1].vx = xOrigin+MUL_FIXED(waterfallX[8],waterfallZScale[z]);
			v[2].vx = 179450;
			v[3].vx = 179450;

			v[0].vy = 4742;
			v[1].vy = 4742;
			v[2].vy = 4742;
			v[3].vy = 4742;

		 	v[0].vz = waterfallZ[z];
			v[1].vz = waterfallZ[z+1];
			v[2].vz = v[1].vz;
			v[3].vz = v[0].vz;
			
			DrawWaterFallPoly(v);
		}

		for (z=0; z<8; z++)
		for (i=0; i<16; i++)
		{
			int xOffset,xOffset2;
			if (z<3) xOffset = 179450;
			else xOffset = xOrigin+MUL_FIXED(waterfallX[8],waterfallZScale[z]);
			if (z<2) xOffset2 = 179450;
			else xOffset2 = xOrigin+MUL_FIXED(waterfallX[8],waterfallZScale[z+1]);

			v[0].vx = xOffset;
			v[1].vx = xOffset2;
			v[2].vx = xOffset2;
			v[3].vx = xOffset;
			
			v[0].vy = 4742+i*4096;
			v[1].vy = 4742+i*4096;
			v[2].vy = 4742+(i+1)*4096;
			v[3].vy = 4742+(i+1)*4096;
		 

		 	v[0].vz = waterfallZ[z];
			v[1].vz = waterfallZ[z+1];
			v[2].vz = v[1].vz;
			v[3].vz = v[0].vz;
			
			DrawWaterFallPoly(v);
		}

	}
}

void DrawWaterFallPoly(VECTORCH *v)
{
   	POLYHEADER fakeHeader;

	{
		extern int CloudyImageNumber;
		fakeHeader.PolyFlags = iflag_transparent;
		fakeHeader.PolyColour = CloudyImageNumber;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
	}
	{
		static int wv=0;
		unsigned int a;
		for (a=0; a<4; a++) 
		{
			VerticesBuffer[a].A = 128;
			VerticesBuffer[a].U = (v[a].vz)<<11;
			VerticesBuffer[a].V = (v[a].vy<<10)-wv;

			TranslatePointIntoViewspace(&v[a]);
			VerticesBuffer[a].X	= v[a].vx;
			VerticesBuffer[a].Y	= v[a].vy;
			VerticesBuffer[a].Z	= v[a].vz;
			VerticesBuffer[a].R = 200;
			VerticesBuffer[a].G	= 200;
			VerticesBuffer[a].B = 255;
			VerticesBuffer[a].SpecularR = 0;
			VerticesBuffer[a].SpecularG = 0;
			VerticesBuffer[a].SpecularB = 0;


		}	
 		wv+=NormalFrameTime*2;
		RenderPolygon.NumberOfVertices=4;
	}
			
	GouraudTexturedPolygon_ClipWithZ();
	if(RenderPolygon.NumberOfVertices>=3)
	{
		GouraudTexturedPolygon_ClipWithNegativeX();
		if(RenderPolygon.NumberOfVertices>=3)
		{
			GouraudTexturedPolygon_ClipWithPositiveY();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudTexturedPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudTexturedPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
					}
				}
			}
		}
	}
}
void RenderPredatorTargetingSegment(int theta, int scale, int drawInRed)
{
	VECTOR2D offset[4];
 	POLYHEADER fakeHeader;
	int centreX,centreY;
	int z = ONE_FIXED-scale;
	z = MUL_FIXED(MUL_FIXED(z,z),2048);
	{
		extern int SmartTargetSightX, SmartTargetSightY;
		extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
		centreY = MUL_FIXED( (SmartTargetSightY-(ScreenDescriptorBlock.SDB_Height<<15)) /Global_VDB_Ptr->VDB_ProjY,z);
		if (MIRROR_CHEATMODE)
		{
			centreX = MUL_FIXED( ( - (SmartTargetSightX-(ScreenDescriptorBlock.SDB_Width<<15)))  /Global_VDB_Ptr->VDB_ProjX,z);
		}
		else
		{
			centreX = MUL_FIXED( (SmartTargetSightX-(ScreenDescriptorBlock.SDB_Width<<15))  /Global_VDB_Ptr->VDB_ProjX,z);
		}
	}
	z = (float)z*CameraZoomScale;

	{
		int a = 160;
		int b = 40;
		
		/* tan(30) = 1/sqrt(3), & 65536/(sqrt(3)) = 37837 */

		int y = MUL_FIXED(37837,a+20);

		offset[0].vx = -a+MUL_FIXED(113512,b);
		offset[0].vy = y-b;

		offset[1].vx = -offset[0].vx;
		offset[1].vy = y-b;

		offset[2].vx = a;
		offset[2].vy = y;

		offset[3].vx = -a;
		offset[3].vy = y;

		if (theta)
		{
			RotateVertex(&offset[0],theta);
			RotateVertex(&offset[1],theta);
			RotateVertex(&offset[2],theta);
			RotateVertex(&offset[3],theta);
		}

		if (MIRROR_CHEATMODE)
		{
			offset[0].vx = -offset[0].vx;
			offset[1].vx = -offset[1].vx;
			offset[2].vx = -offset[2].vx;
			offset[3].vx = -offset[3].vx;
		}
		VerticesBuffer[0].X = offset[0].vx+centreX;
		VerticesBuffer[0].Y = MUL_FIXED(offset[0].vy,87381)+centreY;
		
		VerticesBuffer[1].X = offset[1].vx+centreX;
		VerticesBuffer[1].Y = MUL_FIXED(offset[1].vy,87381)+centreY;

		VerticesBuffer[2].X = offset[2].vx+centreX;
		VerticesBuffer[2].Y = MUL_FIXED(offset[2].vy,87381)+centreY;
		
		VerticesBuffer[3].X = offset[3].vx+centreX;
		VerticesBuffer[3].Y = MUL_FIXED(offset[3].vy,87381)+centreY;
	}
	fakeHeader.PolyFlags = iflag_transparent;
	RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;
	

 	{
		int i;
		for (i=0; i<4; i++) 
		{
			VerticesBuffer[i].A = 128;

			VerticesBuffer[i].Z	= z;

			VerticesBuffer[i].R = 255;

			if (drawInRed)
			{
				VerticesBuffer[i].G	= 0;
				VerticesBuffer[i].B = 0;
			}
			else
			{
				VerticesBuffer[i].G	= 255;
				VerticesBuffer[i].B = 255;
			}

		}
		RenderPolygon.NumberOfVertices=4;
	}
			
	GouraudPolygon_ClipWithZ();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudPolygon_ClipWithNegativeX();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudPolygon_ClipWithPositiveY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudPolygon_ClipWithNegativeY();
	if(RenderPolygon.NumberOfVertices<3) return;
	GouraudPolygon_ClipWithPositiveX();
	if(RenderPolygon.NumberOfVertices<3) return;
	D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);

	if (drawInRed)
	{
		VerticesBuffer[0].X = MUL_FIXED(offset[3].vx,scale*8)+centreX;
		VerticesBuffer[0].Y = MUL_FIXED(MUL_FIXED(offset[3].vy,scale*8),87381)+centreY;
		
		VerticesBuffer[1].X = MUL_FIXED(offset[2].vx,scale*8)+centreX;
		VerticesBuffer[1].Y = MUL_FIXED(MUL_FIXED(offset[2].vy,scale*8),87381)+centreY;

		VerticesBuffer[2].X = offset[2].vx+centreX;
		VerticesBuffer[2].Y = MUL_FIXED(offset[2].vy,87381)+centreY;
		
		VerticesBuffer[3].X = offset[3].vx+centreX;
		VerticesBuffer[3].Y = MUL_FIXED(offset[3].vy,87381)+centreY;
	 	{
			int i;
			for (i=0; i<2; i++) 
			{
				VerticesBuffer[i].A = 0;
				VerticesBuffer[i].Z	= z;
				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 0;
				VerticesBuffer[i].B = 0;
			}
			for (i=2; i<4; i++) 
			{
				VerticesBuffer[i].A = 128;
				VerticesBuffer[i].Z	= z;
				VerticesBuffer[i].R = 255;
				VerticesBuffer[i].G	= 0;
				VerticesBuffer[i].B = 0;
			}
			RenderPolygon.NumberOfVertices=4;
		}
				
		GouraudPolygon_ClipWithZ();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudPolygon_ClipWithNegativeX();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudPolygon_ClipWithPositiveY();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudPolygon_ClipWithNegativeY();
		if(RenderPolygon.NumberOfVertices<3) return;
		GouraudPolygon_ClipWithPositiveX();
		if(RenderPolygon.NumberOfVertices<3) return;
		D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
	}
}
void RenderPredatorPlasmaCasterCharge(int value, VECTORCH *worldOffsetPtr, MATRIXCH *orientationPtr)
{
 	POLYHEADER fakeHeader;
 	VECTORCH translatedPts[4];
	int halfWidth = 100;
	int halfHeight = 4;
	int z = -1;
	translatedPts[0].vx = -halfWidth;
	translatedPts[0].vy = z;
	translatedPts[0].vz = -halfHeight-4;

	translatedPts[1].vx = -halfWidth+MUL_FIXED(value,2*halfWidth-10);
	translatedPts[1].vy = z;
	translatedPts[1].vz = -halfHeight-4;
	
	translatedPts[2].vx = -halfWidth+MUL_FIXED(value,2*halfWidth-10);
	translatedPts[2].vy = z;
	translatedPts[2].vz = halfHeight-4;
	
	translatedPts[3].vx = -halfWidth;
	translatedPts[3].vy = z;
	translatedPts[3].vz = halfHeight-4;

	fakeHeader.PolyFlags = iflag_transparent;
	RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;


	{
		int i;
		for (i=0; i<4; i++) 
		{
			VerticesBuffer[i].A = 255;
		
			RotateVector(&(translatedPts[i]),orientationPtr);
			translatedPts[i].vx += worldOffsetPtr->vx;
			translatedPts[i].vy += worldOffsetPtr->vy;
			translatedPts[i].vz += worldOffsetPtr->vz;
			TranslatePointIntoViewspace(&translatedPts[i]);
		
			VerticesBuffer[i].X	= translatedPts[i].vx;
			VerticesBuffer[i].Y	= translatedPts[i].vy;
			VerticesBuffer[i].Z	= translatedPts[i].vz;

			VerticesBuffer[i].R = 32;
			VerticesBuffer[i].G	= 0;
			VerticesBuffer[i].B = 0;
		}
		RenderPolygon.NumberOfVertices=4;
	}
	{
		int outcode = QuadWithinFrustrum();
										  
		if (outcode)
		{		 
			if (outcode!=2)
			{
				GouraudPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) return;
				GouraudPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) return;
				GouraudPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) return;
				GouraudPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) return;
				GouraudPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) return;
				D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
  			}
			else D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,VerticesBuffer);
		}
	}
}





int LightFlareAlpha = 65535;
void RenderLightFlare(VECTORCH *positionPtr, unsigned int colour)
{
	int centreX,centreY,sizeX,sizeY,z;
	PARTICLE particle;
//	VECTORCH point = {-20947,-8216,2244};
	VECTORCH point = *positionPtr;


	#if 0
	if (IsThisObjectVisibleFromThisPosition(Player,&point,ONE_FIXED))
	{
	  	LightFlareAlpha+=NormalFrameTime*8;
	   	if (LightFlareAlpha > 65535) LightFlareAlpha = 65535;
	   textprint("FLARE VIS\n");
	}
	else
	{
	  	LightFlareAlpha-=NormalFrameTime*8;
	  	if (LightFlareAlpha < 0) LightFlareAlpha = 0;
	   textprint("FLARE INVIS\n");
	}
	#endif
	TranslatePointIntoViewspace(&point);
	if(point.vz<64) return;	
	
	#if 0
	{
		int alpha = ONE_FIXED - point.vz*2;
		if (alpha<0) return;

		alpha = MUL_FIXED(LightFlareAlpha,alpha)>>8;

		particle.ParticleID = PARTICLE_MUZZLEFLASH;
		particle.Colour = 0xffffff+((alpha)<<24);
	}
	#else
	particle.ParticleID = PARTICLE_LIGHTFLARE;
//	particle.Colour = 0xffffff+((LightFlareAlpha>>8)<<24);
	particle.Colour = colour;
	#endif
//	textprint("render fn %d %d %d\n",positionPtr->vx,positionPtr->vy,positionPtr->vz);
//	PARTICLE_DESC *particleDescPtr = &ParticleDescription[particlePtr->ParticleID];
//	int particleSize = particlePtr->Size;
	z=ONE_FIXED;
	{
		extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
		centreX = DIV_FIXED(point.vx,point.vz);
		centreY = DIV_FIXED(point.vy,point.vz);
		sizeX = (ScreenDescriptorBlock.SDB_Width<<13)/Global_VDB_Ptr->VDB_ProjX;
		sizeY = MUL_FIXED(ScreenDescriptorBlock.SDB_Height<<13,87381)/Global_VDB_Ptr->VDB_ProjY;
	}

	VerticesBuffer[0].X = centreX - sizeX;
	VerticesBuffer[0].Y = centreY - sizeY;
	VerticesBuffer[0].Z = z;
	VerticesBuffer[1].X = centreX + sizeX;
	VerticesBuffer[1].Y = centreY - sizeY;
	VerticesBuffer[1].Z = z;
	VerticesBuffer[2].X = centreX + sizeX;
	VerticesBuffer[2].Y = centreY + sizeY;
	VerticesBuffer[2].Z = z;
	VerticesBuffer[3].X = centreX - sizeX;
	VerticesBuffer[3].Y = centreY + sizeY;
	VerticesBuffer[3].Z = z;
	
	{
		int outcode = QuadWithinFrustrum();
										  
		if (outcode)
		{		 
			RenderPolygon.NumberOfVertices=4;
			
//			textprint("On Screen!\n");
			VerticesBuffer[0].U = 192<<16;
			VerticesBuffer[0].V = 0;

			VerticesBuffer[1].U = 255<<16;
			VerticesBuffer[1].V = 0;

			VerticesBuffer[2].U = 255<<16;
			VerticesBuffer[2].V = 63<<16;

			VerticesBuffer[3].U = 192<<16;
			VerticesBuffer[3].V = 63<<16;

			if (outcode!=2)
			{
				TexturedPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) return;
				D3D_Particle_Output(&particle,RenderPolygon.Vertices);
  			
  			}
			else D3D_Particle_Output(&particle,VerticesBuffer);
		}
	}	
}

#if VOLUMETRIC_FOG

int FogValue(VECTORCH *vertexPtr)
{
	float a,b,c,d, lMax,lMin;
	VECTORCHF v;
	v.vx = vertexPtr->vx;
	v.vy = vertexPtr->vy;
	v.vz = vertexPtr->vz;
	
	a = (v.vx*v.vx + v.vy*v.vy + v.vz*v.vz)*2.0;
	b = -2.0*(v.vx*FogPosition.vx+v.vy*FogPosition.vy+v.vz*FogPosition.vz);
	{
//		float s = MUL_FIXED(GetSin(CloakingPhase&4095),GetSin(CloakingPhase&4095));
		c = FogMagnitude - 10000.0*10000.0;
	}
	d = b*b-2.0*a*c;
	if (d<0) return 0;
	
	d = sqrt(d);

	lMin = (-b-d)/(a);
	if (lMin>1.0) return 0;

	lMax = (-b+d)/(a);

	if (lMax<0.0)
	{
		return 0;
	}
	else if (lMax>1.0)
	{
		if (lMin>0.0)
		{
			float m;
			int f;
			m = Approximate3dMagnitude(vertexPtr);
			f2i(f,(lMax-lMin)*m);
			return f;
		}
		else return Approximate3dMagnitude(vertexPtr); 	
	}
	else //(lMax<1.0)
	{
		if (lMin>0.0)
		{
			float m;
			int f;
			m = Approximate3dMagnitude(vertexPtr);
			f2i(f,(lMax-lMin)*m);
			return f;
		}
		else
		{
			float m;
			int f;
			m = Approximate3dMagnitude(vertexPtr);
			f2i(f,(lMax)*m);
			return f;
		}		

	}
}
#endif
#if 0
int SphereGenerated=0;
void RenderSphere(void)
{
	if(!SphereGenerated)								   
	{
		Generate_Sphere();
		SphereGenerated=1;
	}
//	EvolveSphere();
	{
	   	int f;
	   	POLYHEADER fakeHeader;
		MATRIXCH mat = (Global_VDB_Ptr->VDB_Mat);
		VECTORCH *vSphere = SphereRotatedVertex;
		VECTORCH *v2 = SphereAtmosRotatedVertex;
		VECTORCH d;
		{
			fakeHeader.PolyFlags = 0;
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;
		}
		{
			MATRIXCH m2;
			EULER e;
			e.EulerX =0;
			e.EulerY =(CloakingPhase/32)&4095;
			e.EulerZ =0;
			CreateEulerMatrix(&e,&m2);
			TransposeMatrixCH(&m2);
			MatrixMultiply(&mat,&m2,&mat);
			d.vx = -3316 - Global_VDB_Ptr->VDB_World.vx;
			d.vy = -50000 -Global_VDB_Ptr->VDB_World.vy;
			d.vz = 26926  -Global_VDB_Ptr->VDB_World.vz;
			
			RotateVector(&d,&(Global_VDB_Ptr->VDB_Mat));
		
		}
		for (f=0; f<SPHERE_VERTICES; f++)
		{

			int h = SphereVertexHeight[f];
			int radius = 4096;
			if (h>0) radius += h;
			*vSphere = SphereVertex[f];
			
		   	RotateVector(vSphere,&mat);
			v2->vx = MUL_FIXED(vSphere->vx,4096+300);
			v2->vy = MUL_FIXED(vSphere->vy,4096+300);
			v2->vz = MUL_FIXED(vSphere->vz,4096+300);
			v2++;
			vSphere->vx = MUL_FIXED(vSphere->vx,radius);
			vSphere->vy = MUL_FIXED(vSphere->vy,radius);
			vSphere->vz = MUL_FIXED(vSphere->vz,radius);
			vSphere++;
		}
		#if 0
		for (f=0; f<SPHERE_FACES; f++)
		{
		 	{
				int i;
				VECTORCH v[3];

				{
					int facingAway=1;
					for (i=0; i<3; i++) 
					{
						int n = SphereFace[f].v[i];
						v[i] = SphereRotatedVertex[n];
						if (v[i].vz<=0) facingAway=0;
					}
					if(facingAway) continue;
				}
				for (i=0; i<3; i++) 
				{
					int n = SphereFace[f].v[i];
					int l = (v[i].vx*16+ONE_FIXED)/2;
					int h = SphereVertexHeight[n];
					if (l>ONE_FIXED) l = ONE_FIXED;
					if (l<0) l = 0;

					v[i].vx += d.vx;
					v[i].vy += d.vy;
					v[i].vz += d.vz;
					v[i].vy = MUL_FIXED(v[i].vy,87381);
					
					VerticesBuffer[i].X	= v[i].vx;
					VerticesBuffer[i].Y	= v[i].vy;
					VerticesBuffer[i].Z	= v[i].vz;

					VerticesBuffer[i].A = 0;
					if (h<=0)
					{
						VerticesBuffer[i].R = 0;
						VerticesBuffer[i].G	= 255+h;
						VerticesBuffer[i].B = 255;
					}
					else if (h>128)
					{
						VerticesBuffer[i].R = MUL_FIXED(255,h*256);
						VerticesBuffer[i].G	= MUL_FIXED(200,h*256);
						VerticesBuffer[i].B = MUL_FIXED(150,h*256);
					}
					else
					{
						VerticesBuffer[i].R = 0;
						VerticesBuffer[i].G	= 96-h/2;
						VerticesBuffer[i].B = 0;
					}
					VerticesBuffer[i].R = MUL_FIXED(VerticesBuffer[i].R,l);
					VerticesBuffer[i].G	= MUL_FIXED(VerticesBuffer[i].G,l);
					VerticesBuffer[i].B = MUL_FIXED(VerticesBuffer[i].B,l);
				}	
  				RenderPolygon.NumberOfVertices=3;
			}
					
			GouraudPolygon_ClipWithZ();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						GouraudPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices>=3)
						{
							GouraudPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices>=3)
							{
								D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
							}
						}
					}
				}
			}
		
	   	}
		#endif
		{
			extern int CloudyImageNumber;
			fakeHeader.PolyFlags = iflag_transparent;			      
			fakeHeader.PolyColour = CloudyImageNumber;			  
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_OFF;
		}
		for (f=0; f<SPHERE_FACES; f++)
		{
		 	{
				int i;
				VECTORCH vertex[3];

				{
					int facingAway=1;
					for (i=0; i<3; i++) 
					{
						int n = SphereFace[f].v[i];
						vertex[i] = SphereAtmosRotatedVertex[n];
						if (vertex[i].vz<=0) facingAway=0;
					}
				   	if(facingAway) continue;
				}
				for (i=0; i<3; i++) 
				{
					int n = SphereFace[f].v[i];
					int l = (vertex[i].vx*16+ONE_FIXED)/514;
					if (l>ONE_FIXED) l = ONE_FIXED;
					if (l<0) l = 0;

					vertex[i].vx += d.vx;
					vertex[i].vy += d.vy;
					vertex[i].vz += d.vz;
					vertex[i].vy = MUL_FIXED(vertex[i].vy,87381);
					
					VerticesBuffer[i].X	= vertex[i].vx;
					VerticesBuffer[i].Y	= vertex[i].vy;
					VerticesBuffer[i].Z	= vertex[i].vz;
					VerticesBuffer[i].U = SphereAtmosU[n];//+u[0];
					VerticesBuffer[i].V = SphereAtmosV[n];//+v[0];

					VerticesBuffer[i].A = 255;
		  			VerticesBuffer[i].R = l;
					VerticesBuffer[i].G	= l/2;
					VerticesBuffer[i].B = 0;
					VerticesBuffer[i].SpecularR = 0;
					VerticesBuffer[i].SpecularG = 0;
					VerticesBuffer[i].SpecularB = 0;
				}	
  				RenderPolygon.NumberOfVertices=3;
			}
					
			GouraudTexturedPolygon_ClipWithZ();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudTexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudTexturedPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						GouraudTexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices>=3)
						{
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices>=3)
							{
								D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
							}
						}
					}
				}
			}
	   	}
	}

}
void RenderBoom(void)
{
	VECTORCH d;
	if(!SphereGenerated)								   
	{
		Generate_Sphere();
		SphereGenerated=1;
	}
	d.vx = 0;
	d.vy = 0; 
	d.vz = 0;
	
	RenderBoomSphere(&d,4096);  

}
void RenderBoomSphere(VECTORCH *position, int radius)
{
	int Alpha[SPHERE_VERTICES];
	extern D3DTEXTUREHANDLE FMVTextureHandle[];
	
	{
	   	int f;
	   	POLYHEADER fakeHeader;
		VECTORCH *vSphere = SphereRotatedVertex;
			static int o=0;
			o++;

		{
			extern int CloudyImageNumber;
			fakeHeader.PolyFlags = iflag_transparent;			      
			fakeHeader.PolyColour = CloudyImageNumber;			  
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
		}
		for (f=0; f<SPHERE_VERTICES; f++)
		{	
			int r;
			int t = GetSin((f+o+CloakingPhase/32)&4095);
			t = MUL_FIXED(t,t);
			t = MUL_FIXED(t,t);
			t = MUL_FIXED(t,t);
//			t = MUL_FIXED(t,t);
//			t = MUL_FIXED(t,t);
			t = MUL_FIXED(t,t);
			t = MUL_FIXED(t,t);
			t = MUL_FIXED(t,t);
			r = radius+MUL_FIXED(t,t)/128;
	 		Alpha[f] = (r-radius)/2;

			*vSphere = SphereVertex[f];

			vSphere->vx = MUL_FIXED(vSphere->vx,r);
			vSphere->vy = MUL_FIXED(vSphere->vy,r);
			vSphere->vz = MUL_FIXED(vSphere->vz,r);
			
			vSphere->vx += position->vx; 
			vSphere->vy += position->vy;
			vSphere->vz += position->vz; 
			TranslatePointIntoViewspace(vSphere);
			vSphere++;

		}
 		
		for (f=0; f<SPHERE_FACES; f++)
		{
		 	{
				int i;
				VECTORCH vertex[3];

				{
					for (i=0; i<3; i++) 
					{
						int n = SphereFace[f].v[i];
						vertex[i] = SphereRotatedVertex[n];

					}
				}
				for (i=0; i<3; i++) 
				{
					int n = SphereFace[f].v[i];
					
					VerticesBuffer[i].X	= vertex[i].vx;
					VerticesBuffer[i].Y	= vertex[i].vy;
					VerticesBuffer[i].Z	= vertex[i].vz;


//					SphereAtmosU[0]=(vertex[i].vx)*4*128+GetSin((CloakingPhase)&4095)*16;
//					SphereAtmosV[0]=(vertex[i].vy)*4*128+GetCos((CloakingPhase)&4095)*16;
					{
						int t = GetSin((CloakingPhase/20)&4095);
						int u;

						u = t*(128);
//						SphereAtmosU[0]=MUL_FIXED(SphereAtmosU[0],u);
//						SphereAtmosV[0]=MUL_FIXED(SphereAtmosV[0],u);
  						VerticesBuffer[i].U = (SphereAtmosU[n]+u);//+u[0];
						VerticesBuffer[i].V = (SphereAtmosV[n]);//+v[0];
					}
					{
						int d1 = VerticesBuffer[0].U-VerticesBuffer[1].U;
						int d2 = VerticesBuffer[0].U-VerticesBuffer[2].U;
						int d3 = VerticesBuffer[1].U-VerticesBuffer[2].U;

						int ad1=d1,ad2=d2,ad3=d3;
						int i1=0,i2=0,i3=0;

						if (ad1<0) ad1=-ad1;
						if (ad2<0) ad2=-ad2;
						if (ad3<0) ad3=-ad3;

						if (ad1>(128*3+64)*65536)
						{
							if (d1>0) i2=1;
							else i1=1;
						}
						if (ad2>(128*3+64)*65536)
						{
							if (d2>0) i3=1;
							else i1=1; 
						}
						if (ad3>(128*3+64)*65536)
						{
							if (d3>0) i3=1; 
							else i2=1; 
						}

						if(i1) VerticesBuffer[0].U+=128*65536*4;
						if(i2) VerticesBuffer[1].U+=128*65536*4;
						if(i3) VerticesBuffer[2].U+=128*65536*4;
					}	

					VerticesBuffer[i].A = 128+Alpha[n];
		  			VerticesBuffer[i].R = 0;
					VerticesBuffer[i].G	= 128;
					VerticesBuffer[i].B = 255;
					VerticesBuffer[i].SpecularR = 0;
					VerticesBuffer[i].SpecularG = 0;
					VerticesBuffer[i].SpecularB = 0;
				}	
  				RenderPolygon.NumberOfVertices=3;
			}
					
			GouraudTexturedPolygon_ClipWithZ();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudTexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudTexturedPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						GouraudTexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices>=3)
						{
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices>=3)
							{
								D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
//								D3D_FMVParticle_Output(RenderPolygon.Vertices);
							}
						}
					}
				}
			}
	   	}
	}

}
#endif

int Alpha[SPHERE_VERTICES];
void RenderExplosionSurface(VOLUMETRIC_EXPLOSION *explosionPtr)
{
	int red,green,blue;

	switch (CurrentVisionMode)
	{
		default:
		case VISION_MODE_NORMAL:
		{
			red   = 255;
			green = 255;
			blue  = 255;
			break;
		}
		case VISION_MODE_IMAGEINTENSIFIER:
		{
			red   = 0;
			green = 255;
			blue  = 0;
			break;
		}
		case VISION_MODE_PRED_THERMAL:
		case VISION_MODE_PRED_SEEALIENS:
		case VISION_MODE_PRED_SEEPREDTECH:
		{
			red   = 255;
			green = 0;
			blue  = 255;
		  	break;
		}
	}
	{
	   	int f;
	   	POLYHEADER fakeHeader;
		VECTORCH *vSphere = SphereRotatedVertex;
			static int o=0;
			o++;

		if (explosionPtr->ExplosionPhase)
		{
			extern int BurningImageNumber;
			fakeHeader.PolyFlags = iflag_transparent;			      
			fakeHeader.PolyColour = BurningImageNumber;			  
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
		}
		else
		{
			extern int CloudyImageNumber;
			fakeHeader.PolyFlags = iflag_transparent;			      
			fakeHeader.PolyColour = CloudyImageNumber;			  
			RenderPolygon.TranslucencyMode = TRANSLUCENCY_INVCOLOUR;
			red = explosionPtr->LifeTime/256;
			green = explosionPtr->LifeTime/256;
			blue = explosionPtr->LifeTime/256;
		}

		for (f=0; f<SPHERE_VERTICES; f++)
		{	
			*vSphere = explosionPtr->Position[f];
			
			TranslatePointIntoViewspace(vSphere);
			vSphere++;
		}
 		
		for (f=0; f<SPHERE_FACES; f++)
		{
		 	{
				int i;
				VECTORCH vertex[3];

				{
					for (i=0; i<3; i++) 
					{
						int n = SphereFace[f].v[i];
						vertex[i] = SphereRotatedVertex[n];

					}
				}
				for (i=0; i<3; i++) 
				{
					int n = SphereFace[f].v[i];
					
					VerticesBuffer[i].X	= vertex[i].vx;
					VerticesBuffer[i].Y	= vertex[i].vy;
					VerticesBuffer[i].Z	= vertex[i].vz;


//					SphereAtmosU[0]=(vertex[i].vx)*4*128+GetSin((CloakingPhase)&4095)*16;
//					SphereAtmosV[0]=(vertex[i].vy)*4*128+GetCos((CloakingPhase)&4095)*16;
					{
						int u = -(ONE_FIXED-explosionPtr->LifeTime)*128*2;
//						SphereAtmosU[0]=MUL_FIXED(SphereAtmosU[0],u);
//						SphereAtmosV[0]=MUL_FIXED(SphereAtmosV[0],u);
  						VerticesBuffer[i].U = (SphereAtmosU[n]);//+u[0];
						VerticesBuffer[i].V = (SphereAtmosV[n]+u);//+v[0];
					}
					{
						int d1 = VerticesBuffer[0].U-VerticesBuffer[1].U;
						int d2 = VerticesBuffer[0].U-VerticesBuffer[2].U;
						int d3 = VerticesBuffer[1].U-VerticesBuffer[2].U;

						int ad1=d1,ad2=d2,ad3=d3;
						int i1=0,i2=0,i3=0;

						if (ad1<0) ad1=-ad1;
						if (ad2<0) ad2=-ad2;
						if (ad3<0) ad3=-ad3;

						if (ad1>(128*(SPHERE_TEXTURE_WRAP-1)+64)*65536)
						{
							if (d1>0) i2=1;
							else i1=1;
						}
						if (ad2>(128*(SPHERE_TEXTURE_WRAP-1)+64)*65536)
						{
							if (d2>0) i3=1;
							else i1=1; 
						}
						if (ad3>(128*(SPHERE_TEXTURE_WRAP-1)+64)*65536)
						{
							if (d3>0) i3=1; 
							else i2=1; 
						}

						if(i1) VerticesBuffer[0].U+=128*65536*SPHERE_TEXTURE_WRAP;
						if(i2) VerticesBuffer[1].U+=128*65536*SPHERE_TEXTURE_WRAP;
						if(i3) VerticesBuffer[2].U+=128*65536*SPHERE_TEXTURE_WRAP;
					}	

					VerticesBuffer[i].A = explosionPtr->LifeTime/256;
		  			VerticesBuffer[i].R = red;
					VerticesBuffer[i].G	= green;
					VerticesBuffer[i].B = blue;
					VerticesBuffer[i].SpecularR = 0;
					VerticesBuffer[i].SpecularG = 0;
					VerticesBuffer[i].SpecularB = 0;

				}	
  				RenderPolygon.NumberOfVertices=3;
			}
					
			GouraudTexturedPolygon_ClipWithZ();
			if(RenderPolygon.NumberOfVertices>=3)
			{
				GouraudTexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices>=3)
				{
					GouraudTexturedPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices>=3)
					{
						GouraudTexturedPolygon_ClipWithNegativeY();
						if(RenderPolygon.NumberOfVertices>=3)
						{
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices>=3)
							{
								D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
//								D3D_FMVParticle_Output(RenderPolygon.Vertices);
							}
						}
					}
				}
			}
	   	}
	}
}
void RenderInsideAlienTongue(int offset)
{
	#define TONGUE_SCALE 1024
	int TonguePolyVertexList[4][4] = 
	{
		{0,3,7,4},	 //+ve y
		{1,2,3,0},	 //+ve x
		{5,6,7,4},	 //-ve x
		{1,2,6,5},	 //-ve y
	};
	VECTORCH vertices[8]=
	{
		{+TONGUE_SCALE,-TONGUE_SCALE,0},
		{+TONGUE_SCALE,+TONGUE_SCALE,0},
		{+TONGUE_SCALE,+TONGUE_SCALE,+TONGUE_SCALE*4},
		{+TONGUE_SCALE,-TONGUE_SCALE,+TONGUE_SCALE*4},

		{-TONGUE_SCALE,-TONGUE_SCALE,0},
		{-TONGUE_SCALE,+TONGUE_SCALE,0},
		{-TONGUE_SCALE,+TONGUE_SCALE,+TONGUE_SCALE*4},
		{-TONGUE_SCALE,-TONGUE_SCALE,+TONGUE_SCALE*4},
	};
	VECTORCH translatedPts[8];

	POLYHEADER fakeHeader;
	int polyNumber;

	#if 1
	{

		int i = 7;
		do
		{
			translatedPts[i] = vertices[i];
			translatedPts[i].vz -= (ONE_FIXED-offset)/16;
//			TranslatePointIntoViewspace(&translatedPts[i]);
		}
	   	while(i--);
   	}
	#endif
	{
		extern int AlienTongueImageNumber;
		fakeHeader.PolyFlags = 0;
		fakeHeader.PolyColour = AlienTongueImageNumber;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_GLOWING;
	}

	for(polyNumber=0; polyNumber<4; polyNumber++)
	{
		{
			int i;
			for (i=0; i<4; i++) 
			{
				int v = TonguePolyVertexList[polyNumber][i];
				VerticesBuffer[i].A = 255;
				VerticesBuffer[i].X	= translatedPts[v].vx;
				VerticesBuffer[i].Y	= translatedPts[v].vy;
				VerticesBuffer[i].Z	= translatedPts[v].vz;
				VerticesBuffer[i].U = CuboidPolyVertexU[3][i]<<16;
				VerticesBuffer[i].V = CuboidPolyVertexV[3][i]<<16;


				VerticesBuffer[i].R = offset/2048;
				VerticesBuffer[i].G	= offset/2048;
				VerticesBuffer[i].B = offset/2048;
				VerticesBuffer[i].SpecularR = 0;
				VerticesBuffer[i].SpecularG = 0;
				VerticesBuffer[i].SpecularB = 0;

			}
			RenderPolygon.NumberOfVertices=4;
		}
		{
			GouraudTexturedPolygon_ClipWithZ();
			if(RenderPolygon.NumberOfVertices<3) continue;
			GouraudTexturedPolygon_ClipWithNegativeX();
			if(RenderPolygon.NumberOfVertices<3) continue;
			GouraudTexturedPolygon_ClipWithPositiveY();
			if(RenderPolygon.NumberOfVertices<3) continue;
			GouraudTexturedPolygon_ClipWithNegativeY();
			if(RenderPolygon.NumberOfVertices<3) continue;
			GouraudTexturedPolygon_ClipWithPositiveX();
			if(RenderPolygon.NumberOfVertices<3) continue;
			D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
											  
		}
	}	
}



#define NO_OF_STARS 500
typedef struct
{
	VECTORCH Position;
	int Colour;
	int Frequency;
	int Phase;

} STARDESC;
static STARDESC StarArray[NO_OF_STARS];

void CreateStarArray(void)
{
	int i;
	
	SetSeededFastRandom(FastRandom());
	for (i=0; i<NO_OF_STARS; i++)
	{
		int phi = SeededFastRandom()&4095;

		StarArray[i].Position.vy = ONE_FIXED-(SeededFastRandom()&131071);
		{
			float y = ((float)StarArray[i].Position.vy)/65536.0;
			y = sqrt(1-y*y);

			f2i(StarArray[i].Position.vx,(float)GetCos(phi)*y);
			f2i(StarArray[i].Position.vz,(float)GetSin(phi)*y);
		}
		StarArray[i].Colour = 0xff000000 + (FastRandom()&0x7f7f7f)+0x7f7f7f;
		StarArray[i].Frequency = (FastRandom()&4095);
		StarArray[i].Phase = FastRandom()&4095;
	}
}

void RenderStarfield(void)
{
	int i;

	int sizeX;
	int sizeY;

	#if 0
	sizeX = GetSin((MUL_FIXED(CloakingPhase,StarArray[i].Frequency)+StarArray[i].Phase)&4095);
	sizeX = MUL_FIXED(sizeX,sizeX)/1024+256;
	#else
	sizeX = 256;
	#endif

	sizeY = MUL_FIXED(sizeX,87381);

	for (i=0; i<NO_OF_STARS; i++)
	{
		VECTORCH position = StarArray[i].Position;
		PARTICLE particle;
		particle.ParticleID = PARTICLE_STAR;
		particle.Colour = StarArray[i].Colour;
		#if 1
		position.vx += Global_VDB_Ptr->VDB_World.vx;
		position.vy += Global_VDB_Ptr->VDB_World.vy;
		position.vz += Global_VDB_Ptr->VDB_World.vz;

		TranslatePointIntoViewspace(&position);
		#endif
//		RotateVector(&position,&(Global_VDB_Ptr->VDB_Mat));


		VerticesBuffer[0].X = position.vx - sizeX;
		VerticesBuffer[0].Y = position.vy - sizeY;
		VerticesBuffer[0].Z = position.vz;
		VerticesBuffer[1].X = position.vx + sizeX;
		VerticesBuffer[1].Y = position.vy - sizeY;
		VerticesBuffer[1].Z = position.vz;
		VerticesBuffer[2].X = position.vx + sizeX;
		VerticesBuffer[2].Y = position.vy + sizeY;
		VerticesBuffer[2].Z = position.vz;
		VerticesBuffer[3].X = position.vx - sizeX;
		VerticesBuffer[3].Y = position.vy + sizeY;
		VerticesBuffer[3].Z = position.vz;
		
		{
			int outcode = QuadWithinFrustrum();
											  
			if (outcode)
			{		 
				RenderPolygon.NumberOfVertices=4;
				
	//			textprint("On Screen!\n");
				VerticesBuffer[0].U = 192<<16;
				VerticesBuffer[0].V = 0;

				VerticesBuffer[1].U = 255<<16;
				VerticesBuffer[1].V = 0;

				VerticesBuffer[2].U = 255<<16;
				VerticesBuffer[2].V = 63<<16;

				VerticesBuffer[3].U = 192<<16;
				VerticesBuffer[3].V = 63<<16;

				if (outcode!=2)
				{
					TexturedPolygon_ClipWithZ();
					if(RenderPolygon.NumberOfVertices<3) return;
					TexturedPolygon_ClipWithNegativeX();
					if(RenderPolygon.NumberOfVertices<3) return;
					TexturedPolygon_ClipWithPositiveY();
					if(RenderPolygon.NumberOfVertices<3) return;
					TexturedPolygon_ClipWithNegativeY();
					if(RenderPolygon.NumberOfVertices<3) return;
					TexturedPolygon_ClipWithPositiveX();
					if(RenderPolygon.NumberOfVertices<3) return;
					D3D_Particle_Output(&particle,RenderPolygon.Vertices);
	  			
	  			}
				else D3D_Particle_Output(&particle,VerticesBuffer);
			}
		}
	}		
}
