#ifndef savegame_h_
#define savegame_h_ 1

#define SAVELOAD_REQUEST_NONE -1
extern int LoadGameRequest; //slot number of game to be loaded
extern int SaveGameRequest;	//slot number of game to be saved

extern int NumberOfSavesLeft;

typedef enum save_block_type
{
	SaveBlock_MainHeader,
	SaveBlock_DeadStrategy,
	SaveBlock_Strategy,
	SaveBlock_Track,
	SaveBlock_GlobalHive,
	SaveBlock_Hierarchy,
	SaveBlock_HierarchySection,
	SaveBlock_HierarchyDecals,
	SaveBlock_HierarchyTween,
	SaveBlock_HierarchyDelta,
	SaveBlock_MiscGlobal,
	SaveBlock_MarineSquad,
	SaveBlock_Particles,
	SaveBlock_Decals,
	SaveBlock_PheromoneTrail,
	SaveBlock_VolumetricExplosions,
	SaveBlock_LightElements,
	SaveBlock_MessageHistory,
	SaveBlock_WeaponsCGlobals,
	SaveBlock_SoundState,
}SAVE_BLOCK_TYPE;


//structure needs to be at the start of each block
typedef struct save_block_header
{
	SAVE_BLOCK_TYPE type;
	int size;
}SAVE_BLOCK_HEADER;


//structure to go at the start of stategy save blocks
typedef struct save_block_strategy_header
{
	SAVE_BLOCK_TYPE type;
	int size;

	AVP_BEHAVIOUR_TYPE bhvr_type;
	char SBname[SB_NAME_LENGTH];

}SAVE_BLOCK_STRATEGY_HEADER;

typedef struct level_save_block
{
	SAVE_BLOCK_HEADER header;

	char AvP_Save_String[8];
	
	unsigned char	Species;
	unsigned char	Episode;
	unsigned char	ElapsedTime_Hours;
	unsigned char	ElapsedTime_Minutes;
	unsigned int	ElapsedTime_Seconds;
	unsigned char	Difficulty;
	unsigned char 	NumberOfSavesLeft;//For Load/Save menu only (if it gets used)

}LEVEL_SAVE_BLOCK;


#define COPYELEMENT_LOAD(element) SAVELOAD_BEHAV->element = SAVELOAD_BLOCK->element;
#define COPYELEMENT_SAVE(element) SAVELOAD_BLOCK->element = SAVELOAD_BEHAV->element;

#define COPYELEMENT_LOAD_EXT(block,behav) behav=block;
#define COPYELEMENT_SAVE_EXT(block,behav) block=behav;



extern void* GetPointerForSaveBlock(unsigned int size);
extern struct aimodule * GetPointerFromAIModuleIndex(int index);
extern int GetIndexFromAIModulePointer(struct aimodule* module);

extern SAVE_BLOCK_HEADER* GetNextBlockIfOfType(SAVE_BLOCK_TYPE type);

#define GET_SAVE_BLOCK_POINTER(block) block = GetPointerForSaveBlock(sizeof(*block))

#define GET_STRATEGY_SAVE_BLOCK(block,sbPtr)\
	block = GetPointerForSaveBlock(sizeof(*block));\
	block->header.type = SaveBlock_Strategy;\
	block->header.size = sizeof(*block);\
	block->header.bhvr_type = sbPtr->I_SBtype;\
	COPY_NAME(block->header.SBname,sbPtr->SBname);


extern void SaveGame();
extern void LoadSavedGame();
extern void ResetNumberOfSaves();

#endif
