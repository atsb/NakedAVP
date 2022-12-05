/* If this is set to a non-zero value then kzsort.c hijacks the pipeline */
#define KZSORT_ON 1

/* maximum number of modules likely to appear on-screen */
#define MAX_NUMBER_OF_VISIBLE_MODULES 400

struct KItem
{
	POLYHEADER *PolyPtr;

	int SortKey;
};

struct KObject
{
	DISPLAYBLOCK *DispPtr;

	int SortKey;

	int DrawBeforeEnvironment;
};

/* render with new z-sort */
extern void KRenderItems(VIEWDESCRIPTORBLOCK *VDBPtr);

/* generic item shape function */
extern void KShapeItemsInstr(SHAPEINSTR *shapeinstrptr);
extern void OutputKItem(int *shapeitemptr);
							
extern void RenderThisDisplayblock(DISPLAYBLOCK *dbPtr);
