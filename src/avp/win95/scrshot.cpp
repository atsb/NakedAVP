#include "3dc.h"
#include "bmp2.h"
#include "endianio.h"
#include "string.hpp"
#include "scrshot.hpp"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "ourasert.h"
#include "frontend/avp_menus.h"

extern "C"{
 extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
 extern int VideoModeTypeScreen;
 extern unsigned char *ScreenBuffer;
 extern unsigned char TestPalette[];
 extern unsigned char KeyboardInput[];
 extern unsigned char DebouncedKeyboardInput[];
 extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;
 extern MODULE* playerPherModule;	
};

typedef VOID (*PutWord_F)(WORD, FILE *);
typedef VOID (*PutDword_F)(DWORD, FILE *);

extern void LoadModuleData();

void LogCameraPosForModuleLinking()
{
	fprintf(stderr, "STUB: LogCameraPosForModuleLinking()\n");
	
#if 0 /* TODO: commented out because I want to know if its actually used */
	if(!playerPherModule) return;
	if(!playerPherModule->name) return;
	
	char Filename[100]={"avp_rifs/"};
	
	strcat(Filename,Env_List[AvP.CurrentEnv]->main);
	strcat(Filename,".mlf");

	FILE* file=fopen(Filename,"ab");
	if(!file)return;

	char output_buffer[300];
	int length=0;
	
	strcpy(output_buffer,playerPherModule->name);
	length+=(strlen(playerPherModule->name)+4)&~3;

	*(VECTORCH*)&output_buffer[length]=playerPherModule->m_world;
	length+=sizeof(VECTORCH);
	*(MATRIXCH*)&output_buffer[length]=Global_VDB_Ptr->VDB_Mat;
	length+=sizeof(MATRIXCH);
	*(VECTORCH*)&output_buffer[length]=Global_VDB_Ptr->VDB_World;
	length+=sizeof(VECTORCH);
	
	if(length%4 !=0)
	{
		GLOBALASSERT(0);
	}

	fwrite(&output_buffer[0],4,length/4,file);
	fclose(file);		
	textprint("Saving camera for module links");
#endif	
}
int SaveCameraPosKeyPressed=0;
#ifdef AVP_DEBUG_VERSION
static BOOL ModuleLinkAssist=FALSE;
#endif

void HandleScreenShot()
{
	#ifdef AVP_DEBUG_VERSION

	if (DebouncedKeyboardInput[KEY_F8])
		ScreenShot();

	if (KeyboardInput[KEY_F7])
	{
		if(!SaveCameraPosKeyPressed)
		{
			if(KeyboardInput[KEY_LEFTSHIFT]||KeyboardInput[KEY_RIGHTSHIFT])
			{
				ModuleLinkAssist=TRUE;
				DeleteFile("avp_rifs\\module.aaa");
			}
			else
			{
				LogCameraPosForModuleLinking();
				SaveCameraPosKeyPressed=1;
			}
		}
	}
	else
		SaveCameraPosKeyPressed=0;
	
	if(AvP.MainLoopRunning && ModuleLinkAssist)LoadModuleData();

	#endif
}

extern "C" {
unsigned char *GetScreenShot24(int *width, int *height);
};
				 
void ScreenShot()
{
	int i;
	char Name[40];
	
	int width, height;
	unsigned char *buf = GetScreenShot24(&width, &height);
	if (buf == NULL)
		return;
	
	strcpy(Name,"avp");
	int length=strlen(Name);
	strncpy(&Name[length],"00.bmp",8);
	for(i=0;i<100;i++)
	{
		Name[length]=i/10+'0';
		Name[length+1]=(i%10)+'0';
		FILE* tempfp = OpenGameFile(Name, FILEMODE_READONLY, FILETYPE_CONFIG);
		if(!tempfp)break;
		else
 		{
			fclose(tempfp);
		}
	}
	if(i==100) return;
	
	FILE *fp = OpenGameFile(Name, FILEMODE_WRITEONLY, FILETYPE_CONFIG);
	if (!fp)
	{
		return;
	}
	
	BMPHEADER2 h;

	// fill out header
	
	h.Header.Type      = 'B'+'M'*256;
	h.Header.Reserved1 = 0;
	h.Header.Reserved2 = 0;
	h.Header.Offset    = 14+40+0;

	/*
	** The type of information found in a BMP structure is indicated by
	** the Size (Information Headere Size) field with a non-zero value.
	*/
	h.PmInfo.Size   = 0;
	h.Pm2Info.Size  = 0;

	h.WinInfo.Size          = 40;
	h.WinInfo.Width         = width;
	h.WinInfo.Height        = height;
	h.WinInfo.Planes        = 1;
	h.WinInfo.BitCount      = 24;
	h.WinInfo.Compression   = 0;
	h.WinInfo.SizeImage     = h.WinInfo.Width*h.WinInfo.Height*3;
	h.WinInfo.XPelsPerMeter = h.WinInfo.Width;
	h.WinInfo.YPelsPerMeter = h.WinInfo.Height;
	h.WinInfo.ClrUsed       = 0;
	h.WinInfo.ClrImportant  = 0;

	h.Header.FileSize  = h.WinInfo.SizeImage + h.Header.Offset + 8;

	// write header

	PutWord_F PutWord = PutLittleWord;
	PutDword_F PutDword = PutLittleDword;

    PutWord(h.Header.Type, fp);
    PutDword(h.Header.FileSize, fp);
    PutWord(h.Header.Reserved1, fp);
    PutWord(h.Header.Reserved2, fp);
    PutDword(h.Header.Offset, fp);

	PutDword(h.WinInfo.Size, fp);

    PutDword(h.WinInfo.Width, fp);
    PutDword(h.WinInfo.Height, fp);
    PutWord(h.WinInfo.Planes, fp);
    PutWord(h.WinInfo.BitCount, fp);
    PutDword(h.WinInfo.Compression, fp);
    PutDword(h.WinInfo.SizeImage, fp);
    PutDword(h.WinInfo.XPelsPerMeter, fp);
    PutDword(h.WinInfo.YPelsPerMeter, fp);
    PutDword(h.WinInfo.ClrUsed, fp);
    PutDword(h.WinInfo.ClrImportant, fp);

	// write 24 bit image

//	unsigned char *BufferPtr = &buf[(width * 3) * (height - 1)];
	unsigned char *BufferPtr = &buf[0];
	for (i=h.WinInfo.Height-1; i>=0; --i)
	{
		unsigned int j;
		for (j=0; j<h.WinInfo.Width; ++j)
		{
			PutByte((BYTE)BufferPtr[j*3+2],fp);  //b
			PutByte((BYTE)BufferPtr[j*3+1],fp);  //g
			PutByte((BYTE)BufferPtr[j*3],fp);  //r
		}
			
		// pad to 4 byte boundary
		for (j=~(h.WinInfo.Width*3-1) & 3; j; --j) PutByte(0,fp);
//		BufferPtr -= width * 3;
		BufferPtr += width * 3;
	}

	free(buf);
	fclose(fp);	
}
