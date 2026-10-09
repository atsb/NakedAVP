#ifndef _included_ffstdio_h_
#define _included_ffstdio_h_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FFF_EOF  0x0001
#define FFF_ERR  0x0002
#define FFF_ALOC 0x0004

struct _FFile
{
	void const * data_start;
	unsigned char const * data_ptr;
	size_t length;
	size_t pos;
	size_t remaining;
	int flag;
};
typedef struct _FFile FFILE;
typedef size_t ffpos_t;

extern int ffInit(char const * infofilename, char const * ffpath);
extern void ffKill(void); /* only need to call this to prevent misreported memory leaks */

/* only mode supported is "rb" */
extern int     ffclearerr(FFILE * fp);
extern int     ffclose(FFILE * fp);
extern int     ffcloseall(void);
extern int     ffclose_almost_all(void);
extern int     ffeof(FFILE * fp);
extern int     fferror(FFILE * fp);
extern int     ffgetc(FFILE * fp);
extern int     ffgetpos(FFILE * fp, ffpos_t * pos);
extern char *  ffgets(char * s, int n, FFILE * fp);
extern size_t  fflook(void const * * ptr, size_t size, size_t n, FFILE * fp);
extern size_t  fflookb(void const * * ptr, size_t n, FFILE * fp);
extern FFILE * ffopen(char const * filename, char const * mode);
extern size_t  ffread(void * ptr, size_t size, size_t n, FFILE * fp);
extern size_t  ffreadb(void * ptr, size_t n, FFILE * fp);
extern int     ffseek(FFILE * fp, long offset, int whence);
extern int     ffsetpos(FFILE * fp, ffpos_t const * pos);
extern long    fftell(FFILE * fp);

/* this function is called by ffopen, but you can call it direct
   to get a pointer to memory where the contents of a file exist */
/* nb. the buffer remains valid until a call to ffcloseall */
extern void const * ffreadbuf(char const * filename, size_t * len);

/* speedy macros */
#define ffclearerr(fp) ((fp)->flag &= ~(FFF_ERR|FFF_EOF))
#define ffeof(fp) ((fp)->flag & FFF_EOF)
#define fferror(fp) ((fp)->flag & FFF_ERR)
#define ffgetpos(fp,pos) (*(pos) = (fp)->pos,0)
#define fftell(fp) ((fp)->pos)
#define ffread(ptr,size,n,fp) ffreadb(ptr,(size)*(n),fp)
#define fflook(ptr,size,n,fp) fflookb(ptr,(size)*(n),fp)

#ifdef __cplusplus
}
#endif

#endif /* ! _included_ffstdio_h_ */
