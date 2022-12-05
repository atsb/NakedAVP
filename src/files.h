#ifndef FILES_H
#define FILES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>

#define	FILEMODE_READONLY	0x01
#define	FILEMODE_WRITEONLY	0x02
#define	FILEMODE_READWRITE	0x04
#define FILEMODE_APPEND		0x08

#define	FILETYPE_PERM		0x10
#define	FILETYPE_OPTIONAL	0x20
#define	FILETYPE_CONFIG		0x40

#define	FILEATTR_DIRECTORY	0x0100
#define	FILEATTR_READABLE	0x0200
#define	FILEATTR_WRITABLE	0x0400

typedef struct GameDirectoryFile
{
	char *filename;
	int attr;
	time_t timestamp;
} GameDirectoryFile;

int SetGameDirectories(const char *local, const char *global);
FILE *OpenGameFile(const char *filename, int mode, int type);
int CloseGameFile(FILE *pfd);
int GetGameFileAttributes(const char *filename, int type);
int DeleteGameFile(const char *filename);
int CreateGameDirectory(const char *dirname);
void *OpenGameDirectory(const char *dirname, const char *pattern, int type);
GameDirectoryFile *ScanGameDirectory(void *dir);
int CloseGameDirectory(void *dir);
void InitGameDirectories(char *argv0);

#ifdef __cplusplus
};
#endif

#endif
