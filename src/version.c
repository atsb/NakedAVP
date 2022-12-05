#include "version.h"

extern void NewOnScreenMessage(unsigned char *messagePtr);

const char *AvPVersionString = "Aliens vs Predator Linux \n     Build 000 (CVS) \n     Based on Rebellion Developments AvP Gold source \n";

void GiveVersionDetails(void)
{
	NewOnScreenMessage((unsigned char *)AvPVersionString);
}
