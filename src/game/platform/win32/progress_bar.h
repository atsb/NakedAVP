
#ifndef _progress_bar_h
#define _progress_bar_h

#ifdef __cplusplus
extern "C"
{
#endif

#define PBAR_HUD_START		0
#define PBAR_LEVEL_START	1000
#define PBAR_NPC_START		3000
#define PBAR_LENGTH			5000

#define PBAR_HUD_INTERVAL	(PBAR_LEVEL_START)
#define PBAR_LEVEL_INTERVAL	(PBAR_NPC_START-PBAR_LEVEL_START)
#define PBAR_NPC_INTERVAL	(PBAR_LENGTH-PBAR_NPC_START)

void Start_Progress_Bar();
void Set_Progress_Bar_Position(int pos);
void Game_Has_Loaded(void);

#ifdef __cplusplus
};
#endif



#endif
