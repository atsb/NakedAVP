#include <string.h>

#include "fixer.h"

#include "string.hpp"
#ifndef DB_LEVEL
	#define DB_LEVEL 4
#endif
#include "db.h"

#include "awtexld.h"

#include "chnkload.hpp"
#include "chunkpal.hpp"

#ifndef CL_SUPPORT_FASTFILE
	#error "Please #define CL_SUPPORT_FASTFILE to 0 or 1 in projload.hpp"
#endif
#if CL_SUPPORT_FASTFILE
	#include "ffstdio.h"
#endif

//#ifndef CL_SUPPORT_ALTTAB
//	#error "Please #define CL_SUPPORT_ALTTAB to 0 or 1 in projload.hpp"
//#endif
//#if CL_SUPPORT_ALTTAB
//	#include "alt_tab.h"
//#endif

#include "chnktexi.h"

#if !defined(NDEBUG) && defined(_CPPRTTI)
	#include <typeinfo.h>
#else
	#define dynamic_cast static_cast
#endif
	
char const * cl_pszGameMode = NULL;

// used to determine if the display is palettized
// currently assuming that if this is <= 8 then all
// surfaces et. except d3d textures have a global palette
extern "C" {
	extern int VideoModeColourDepth;
};

// useful filename handling functions

// returns pointer into string pointing to filename without dirname
template <class C> // C can be char or char const
static C * StripPath(C * n)
{
	C * rm = strrchr(n,':');
	if (rm) n = rm+1;
	rm = strrchr(n,'/');
	if (rm) n = rm+1;
	rm = strrchr(n,'\\');
	if (rm) n = rm+1;

	return n;
}

// removes any .extension from filename by inserting null character
static void StripFileExtension(char * n)
{
	char * dotpos = strrchr(n,'.');
	if (dotpos) *dotpos = 0;
}

static char * StripFileExtension(char const * n)
{
	char * nn = new char[strlen(n)+1];
	strcpy(nn,n);
	StripFileExtension(nn);
	return nn;
}

// get the directory associated with the riff - free with delete[]
static char * RiffBasename(Chunk_With_Children * pEnvDataChunk)
{
	Chunk * pChunk = pEnvDataChunk->lookup_single_child("RIFFNAME");
	
	if (pChunk)
	{
		RIF_Name_Chunk * pRifNameChunk = dynamic_cast<RIF_Name_Chunk *>(pChunk);
		
		const char * pszRifName = StripPath(pRifNameChunk->rif_name);

		char * pszBaseName = new char[strlen(pszRifName)+1];
		strcpy(pszBaseName,pszRifName);
		StripFileExtension(pszBaseName);

		return pszBaseName;
	}
	const char * pszDefault = "empty";
	char * pszBaseName = new char [strlen(pszDefault)+1];
	strcpy(pszBaseName,pszDefault);

	return pszBaseName;
}

#if CL_SUPPORT_FASTFILE
static inline bool IsFileInFastFile(char const * pszFileName)
{
	size_t nLen;
	return ffreadbuf(pszFileName,&nLen) ? true : false;
}
#endif

static bool DoesFileExist(char const * pszFileName)
{
	unsigned int attr = GetGameFileAttributes(pszFileName, FILETYPE_PERM);

	if ((attr & FILEATTR_DIRECTORY) != 0)
		return false;
	if ((attr & FILEATTR_READABLE) == 0)
		return false;
	return true;
}

static char * GetPath(char * pszFileNameBuf, unsigned nBufSize, ImageDescriptor const & idsc, Chunk_With_Children * pEnvDataChunk, bool bGloballyPalettized)
{
	// set the name
	char const * pszRawName = StripPath(idsc.filename);
	char * pszName = new char[strlen(pszRawName)+1];
	strcpy(pszName,pszRawName);
	StripFileExtension(pszName);
					
	// load this image
	char const * pg0ext = ".PG0";
	
	if (!bGloballyPalettized)
	{
		char const * dir2 = idsc.flags & IDSCF_SPRITE ? "Sprites\\" : idsc.flags & IDSCF_SUBSHAPE ? SubShps_Directory : "";
		char * riffname = RiffBasename(pEnvDataChunk);
		char const * dir3 = idsc.flags & IDSCF_INCLUDED ? idsc.rifname : riffname;
		if (nBufSize < strlen(GenTex_Directory)+strlen(dir2)+strlen(dir3)+1+strlen(pszName)+5)
		{
			db_log1("CL_LoadImageOnce(): ERROR: buffer not big enough for filename");
			pszFileNameBuf = NULL;
		}
		else
		{
			strcpy(pszFileNameBuf,GenTex_Directory);
			strcat(pszFileNameBuf,dir2);
			strcat(pszFileNameBuf,dir3);
			strcat(pszFileNameBuf,"\\");
			strcat(pszFileNameBuf,pszName);
			strcat(pszFileNameBuf,".BM0");
		}
		delete[] riffname;
	}
	else
	{
		if (idsc.flags & IDSCF_FIXEDPALETTE)
		{
			char const * dir2 = idsc.fixrifname ? *idsc.fixrifname ? idsc.fixrifname : 0 : 0;
			char const * dir3 = idsc.flags & IDSCF_SPRITE ? "Sprites\\" : "";
			char * riffname = RiffBasename(pEnvDataChunk);
			char const * dir4 = idsc.flags & IDSCF_INCLUDED ? idsc.rifname : riffname;
			if (nBufSize < strlen(FixTex_Directory)+(dir2 ? strlen(dir2)+1 : 0)+strlen(dir3)+strlen(dir4)+1+strlen(pszName)+5)
			{
				db_log1("CL_LoadImageOnce(): ERROR: buffer not big enough for filename");
				pszFileNameBuf = NULL;
			}
			else
			{
				strcpy(pszFileNameBuf,FixTex_Directory);
				if (dir2)
				{
					strcat(pszFileNameBuf,dir2);
					strcat(pszFileNameBuf,"\\");
				}
				strcat(pszFileNameBuf,dir3);
				strcat(pszFileNameBuf,dir4);
				strcat(pszFileNameBuf,"\\");
				strcat(pszFileNameBuf,pszName);
				strcat(pszFileNameBuf,pg0ext);
			}
			delete[] riffname;
		}
		else
		{
			char const * dir1 = cl_pszGameMode ? GameTex_Directory : ToolsTex_Directory;
			char * dir2 = RiffBasename(pEnvDataChunk);
			char const * dir4 = idsc.flags & IDSCF_SPRITE ? "Sprites\\" : "";
			char const * dir5 = idsc.flags & IDSCF_INCLUDED ? idsc.rifname : 0;
			if (nBufSize < strlen(dir1)+strlen(dir2)+1+(cl_pszGameMode ? strlen(cl_pszGameMode)+1 : 0)+strlen(dir4)+(dir5 ? strlen(dir5)+1 : 0)+strlen(pszName)+5)
			{
				db_log1("CL_LoadImageOnce(): ERROR: buffer not big enough for filename");
				pszFileNameBuf = NULL;
			}
			else
			{
				strcpy(pszFileNameBuf,dir1);
				strcat(pszFileNameBuf,dir2);
				strcat(pszFileNameBuf,"\\");
				if (cl_pszGameMode)
				{
					strcat(pszFileNameBuf,cl_pszGameMode);
					strcat(pszFileNameBuf,"\\");
				}
				strcat(pszFileNameBuf,dir4);
				if (dir5)
				{
					strcat(pszFileNameBuf,dir5);
					strcat(pszFileNameBuf,"\\");
				}
				strcat(pszFileNameBuf,pszName);
				strcat(pszFileNameBuf,pg0ext);
			}
			delete[] dir2;
		}
	}

	if (pszFileNameBuf)
		db_logf4(("\tfile expected to be '%s'",pszFileNameBuf));
		
	delete[] pszName;
		
	return pszFileNameBuf;
}

char * CL_GetImageFileName(char * pszDestBuf, unsigned nBufSize, char const * pszFileName, unsigned fFlagsEtc)
{
	db_assert1(pszFileName);
	db_assert1(pszDestBuf);
	db_assert1(nBufSize>0);
	db_logf4(("CL_LoadImageOnce(): Getting the full pathname for %s",pszFileName));
	switch (fFlagsEtc & _LIO_PATHTYPEMASK)
	{
		case LIO_ABSOLUTEPATH:
			db_log4("\t(which is an absolute path)");
			if (strlen(pszFileName)<nBufSize)
			{
				strcpy(pszDestBuf,pszFileName);
				return pszDestBuf;
			}
			else
			{
				db_log1("CL_LoadImageOnce(): ERROR: buffer not large enough to hold filename");
				return NULL;
			}
		case LIO_RELATIVEPATH:
			db_logf4(("\t(which is a path relative to %s or %s)",
				FirstTex_Directory ? FirstTex_Directory : "<not-specified>",
				SecondTex_Directory ? SecondTex_Directory : "<not-specified>"));
			#define _GET_RELATIVE_PATH(pszDirectory,fnDoesExist) \
				if (pszDirectory) { \
					String str = pszDirectory; \
					if (str.length()) { \
						int chLast = str.get_at(str.length()-1); \
						if (chLast != '\\' && chLast != '/') \
							str += '\\'; \
						str += pszFileName; \
						if (fnDoesExist(str)) { \
							if (str.length() < nBufSize) { \
								strcpy(pszDestBuf,str); \
								return pszDestBuf; \
							} else { \
								db_log1("CL_LoadImageOnce(): ERROR: buffer not large enough to hold filename"); \
								return NULL; /* fail because the buffer isnt big enough */ \
				}	}	}	}
				
			#if CL_SUPPORT_FASTFILE
			_GET_RELATIVE_PATH(FirstTex_Directory,IsFileInFastFile)
			_GET_RELATIVE_PATH(SecondTex_Directory,IsFileInFastFile)
			#endif
			#if !defined(CL_SUPPORTONLY_FASTFILE) || !CL_SUPPORTONLY_FASTFILE
			_GET_RELATIVE_PATH(FirstTex_Directory,DoesFileExist)
			_GET_RELATIVE_PATH(SecondTex_Directory,DoesFileExist)
			#endif
			db_log1("CL_LoadImageOnce(): ERROR: file not found in relative path");
			return NULL;
		case LIO_RIFFPATH:
		{
			int enum_id = -1; // may be supported at a later date, valid when pszFileName is NULL
			
			bool bGloballyPalettized = VideoModeColourDepth <= 8 && (fFlagsEtc & _LIO_SURFTYPEMASK) != LIO_D3DTEXTURE;
			
			db_log4("\t(whose path is determined by the current .RIF file)");
			if (!Env_Chunk)
			{
				db_log1("CL_LoadImageOnce(): ERROR: no RIF file");
				return NULL;
			}
			Chunk * pChunk = Env_Chunk->lookup_single_child("REBENVDT");
			if (!pChunk)
			{
				db_log1("CL_LoadImageOnce(): ERROR: no environment data chunk");
				return NULL;
			}
			Chunk_With_Children * pEnvDataChunk = dynamic_cast<Chunk_With_Children *>(pChunk);
			db_assert1(pEnvDataChunk);
			
			Environment_Game_Mode_Chunk * pGameModeChunk = NULL;
			if (cl_pszGameMode)
			{
				if (*cl_pszGameMode)
				{
					List<Chunk *> listGmChunks;
					pEnvDataChunk->lookup_child("GAMEMODE",listGmChunks);
					
					for (LIF<Chunk *> itGmChunks(&listGmChunks); !itGmChunks.done(); itGmChunks.next())
					{
						Environment_Game_Mode_Chunk * pGmChunk = dynamic_cast<Environment_Game_Mode_Chunk *>(itGmChunks());
						db_assert1(pGmChunk);
						if (pGmChunk->id_equals(cl_pszGameMode))
						{
							pGameModeChunk = pGmChunk;
							break;
						}
					}

					if (!pGameModeChunk)
						db_logf3(("CL_LoadImageOnce(): WARNING: Game Mode '%s' not found",cl_pszGameMode));
				}
			}

			char * pszName = StripFileExtension(StripPath(pszFileName));
			
			char * pszSubName = 0;
			if (pszFileName)
			{
				if (strchr(pszFileName,'\\'))
				{
					pszSubName = new char[strlen(pszFileName)+1];
					strcpy(pszSubName,pszFileName);
					*strchr(pszSubName,'\\')=0;
				}
				else if (strchr(pszFileName,'/'))
				{
					pszSubName = new char[strlen(pszFileName)+1];
					strcpy(pszSubName,pszFileName);
					*strchr(pszSubName,'/')=0;
				}
			}

			if (pGameModeChunk)
			{
				bool bShapeInGm = pszSubName ? false : true;
				// Get the matching image 'Processor' chunk
				Chunk * pMiChunk = pGameModeChunk->lookup_single_child("MATCHIMG");
				Matching_Images_Chunk * pMatchImageChunk = NULL;
				
				if (pMiChunk)
				{
					pMatchImageChunk = dynamic_cast<Matching_Images_Chunk *>(pMiChunk);
					db_assert1(pMatchImageChunk);
				}
				
				List<Chunk *> listRifChildChunks;
				pGameModeChunk->lookup_child("RIFCHILD",listRifChildChunks);
				
				for (LIF<Chunk *> itRifChildChunks(&listRifChildChunks); !itRifChildChunks.done(); itRifChildChunks.next())
				{
					RIF_Child_Chunk * pRifChildChunk = dynamic_cast<RIF_Child_Chunk *>(itRifChildChunks());
					db_assert1(pRifChildChunk);

					if (pszSubName)
					{
						if (_stricmp(pszSubName,pRifChildChunk->rifname) && (*pszSubName || *pRifChildChunk->filename))
							continue;
						bShapeInGm = true;
					}

					for (LIF<BMP_Flags> itBmpFlags(&pRifChildChunk->bmps); !itBmpFlags.done(); itBmpFlags.next())
					{
						BMP_Flags bmpfTemp(itBmpFlags());
						StripFileExtension(bmpfTemp.filename);

						if (pszFileName ? !_stricmp(pszName,StripPath(bmpfTemp.filename)) : enum_id == bmpfTemp.enum_id)
						{
							// select image descriptor
							ImageDescriptor const idsc
								(
									*pRifChildChunk->filename ? 
										(IDscFlags)((bmpfTemp.flags & ChunkBMPFlag_FixedPalette ?
											IDSCF_FIXEDPALETTE
										:
											IDSCF_0)
										|IDSCF_INCLUDED)
									:
										IDSCF_0,
									itBmpFlags().filename,
									*pRifChildChunk->filename ? pRifChildChunk->rifname : 0
								);
							ImageDescriptor const * pIdsc = &idsc;

							if (pMatchImageChunk) pIdsc = &pMatchImageChunk->GetLoadImage(idsc);
							else db_logf3(("CL_LoadImageOnce(): WARNING! no rule to find matching images in game mode '%s'\n",pGameModeChunk->header->mode_identifier));

							// load this image
							if (GetPath(pszDestBuf,nBufSize,*pIdsc,pEnvDataChunk,bGloballyPalettized))
							{
								if (pszSubName)
								{
									delete[] pszSubName;
								}
								delete[] pszName;
								return pszDestBuf;
							}
						}
					}
				}
				
				List<Chunk *> listExtShapes;
				pGameModeChunk->lookup_child("SHBMPNAM",listExtShapes);

				for (LIF<Chunk *> itExtShapes(&listExtShapes); !itExtShapes.done(); itExtShapes.next())
				{
					External_Shape_BMPs_Store_Chunk * pShapeBmpsChunk = dynamic_cast<External_Shape_BMPs_Store_Chunk *>(itExtShapes());
					db_assert1(pShapeBmpsChunk);

					if (pszSubName)
						if (_stricmp(pszSubName,pShapeBmpsChunk->shapename) && *pszSubName)
							continue;

					for (LIF<BMP_Name> itBmpNames(&pShapeBmpsChunk->bmps); !itBmpNames.done(); itBmpNames.next())
					{
						BMP_Name bmpnTemp(itBmpNames());
						StripFileExtension(bmpnTemp.filename);

						if (pszFileName ? !_stricmp(pszName,StripPath(bmpnTemp.filename)) : enum_id == bmpnTemp.enum_id)
						{

							// select image descriptor
							ImageDescriptor const idsc
								(
									(IDscFlags)((bmpnTemp.flags & ChunkBMPFlag_FixedPalette ?
										IDSCF_FIXEDPALETTE
									:
										IDSCF_0)
									|(pShapeBmpsChunk->GetExtendedData()->flags & GBF_SPRITE ?
										IDSCF_SPRITE
									:
										IDSCF_SUBSHAPE)
									|IDSCF_INCLUDED),
									itBmpNames().filename,
									pShapeBmpsChunk->shapename,
									bmpnTemp.flags & ChunkBMPFlag_FixedPalette ? pShapeBmpsChunk->rifname : 0
								);
							ImageDescriptor const * pIdsc = &idsc;

							if (pMatchImageChunk) pIdsc = &pMatchImageChunk->GetLoadImage(idsc);
							else db_logf3(("WARNING! no rule to find matching images in game mode %s\n",pGameModeChunk->header->mode_identifier));

							// load this image
							if (GetPath(pszDestBuf,nBufSize,*pIdsc,pEnvDataChunk,bGloballyPalettized))
							{
								if (pszSubName)
								{
									delete[] pszSubName;
								}
								delete[] pszName;
								return pszDestBuf;
							}
						}
					}
				}
				
				if (pszSubName)
				{
					if (!bShapeInGm)
						db_logf3(("CL_LoadImageOnce(): WARNING! shape/sprite %s not found in this RIF file\n",pszSubName));
					else
						db_logf3(("CL_LoadImageOnce(): WARNING! shape/sprite %s does not appear to list %s\n",pszSubName,pszName));
				}

			}

			List<Chunk *> listMatchImageChunk;
			pEnvDataChunk->lookup_child("MATCHIMG",listMatchImageChunk);
			
			Matching_Images_Chunk * pMicFix = NULL;
			Matching_Images_Chunk * pMicNorm = NULL;
			
			for (LIF<Chunk *> itMatchImageChunk(&listMatchImageChunk); !itMatchImageChunk.done(); itMatchImageChunk.next())
			{
				Matching_Images_Chunk * pMatchImageChunk = dynamic_cast<Matching_Images_Chunk *>(itMatchImageChunk());
				db_assert1(pMatchImageChunk);
				if (pMatchImageChunk->flags & MICF_FIXEDPALETTE)
					pMicFix = pMatchImageChunk;
				else
					pMicNorm = pMatchImageChunk;
			}
				
			List<Chunk_With_Children *> listShapesSprites;
			
			List<Chunk *> listShapes;
			Env_Chunk->lookup_child("REBSHAPE",listShapes);
			for (LIF<Chunk *> itShapes(&listShapes); !itShapes.done(); itShapes.next())
			{	
				Shape_Chunk * pShapeChunk = dynamic_cast<Shape_Chunk *>(itShapes());
				db_assert1(pShapeChunk);
				Chunk * pSxfnChunk = pShapeChunk->lookup_single_child("SHPEXTFL");
				if (pSxfnChunk)
				{
					Shape_External_File_Chunk * pShpExtFnameChunk = dynamic_cast<Shape_External_File_Chunk *>(pSxfnChunk);
					db_assert1(pShpExtFnameChunk);
					listShapesSprites.add_entry(pShpExtFnameChunk);
				}
			}
			Chunk * pSprChunk = Env_Chunk->lookup_single_child("RSPRITES");
			if (pSprChunk)
			{
				Chunk_With_Children * pSpritesChunk = dynamic_cast<Chunk_With_Children *>(pSprChunk);
				db_assert1(pSpritesChunk);
				
				List<Chunk *> listSprHeadChunks;
				pSpritesChunk->lookup_child("SPRIHEAD",listSprHeadChunks);
				
				for (LIF<Chunk *> itSprites(&listSprHeadChunks); !itSprites.done(); itSprites.next())
				{	
					Chunk_With_Children * pSpriteHead = dynamic_cast<Chunk_With_Children *>(itSprites());
					db_assert1(pSpriteHead);
					listShapesSprites.add_entry(pSpriteHead);
				}
			}

			int bShapeFound = pszSubName ? false : true;
			
			for (LIF<Chunk_With_Children *> itShapesSprites(&listShapesSprites); !itShapesSprites.done(); itShapesSprites.next())
			{	
				char * pszSubRifName = RiffBasename(itShapesSprites());

				if (pszSubName)
				{
					if (_stricmp(pszSubRifName,pszSubName)) // must match shapes name exactly
					{
						delete[] pszSubRifName;
						continue;
					}
					bShapeFound = true;
				}
				
				Chunk * pBmpLstStChunk = itShapesSprites()->lookup_single_child("BMPLSTST");
				if (pBmpLstStChunk)
				{
					Bitmap_List_Store_Chunk * pBmpListStoreChunk = dynamic_cast<Bitmap_List_Store_Chunk *>(pBmpLstStChunk);
					db_assert1(pBmpListStoreChunk);

					for (LIF<BMP_Name> itBmpName(&pBmpListStoreChunk->bmps); !itBmpName.done(); itBmpName.next())
					{
						BMP_Name bmpnTemp(itBmpName());
						StripFileExtension(bmpnTemp.filename);

						if (pszFileName ? !_stricmp(pszName,StripPath(bmpnTemp.filename)) : enum_id == bmpnTemp.enum_id)
						{
						
							// select image descriptor
							char * pszRifName = RiffBasename(pEnvDataChunk);
							ImageDescriptor const idsc
								(
									(IDscFlags)((bmpnTemp.flags & ChunkBMPFlag_FixedPalette ?
										IDSCF_FIXEDPALETTE
									:
										IDSCF_0)
									|(pBmpListStoreChunk->GetExtendedData()->flags & GBF_SPRITE ?
										IDSCF_SPRITE
									:
										IDSCF_SUBSHAPE)
									|IDSCF_INCLUDED),
									itBmpName().filename,
									pszSubRifName,
									bmpnTemp.flags & ChunkBMPFlag_FixedPalette ? pszRifName : 0
								);
							ImageDescriptor const * pIdsc = &idsc;
							delete[] pszRifName;

							if (bmpnTemp.flags & ChunkBMPFlag_FixedPalette)
							{
								if (pMicFix) pIdsc = &pMicFix->GetLoadImage(idsc);
								else db_log3("CL_LoadImageOnce(): WARNING! no rule to find fixed palette matching images in environment data\n");
							}
							else
							{
								if (pMicNorm) pIdsc = &pMicNorm->GetLoadImage(idsc);
								else db_log3("CL_LoadImageOnce(): WARNING! no rule to find matching images in environment data (interface engine?)\n");
							}

							// load this image
							if (GetPath(pszDestBuf,nBufSize,*pIdsc,pEnvDataChunk,bGloballyPalettized))
							{
								delete[] pszSubRifName;
								if (pszSubName)
								{
									delete[] pszSubName;
								}
								delete[] pszName;
								return pszDestBuf;
							}
						}
					}
				}
				delete[] pszSubRifName;
			}

			if (pszSubName)
			{
				if (!bShapeFound)
					db_logf3(("CL_LoadImageOnce(): WARNING! shape/sprite %s not found in this RIF file\n",pszSubName));
				else
					db_logf3(("CL_LoadImageOnce(): WARNING! shape/sprite %s does not appear to list %s\n",pszSubName,pszName));
				delete[] pszSubName;
			}
			
			// not found in game textures, so look in default
			
			else // but only if there is no virtual shape directory
			{
				Chunk * pBmpNames = pEnvDataChunk->lookup_single_child("BMPNAMES");
				if (pBmpNames)
				{
					Global_BMP_Name_Chunk * pGbnc = dynamic_cast<Global_BMP_Name_Chunk *>(pBmpNames);
					db_assert1(pGbnc);

					for (LIF<BMP_Name> itBmpName(&pGbnc->bmps); !itBmpName.done(); itBmpName.next())
					{
						BMP_Name bmpnTemp(itBmpName());
						StripFileExtension(bmpnTemp.filename);

						if (pszFileName ? !_stricmp(pszName,StripPath(bmpnTemp.filename)) : enum_id == bmpnTemp.enum_id)
						{
							// select image descriptor
							ImageDescriptor const idsc (bmpnTemp.flags & ChunkBMPFlag_FixedPalette ? IDSCF_FIXEDPALETTE : IDSCF_0, itBmpName().filename);
							ImageDescriptor const * pIdsc = &idsc;

							if (bmpnTemp.flags & ChunkBMPFlag_FixedPalette)
							{
								if (pMicFix) pIdsc = &pMicFix->GetLoadImage(idsc);
								else db_log3("CL_LoadImageOnce(): WARNING! no rule to find fixed palette matching images in environment data\n");
							}
							else
							{
								if (pMicNorm) pIdsc = &pMicNorm->GetLoadImage(idsc);
								else db_log3("CL_LoadImageOnce(): WARNING! no rule to find matching images in environment data (interface engine?)\n");
							}

							// load this image
							if (GetPath(pszDestBuf,nBufSize,*pIdsc,pEnvDataChunk,bGloballyPalettized))
							{
								delete[] pszName;
								return pszDestBuf;
							}
						}
					}
				}
			}
			delete[] pszName;
			return NULL;
		}
		default: // invalid arguments
			db_log1("CL_LoadImageOnce(): ERROR: invalid parameter passed to CL_GetImageFileName()");
			return NULL;
	}
}

extern "C" {
	extern void CheckForWindowsMessages(void);
};

int CL_LoadImageOnce(char const * pszFileName, unsigned fFlagsEtc)
{
	// safe to handle windows messages at this point (I think - JCWH)
	CheckForWindowsMessages();
	
	// get the filename
	char szBuf[MAX_PATH];
	if (!CL_GetImageFileName(szBuf,sizeof szBuf/sizeof szBuf[0],pszFileName,fFlagsEtc))
	{
		db_log1("CL_LoadImageOnce(): ERROR: unable to determine path of file");
		return GEI_NOTLOADED;
	}
	
	db_logf4(("\tLoading '%s'",szBuf));
	
	// already loaded ?
	int iExistingImage = GetExistingImageNum(szBuf);
	if (GEI_NOTLOADED != iExistingImage)
	{
		db_logf4(("\tImage already loaded to image number %d",iExistingImage));
		return iExistingImage;
	}
	
	// what flags do we want?
	unsigned fAwLoad = AW_TLF_DEFAULT;
	if (fFlagsEtc & LIO_VIDMEM)
		fAwLoad |= AW_TLF_VIDMEM;
	if (fFlagsEtc & LIO_TRANSPARENT)
		fAwLoad |= AW_TLF_TRANSP;
	if (fFlagsEtc & LIO_CHROMAKEY)
		fAwLoad |= AW_TLF_CHROMAKEY;
	if (fFlagsEtc & LIO_LOADMIPMAPS)
		db_log1("CL_LoadImageOnce(): WARNING: mip maps not supported yet - will not be created/loaded");

	// what are we loading into ?
	switch (fFlagsEtc & _LIO_SURFTYPEMASK)
	{
		case LIO_CHIMAGE:
		{
			db_log1("CL_LoadImageOnce(): WARNING: having to call LoadImageCH()");
			IMAGEHEADER * pImageHdr = GetImageHeader();
			db_assert1(pImageHdr);
			memset(pImageHdr,0,sizeof(IMAGEHEADER));
			if (LoadImageCH(szBuf,pImageHdr))
				return NumImages-1;
			else
			{
				db_log1("CL_LoadImageOnce(): ERROR: LoadImageCH() failed");
				return GEI_NOTLOADED;
			}
		}
		case LIO_DDSURFACE:
		{
			#if CL_SUPPORT_FASTFILE
			size_t nFastFileLen;
			void const * pFastFileData = ffreadbuf(szBuf,&nFastFileLen);
			if (pFastFileData)
			{
				db_log4("\tfile is in a fast file");
				unsigned nWidth, nHeight;
				AW_BACKUPTEXTUREHANDLE hBackup = NULL;
				DDSurface * pSurface =
					AwCreateSurface
					(
						fFlagsEtc & LIO_RESTORABLE ? "pxfXYB" : "pxfXY",
						pFastFileData,
						nFastFileLen,
						fAwLoad,
						&nWidth,
						&nHeight,
						&hBackup
					);
				if (pSurface)
				{
					IMAGEHEADER * pImageHdr = GetImageHeader();
					db_assert1(pImageHdr);
					memset(pImageHdr,0,sizeof(IMAGEHEADER));
					pImageHdr->ImageWidth = nWidth;
					pImageHdr->ImageHeight = nHeight;
					db_assert1(sizeof pImageHdr->ImageName / sizeof pImageHdr->ImageName[0] > strlen(szBuf));
					strcpy(pImageHdr->ImageName,szBuf);
					pImageHdr->DDSurface = pSurface;
					pImageHdr->hBackup = hBackup;
					db_logf4(("\tloaded to image number %d",NumImages-1));
					#if CL_SUPPORT_ALTTAB
						if (fFlagsEtc & LIO_RESTORABLE)
						#ifdef NDEBUG
							ATIncludeSurface(pSurface,hBackup);
						#else
						{
							char szDbInf[512];
							sprintf(szDbInf,"Name \"%s\" Number %d",szBuf,NumImages-1);
							ATIncludeSurfaceDb(pSurface,hBackup,szDbInf);
						}
						#endif
					#endif // CL_SUPPORT_ALTTAB
					return NumImages-1;
				}
				else
				{
					db_log1("CL_LoadImageOnce(): ERROR copying data to surface");
					return GEI_NOTLOADED;
				}
			}
			else
			#endif // CL_SUPPORT_FASTFILE
			{
				#if !defined(CL_SUPPORTONLY_FASTFILE) || !CL_SUPPORTONLY_FASTFILE
				db_log4("\tloading the actual file");
				unsigned nWidth, nHeight;
				AW_BACKUPTEXTUREHANDLE hBackup = NULL;
				DDSurface * pSurface =
					AwCreateSurface
					(
						fFlagsEtc & LIO_RESTORABLE ? "sfXYB" : "sfXY",
						&szBuf[0],
						fAwLoad,
						&nWidth,
						&nHeight,
						&hBackup
					);
				if (pSurface)
				{
					IMAGEHEADER * pImageHdr = GetImageHeader();
					db_assert1(pImageHdr);
					memset(pImageHdr,0,sizeof(IMAGEHEADER));
					pImageHdr->ImageWidth = nWidth;
					pImageHdr->ImageHeight = nHeight;
					db_assert1(sizeof pImageHdr->ImageName / sizeof pImageHdr->ImageName[0] > strlen(szBuf));
					strcpy(pImageHdr->ImageName,szBuf);
					pImageHdr->DDSurface = pSurface;
					pImageHdr->hBackup = hBackup;
					db_logf4(("\tloaded to image number %d",NumImages-1));
					db_logf4(("\t\tsurface:%p",pSurface));
					#if CL_SUPPORT_ALTTAB
						if (fFlagsEtc & LIO_RESTORABLE)
						#ifdef NDEBUG
							ATIncludeSurface(pSurface,hBackup);
						#else
						{
							char szDbInf[512];
							sprintf(szDbInf,"Name \"%s\" Number %d",szBuf,NumImages-1);
							ATIncludeSurfaceDb(pSurface,hBackup,szDbInf);
						}
						#endif
					#endif // CL_SUPPORT_ALTTAB
					return NumImages-1;
				}
				else
				{
					db_log1("CL_LoadImageOnce(): ERROR copying data to surface");
					return GEI_NOTLOADED;
				}
				#elif !CL_SUPPORT_FASTFILE
					#error "CL_SUPPORTONLY_FASTFILE set but CL_SUPPORT_FASTFILE not set"
				#endif // !CL_SUPPORTONLY_FASTFILE
			}
			//db_msg1("THIS CODE SHOULD BE UNREACHABLE");
		}
		case LIO_D3DTEXTURE:
		{
			fAwLoad |= AW_TLF_COMPRESS; // required on some cards!!
			#if CL_SUPPORT_FASTFILE
			size_t nFastFileLen;
			void const * pFastFileData = ffreadbuf(szBuf,&nFastFileLen);
			if (pFastFileData)
			{
				db_log4("\tfile is in a fast file");
				unsigned nWidth, nHeight;
				AW_BACKUPTEXTUREHANDLE hBackup = NULL;
				D3DTexture * pTexture =
					AwCreateTexture
					(
						fFlagsEtc & LIO_RESTORABLE ? "pxfWHB" : "pxfWH",
						pFastFileData,
						nFastFileLen,
						fAwLoad,
						&nWidth,
						&nHeight,
						&hBackup
					);
				if (pTexture)
				{
					IMAGEHEADER * pImageHdr = GetImageHeader();
					db_assert1(pImageHdr);
					memset(pImageHdr,0,sizeof(IMAGEHEADER));
					pImageHdr->ImageWidth = nWidth;
					pImageHdr->ImageHeight = nHeight;
					db_assert1(sizeof pImageHdr->ImageName / sizeof pImageHdr->ImageName[0] > strlen(szBuf));
					strcpy(pImageHdr->ImageName,szBuf);
					pImageHdr->D3DTexture = pTexture;
					pImageHdr->hBackup = hBackup;
					db_logf4(("\tloaded to image number %d",NumImages-1));

					/* KJL 16:05:50 26/02/98 - attempt to get a texture handle */
					{
						int gotTextureHandle = GetTextureHandle(pImageHdr);
						db_assert1(gotTextureHandle==TRUE);
					}
					
					#if CL_SUPPORT_ALTTAB
						if (fFlagsEtc & LIO_RESTORABLE)
						#ifdef NDEBUG
							ATIncludeTexture(pTexture,hBackup);
						#else
						{
							char szDbInf[512];
							sprintf(szDbInf,"Name \"%s\" Number %d",szBuf,NumImages-1);
							ATIncludeTextureDb(pTexture,hBackup,szDbInf);
						}
						#endif
					#endif // CL_SUPPORT_ALTTAB
					return NumImages-1;
				}
				else
				{
					db_log1("CL_LoadImageOnce(): ERROR copying data to texture");
					return GEI_NOTLOADED;
				}
			}
			else
			#endif // CL_SUPPORT_FASTFILE
			{
				#if !defined(CL_SUPPORTONLY_FASTFILE) || !CL_SUPPORTONLY_FASTFILE
				db_log4("\tloading the actual file");
				unsigned nWidth, nHeight;
				AW_BACKUPTEXTUREHANDLE hBackup = NULL;
				D3DTexture * pTexture =
					AwCreateTexture
					(
						fFlagsEtc & LIO_RESTORABLE ? "sfWHB" : "sfWH",
						&szBuf[0],
						fAwLoad,
						&nWidth,
						&nHeight,
						&hBackup
					);
				if (pTexture)
				{
					IMAGEHEADER * pImageHdr = GetImageHeader();
					db_assert1(pImageHdr);
					memset(pImageHdr,0,sizeof(IMAGEHEADER));
					pImageHdr->ImageWidth = nWidth;
					pImageHdr->ImageHeight = nHeight;
					db_assert1(sizeof pImageHdr->ImageName / sizeof pImageHdr->ImageName[0] > strlen(szBuf));
					strcpy(pImageHdr->ImageName,szBuf);
					pImageHdr->D3DTexture = pTexture;
					pImageHdr->hBackup = hBackup;
					db_logf4(("\tloaded to image number %d",NumImages-1));

					/* KJL 16:05:50 26/02/98 - attempt to get a texture handle */
					{
						int gotTextureHandle = GetTextureHandle(pImageHdr);
						db_assert1(gotTextureHandle==TRUE);
					}

					#if CL_SUPPORT_ALTTAB
						if (fFlagsEtc & LIO_RESTORABLE)
						#ifdef NDEBUG
							ATIncludeTexture(pTexture,hBackup);
						#else
						{
							char szDbInf[512];
							sprintf(szDbInf,"Name \"%s\" Number %d",szBuf,NumImages-1);
							ATIncludeTextureDb(pTexture,hBackup,szDbInf);
						}
						#endif
					#endif // CL_SUPPORT_ALTTAB
					return NumImages-1;
				}
				else
				{
					db_log1("CL_LoadImageOnce(): ERROR copying data to texture");
					return GEI_NOTLOADED;
				}
				#elif !CL_SUPPORT_FASTFILE
					#error "CL_SUPPORTONLY_FASTFILE set but CL_SUPPORT_FASTFILE not set"
				#endif // !CL_SUPPORTONLY_FASTFILE
			}
			//db_msg1("THIS CODE SHOULD BE UNREACHABLE");
		}
		default:
			db_log1("CL_LoadImageOnce(): ERROR: Invalid destination surface specification");
			return GEI_NOTLOADED;
	}
}

