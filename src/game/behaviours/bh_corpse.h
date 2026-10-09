#ifndef bh_corpse_h_included
#define bh_corpse_h_included
#ifdef __cplusplus
extern "C" {
#endif

#define CORPSE_SIGHTINGS 1

typedef struct netcorpsedatablock {
	int timer;
	int validityTimer;
	int SoundHandle; /* Just in case. */
	int SoundHandle2;
	int SoundHandle3;
	int SoundHandle4;
	HMODELCONTROLLER HModelController;
	
	AVP_BEHAVIOUR_TYPE Type;
	HITLOCATIONTABLE *hltable;
	int GibbFactor;
	DEATH_DATA *This_Death;
	/* If you're a predator... */
	PRED_CLOAKSTATE CloakStatus;
	int CloakTimer;
	int destructTimer;
	/* If you're a marine... */
	void (*WeaponMisfireFunction)(SECTION_DATA *, int *);
	SECTION_DATA *My_Gunflash_Section;
	SECTION *TemplateRoot;
	struct marine_weapon_data *My_Weapon;
	int weapon_variable;
	int Android;
	int ARealMarine;
	/* If you're an alien... */
	int subtype;

	int Wounds;

	int DeathFiring	:1;
	

}NETCORPSEDATABLOCK;

extern void Convert_Alien_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death,DAMAGE_PROFILE* damage);
extern void Convert_Predator_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death);
extern void Convert_Marine_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death);
extern void Convert_Xenoborg_To_Corpse(STRATEGYBLOCK *sbPtr,DEATH_DATA *this_death);
extern void CorpseBehaveFun(STRATEGYBLOCK *sbPtr);
extern void MakeCorpseNear(STRATEGYBLOCK *sbPtr);
extern void MakeCorpseFar(STRATEGYBLOCK *sbPtr);
extern void CorpseIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple, int wounds,SECTION_DATA *Section,VECTORCH *incoming);

#define CORPSE_EXPIRY_TIME		(ONE_FIXED*10)
#define CORPSE_VALIDITY_TIME	(ONE_FIXED>>2)
#define ALIEN_DYINGTIME			(ONE_FIXED*8)
#define PRED_DIETIME			(ONE_FIXED*16)
#define MARINE_DYINGTIME		(ONE_FIXED*16)
#define XENO_DYINGTIME			(ONE_FIXED*8)
#define AGUN_DYINGTIME			(ONE_FIXED*8)
#define HDEBRIS_LIFETIME		(ONE_FIXED*8)
/* Was (ONE_FIXED*3)... */

#ifdef __cplusplus
}
#endif

#endif
