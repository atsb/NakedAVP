#include <string.h>
#include <stdlib.h>
#include "debuglog.hpp"

LogFile::LogFile(char const * const _fname) : fname(0) , ever_written(0)
{
	FILE * fp = fopen(_fname,"w");
	if (fp)
	{
		fclose(fp);
		fname = new char[strlen(_fname)+1];
		strcpy(fname,_fname);
		return;
	}
	char const * path = getenv("TEMP");
	if (!path) path = getenv("TMP");
	if (!path) return;
	fname = new char[strlen(path)+1+strlen(_fname)+1];
	strcpy(fname,path);
	strncat(fname,"\\",1);
	strcat(fname,_fname);
	fp = fopen(fname,"w");
	if (fp)
		fclose(fp);
	else
	{
		delete[] fname;
		fname = 0;
	}
}

LogFile::~LogFile()
{
	if (unwritten.size())
	{
		FILE * fp = fopen(fname,"a");
		for (int attempt=0; !fp && attempt<10; ++attempt)
		{
			/* Sleep(100); */
			fp = fopen(fname,"a");
		}
		if (fp)
		{
			FlushOut(fp);
			fclose(fp);
		}
	}
	if (fname) delete[] fname;
}

LogFile & LogFile::operator = (LogFile const & l)
{
	if (&l != this)
	{
		if (fname) delete[] fname;
		if (l.fname)
		{
			fname = new char[strlen(l.fname)+1];
			strcpy(fname,l.fname);
		}
		else
			fname = 0;
			
		unwritten = l.unwritten;
		ever_written = l.ever_written;
	}
	return *this;
}

LogFile::LogFile(LogFile const & l)
: unwritten(l.unwritten)
, ever_written(l.ever_written)
{
	if (l.fname)
	{
		fname = new char[strlen(l.fname)+1];
		strcpy(fname,l.fname);
	}
	else
		fname = 0;
}

void LogFile::FlushOut(FILE * fp)
{
	while (unwritten.size())
	{
		char * str = unwritten.first_entry();
		unwritten.delete_first_entry();
		fputs(str,fp);
		delete[] str;
	}
}

int vlfprintf(LOGFILE * lfp, char const * format, va_list args )
{
	return lfp->vlprintf(format,args);
}

int lfprintf(LOGFILE * lfp, char const * format, ... )
{
	va_list ap;
	va_start(ap, format);
	int rv = lfp->vlprintf(format,ap);
	va_end(ap);
	return rv;
}

int lfputs(LOGFILE * lfp, char const * str)
{
	return lfp->lputs(str);
}

LOGFILE * lfopen(char const * fname)
{
	return new LogFile(fname);
}

void lfclose(LOGFILE * lfp)
{
	delete lfp;
}
