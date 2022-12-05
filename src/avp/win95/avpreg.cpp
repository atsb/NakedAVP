#include <windows.h>
#include <stdio.h>
#include "avpreg.hpp"

extern "C"
{
char* AvpCDPath=0;
extern char const * SecondTex_Directory;
extern char * SecondSoundDir;

void GetPathFromRegistry()
{
	HKEY hKey;
	
	if(AvpCDPath)
	{
		delete AvpCDPath;
		AvpCDPath=0;
	}
	
	if
	(
		ERROR_SUCCESS == RegOpenKeyEx
		(
			HKEY_LOCAL_MACHINE,
			"Software\\Fox Interactive\\Aliens vs Predator\\1.00",
			REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,
			&hKey
		)
	)
	{
		char szBuf[MAX_PATH+1];
		DWORD dwType = REG_NONE;
		DWORD dwSize = sizeof szBuf;
		
		if
		(
			ERROR_SUCCESS == RegQueryValueEx
			(
				hKey,
				const_cast<char *>("Source Path"),
				0,
				&dwType,
				reinterpret_cast<LPBYTE>(szBuf),
				&dwSize
			)
			&& REG_SZ == dwType
		)
		{
			int length=strlen(szBuf);
			if(length)
			{
				AvpCDPath=new char[length+1];
				strcpy(AvpCDPath,szBuf);
			}
				
		}

		RegCloseKey(hKey);
	}

	//now set second texture directory
	if(!SecondTex_Directory)
	{
		char* directory;
		if(AvpCDPath)
		{
			directory=new char[strlen(AvpCDPath)+10];
			sprintf(directory,"%sGraphics",AvpCDPath);		
		}
		else
		{
			directory=new char[40];
			strcpy(directory,"\\\\bob\\textures\\avp_graphics");
		}
		*(char**)&SecondTex_Directory=directory;
	}

	//and the second sound directory
	if(!SecondSoundDir)
	{
		char* directory;
		if(AvpCDPath)
		{
			directory=new char[strlen(AvpCDPath)+20];
			sprintf(directory,"%ssound\\",AvpCDPath);		
		}
		else
		{
			directory=new char[40];
			strcpy(directory,"\\\\bob\\vss\\avp\\sound\\");
		}
		SecondSoundDir=directory;
	}


}

};