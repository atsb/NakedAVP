
/*******************************************************************
 *
 *    DESCRIPTION:  mem3dc.c
 *
 *    AUTHOR: Rob Rodger
 *
 *    HISTORY: 23/12/96   
 *
 *******************************************************************/


/* mem3dc.c simply keeps a record of all memory allocations called by             */
/* AllocateMem() and checks if DeallocateMem() calls are legitimate            */
/* i.e. the address freed has been allocated. This record           */
/* includes the address, size, filename and line number for the associated     */
/* AllocateMem() call.                                                         */
/*                                                                             */
/* Also, fills in MALLOC_FILL_VALUE in memory malloced and FREE_FILL_VALUE     */
/* in memory freed.                                                            */
/*                                                                             */
/* A record is kept of the total amount of allocated memory outstanding in     */
/* global TotalMemAllocated as well as the total number of outstanding mallocs */
/* in the global TotalMallocNum. Finally, a call to DumpMallocInfo(DUMPTOSCREEN)       */
/* will give a textprint of each outstanding malloc record while               */
/* while DumpMallocInfo(DUMPTOFILE) writes malloc records to a file with       */
/* filename defined by MALLOCDUMPFILE define. Set APPEND_TO_DUMPFILE to 1      */ 
/* in define below if you wish to append malloc info to MALLOCDUMPFILE rather  */
/* than over writing file each time.                                           */
/*                                                                             */
/* To use, define DBGMALLOC as 1 in "mem3dc.h". Obviously for final code       */
/* set DBGMALLOC to 0.                                                         */
/*                                                                             */
/* Note, must use AllocateMem() and DeallocateMem() in code - Do not           */
/* make direct calls to AllocMem()/DeallocMem() contained in platform          */
/* file our_mem.c                                                              */

#include "3dc.h"

#include <string.h>
#include "mem3dc.h"  /* contains extern declarations for platform 
                     specific memory allocation/deallocation.
                     Also contains DBGMALLOC define */ 

#if DBGMALLOC

#include "ourasert.h"

#define MAXMALLOCS	2500000
	/* assertion fires if max exceeded */
	/* changed to 1000000 by DHM 7/4/98; was 70001 */

#define MALLOC_FILL_VALUE 0x21
#define FREE_FILL_VALUE   0x89
#define MALLOCDUMPFILE    "dbgdump.txt"
#define APPEND_TO_DUMPFILE  0  /* define as 0 if overwrite dbgdump.txt rather than append */
#define FREEING_MEMORY    -1

#define AllowedToDeleteNULL Yes
	/*
		Option added 7/4/98 by DHM:
		---------------------------
		This option checks for NULL in record_free() and doesn't update the records.

		According to 2nd and 3rd edition Stroustrup, it's legal to "delete NULL", and it
		has no effect (see e.g. 3rd Edition Stroustrup, p128 paragraph 2).
		According to Appendix B of K&R, free NULL has no effect.
	*/

#if  APPEND_TO_DUMPFILE
#define FILEPERM "a"
#else
#define FILEPERM "w"
#endif

#if !defined(_MSC_VER) /* not required for MS C since MS C has CRT debugging available */
#define OVERRUN_SIZEMIN 2
#define OVERRUN_SIZEMAX 128
#define OVERRUN_SIZEFACTOR 2 /* this is a shift down */

#define OVERRUN_SIZE(sz) ((sz)>OVERRUN_SIZEMAX<<OVERRUN_SIZEFACTOR ? OVERRUN_SIZEMAX : (sz)<OVERRUN_SIZEMIN<<OVERRUN_SIZEFACTOR ? OVERRUN_SIZEMIN : (sz)>>OVERRUN_SIZEFACTOR)
/* I just selected these at random - the 0 term is necessary for wrap around */
static unsigned char const overrun_code [] = { 0xef, 0x94, 0x56, 0x27, 0xf6, 0x76, 0x23, 0x43, 0 };

#else
#undef OVERRUN_SIZE
#endif



typedef struct{
  unsigned long addr;
  unsigned long size;
  #if COPY_FILENAME
  char filename[40];
  #else
  char const * filename; /* JH 30/5/97 - since __FILE__ generates a string in the executable, to which we get passed a pointer, we don't need to make another copy unnecessarily wasting 720K of space on PC !! */
  #endif
  unsigned long linenum;
}MALLOC_RECORD;

/* globals */
MALLOC_RECORD MallocRecord[MAXMALLOCS];
unsigned long TotalMemAllocated = 0;
unsigned long TotalMallocNum = 0;

/* extern function declarations */
extern int textprint(const char* string, ...);
extern void ExitSystem(void);
extern void WaitForReturn(void);

#define textprint2 textprint

/* function declarations */
#if COPY_FILENAME
void record_free(void *ptr, char string[], unsigned long lineno);
void *record_malloc(long size, char string[], unsigned long lineno);
static int AdjustMallocRecord(unsigned long addr, long size, char string[], unsigned long lineno);
#else /* new prototypes to take just pointers - dunno if it's really necessary */
void record_free(void *ptr, char const * string, unsigned long lineno);
void *record_malloc(long size, char const * string, unsigned long lineno);
static int AdjustMallocRecord(unsigned long addr, long size, char const * string, unsigned long lineno);
#endif
void DumpMallocInfo(int type);
static void InitMallocRecords(void);

/* function definitions */

#if COPY_FILENAME
void *record_malloc(long size, char string[], unsigned long lineno)
#else
void *record_malloc(long size, char const * string, unsigned long lineno)
#endif
{
  #ifdef OVERRUN_SIZE
  void *ptr = (void *)AllocMem((size_t) size + OVERRUN_SIZE(size));
  #else
  void *ptr = (void *)AllocMem((size_t) size);
  #endif
  if(ptr==NULL) 
  {
    textprint2("\nMalloc Error! %d bytes attempted\n", size);
    return((void *)NULL);
  }
  
  GLOBALASSERT(size>0);  /* should be redundant cos picked up by above malloc attempt */

  AdjustMallocRecord((long)ptr,size, string, lineno);

  return(ptr);
}

#if COPY_FILENAME
void record_free(void *ptr, char string[], unsigned long lineno)
#else
void record_free(void *ptr, char const * string, unsigned long lineno)
#endif
{
  if(AdjustMallocRecord((long)ptr,FREEING_MEMORY, string, lineno))
          DeallocMem((void *)ptr);      /* free previously malloced ptr */
  return;
}

#if COPY_FILENAME
static int AdjustMallocRecord(unsigned long addr, long size, char string[], unsigned long lineno)
#else
static int AdjustMallocRecord(unsigned long addr, long size, char const * string, unsigned long lineno)
#endif
{

  int Starti=addr % MAXMALLOCS;
  int i=Starti;
  char *ptr = (char *)addr;
  static int no_record_init = 1;
  MALLOC_RECORD *recordPtr,*StartRecordPtr;
  recordPtr=StartRecordPtr=&MallocRecord[Starti];

  if(no_record_init)
  {
    InitMallocRecords();
    no_record_init = 0;
  }


  if(size==FREEING_MEMORY) /* must be freeing memory */
  {
	#if AllowedToDeleteNULL
	if (NULL == addr)
	{
		return 1;
			/*
				...so the record_free() function calls
				DeallocMem(NULL), which ought to do nothing
				(refer to citations in comment near the top of
				this file)
			*/
	}
	#endif
	
    GLOBALASSERT(addr); /* ensure not null addr */

    while(i<MAXMALLOCS)
    {
       if(recordPtr->addr==addr)
       {
          TotalMallocNum--;

          size = recordPtr->size;

          while (size--)
          {
            *ptr++ = FREE_FILL_VALUE;
          }
		  #ifdef OVERRUN_SIZE
		  {
			char const * overrun_ptr = overrun_code;
			int ov_cnt = OVERRUN_SIZE(recordPtr->size);
			do /* a memcmp - dunno if its supported on all platforms */
			{
				if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
				if (*overrun_ptr++ != *ptr++)
				{
			        textprint2("\nOut of Bounds!\n%lu bytes allocated to %p\nat %s, line %lu\n", recordPtr->size,(void *)recordPtr->addr,recordPtr->filename,recordPtr->linenum);
					GLOBALASSERT(!"OUT OF BOUNDS detected in FREE");
				}
			}
			while (--ov_cnt);
		  }
		  #endif

          recordPtr->addr = 0;
          TotalMemAllocated -= recordPtr->size;
          recordPtr->size = 0;
		  #if COPY_FILENAME
          recordPtr->filename[0] = 0;
		  #else
          recordPtr->filename = "";
		  #endif
          recordPtr->linenum = 0;  
          break; /* exit while loop */
       }
      i++;
      recordPtr++;
     }
	 if(i==MAXMALLOCS)
	 {	
		i=0;
		recordPtr=&MallocRecord[0];
     	
    	while(i<Starti)
    	{
    	   if(recordPtr->addr==addr)
    	   {
    	      TotalMallocNum--;

    	      size = recordPtr->size;

    	      while (size--)
    	      {
    	        *ptr++ = FREE_FILL_VALUE;
    	      }
			  #ifdef OVERRUN_SIZE
			  {
				char const * overrun_ptr = overrun_code;
				int ov_cnt = OVERRUN_SIZE(recordPtr->size);
				do /* a memcmp - dunno if its supported on all platforms */
				{
					if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
					if (*overrun_ptr++ != *ptr++)
					{
				        textprint2("\nOut of Bounds!\n%lu bytes allocated to %p\nat %s, line %lu\n", recordPtr->size,(void *)recordPtr->addr,recordPtr->filename,recordPtr->linenum);
						GLOBALASSERT(!"OUT OF BOUNDS detected in FREE");
					}
				}
				while (--ov_cnt);
			  }
			  #endif

    	      recordPtr->addr = 0;
    	      TotalMemAllocated -= recordPtr->size;
    	      recordPtr->size = 0;
			  #if COPY_FILENAME
    	      recordPtr->filename[0] = 0;
			  #else
    	      recordPtr->filename = "";
			  #endif
    	      recordPtr->linenum = 0;  
    	      break; /* exit while loop */
    	   }
    	  i++;
    	  recordPtr++;
    	}
     	if(i>=Starti)
     	{
     	   textprint2("\n\n\n\nFree Error! %s, line %d\n", string, (int)lineno);
     	   GLOBALASSERT(0); 
     	   return(0);
     	} 
	 }
   }

   else /* must be mallocing memory */
   {
    TotalMallocNum++;
    GLOBALASSERT(TotalMallocNum<MAXMALLOCS); /* just increase MAXMALLOCS define above if this fires */


	// RWH chack to see that this address isn't already in use
    while(i<MAXMALLOCS)
    {
     if(recordPtr->addr==0)
     {
        recordPtr->addr = addr;
        recordPtr->size = size;
        TotalMemAllocated += size;
		#if COPY_FILENAME
        strcpy(recordPtr->filename, string);
		#else
		recordPtr->filename = string;
		#endif
        recordPtr->linenum = lineno;
        while (size--)
        {
          *ptr++ = MALLOC_FILL_VALUE;
        }
		#ifdef OVERRUN_SIZE
		{
			char const * overrun_ptr = overrun_code;
			int ov_cnt = OVERRUN_SIZE(recordPtr->size);
			do /* a memcpy */
			{
				if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
				*ptr++ = *overrun_ptr++;
			}
			while (--ov_cnt);
		}
		#endif
        break; /* exit while loop */
      }
      i++;
      recordPtr++;
    }
	if(i>=MAXMALLOCS)
	{
    	i=0;
		recordPtr=&MallocRecord[0];
    	while(i<Starti)
    	{
    	 if(recordPtr->addr==0)
    	 {
    	    recordPtr->addr = addr;
    	    recordPtr->size = size;
    	    TotalMemAllocated += size;
			#if COPY_FILENAME
	        strcpy(recordPtr->filename, string);
			#else
			recordPtr->filename = string;
			#endif
    	    recordPtr->linenum = lineno;
    	    while (size--)
    	    {
    	      *ptr++ = MALLOC_FILL_VALUE;
    	    }
			#ifdef OVERRUN_SIZE
			{
				char const * overrun_ptr = overrun_code;
				int ov_cnt = OVERRUN_SIZE(recordPtr->size);
				do /* a memcpy */
				{
					if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
					*ptr++ = *overrun_ptr++;
				}
				while (--ov_cnt);
			}
			#endif
    	    break; /* exit while loop */
    	  }
    	  i++;
    	  recordPtr++;
    	}
    	GLOBALASSERT(i<Starti);  /* This should never fire - oh well */
	}
  }
  return(1);

}       

void DumpMallocInfo(int type)
{
   int i;

   if(type==DUMPTOSCREEN)
   {
     for (i=0;i<MAXMALLOCS;i++ )
     {
        if(MallocRecord[i].addr!=0)
        {
	       if (!(MallocRecord[i].linenum & CPPGLOBAL))
		   {
	           textprint2("\nmalloc: %d bytes,line %d in %s", 
	                (int)MallocRecord[i].size, (int)MallocRecord[i].linenum, MallocRecord[i].filename);
		   }
		   else
		   {
	           textprint2("\nC++ global construct malloc: %d bytes,line %d in %s", 
	                (int)MallocRecord[i].size, (int)(MallocRecord[i].linenum &~CPPGLOBAL), MallocRecord[i].filename);
		   }
        }
     } 
     textprint2("\n\n\n\nTotalMemAllocated: %d\nTotalMallocNum: %d",
                    (int)TotalMemAllocated, (int)TotalMallocNum);
     WaitForReturn();
   }
   else if (type==DUMPTOFILE)
   {
     FILE *fp;

     if( (fp = fopen(MALLOCDUMPFILE,FILEPERM))== (FILE *)NULL) 
     {
       textprint2("\n\n\nfile open error %s", MALLOCDUMPFILE);
     }
     else
     {
        fprintf(fp,"\n\n\n\nOUTSTANDING MEMORY ALLOCATIONS\n\n"); 
        for (i=0;i<MAXMALLOCS;i++ )
         {
            if(MallocRecord[i].addr!=0)
            {
		       if (!(MallocRecord[i].linenum & CPPGLOBAL))
			   {
	               fprintf(fp,"\nmalloc: %d bytes,line %d in %s", 
	                    (int)MallocRecord[i].size, (int)MallocRecord[i].linenum, MallocRecord[i].filename);
			   }
		       else
			   {
	               fprintf(fp,"\nC++ global construct malloc: %d bytes,line %d in %s", 
	                    (int)MallocRecord[i].size, (int)(MallocRecord[i].linenum &~CPPGLOBAL), MallocRecord[i].filename);
			   }
            }
         }
         fprintf(fp,"\n\nTotalMemAllocated: %d\nTotalMallocNum: %d\n",
                    (int)TotalMemAllocated, (int)TotalMallocNum);
         
         fclose(fp); 
     }

   }

}

void DumpBoundsCheckInfo(int type)
{
   #ifdef OVERRUN_SIZE
   int i;

   if(type==DUMPTOSCREEN)
   {
	int bc_errcnt = 0;
    for (i=0;i<MAXMALLOCS;i++ )
     {
        if(MallocRecord[i].addr!=0)
        {
			MALLOC_RECORD *recordPtr = &MallocRecord[i];
			
			unsigned char const * overrun_ptr = overrun_code;
			unsigned char const * ptr = (unsigned char const *)recordPtr->addr + recordPtr->size;
			int ov_cnt = OVERRUN_SIZE(recordPtr->size);
			do /* a memcmp - dunno if its supported on all platforms */
			{
				if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
				if (*overrun_ptr++ != *ptr++)
				{
			        textprint2("\nOut of Bounds!\n%lu bytes allocated to %p\nat %s, line %lu\n", recordPtr->size,(void *)recordPtr->addr,recordPtr->filename,recordPtr->linenum);
					++bc_errcnt;
					break;
				}
			}
			while (--ov_cnt);
        }
     }
     if (bc_errcnt)
	 	WaitForReturn();
	 else
     	textprint2("No bounds errors detected\n");

   }
   else if (type==DUMPTOFILE)
   {
     FILE *fp;

     if( (fp = fopen(MALLOCDUMPFILE,FILEPERM))== (FILE *)NULL) 
     {
       textprint2("\n\n\nfile open error %s", MALLOCDUMPFILE);
     }
     else
     {
        fprintf(fp,"\n\n\n\nBOUNDS CHECK PROBLEMS\n\n"); 
        for (i=0;i<MAXMALLOCS;i++ )
         {
            if(MallocRecord[i].addr!=0)
            {
				MALLOC_RECORD *recordPtr = &MallocRecord[i];
				
				unsigned char const * overrun_ptr = overrun_code;
				unsigned char const * ptr = (unsigned char const *)recordPtr->addr + recordPtr->size;
				int ov_cnt = OVERRUN_SIZE(recordPtr->size);
				do /* a memcmp - dunno if its supported on all platforms */
				{
					if (!*overrun_ptr) overrun_ptr = overrun_code; /* repeat */
					if (*overrun_ptr++ != *ptr++)
					{
				        fprintf(fp,"\nOut of Bounds!\n%lu bytes allocated to %p\nat %s, line %lu\n", recordPtr->size,(void *)recordPtr->addr,recordPtr->filename,recordPtr->linenum);
						break;
					}
				}
				while (--ov_cnt);
            }
         }
         fprintf(fp,"\n\nTotalMemAllocated: %d\nTotalMallocNum: %d\n",
                    (int)TotalMemAllocated, (int)TotalMallocNum);
         
         fclose(fp); 
     }

   }

   #endif
}

void InitMallocRecords(void)
{
  int i;
  MALLOC_RECORD *recordPtr = MallocRecord;

  for (i=0;i<MAXMALLOCS;i++,recordPtr++)
  {
    recordPtr->addr = 0;
    recordPtr->size = 0;
	#if COPY_FILENAME
    recordPtr->filename[0] = 0;
	#else
    recordPtr->filename = "";
	#endif
    recordPtr->linenum = 0;  
  }

  return;
}

#else
void DumpMallocInfo(int type);
void DumpMallocInfo(int type)
{
  /* empty if not debugging */
}
void DumpBoundsCheckInfo(int type)
{}
#endif
