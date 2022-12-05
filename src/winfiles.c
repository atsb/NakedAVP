#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

#include "files.h"

static char *local_dir;
static char *global_dir;

/*
Sets the local and global directories used by the other functions.
Local = ~/.dir, where config and user-installed files are kept.
Global = installdir, where installed data is stored.
*/
int SetGameDirectories(const char *local, const char *global)
{
	local_dir = _strdup(local);
	global_dir = _strdup(global);

	if( GetFileAttributes( local_dir ) == INVALID_FILE_ATTRIBUTES ) {
		_mkdir( local_dir );
	}

	return 0;
}


#define DIR_SEPARATOR	"\\"

static char *FixFilename(const char *filename, const char *prefix, int force)
{
	char *f, *ptr;
	size_t flen;
	size_t plen;
	
	plen = strlen(prefix) + 1;
	flen = strlen(filename) + plen + 1;
	
	f = (char *)malloc(flen);
	strcpy(f, prefix);
	strcat(f, DIR_SEPARATOR);
	strcat(f, filename);
	
	/* only the filename part needs to be modified */
	ptr = &f[plen+1];
	
	while (*ptr) {
		if ((*ptr == '/') || (*ptr == '\\') || (*ptr == ':')) {
			*ptr = DIR_SEPARATOR[0];
		} else if (*ptr == '\r' || *ptr == '\n') {
			*ptr = 0;
			break;
		} else {
			if (force) {
				*ptr = tolower(*ptr);
			}
		}
		ptr++;
	}

	return f;
}

/*
Open a file of type type, with mode mode.

Mode can be:
#define	FILEMODE_READONLY	0x01
#define	FILEMODE_WRITEONLY	0x02
#define	FILEMODE_READWRITE	0x04
#define FILEMODE_APPEND		0x08
Type is (mode = ReadOnly):
#define	FILETYPE_PERM		0x08 // try the global dir only 
#define	FILETYPE_OPTIONAL	0x10 // try the global dir first, then try the local dir
#define	FILETYPE_CONFIG		0x20 // try the local dir only

Type is (mode = WriteOnly or ReadWrite):
FILETYPE_PERM: error
FILETYPE_OPTIONAL: error
FILETYPE_CONFIG: try the local dir only
*/
FILE *OpenGameFile(const char *filename, int mode, int type)
{
	char *rfilename;
	char *openmode;
	FILE *fp;
	
	if ((type != FILETYPE_CONFIG) && (mode != FILEMODE_READONLY)) 
		return NULL;
	
	switch(mode) {
		case FILEMODE_READONLY:
			openmode = "rb";
			break;
		case FILEMODE_WRITEONLY:
			openmode = "wb";
			break;
		case FILEMODE_READWRITE:
			openmode = "w+";
			break;
		case FILEMODE_APPEND:
			openmode = "ab";
			break;
		default:
			return NULL;
	}

	if (type != FILETYPE_CONFIG) {
		rfilename = FixFilename(filename, global_dir, 0);
		
		fp = fopen(rfilename, openmode);
		
		free(rfilename);
		
		if (fp != NULL) {
			return fp;
		}
		
		rfilename = FixFilename(filename, global_dir, 1);
		
		fp = fopen(rfilename, openmode);
		
		free(rfilename);
		
		if (fp != NULL) {
			return fp;
		}
	}
	
	if (type != FILETYPE_PERM) {
		rfilename = FixFilename(filename, local_dir, 0);
		
		fp = fopen(rfilename, openmode);
		
		free(rfilename);
		
		if (fp != NULL) {
			return fp;
		}
		
		rfilename = FixFilename(filename, local_dir, 1);
		
		fp = fopen(rfilename, openmode);
		
		free(rfilename);
		
		return fp;
	}
	
	return NULL;
}

int CloseGameFile(FILE *pfd)
{
	return fclose(pfd);
}

/*
Get the filesystem attributes of a file

#define	FILEATTR_DIRECTORY	0x0100
#define FILEATTR_READABLE	0x0200
#define FILEATTR_WRITABLE	0x0400

Error or can't access it: return value of 0 (What is the game going to do about it anyway?)
*/
static int GetFA(const char *filename)
{
	struct stat buf;
	int attr;

	attr = 0;
	if (stat(filename, &buf) == 0) {
		if (buf.st_mode & _S_IFDIR) {
			attr |= FILEATTR_DIRECTORY;
		}
			
		if (buf.st_mode & _S_IREAD) {
			attr |= FILEATTR_READABLE;
		}
			
		if (buf.st_mode & _S_IWRITE) {
			attr |= FILEATTR_WRITABLE;
		}
	}
	
	return attr;
}

static time_t GetTS(const char *filename)
{
	struct stat buf;

	if (stat(filename, &buf) == 0) {
		return buf.st_mtime;
	}
	
	return 0;
}

int GetGameFileAttributes(const char *filename, int type)
{
	struct stat buf;
	char *rfilename;
	int attr;
	
	attr = 0;	
	if (type != FILETYPE_CONFIG) {
		rfilename = FixFilename(filename, global_dir, 0);
		
		if (stat(rfilename, &buf) == 0) {
			if (buf.st_mode & _S_IFDIR) {
				attr |= FILEATTR_DIRECTORY;
			}
				
			if (buf.st_mode & _S_IREAD) {
				attr |= FILEATTR_READABLE;
			}
				
			if (buf.st_mode & _S_IWRITE) {
				attr |= FILEATTR_WRITABLE;
			}
			
			free(rfilename);
		
			return attr;
		}
		
		free(rfilename);
		
		rfilename = FixFilename(filename, global_dir, 1);
		
		if (stat(rfilename, &buf) == 0) {
			if (buf.st_mode & _S_IFDIR) {
				attr |= FILEATTR_DIRECTORY;
			}
				
			if (buf.st_mode & _S_IREAD) {
				attr |= FILEATTR_READABLE;
			}
				
			if (buf.st_mode & _S_IWRITE) {
				attr |= FILEATTR_WRITABLE;
			}
			
			free(rfilename);
		
			return attr;
		}
		
		free(rfilename);
	}
	
	if (type != FILETYPE_PERM) {
		rfilename = FixFilename(filename, local_dir, 0);
		
		if (stat(rfilename, &buf) == 0) {
			if (buf.st_mode & _S_IFDIR) {
				attr |= FILEATTR_DIRECTORY;
			}
				
			if (buf.st_mode & _S_IREAD) {
				attr |= FILEATTR_READABLE;
			}
				
			if (buf.st_mode & _S_IWRITE) {
				attr |= FILEATTR_WRITABLE;
			}

			free(rfilename);
		
			return attr;
		}
		
		free(rfilename);
		
		rfilename = FixFilename(filename, local_dir, 1);
		
		if (stat(rfilename, &buf) == 0) {
			if (buf.st_mode & _S_IFDIR) {
				attr |= FILEATTR_DIRECTORY;
			}
				
			if (buf.st_mode & _S_IREAD) {
				attr |= FILEATTR_READABLE;
			}
				
			if (buf.st_mode & _S_IWRITE) {
				attr |= FILEATTR_WRITABLE;
			}
			
			free(rfilename);
		
			return attr;
		}
		
		free(rfilename);
		
	}
	
	return 0;
}

/*
Delete a file: local dir only
*/
int DeleteGameFile(const char *filename)
{
	char *rfilename;
	int ret;
	
	rfilename = FixFilename(filename, local_dir, 0);
	ret = _unlink(rfilename);
	free(rfilename);
	
	if (ret == -1) {
		rfilename = FixFilename(filename, local_dir, 1);
		ret = _unlink(rfilename);
		free(rfilename);
	}
	
	return ret;
}

/*
Create a directory: local dir only

TODO: maybe also mkdir parent directories, if they do not exist?
*/
int CreateGameDirectory(const char *dirname)
{
	char *rfilename;
	int ret;

	rfilename = FixFilename(dirname, local_dir, 0);
	ret = _mkdir(rfilename);
	free(rfilename);
	
	if (ret == -1) {
		rfilename = FixFilename(dirname, local_dir, 1);
		ret = _mkdir(rfilename);
		free(rfilename);
	}
	
	return ret;
}


/* This struct is private. */
typedef struct GameDirectory
{
	intptr_t localdir;		/* directory opened with _findfirst */
	intptr_t globaldir;
	
	struct _finddata_t tmplocalfinddata;
	struct _finddata_t localfinddata;
	struct _finddata_t tmpglobalfinddata;
	struct _finddata_t globalfinddata;

	char *localdirname;
	char *globaldirname;

	GameDirectoryFile tmp;	/* Temp space */
} GameDirectory;

/*
"Open" a directory dirname, with type type
Returns a pointer to a directory datatype

Pattern is the pattern to match
*/
void *OpenGameDirectory(const char *dirname, const char *pattern, int type)
{
	char* localdirname;
	char* globaldirname;
	char* filespec;

	intptr_t localdir;
	intptr_t globaldir;
	GameDirectory *gd;
	
	gd = (GameDirectory *)malloc(sizeof(GameDirectory));
	memset( gd, 0, sizeof(GameDirectory) );

	globaldir = -1;
	globaldirname = NULL;
	if (type != FILETYPE_CONFIG) {
		globaldirname = FixFilename(dirname, global_dir, 0);

		filespec = (char*) malloc(strlen(globaldirname)+1+strlen(pattern)+1);
		strcpy( filespec, globaldirname );
		strcat( filespec, DIR_SEPARATOR );
		strcat( filespec, pattern );

		globaldir = _findfirst(filespec, &gd->tmpglobalfinddata);
		free(filespec);

		if (globaldir == -1L) {
			free(globaldirname);
			
			globaldirname = FixFilename(dirname, global_dir, 1);

			filespec = (char*) malloc(strlen(globaldirname)+1+strlen(pattern)+1);
			strcpy( filespec, globaldirname );
			strcat( filespec, DIR_SEPARATOR );
			strcat( filespec, pattern );

			globaldir = _findfirst(filespec, &gd->tmpglobalfinddata);
			free(filespec);

			if (globaldir == -1L) {
				free(globaldirname);
				globaldirname = NULL;
			}
		}		
	}
	
	localdir = -1;
	localdirname = NULL;
	if (type != FILETYPE_PERM) {
		localdirname = FixFilename(dirname, local_dir, 0);

		filespec = (char*) malloc(strlen(localdirname)+1+strlen(pattern)+1);
		strcpy( filespec, localdirname );
		strcat( filespec, DIR_SEPARATOR );
		strcat( filespec, pattern );

		localdir = _findfirst(filespec, &gd->tmplocalfinddata);
		free(filespec);

		if (localdir == -1L) {
			free(localdirname);
			
			localdirname = FixFilename(dirname, local_dir, 1);

			filespec = (char*) malloc(strlen(localdirname)+1+strlen(pattern)+1);
			strcpy( filespec, localdirname );
			strcat( filespec, DIR_SEPARATOR );
			strcat( filespec, pattern );

			localdir = _findfirst(filespec, &gd->tmplocalfinddata);
			free( filespec );

			if (localdir == -1L) {
				free(localdirname);
				localdirname = NULL;
			}
		}
	}
	
	if (localdir == -1L && globaldir == -1L) {
		free( gd );
		return NULL;
	}

	gd->localdir = localdir;
	gd->globaldir = globaldir;

	gd->localdirname = localdirname;
	gd->globaldirname = globaldirname;

	return gd;
}

/*
This struct is public.

typedef struct GameDirectoryFile
{
	char *filename;
	int attr;
} GameDirectoryFile;
*/

/*
Returns the next match of pattern with the contents of dir

f is the current file
*/
GameDirectoryFile *ScanGameDirectory(void *dir)
{
	char *ptr;
	GameDirectory *directory;
	
	directory = (GameDirectory *)dir;
	
	if (directory->globaldir != -1L) {
		directory->globalfinddata = directory->tmpglobalfinddata;

		ptr = FixFilename(directory->globalfinddata.name, directory->globaldirname, 0);
		directory->tmp.attr = GetFA(ptr);
		directory->tmp.timestamp = GetTS(ptr);
		free(ptr);

		directory->tmp.filename = &directory->globalfinddata.name[0];

		if( _findnext( directory->globaldir, &directory->tmpglobalfinddata ) == -1 ) {
				_findclose(directory->globaldir);
				free(directory->globaldirname);
		
				directory->globaldir = -1L;
				directory->globaldirname = NULL;
		}

		return &directory->tmp;
	}
	
	if (directory->localdir != -1L) {
		directory->localfinddata = directory->tmplocalfinddata;

		ptr = FixFilename(directory->localfinddata.name, directory->localdirname, 0);
		directory->tmp.attr = GetFA(ptr);
		directory->tmp.timestamp = GetTS(ptr);
		free(ptr);

		directory->tmp.filename = &directory->localfinddata.name[0];

		if( _findnext( directory->localdir, &directory->tmplocalfinddata ) == -1 ) {
				_findclose(directory->localdir);
				free(directory->localdirname);
		
				directory->localdir = -1L;
				directory->localdirname = NULL;
		}

		return &directory->tmp;
	}
	
	return NULL;
}

/*
Close directory
*/
int CloseGameDirectory(void *dir)
{
	GameDirectory *directory = (GameDirectory *)dir;
	
	if (directory != NULL) {

		if (directory->localdirname != NULL) {
			free(directory->localdirname);
		}
		if (directory->globaldirname != NULL) {
			free(directory->globaldirname);
		}
		if (directory->localdir != -1L) {
			_findclose(directory->localdir);
		}
		if (directory->globaldir != -1L) {
			_findclose(directory->globaldir);
		}
			
		return 0;
	}
	return -1;
}

/*
  Game-specific helper function.
 */
static int try_game_directory(const char *dir, const char *file)
{
	char tmppath[MAX_PATH];
	DWORD retr;

	strncpy(tmppath, dir, MAX_PATH-32);
	tmppath[MAX_PATH-32] = 0;
	strcat(tmppath, file);
	
	retr = GetFileAttributes(tmppath);

	if( retr == INVALID_FILE_ATTRIBUTES ) {
		return 0;
	}

	/*
	  TODO - expand this check to check for read access
     */
	return 1;
}

/*
  Game-specific helper function.
 */
static int check_game_directory(const char *dir)
{
	if (!dir || !*dir) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\avp_huds")) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\avp_huds\\alien.rif")) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\avp_rifs")) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\avp_rifs\\temple.rif")) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\fastfile")) {
		return 0;
	}
	
	if (!try_game_directory(dir, "\\fastfile\\ffinfo.txt")) {
		return 0;
	}
	
	return 1;
}


static char* GetLocalDirectory(void)
{
	char folderPath[2 * MAX_PATH + 10];
	char* localdir;

	const char* homedrive;
	const char* homepath;
	char* homedir;

	homedir = NULL;

	/*
	  TODO - should check that the directory is actually usable.
     */

	/*
	   1. Check registry (not currently implemented)
	 */

	/*
	   2. CSIDL_LOCAL_APPDATA with SHGetFolderPath
	 */
	if( homedir == NULL ) {
		if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA,
			NULL, SHGFP_TYPE_CURRENT, &folderPath[0] ) ) ) {

			homedir = _strdup( folderPath );
		}
	}

	/*
	   3. CSIDL_APPDATA with SHGetFolderPath
     */
	if( homedir == NULL ) {
		if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA,
			NULL, SHGFP_TYPE_CURRENT, &folderPath[0] ) ) ) {

			homedir = _strdup( folderPath );
		}
	}

	/*
	   4. HOMEDRIVE+HOMEPATH
     */

	if( homedir == NULL ) {
		homedrive = getenv("HOMEDRIVE");
		homepath  = getenv("HOMEPATH");

		if( homedrive == NULL ) {
			homedrive = "";
		}

		if( homepath != NULL ) {

			homedir = (char*)malloc(strlen(homedrive)+strlen(homepath)+1);
			
			strcpy(homedir, homedrive);
			strcat(homedir, homepath);
		}
	}

	/*
	   5. HOME
     
     */
	if( homedir == NULL ) {
		homepath = getenv("HOME");

		if( homepath != NULL ) {
			homedir = _strdup(homepath);
		}
	}

	/* 
	  6. CWD
     */
	if( homedir == NULL ) {
		homedir = _strdup(".");
	}

	localdir = (char*)malloc(strlen(homedir) + 10);
	strcpy(localdir, homedir);
	strcat(localdir, "\\AvPLinux"); // temp name, maybe

	free(homedir);

	return localdir;
}

static const char* GetGlobalDirectory(const char* argv0)
{
	char* gamedir;
	char* tmp;

	/*
	1. $AVP_DATA overrides all
	2. Registry Setting
	3. executable path from argv[0]
	4. current directory
	*/
	
	/* 1. $AVP_DATA */
	gamedir = getenv("AVP_DATA");
	
	/* $AVP_DATA overrides all, so no check */
	
	/* 2. Registry Setting */
	/* TODO */

	if (gamedir == NULL) {
		/* 3. executable path from argv[0] */
		tmp = _strdup(argv0);
		
		if (tmp == NULL) {
			/* ... */
			fprintf(stderr, "GetGlobalDirectory failure\n");
			exit(EXIT_FAILURE);
		}

		gamedir = strrchr(tmp, DIR_SEPARATOR[0]);

		if (gamedir != NULL) {
			*gamedir = 0;
			gamedir = tmp;
		
			if (!check_game_directory(gamedir)) {
				gamedir = NULL;
			}
		}
	}

	/* 4. current directory */
	return _strdup(".");
}

/*
  Game-specific initialization
 */
extern char const *SecondTex_Directory;
extern char const *SecondSoundDir;

void InitGameDirectories(char *argv0)
{
	const char* localdir;
	const char* globaldir;

	SecondTex_Directory = "graphics\\";
	SecondSoundDir = "sound\\";

	localdir  = GetLocalDirectory();
	globaldir = GetGlobalDirectory(argv0);

	assert(localdir != NULL);
	assert(globaldir != NULL);
	
	/* last chance sanity check */
	if (!check_game_directory(globaldir)) {
		fprintf(stderr, "Unable to find the AvP gamedata.\n");
		fprintf(stderr, "The directory last examined was: %s\n", globaldir);
		fprintf(stderr, "Has the game been installed and\n");
		fprintf(stderr, "are all game files lowercase?\n");
		exit(EXIT_FAILURE);
	}

	SetGameDirectories(localdir, globaldir);
}
