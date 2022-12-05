#include <errno.h>
#include <string.h>
#include "ffstdio.h"
#include "ffread.hpp"
#include "list_tem.hpp"
#include "dxlog.h"
#include "system.h"

class FFileDesc
{
private:
	size_t dname_len;
	char * dir_name;
	char * file_name;
public:
	FFileDesc(char const * infoline = 0, char const * path = 0);
	FFileDesc(FFileDesc const &);
	FFileDesc & operator = (FFileDesc const &);
	~FFileDesc();

	int CouldInclude(char const * fname) const;
	FFHeaderI * Load() const;

	//mark files that should kept when using ffclose_almost_all with a capital letter
	//should probably be done with an extra entry in the info text file, but I want to mantain
	//compatibility with the predator demo
	BOOL ShouldBeKept() const {return *dir_name>='A' && *dir_name<='Z';} 

	int operator == (FFileDesc const &) const;
	int operator != (FFileDesc const & ffd) const
		{ return ! operator == (ffd); }
};

FFileDesc::FFileDesc(char const * infoline, char const * path)
: dir_name(0)
, file_name(0)
{
	if (infoline)
	{
		char dbuf[256];
		char fbuf[256];

		char * dbufP = dbuf;
		char * fbufP = fbuf;
		if (path)
		{
			strcpy(fbuf,path);
			fbufP += strlen(path);
		}
		
		int c1;
		int c2;
		
		do
		{
			c2 = c1 = *infoline++;
			if (';'==c1||'\n'==c1||'\r'==c1)
				c1 = 0;
			*dbufP++ = (char)c1;
		}
		while (c1);

		if (c2)
		{
			do
			{
				c1 = *infoline++;
				if ('\n'==c1||'\r'==c1)
					c1 = 0;
				*fbufP++ = (char)c1;
			}
			while (c1);

			dname_len = strlen(dbuf);
			dir_name = new char[1+dname_len];
			strcpy(dir_name,dbuf);
			file_name = new char[1+strlen(fbuf)];
			strcpy(file_name,fbuf);
		}
	}
}

FFileDesc::FFileDesc(FFileDesc const & ffd)
: dname_len(ffd.dname_len)
, dir_name(0)
, file_name(0) 
{
	if (ffd.dir_name)
	{
		dir_name = new char [1+dname_len];
		strcpy(dir_name,ffd.dir_name);
	}
	if (ffd.file_name)
	{
		file_name = new char [1+strlen(ffd.file_name)];
		strcpy(file_name,ffd.file_name);
	}
}

FFileDesc & FFileDesc::operator = (FFileDesc const & ffd)
{
	if (&ffd != this)
	{
		if (dir_name) delete[] dir_name;
		if (file_name) delete[] file_name;
		dir_name = 0;
		file_name = 0;
		dname_len = ffd.dname_len;
		if (ffd.dir_name)
		{
			dir_name = new char [1+dname_len];
			strcpy(dir_name,ffd.dir_name);
		}
		if (ffd.file_name)
		{
			file_name = new char [1+strlen(ffd.file_name)];
			strcpy(file_name,ffd.file_name);
		}
	}
	return *this;
}

FFileDesc::~FFileDesc()
{
	if (dir_name) delete[] dir_name;
	if (file_name) delete[] file_name;
}

int FFileDesc::CouldInclude(char const * fname) const
{
	if (dir_name)
	{
		return ! _strnicmp(dir_name,fname,dname_len);
	}
	return 0;
}

FFHeaderI * FFileDesc::Load() const
{
	LOGDXFMT(("Loaded FastFile: %s\n(for directory: %s)\n",file_name,dir_name));
	return new FFHeaderI(file_name,ShouldBeKept());
}

int FFileDesc::operator == (FFileDesc const & ffd) const
{
	return ! _stricmp(file_name ? file_name : "", ffd.file_name ? ffd.file_name : "");
}


static List<FFileDesc> fdesclist;

static List<FFileDesc> floadeddesclist;

static List<FFHeaderI *> fflist;

static List<FFILE *> openlist;


int ffInit(char const * infofilename, char const * ffpath)
{
	FILE * fp = OpenGameFile(infofilename, FILEMODE_READONLY, FILETYPE_PERM);
	if (!fp) return 0;
	
	while (fdesclist.size())
		fdesclist.delete_first_entry();

	// read data
	char buf[512];
	buf[0]=0;

	fgets(buf,sizeof buf,fp);
	do
	{
		//Only look at lines that start with a letter
		//mainly so as to avoid using blank lines
		
		if((buf[0]>='a' && buf[0]<='z') ||
		   (buf[0]>='A' && buf[0]<='Z'))
		{
			fdesclist.add_entry(FFileDesc(buf,ffpath));
		}
	}
	while (fgets(buf,sizeof buf,fp));

	fclose(fp);

	return 1;
}

void ffKill(void)
{
	#define EMPTY_LIST(list) while ((list).size()) (list).delete_first_entry();
	#define EMPTY_POINTER_LIST(list) while ((list).size()) {delete (list).first_entry();(list).delete_first_entry();}

	EMPTY_LIST(fdesclist)
	EMPTY_LIST(floadeddesclist)
	EMPTY_POINTER_LIST(fflist)
	EMPTY_POINTER_LIST(openlist)
}

int ffcloseall(void)
{
	int cnt = openlist.size();
	LOGDXFMT(("Unloading all fastfiles: %d subfile(s) still open",cnt));
	for (LIF<FFILE *> i_open(&openlist); !i_open.done();)
	{
		FFILE * fp = i_open();
		if (fp->flag & FFF_ALOC) free((void *)fp->data_start);
		fp->data_start = 0;
		fp->data_ptr = 0;
		delete fp;
		i_open.delete_current();
	}
	while (fflist.size())
	{
		delete fflist.first_entry();
		fflist.delete_first_entry();
	}
	while (floadeddesclist.size())
	{
		floadeddesclist.delete_first_entry();
	}
	return cnt;
}

int ffclose_almost_all(void)
{
	//unload all fastfiles except for the common ones
	int cnt = openlist.size();
	LOGDXFMT(("Unloading almost all fastfiles: %d subfile(s) still open",cnt));
	for (LIF<FFILE *> i_open(&openlist); !i_open.done();)
	{
		FFILE * fp = i_open();
		if (fp->flag & FFF_ALOC) free((void *)fp->data_start);
		fp->data_start = 0;
		fp->data_ptr = 0;
		delete fp;
		i_open.delete_current();
	}
	
	for(LIF<FFHeaderI*> fflif(&fflist);!fflif.done();)
	{
		if(fflif()->ShouldBeKept())
		{
			fflif.next();
		}
		else
		{
			delete fflif();
			fflif.delete_current();
		}
	}

	for(LIF<FFileDesc> desc_lif(&floadeddesclist);!desc_lif.done();)
	{
		if(desc_lif().ShouldBeKept())
		{
			desc_lif.next();
		}
		else
		{
			desc_lif.delete_current();
		}
	}
	
	LOGDXFMT(("Keeping %d fastfile(s)",fflist.size()));
	return cnt;
}


int ffclose(FFILE * fp)
{
	if (fp->flag & FFF_ALOC) free((void *)fp->data_start);
	fp->data_start = 0;
	fp->data_ptr = 0;
	openlist.delete_entry(fp);
	delete fp;
	return 0;
}

int ffgetc(FFILE * fp)
{
	if (fp->remaining)
	{
		-- fp->remaining;
		++ fp->pos;
		return *(signed char const *)fp->data_ptr++;
	}
	else
	{
		fp->flag |= FFF_EOF;
		return EOF;
	}
}

char * ffgets(char * s, int n, FFILE * fp)
{
	char * sP = s;

	for (int n_read = 0; n_read<n-1; ++n_read)
	{
		if (!fp->remaining)
		{
			fp->flag |= FFF_EOF;
			return 0;
		}
		int ch = *(char const *)fp->data_ptr++;
		-- fp->remaining;
		++ fp->pos;
		*sP++ = (char)ch;
		if ('\n'==ch) break;
	}
	*sP=0;
	return s;
}

void const * ffreadbuf(char const * filename, size_t * p_len)
{
	void const * data;
	
	for (LIF<FFHeaderI *> i_ff(&fflist); !i_ff.done(); i_ff.next())
	{
		data = i_ff()->FindFile(filename,p_len);
		if (data) return data;
	}
		
	// try loading another big catfile
	
	for (LIF<FFileDesc> i_fdesc(&fdesclist); !i_fdesc.done(); i_fdesc.next())
	{
		if (i_fdesc().CouldInclude(filename) && !floadeddesclist.contains(i_fdesc()))
		{
			floadeddesclist.add_entry(i_fdesc());
			FFHeaderI * newffh = i_fdesc().Load();
			fflist.add_entry(newffh);

			data = newffh->FindFile(filename,p_len);
			if (data)
			{
				return data;
			}
		}
	}
	
	return NULL;
}

FFILE * ffopen(char const * filename, char const * mode)
{
	if (mode[0]!='r' || mode[1]!='b') return 0;
	
	size_t length;
	void const * data = ffreadbuf(filename,&length);
	
	if (data)
	{
		FFILE * fp = new FFILE;
		fp->data_start = data;
		fp->data_ptr = (unsigned char const *)data;
		fp->length = length;
		fp->pos = 0;
		fp->remaining = length;
		fp->flag = 0;
		openlist.add_entry(fp);
		return fp;
	}

	errno = ENOENT;
	// ok, just use fopen (if debug)
	#if debug && 0 // dont do this, caller should handle this situation
	LOGDXFMT(("%s not in any fastfile",filename));

	/* mode is always "rb" */
	FILE *sfp = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_PERM);
	
	if (!sfp) return 0;

	fseek(sfp,0,SEEK_END);
	length = ftell(sfp);
	fseek(sfp,0,SEEK_SET);
	data = malloc(length);
	if (!data)
	{
		fclose(sfp);
		return 0;
	}
	length = fread((void *)data,1,length,sfp);
	
	FFILE * fp = new FFILE;
	fp->data_start = data;
	fp->data_ptr = (unsigned char const *)data;
	fp->length = length;
	fp->pos = 0;
	fp->remaining = length;
	fp->flag = FFF_ALOC;
	if (ferror(sfp))
	{
		fp->flag |= FFF_ERR;
	}
	fclose(sfp);
	openlist.add_entry(fp);
	return fp;
	#else
	return 0;
	#endif
}

size_t ffreadb(void * ptr, size_t n, FFILE * fp)
{
	if (fp->remaining < n)
	{
		n = fp->remaining;
		fp->flag |= FFF_EOF;
	}

	memcpy(ptr,fp->data_ptr,n);

	fp->data_ptr += n;
	fp->pos += n;
	fp->remaining -= n;

	return n;
}

size_t fflookb(void const * * ptr, size_t n, FFILE * fp)
{
	if (fp->remaining < n)
	{
		n = fp->remaining;
		fp->flag |= FFF_EOF;
	}

	*ptr = fp->data_ptr;

	fp->data_ptr += n;
	fp->pos += n;
	fp->remaining -= n;

	return n;
}

int ffseek(FFILE * fp, long offset, int whence)
{
	switch (whence)
	{
		case SEEK_SET:
			if ((unsigned long)offset > fp->length || offset < 0)
			{
				fp->flag |= FFF_ERR;
				errno = EDOM;
				return 1;
			}
			fp->pos = offset;
			break;
		case SEEK_CUR:
			if (offset+fp->pos > fp->length || offset+(long)fp->pos < 0)
			{
				fp->flag |= FFF_ERR;
				errno = EDOM;
				return 1;
			}
			fp->pos += offset;
			break;
		case SEEK_END:
			if (offset < -(signed int)fp->length || offset>0)
			{
				fp->flag |= FFF_ERR;
				errno = EDOM;
				return 1;
			}
			fp->pos = fp->length + offset;
			break;
		default:
			errno = EINVAL;
			fp->flag |= FFF_ERR;
			return 1;
	}
	fp->flag &= ~FFF_EOF;
	fp->remaining = fp->length - fp->pos;
	fp->data_ptr = (unsigned char const *)fp->data_start + fp->pos;
	return 0;
}

int ffsetpos(FFILE * fp, ffpos_t const * pos)
{
	if (*pos > fp->length || *pos < 0)
	{
		fp->flag |= FFF_ERR;
		errno = EDOM;
		return 1;
	}

	fp->pos = *pos;
	fp->remaining = fp->length - *pos;
	fp->data_ptr = (unsigned char const *)fp->data_start + *pos;

	return 0;
}
