extern "C"
{

#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "gammacontrol.h"

static int ActualGammaSetting;
int RequestedGammaSetting;

unsigned char GammaValues[256];

void InitialiseGammaSettings(int gamma)
{
	ActualGammaSetting = gamma+1;
	RequestedGammaSetting = gamma;
	UpdateGammaSettings();
}

void UpdateGammaSettings(void)
{
	if (RequestedGammaSetting==ActualGammaSetting) return;

	for (int i=0; i<=255; i++)
	{
		int u = ((i*65536)/255);
		int m = MUL_FIXED(u,u);
		int l = MUL_FIXED(2*u,ONE_FIXED-u);

		int a;
		
		a = m+MUL_FIXED(RequestedGammaSetting*256,l);


		m = MUL_FIXED(a,a);
		l = MUL_FIXED(2*a,ONE_FIXED-a);

		a = m/256+MUL_FIXED(RequestedGammaSetting,l);

		if (a<0) a=0;
		if (a>255) a=255;

		GammaValues[i]=a;
	}

	ActualGammaSetting=RequestedGammaSetting;

}

};
