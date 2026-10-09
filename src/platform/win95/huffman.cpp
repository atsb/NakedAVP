#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixer.h"

#include "huffman.hpp"

/* KJL 17:12:25 17/09/98 - Huffman compression/decompression routines */

typedef struct
{
    int          Symbol;
    unsigned int Count;
} HuffItem;

typedef struct HuffNode // 16-byte node structure
{
    union
    { 		                        // the FIRST four bytes
        struct HuffNode *zero;   // points to the "zero" branch or...
        unsigned int       value;	// holds the value of an end node
    };
    
    union
    {								// the SECOND four bytes
        struct HuffNode *one;    // points to the "one" branch or...
        struct HuffNode *link;   // points to next end node in list
    };
    
    struct HuffNode     *parent; // the THIRD four bytes, parent node
    
//    union
//   {   	                        // the FOURTH four bytes
        unsigned int       bits;    // the bit pattern of this end node
//        struct
//       {
//            unsigned char  flag;
//            unsigned char  curdepth;
//            unsigned char  maxdepth;
//            unsigned char  unused;  
//        };
//    };

} HuffNode;

typedef struct
{
    long wid;
    long bits;

} HuffEncode;

static HuffItem SymbolCensus[257];
static HuffNode TreeNodes[2*257];
static int Depths[MAX_DEPTH+1];
static HuffEncode EncodingTable[257];

#define AllocateMemory malloc


/* KJL 17:16:03 17/09/98 - Compression */
static void PerformSymbolCensus(unsigned char *sourcePtr, int length);
static int __cdecl HuffItemsSortSub(const void *cmp1, const void *cmp2);
static void SortCensusData(void);
static void BuildHuffmanTree(void);
static void MakeHuffTreeFromHuffItems(HuffNode *base, HuffItem *source, int count);
static void MakeCodeLengthsFromHuffTree(int *dest, HuffNode *source, int maxdepth);
static int HuffDepthsAdjust(int *depth, int maxdepth);
static void MakeHuffmanEncodeTable(HuffEncode *encodetable, HuffItem *item, int *depths);
static int HuffEncodeBytes(int *dest, unsigned char *source, int count, HuffEncode *table);


extern HuffmanPackage *HuffmanCompression(unsigned char *sourcePtr, int length)
{
	HuffmanPackage *outpackage;


	// Step 1: Perform the symbol census
	PerformSymbolCensus(sourcePtr,length);
	// Step 2: Sorting the census data
	SortCensusData();
	// Step 3: Building the Huffman tree
	BuildHuffmanTree();
	// Step 4: Making the code lengths table
	MakeCodeLengthsFromHuffTree(Depths, TreeNodes, MAX_DEPTH);
	// Step 5: Limiting code lengths
	HuffDepthsAdjust(Depths, MAX_DEPTH);
	// Step 6: Making the encoding table
	MakeHuffmanEncodeTable(EncodingTable,&SymbolCensus[256],Depths);
	// Step 7: Encoding data
	outpackage = (HuffmanPackage*)AllocateMemory(sizeof(HuffmanPackage)+length);
	strncpy(outpackage->Identifier,COMPRESSED_RIF_IDENTIFIER,8);
	outpackage->CompressedDataSize = HuffEncodeBytes((int*)(outpackage+1), sourcePtr, length, EncodingTable);
    outpackage->UncompressedDataSize = length;
    for (int n = 0; n < MAX_DEPTH; n++)  
	{
    	outpackage->CodelengthCount[n] = Depths[n + 1];
	}
    for (int n = 0; n < 256; n++)
	{
	   	outpackage->ByteAssignment[n]  = SymbolCensus[n + 1].Symbol;
	}
	return outpackage;
}
			
static void PerformSymbolCensus(unsigned char *sourcePtr, int length)
{
	// init array
	for (int i=0; i<257; i++)
	{
		SymbolCensus[i].Symbol = i;
		SymbolCensus[i].Count = 0;
	}

	// count 'em
	do
	{
		SymbolCensus[*sourcePtr++].Count++;
	}
	while (--length);
}			

static int __cdecl HuffItemsSortSub(const void *cmp1, const void *cmp2)
{
    if (((HuffItem *)cmp1)->Count > ((HuffItem *)cmp2)->Count)
        return  1;
    if (((HuffItem *)cmp1)->Count < ((HuffItem *)cmp2)->Count)
        return -1;
    return 0;
}
static void SortCensusData(void)
{
	qsort(SymbolCensus, 257, sizeof(HuffItem), HuffItemsSortSub);
}

static void BuildHuffmanTree(void)
{
	MakeHuffTreeFromHuffItems(TreeNodes,SymbolCensus,257);
}

static void MakeHuffTreeFromHuffItems(HuffNode *base, HuffItem *source, int count)
{
    HuffNode *movdest, *temp;
    int n, upperlim, lowerlim, index;
    unsigned int sum;

    if (!count) return;

    movdest = base + 1;
    temp    = base + count;
    memset(temp, 0, count * sizeof(HuffNode));
    for (n = 0; n < count; n++)
    {
    	temp[n].bits = source[n].Count;
	}
    while ((upperlim = --count))
    {
        if (temp[0].zero)
            temp[0].zero->parent = temp[0].one->parent = movdest;
        if (temp[1].zero)
            temp[1].zero->parent = temp[1].one->parent = movdest + 1;
        movdest[0]   = *temp++;
        movdest[1]   = *temp;
        sum = movdest[0].bits + movdest[1].bits;
        lowerlim = 1;
        
        while (lowerlim != upperlim)
        {
            index = (lowerlim + upperlim) >> 1;
            if (sum >= temp[index].bits)
            {
            	lowerlim = index + 1;
            }
            else
            {
                upperlim = index;
            }
        }
        index            = lowerlim - 1;
        memmove(temp, temp + 1, index * sizeof(HuffNode));
        temp[index].bits = sum;
        temp[index].zero = movdest;
        temp[index].one  = movdest + 1;
        movdest         += 2;
    }
    base[0] = temp[0];
    if (base[0].zero)
        base[0].zero->parent = base[0].one->parent = base;
}

static void MakeCodeLengthsFromHuffTree(int *dest, HuffNode *source, int maxdepth)
{
    int n, depth;
    HuffNode *back;

    for (n = 0; n < maxdepth + 1; n++)
        dest[n] = 0;
    depth = 0;
    while (1)
    {
        while (source->one)
        {
            source = source->one;
            depth++;
        }
        
        if (depth > maxdepth) dest[maxdepth]++;
        else dest[depth]++;

        do
        {
            back   = source;
            source = source->parent;
            if (!depth--)
                return;
        }
        while (back == source->zero);

        source = source->zero;
	    depth++;
    }
}

static int HuffDepthsAdjust(int *depth, int maxdepth)
{
    unsigned int n, m, items, sum, goal, gain, busts;
    unsigned int promotions, excess, hi;

    goal = 1 << maxdepth;
    for (n = 0, sum = 0, items = 0; n <= (unsigned int)maxdepth; n++)
    {
        items += depth[n];
        sum   += (goal >> n) * depth[n];
    }
    if (items > goal)
        return -1;                              // failure
    for (n = maxdepth - 1; sum > goal; n--)
    {
        if (depth[n])
        {
            gain             = (1 << (maxdepth - n)) - 1;
            busts            = (sum - goal + gain - 1) / gain;
            busts            = (unsigned int)depth[n] < busts ? depth[n] : busts;
            depth[n]        -= busts;
            depth[maxdepth] += busts;
            sum             -= busts * gain;
        }
    }
    excess = goal - sum;
    for (n = 0; excess; n++)
    {
        hi = 1 << (maxdepth - n);
        for (m = n + 1; m <= (unsigned int)maxdepth; m++)
        {
            gain = hi - (1 << (maxdepth - m));
            if (excess < gain)
                break;
            if (depth[m])
            {
                promotions  = excess / gain;
                promotions  = (unsigned int)depth[m] > promotions ? promotions : depth[m];
                depth[n]   += promotions;
                depth[m]   -= promotions;
                excess     -= promotions * gain;
            }
        }
    }
    return 0;                           // success
}

static void MakeHuffmanEncodeTable(HuffEncode *encodetable, HuffItem *item, int *depths)
{
    unsigned int d, bitwidth, depthbit, bt, cur;
    int *dep;

    dep      = depths + 1;              // skip depth zero
    bitwidth = 0;                       // start from small bitwidths
    cur      = 0;                       // current bit pattern
    do
    {
        do
        {
            bitwidth++;                         // go deeper
            depthbit = 1 << (bitwidth - 1);     // keep depth marker
            d = *dep++;                         // get count here
        }
        while (!d);                           // until count non-zero
        while (d--)
        {                           // for all on this level
            encodetable[item->Symbol].wid  = bitwidth; // record width
            encodetable[item->Symbol].bits = cur;      // record bits
            item--;                             // count backwards an item
            bt = depthbit;                      // bt is a temp value
            while (1)
            {
                cur  ^= bt;                     // do an add modulo 1
                if ((cur & bt) || !bt)          // break if now a 1
                    break;                      // or out of bits
                bt  >>=  1;                     // do next bit position
            }
        }
    }
    while (cur);                              // until cur exhausted
}

static int HuffEncodeBytes(int *dest, unsigned char *source, int count, HuffEncode *table)
{
    int          *start;
    int wid, val, available;
    unsigned int  accum, bits;
    unsigned char *sourcelim, *sourceend;

    if (!count) return 0;

	accum = 0;
    start = dest;
    sourcelim = sourceend = source + count;
    available = 32;
    if (sourcelim - 32 < sourcelim)
	{
        sourcelim -= 32;
	}
    else
	{
        sourcelim = source;
	}
    if (source < sourcelim)
    {
        do
        {
            goto lpstart;
            do
            {
                accum = (accum >> wid) | (bits << (32 - wid));
lpstart:        val  = *source++;
                wid  = table[val].wid;
                bits = table[val].bits;
            }
            while ((available -= wid) >= 0);
           
            wid       += available;
            if (wid) accum = (accum >> wid) | (bits << (32 - wid));
            *dest++    = accum;
            wid       -= available;
            accum      = bits << (32 - wid);
            available += 32;
        }
        while (source < sourcelim);
    }
    while (1)
    {
        if (source < sourceend)
		{
            val = *source++;
        }
        else if (source == sourceend)
        {
            val = 0x100;                        // terminator
            source++;
        }
        else break;                             // done

        wid  = table[val].wid;
        bits = table[val].bits;

        if ((available -= wid) < 0)
        {
            wid       += available;
            if (wid)
                accum  = (accum >> wid) | (bits << (32 - wid));
            *dest++    = accum;
            wid       -= available;
            accum      = bits << (32 - wid);
            available += 32;
        }
        else
		{
            accum = (accum >> wid) | (bits << (32 - wid));
		}
    }    
    *dest++ = accum >> available;
    return (int)((dest - start) * 4);
}






/* KJL 17:16:24 17/09/98 - Decompression */
static int DecodeTable[1<<MAX_DEPTH];

static void MakeHuffmanDecodeTable(const int *depth, int depthmax, const unsigned char *list);
static int HuffmanDecode(unsigned char *dest, const int *source, const int *table, int length);


extern char *HuffmanDecompress(const HuffmanPackage *inpackage)
{
	unsigned char *uncompressedData = NULL;
	// Step 1: Make the decoding table
	MakeHuffmanDecodeTable(inpackage->CodelengthCount, MAX_DEPTH, inpackage->ByteAssignment);

	// Step 2: Decode data
	uncompressedData = (unsigned char*)AllocateMemory(inpackage->UncompressedDataSize+16);
	if (uncompressedData)
	{
		HuffmanDecode(uncompressedData,(int*)(inpackage+1),DecodeTable,inpackage->UncompressedDataSize);
	}

	return (char*)uncompressedData;	
}

static void MakeHuffmanDecodeTable(const int *depth, int depthmax, const unsigned char *list)
{
    int thisdepth, depthbit, repcount, repspace, lenbits, temp, count;
	int *outp;
    int o = 0;
    const unsigned char *p;
	int *outtbl = DecodeTable;

    lenbits   = 0;
    repcount  = 1 << depthmax;
    repspace  = 1;
    thisdepth = 0;
    depthbit  = 4;
    p         = list + 255;
    while (1)
    {
        do
        {
            lenbits++;
            depthbit <<= 1;
            repspace <<= 1;
            repcount >>= 1;
        }
        while (!(thisdepth = *depth++));
        do
        {
            if (p < list)
			{
                temp = 0xff;
            }
            else 
            {
                temp = lenbits | (*p-- << 8);
            }
            outp  = outtbl + (o >> 2);
            count = repcount;
            do
            {
                *outp  = temp;
                outp  += repspace;
            }
            while (--count);
            temp = depthbit;
            do
            {
                temp >>= 1;
                if (temp & 3) return;
                o ^= temp;
            }
            while (!(o & temp));
        }
        while (--thisdepth);
    }
}


#define EDXMASK ((((1 << (MAX_DEPTH + 1)) - 1) ^ 1) ^ -1)

static int HuffmanDecode(unsigned char *dest, const int *source, const int *table, int length)
{
    unsigned char          *start;
    int                    available, reserve, fill, wid;
    unsigned int           bits=0, resbits;
    const unsigned char    *p;

    start     = dest;
    available = 0;
    reserve   = 0;
    wid       = 0;
	resbits   = 0;
    do 
    {
        available     += wid;
		fill = 31 - available;							  
        bits         <<= fill;
        if (fill > reserve)
        {
            fill      -= reserve;
            available += reserve;
            if (reserve)
            {
            	bits   = (bits >> reserve) | (resbits << (32 - reserve));
			}
            resbits    = *source++;
            reserve    = 32;
        }
        bits           = (bits >> fill) | (resbits << (32 - fill));
        resbits      >>= fill;
        reserve       -= fill;
        available      = 31;
        goto lpent;
        do
        {
            bits      >>= wid;
            *dest++   = p[1];
lpent:      p         = (const unsigned char *)(((const short *)table)+(bits & ~EDXMASK));
        }
        while ((available  -= (wid = *p)) >= 0 && (dest-start)!=length);

    }
    while (available > -32 && (dest-start)!=length);
    return (int)(dest - start);
}
