#ifndef scream_h 
#define scream_h 1

#ifdef __cplusplus
extern "C" {
#endif

typedef enum sound_category {
	SC_Angry=0,
	SC_Panic,
	SC_Pain,
	SC_Death,
	SC_Surprise,
	SC_Oooph,
	SC_OnFire,
	SC_Electrocution,
	SC_Sobbing,
	SC_Acid,
	SC_Facehugged,
	SC_PC_OnFire,
	SC_Taunt,
	SC_Falling,
	SC_Jump,
} SOUND_CATERGORY;

typedef enum alien_sound_category
{
	ASC_TailSound,
	ASC_Swipe,
	ASC_Scream_Hurt,
	ASC_Scream_Dying,
	ASC_Scream_General,
	ASC_Taunt,
	ASC_LimbLoss,
	ASC_Death,
	ASC_PC_OnFire,
}ALIEN_SOUND_CATEGORY;

typedef enum predator_sound_category
{
	PSC_Swipe,
	PSC_Scream_Hurt,
	PSC_Scream_Dying,
	PSC_Scream_General,
	PSC_Taunt,
	PSC_Acid,
	PSC_Facehugged,
	PSC_PC_OnFire,
	PSC_Jump,
	PSC_Medicomp_Special,
}PREDATOR_SOUND_CATEGORY;

typedef enum queen_sound_category
{
	QSC_Hiss,
	QSC_Scream_Hurt,

	QSC_Object_Bounce, //not actually used by the queen , but only occurs in queen's level
}QUEEN_SOUND_CATEGORY;

void UnloadScreamSounds();

void LoadMarineScreamSounds();
void LoadAlienScreamSounds();
void LoadPredatorScreamSounds();
void LoadQueenScreamSounds();

void PlayMarineScream(int VoiceType,int SoundCategory,int PitchShift,int* ExternalRef,VECTORCH* Location);
void PlayAlienSound(int VoiceType,int SoundCategory,int PitchShift,int* ExternalRef,VECTORCH* Location);
void PlayPredatorSound(int VoiceType,int SoundCategory,int PitchShift,int* ExternalRef,VECTORCH* Location);
void PlayQueenSound(int VoiceType,int SoundCategory,int PitchShift,int* ExternalRef,VECTORCH* Location);

#ifdef __cplusplus
};
#endif

#endif
