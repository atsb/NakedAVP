#include "detaillevels.h"

DETAIL_LEVELS LocalDetailLevels;
MENU_DETAIL_LEVEL_OPTIONS MenuDetailLevelOptions;

extern int GlobalLevelOfDetail_Hierarchical;


extern void SetToDefaultDetailLevels(void)
{
#if 0
	LocalDetailLevels.BloodCollidesWithEnvironment=1;
	LocalDetailLevels.DrawLightCoronas=1;
	LocalDetailLevels.DrawHierarchicalDecals=1;
	LocalDetailLevels.ExplosionsDeformToEnvironment=1;
	LocalDetailLevels.GhostFlameThrowerCollisions=0;

	LocalDetailLevels.MaximumAllowedNumberOfDecals = 1024;
	LocalDetailLevels.AlienEnergyViewThreshold = 0;
	LocalDetailLevels.NumberOfSmokeParticlesFromLargeExplosion=10;
	LocalDetailLevels.NumberOfSmokeParticlesFromSmallExplosion=5;
	
	GlobalLevelOfDetail_Hierarchical = 65536;
#endif

	MenuDetailLevelOptions.DecalNumber = 3;
	MenuDetailLevelOptions.LightCoronas = 1;
	MenuDetailLevelOptions.DecalsOnCharacters = 1;
	MenuDetailLevelOptions.DeformableExplosions = 1;
	MenuDetailLevelOptions.CharacterComplexity = 3;
	MenuDetailLevelOptions.ParticleComplexity = 1;
	SetDetailLevelsFromMenu();
}

extern void SetToMinimalDetailLevels(void)
{
#if 0
	LocalDetailLevels.BloodCollidesWithEnvironment=0;
	LocalDetailLevels.DrawLightCoronas=0;
	LocalDetailLevels.DrawHierarchicalDecals=0;
	LocalDetailLevels.ExplosionsDeformToEnvironment=0;
	LocalDetailLevels.GhostFlameThrowerCollisions=0;

	LocalDetailLevels.MaximumAllowedNumberOfDecals = 16;
	LocalDetailLevels.AlienEnergyViewThreshold = 450;
	LocalDetailLevels.NumberOfSmokeParticlesFromLargeExplosion=5;
	LocalDetailLevels.NumberOfSmokeParticlesFromSmallExplosion=2;

	GlobalLevelOfDetail_Hierarchical = 128*65536;
#endif

	MenuDetailLevelOptions.DecalNumber = 0;
	MenuDetailLevelOptions.LightCoronas = 0;
	MenuDetailLevelOptions.DecalsOnCharacters = 0;
	MenuDetailLevelOptions.DeformableExplosions = 0;
	MenuDetailLevelOptions.CharacterComplexity = 0;
	MenuDetailLevelOptions.ParticleComplexity = 0;
	SetDetailLevelsFromMenu();
}


extern void SetDetailLevelsFromMenu(void)
{
	switch (MenuDetailLevelOptions.DecalNumber)
	{
		default:
		case 0:
		{
			LocalDetailLevels.MaximumAllowedNumberOfDecals = 16;
			break;
		}
		case 1:
		{
			LocalDetailLevels.MaximumAllowedNumberOfDecals = 128;
			break;
		}
		case 2:
		{
			LocalDetailLevels.MaximumAllowedNumberOfDecals = 512;
			break;
		}
		case 3:
		{
			LocalDetailLevels.MaximumAllowedNumberOfDecals = 1024;
			break;
		}
	}

	LocalDetailLevels.DrawLightCoronas = MenuDetailLevelOptions.LightCoronas;
	LocalDetailLevels.DrawHierarchicalDecals = MenuDetailLevelOptions.DecalsOnCharacters;
	LocalDetailLevels.ExplosionsDeformToEnvironment = MenuDetailLevelOptions.DeformableExplosions;
	
	switch (MenuDetailLevelOptions.CharacterComplexity)
	{
		default:
		case 0:
		{
			GlobalLevelOfDetail_Hierarchical = 65536*128;
			break;
		}
		case 1:
		{
			GlobalLevelOfDetail_Hierarchical = 65536*4;
			break;
		}
		case 2:
		{
			GlobalLevelOfDetail_Hierarchical = 65536*2;
			break;
		}
		case 3:
		{
			GlobalLevelOfDetail_Hierarchical = 65536;
			break;
		}
	}
	
	switch (MenuDetailLevelOptions.ParticleComplexity)
	{
		default:
		case 0:
		{
			LocalDetailLevels.BloodCollidesWithEnvironment=0;
			LocalDetailLevels.AlienEnergyViewThreshold = 300;
			LocalDetailLevels.NumberOfSmokeParticlesFromLargeExplosion=5;
			LocalDetailLevels.NumberOfSmokeParticlesFromSmallExplosion=2;
			LocalDetailLevels.GhostFlameThrowerCollisions=0;
			break;
		}
		case 1:
		{
			LocalDetailLevels.BloodCollidesWithEnvironment=1;
			LocalDetailLevels.AlienEnergyViewThreshold = 0;
			LocalDetailLevels.NumberOfSmokeParticlesFromLargeExplosion=10;
			LocalDetailLevels.NumberOfSmokeParticlesFromSmallExplosion=5;
			LocalDetailLevels.GhostFlameThrowerCollisions=1;
			break;
		}
	}

}





















