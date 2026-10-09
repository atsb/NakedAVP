#ifndef _huffman_hpp_included
#define _huffman_hpp_included 1

#ifdef __cplusplus
	extern "C"
	{
#endif

#define MAX_DEPTH 11

typedef struct
{
	char			Identifier[8];
    int				CompressedDataSize;
    int				UncompressedDataSize;
    int				CodelengthCount[MAX_DEPTH];
    unsigned char	ByteAssignment[256];
} HuffmanPackage;


/* KJL 17:16:03 17/09/98 - Compression */
extern HuffmanPackage *HuffmanCompression(unsigned char *sourcePtr, int length);

/* KJL 16:53:53 19/09/98 - Decompression */
extern char *HuffmanDecompress(const HuffmanPackage *inpackage);


#define COMPRESSED_RIF_IDENTIFIER "REBCRIF1"
#ifdef __cplusplus
	};
#endif

#endif
