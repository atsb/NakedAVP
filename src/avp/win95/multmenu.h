/*-----------------------Patrick 7/5/97---------------------------
  Header file for Multiplayer menus: I have heavily modifies these
------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef MULTMENU_HPP_INCLUDED
#define MULTMENU_HPP_INCLUDED 1

typedef enum muliplayer_select_options
{
	/* these options can be selected by host & peers */
	MMSelect_Start,
	MMSelect_Cancel,
	MMSelect_CharacterSelection,
	/* these options can be selected by host only */
	MMSelect_LevelSelection,
	MMSelect_ModeSelection,
	MMSelect_ScoreLimitSelection,
	MMSelect_TimeLimitSelection,
	/* these options cannot be selected */
	MMSelect_Title,
	MMSelect_P1Name,
	MMSelect_P2Name,
	MMSelect_P3Name,
	MMSelect_P4Name,
	MMSelect_P5Name,
	MMSelect_P6Name,
	MMSelect_P7Name,
	MMSelect_P8Name,
	MMSelect_P1Type,
	MMSelect_P2Type,
	MMSelect_P3Type,
	MMSelect_P4Type,
	MMSelect_P5Type,
	MMSelect_P6Type,
	MMSelect_P7Type,
	MMSelect_P8Type,
	MMSelect_P1Ok,
	MMSelect_P2Ok,
	MMSelect_P3Ok,
	MMSelect_P4Ok,
	MMSelect_P5Ok,
	MMSelect_P6Ok,
	MMSelect_P7Ok,
	MMSelect_P8Ok,
	MMSelect_Character,
	MMSelect_Level,
	MMSelect_Mode,
	MMSelect_ScoreLimit,
	MMSelect_TimeLimit,
	MMSelect_Max,
}MULTIPLAYER_SELECT_OPTIONS;


typedef enum muliplayer_endgame_items
{
	MMEndItem_PlayerName1,
	MMEndItem_PlayerName2,
	MMEndItem_PlayerName3,
	MMEndItem_PlayerName4,
	MMEndItem_PlayerName5,
	MMEndItem_PlayerName6,
	MMEndItem_PlayerName7,
	MMEndItem_PlayerName8,
	MMEndItem_PlayerCharacter1,
	MMEndItem_PlayerCharacter2,
	MMEndItem_PlayerCharacter3,
	MMEndItem_PlayerCharacter4,
	MMEndItem_PlayerCharacter5,
	MMEndItem_PlayerCharacter6,
	MMEndItem_PlayerCharacter7,
	MMEndItem_PlayerCharacter8,
	MMEndItem_PlayerTotalScore1,
	MMEndItem_PlayerTotalScore2,
	MMEndItem_PlayerTotalScore3,
	MMEndItem_PlayerTotalScore4,
	MMEndItem_PlayerTotalScore5,
	MMEndItem_PlayerTotalScore6,
	MMEndItem_PlayerTotalScore7,
	MMEndItem_PlayerTotalScore8,	
	MMEndItem_Player1Initial1,
	MMEndItem_Player2Initial1,
	MMEndItem_Player3Initial1,
	MMEndItem_Player4Initial1,
	MMEndItem_Player5Initial1,
	MMEndItem_Player6Initial1,
	MMEndItem_Player7Initial1,
	MMEndItem_Player8Initial1,
	MMEndItem_Player1Initial2,
	MMEndItem_Player2Initial2,
	MMEndItem_Player3Initial2,
	MMEndItem_Player4Initial2,
	MMEndItem_Player5Initial2,
	MMEndItem_Player6Initial2,
	MMEndItem_Player7Initial2,
	MMEndItem_Player8Initial2,
	MMEndItem_Player1Score1,
	MMEndItem_Player1Score2,
	MMEndItem_Player1Score3,
	MMEndItem_Player1Score4,
	MMEndItem_Player1Score5,
	MMEndItem_Player1Score6,
	MMEndItem_Player1Score7,
	MMEndItem_Player1Score8,
	MMEndItem_Player2Score1,
	MMEndItem_Player2Score2,
	MMEndItem_Player2Score3,
	MMEndItem_Player2Score4,
	MMEndItem_Player2Score5,
	MMEndItem_Player2Score6,
	MMEndItem_Player2Score7,
	MMEndItem_Player2Score8,
	MMEndItem_Player3Score1,
	MMEndItem_Player3Score2,
	MMEndItem_Player3Score3,
	MMEndItem_Player3Score4,
	MMEndItem_Player3Score5,
	MMEndItem_Player3Score6,
	MMEndItem_Player3Score7,
	MMEndItem_Player3Score8,
	MMEndItem_Player4Score1,
	MMEndItem_Player4Score2,
	MMEndItem_Player4Score3,
	MMEndItem_Player4Score4,
	MMEndItem_Player4Score5,
	MMEndItem_Player4Score6,
	MMEndItem_Player4Score7,
	MMEndItem_Player4Score8,
	MMEndItem_Player5Score1,
	MMEndItem_Player5Score2,
	MMEndItem_Player5Score3,
	MMEndItem_Player5Score4,
	MMEndItem_Player5Score5,
	MMEndItem_Player5Score6,
	MMEndItem_Player5Score7,
	MMEndItem_Player5Score8,
	MMEndItem_Player6Score1,
	MMEndItem_Player6Score2,
	MMEndItem_Player6Score3,
	MMEndItem_Player6Score4,
	MMEndItem_Player6Score5,
	MMEndItem_Player6Score6,
	MMEndItem_Player6Score7,
	MMEndItem_Player6Score8,
	MMEndItem_Player7Score1,
	MMEndItem_Player7Score2,
	MMEndItem_Player7Score3,
	MMEndItem_Player7Score4,
	MMEndItem_Player7Score5,
	MMEndItem_Player7Score6,
	MMEndItem_Player7Score7,
	MMEndItem_Player7Score8,
	MMEndItem_Player8Score1,
	MMEndItem_Player8Score2,
	MMEndItem_Player8Score3,
	MMEndItem_Player8Score4,
	MMEndItem_Player8Score5,
	MMEndItem_Player8Score6,
	MMEndItem_Player8Score7,
	MMEndItem_Player8Score8,
	MMEndItem_Ok,
	MMEndItem_Max
}MULTIPLAYER_ENDGAME_ITEMS;




/* prototypes */
extern int RunMultiplayerStartUp(int lobbied);
extern void EndOfNetworkGameScreen(void);


#endif

#ifdef __cplusplus
}
#endif
