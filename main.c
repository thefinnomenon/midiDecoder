/*
 * Computer Systems Lab I
 * Lab A: Cache Simulator
 * Fall 2011
 *************
 * main.c
 *
 *  Created on: Sep 11, 2011
 *      Author: Chris Finn, Paulo Leal, & Shuwen Cao
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* --------------------------------
**    GLOBAL VARIABLES
** --------------------------------
*/

int C; // Total cache size
int K;    // K-Way associative cache
int L;    // Line size
int nSets;  // Number of sets in the cache
int nTagBits, nOffsetBits, nSetBits;
int tag;
unsigned int set;
int instCache = 0;
// Data cache
int *V;     // Valid bit array
int *Tag;   // Tag array
int *LRU;   // LRU array
int miss;
int hit;
// Instruction cache
int *iV;     // Valid bit array
int *iTag;   // Tag array
int *iLRU;   // LRU array
int iMiss;
int iHit;





/* --------------------------------
**       DATA CACHE METHODS
** --------------------------------
*/
/**
 ** Adds 1 to every LRU value in the set and then resets the LRU value of the
 ** last used index to 0
 **/
void updateLRU(int index, int s)
{
	int i;
	for(i=0;i<K;i++)
        LRU[nSets*i+s]++;
    LRU[index] = 0;
}

/**
 ** Checks set valid bits, searches for tag match and finds largest LRU value and writes
 ** to that index if all indexes in set are valid and tag search fails
 **/
void writeCache(int address)
{
	int index = -1;
    int i;
    for(i=0;i<K;i++)
    {
        if(V[nSets*i+set] == 1)
        {
            index = nSets*i+set;
            if(Tag[index] == tag)
            {
                updateLRU(nSets*i+set, set);
                hit++;
                return;
            }
        }
        else
        {
            index = nSets*i+set;
            break;
        }
    }
	if(index == -1)
	{
	    index = set;
		int lruVal = LRU[set];
		for(i=1;i<K;i++)
		{
			if(LRU[nSets*i+set] > lruVal)
			{
			    index = nSets*i+set;
				lruVal = LRU[nSets*i+set];
			}
		}
	}
    miss++;
	Tag[index] = tag;
	updateLRU(index, set);
	V[index] = 1;
}

/**
 ** Checks set valid bits, searches for matching tag
 ** and calls write if search fails
 **/
void readCache(int address)
{
    int i;
    for(i=0;i<K;i++)
    {
        if(V[nSets*i+set] == 1)
        {
            if(Tag[nSets*i+set] == tag)
            {
                updateLRU(nSets*i+set, set);
                hit++;
                return;
            }
        }
	}
	writeCache(address);
}

/**
 ** Calculates number of sets, allocates memory for
 ** arrays and initializes arrays to all 0's
 **/
int initializeCache()
{
	nSets = C/(L*K);
	if ((V = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
	    return 0;
	}
	if ((Tag = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
		return 0;
	}
	if ((LRU = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
		return 0;
	}
	int i;
	for(i=0; i<nSets*K; i++)
	{
	    V[i] = 0;
	    LRU[i] = 0;
	    Tag[i] = 0;
	}
	return 1;
}

/**
 ** Frees all manually allocated memory
 **/
void freeMemory()
{
    free(V);
    free(LRU);
    free(Tag);
}

/**
 ** Invalidates cache
 **/
void flush()
{
    int i;
    for(i=0; i<nSets*K; i++)
	    V[i] = 0;
}

/**
 ** Outputs data cache to text file
 **/
void displayCache()
{
    FILE *fp;
    fp=fopen("cacheOutput.txt", "w");
	fprintf(fp, "-------------------\n");
	fprintf(fp, "Data Cache display \n");
	fprintf(fp, "---------------\n");
	fprintf(fp, "Set#\tValid\tLRU\tTag");
	int i;
	for(i=1;i<K;i++)
	{
	    fprintf(fp, "\t\tValid\tLRU\tTag");
	}
	fprintf(fp, "\n");
	for(i=0;i<nSets;i++)
	{
	    fprintf(fp, "%d\t", i);
		int j;
		for(j=0;j<K;j++)
		{
		    if(Tag[i+i*j]<1000000)
                fprintf(fp, "|%d|\t|%d\t|%d\t\t", V[i], LRU[i+i*j], Tag[i+i*j]);
            else
                fprintf(fp, "|%d|\t|%d\t|%d\t", V[i], LRU[i+i*j], Tag[i+i*j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

/**
 ** Calculates and displays miss rate
 **/
void displayResults()
{
    printf("Data Cache Misses: %d \n", miss);
    printf("Total: %d \n", (miss + hit));
	if(miss+hit > 0)
        printf("Data Cache Miss rate: %.2f%%\n", ((double)miss/(miss+hit))*100);
    else
        printf("Data Cache Miss rate: %.2f%%\n", 0);
}

/**
 ** Shifts bit string to aquire tag and set values
 **/
void parseBitString(address)
{
    tag = address >> nSetBits;
    tag = tag >> nOffsetBits;
    set = address << nTagBits;
    set = set >> nTagBits;
    set = set >> nOffsetBits;
}

/**
 ** Performs action depending upon type
 **/
void access(int type, int address)
{
    switch(type)
    {
        case 0:readCache(address); break;
        case 1:writeCache(address); break;
        case 2:if(instCache == 0)
                    readCache(address);
               break;
        case 3:break;
        case 4:flush(); break;
    }
}

void parseTrace()
{
	unsigned int address;
	int type;
	FILE *ifp;
	ifp = fopen("trace.txt", "r");
	while (!feof(ifp))
	{
		fscanf(ifp, "%d %x", &type, &address);
		if(instCache == 1 && type != 2 || instCache == 0)
        {
            parseBitString(address);
            access(type, address);
        }
	}
	fclose(ifp);
	displayResults();
}


void parseRandomTrace()
{
	unsigned int address;
	int count = 0;
	int type = 0;
	unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
	int i;
	for(i=0; i<10000; i++)
	{
        address = rand() << 16;
        address = address + rand();
		if(instCache == 1 && type != 2 || instCache == 0)
        {
            parseBitString(address);
            access(type, address);
            count++;
        }
	}
	displayResults(count);
}

void parseSequentialTrace()
{
	unsigned int address = 0;
	int type;
	unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
	int i;
	for(i=0; i<1000; i++)
	{
	    type = rand() % 3;
        if(instCache == 1 && type != 2 || instCache == 0)
        {
            parseBitString(address);
            access(type, address);
        }
        address = address + 4;
	}
	displayResults();
}


/* --------------------------------
**    INSTRUCTION CACHE METHODS
** --------------------------------
*/

/**
 ** Adds 1 to every LRU value in the set and then resets the LRU value of the
 ** last used index to 0
 **/
void instUpdateLRU(int index, int s)
{
	int i;
	for(i=0;i<K;i++)
		iLRU[nSets*i+s]++;
    iLRU[index] = 0;
}

/**
 ** Checks set valid bits, searches for tag match and finds largest LRU value and writes
 ** to that index if all indexes in set are valid and tag search fails
 **/
void instWriteCache(int address)
{
	int index = -1;
    int i;
    for(i=0;i<K;i++)
    {
        if(iV[nSets*i+set] == 1)
        {
            index = nSets*i+set;
            if(iTag[index] == tag)
            {
                instUpdateLRU(nSets*i+set, set);
                iHit++;
                return;
            }
        }
        else
        {
            index = nSets*i+set;
            break;
        }
    }
	if(index == -1)
	{
	    index = set;
		int lruVal = iLRU[set];
		for(i=1;i<K;i++)
		{
			if(iLRU[nSets*i+set] > lruVal)
			{
			    index = nSets*i+set;
				lruVal = iLRU[nSets*i+set];
			}
		}
	}
    iMiss++;
	iTag[index] = tag;
	instUpdateLRU(index, set);
	iV[index] = 1;
}

/**
 ** Checks set valid bits, searches for matching tag
 ** and calls write if search fails
 **/
void instReadCache(int address)
{
    int i;
    for(i=0;i<K;i++)
    {
        if(iV[nSets*i+set] == 1)
        {
            if(iTag[nSets*i+set] == tag)
            {
                instUpdateLRU(nSets*i+set, set);
                iHit++;
                return;
            }
        }
	}
	instWriteCache(address);
}

/**
 ** Calculates number of sets, allocates memory for
 ** arrays and initializes arrays to all 0's
 **/
int initializeInstCache()
{
	nSets = C/(L*K);
	if ((iV = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
	    return 0;
	}
	if ((iTag = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
		return 0;
	}
	if ((iLRU = (int *)malloc(nSets * K * sizeof(int))) == NULL) {
		printf("Malloc error");
		return 0;
	}
	int i;
	for(i=0; i<nSets*K; i++)
	{
	    iV[i] = 0;
	    iLRU[i] = 0;
	    iTag[i] = 0;
	}
	return 1;
}

/**
 ** Frees manually allocated memory for instruction cache
 **/
void freeInstMemory()
{
    free(iV);
    free(iLRU);
    free(iTag);
}

/**
 ** Invalidates instruction instruction cache
 **/
void instFlush()
{
    int i;
    for(i=0; i<nSets*K; i++)
	    iV[i] = 0;
}

/**
 ** Outputs instruction cache to text file
 **/
void displayInstCache()
{
    FILE *fp;
    fp=fopen("instCacheOutput.txt", "w");
	fprintf(fp, "--------------------------\n");
	fprintf(fp, "Instruction Cache display \n");
	fprintf(fp, "---------------\n");
	fprintf(fp, "Set#\tValid\tLRU\tTag");
	int i;
	for(i=1;i<K;i++)
	{
	    fprintf(fp, "\t\tValid\tLRU\tTag");
	}
	fprintf(fp, "\n");
	for(i=0;i<nSets;i++)
	{
	    fprintf(fp, "%d\t", i);
		int j;
		for(j=0;j<K;j++)
		{
		    if(Tag[i+i*j]<1000000)
                fprintf(fp, "|%d|\t|%d\t|%d\t\t", iV[i], iLRU[i+i*j], iTag[i+i*j]);
            else
                fprintf(fp, "|%d|\t|%d\t|%d\t", iV[i], iLRU[i+i*j], iTag[i+i*j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

/**
 ** Calculates and displays instruction cache miss rate
 **/
void displayInstResults()
{
    printf("Instruction Cache Misses: %d \n", iMiss);
    printf("Total: %d \n", iMiss+iHit);
    if(iMiss+iHit > 0)
        printf("Instruction Cache Miss rate: %.2f%%\n", ((double)iMiss/(iMiss+iHit))*100);
    else
        printf("Instruction Cache Miss rate: %.2f%%\n", 0);
}

/**
 ** Performs action depending upon type
 **/
void accessWithInst(int type, int address)
{
    switch(type)
    {
        case 2:instReadCache(address); break;
        case 4:instFlush(); break;
        default: break;
    }
}

void instParseTrace()
{
	unsigned int address;
	int type;
	FILE *ifp;
	ifp = fopen("trace.txt", "r");
	int i;
	while (!feof(ifp))
	{
		fscanf(ifp, "%d %x", &type, &address);
        parseBitString(address);
        accessWithInst(type, address);
	}
	fclose(ifp);
	displayInstResults();
}


void instParseRandomTrace()
{
	unsigned int address;
	int type = 0;
	unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
	int i;
	for(i=0; i<10000; i++)
	{
        address = rand() << 16;
        address = address + rand();
        parseBitString(address);
        accessWithInst(type, address);
	}
	displayInstResults();
}

void instParseSequentialTrace()
{
	unsigned int address = 0;
	int type;
	unsigned int seed = (unsigned int)time(NULL);
	srand(seed);
	int i;
	for(i=0; i<1000; i++)
	{
	    type = rand() % 3;
        parseBitString(address);
        accessWithInst(type, address);
		address = address + 4;
	}
	displayInstResults();
}




/* --------------------------------
**    MAIN METHOD
** --------------------------------
*/

int main()
{
    printf("             Cache Simulator               |\n");
    printf("-------------------------------------------\n");
    printf("      Computer Systems Lab I, Lab A        |\n");
    printf("-------------------------------------------\n");
    printf("Authors: Chris Finn, Paulo Leal, Shuwen Cao|\n");
    printf("-------------------------------------------\n");
    while(1)
    {
        instCache = 0;
        int traceType;
		printf("Trace type\n 1 = trace.txt\n 2 = random\n 3 = sequential\n");
		scanf("%d", &traceType);
		if(traceType == -1)
            break;
        int q;
        printf("Separate data & instruction cache\n yes (1)\n no (0)\n");
        scanf("%d", &q);
        printf("C=");
		scanf("%d", &C);
		printf("K=");
		scanf("%d", &K);
		printf("L=");
		scanf("%d", &L);
        nOffsetBits = log2(L);
        nSetBits = log2(C/(K*L));
        nTagBits = 32 - nOffsetBits - nSetBits;
        freeMemory();
        miss = 0;
        hit = 0;
        iMiss = 0;
        iHit = 0;
        if(q != 1 && q != 0)
        {
            printf("Please enter 0 or 1 for question 2");
            traceType = -1;
        }
        if(q == 1)
        {
            instCache = 1;
            if(initializeInstCache() == 0)
                exit(1);
            switch(traceType)
            {
                case(1): instParseTrace(); break;
                case(2): instParseRandomTrace(); break;
                case(3): instParseSequentialTrace(); break;
                default: printf("Please choose a proper trace type\n");
            }
        }
        if(initializeCache() == 0)
            exit(1);
        switch(traceType)
        {
            case(1): parseTrace(); break;
            case(2): parseRandomTrace(); break;
            case(3): parseSequentialTrace(); break;
            default: printf("Please choose a proper trace type\n");
        }
        displayCache();
        if(instCache == 1)
            displayInstCache();
    }
}
