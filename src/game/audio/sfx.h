#ifndef _included_sfx_h_ /* Is this your first time? */
#define _included_sfx_h_ 1

#define MAX_NO_OF_SFX_BLOCKS 10
enum SFX_ID
{
	SFX_NONE,
	SFX_MUZZLE_FLASH_SMARTGUN,
	SFX_MUZZLE_FLASH_AMORPHOUS,
	SFX_PREDATOR_PLASMA_BOLT,
	SFX_SMALL_PREDATOR_PLASMA_BOLT,
	SFX_FRISBEE_PLASMA_BOLT,
	SFX_MUZZLE_FLASH_SKEETER,
	MAX_NO_OF_SFXS,
};

typedef struct sfxblock
{
	enum SFX_ID SfxID;
	
	unsigned int EffectDrawnLastFrame:1; // useful for strobing effects


} SFXBLOCK;

#define SFXFLAG_ISAFFECTEDBYHEAT	0x1
#define SFXFLAG_MELTINGINTOGROUND	0x2
#define SFXFLAG_ONFIRE				0x4
#define SFXFLAG_SPARKING			0x8

typedef struct forcefield
{
	VECTORCH Corner;
	VECTORCH Scale;
	int ModuleIndex;
	
} FORCEFIELD;



extern void InitialiseSfxBlocks(void);
extern SFXBLOCK* AllocateSfxBlock(void);
extern void DeallocateSfxBlock(SFXBLOCK *sfxPtr);
DISPLAYBLOCK *CreateSFXObject(enum SFX_ID sfxID);


extern struct displayblock *CreateSFXObject(enum SFX_ID sfxID);
extern void DrawSfxObject(struct displayblock *dispPtr);

extern void HandleSfxForObject(DISPLAYBLOCK *dispPtr);
void HandleObjectOnFire(DISPLAYBLOCK *dispPtr);

#endif
