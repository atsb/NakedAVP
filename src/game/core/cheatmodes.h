#ifndef CHEATMODES_H
#define CHEATMODES_H

int AnyCheatModesAllowed(void);
void CheatMode_GetNextAllowedSpecies(int *speciesPtr, int searchForward);
void CheatMode_GetNextAllowedEnvironment(int *environmentPtr, int searchForward);
void CheatMode_GetNextAllowedMode(int *cheatModePtr, int searchForward);
void CheatMode_CheckOptionsAreValid(void);

#endif
