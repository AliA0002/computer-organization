#define _CRT_SECURE_NO_WARNINGS
/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256
#define MAX_NUM_SETS 256
#define NUMREGS 8

extern int mem_access(int addr, int write_flag, int write_data);
extern int get_num_mem_accesses();

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int memory_location;
    int tag;
    int valid;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_NUM_SETS][MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
    int blockOffset;
    int setIndex;
    int cacheLines;
    int misses;
    int hits;
    int writebacks;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

int LRU = 0;

void printAction(int, int, enum actionType);
void printCache();
void store(int addr, int writeData);
int load(int addr);
int getLRU(int set);
void updateLRU(int set, int idx);

/*
 * Set up the cache with given command line parameters. This is 
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet){
    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;
    cache.cacheLines = blockSize * blocksPerSet * numSets;
    cache.blockOffset = log10(blockSize) / log10(2);
    cache.setIndex = log10(numSets) / log10(2);
    

    for (int i = 0; i < numSets; ++i)
    {
        for (int j = 0; j < blocksPerSet; ++j)
        {
            cache.blocks[i][j].dirty = 0;
            cache.blocks[i][j].valid = 0;
            cache.blocks[i][j].lruLabel = -1;
            cache.blocks[i][j].tag = -1;
        }
    }

    printf("Simulating a cache with %d total lines; each like has %d words\n"
        , cache.cacheLines, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n"
        , blocksPerSet, numSets);

    return;
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data) {

    if (write_flag == 0) return(load(addr));
    else store(addr, write_data);

    return 0;

    //return mem_access(addr, write_flag, write_data);
}


/*
 * print end of run statistics like in the spec. This is not required,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(){
    printf("$$$ Main memory words accessed: %d\n", get_num_mem_accesses());
    printf("End of run statistics:\n");
    printf("hits %d, misses %d, writebacks %d\n", cache.hits, cache.misses, cache.writebacks);
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache()
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                printf(" %i", cache.blocks[set][block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}

int load(int addr)
{
    //printCache();       //MAKE SURE TO GET RID OF
    int set_idx = 0, tag = 0, offset = 0, mem_idx = 0;
    tag = addr / cache.blockSize / cache.numSets;
    set_idx = (addr / cache.blockSize) % cache.numSets;
    offset = addr % cache.blockSize;
    mem_idx = addr - offset;
    int temp = mem_idx;

    //HIT       no memory access
    for (int i = 0; i < cache.blocksPerSet; ++i)
    {
        if (cache.blocks[set_idx][i].tag == tag)    //tag found
        {
            cache.hits++;
            //cache.blocks[set_idx][i].lruLabel++;
            updateLRU(set_idx, i);
            printAction(addr, 1, cacheToProcessor);
            return cache.blocks[set_idx][i].data[offset];
        }
    }

    cache.misses++;

    //MISS: look for empty block        memory access = blocksize
    for (int i = 0; i < cache.blocksPerSet; ++i)
    {
        if (cache.blocks[set_idx][i].valid == 0)
        {
            cache.blocks[set_idx][i].memory_location = mem_idx;
            cache.blocks[set_idx][i].tag = tag;
            cache.blocks[set_idx][i].dirty = 0;
            //cache.blocks[set_idx][i].lruLabel++;
            cache.blocks[set_idx][i].valid = 1;
            updateLRU(set_idx,i);

            for (int j = 0; j < cache.blockSize; ++j)
            {
                cache.blocks[set_idx][i].data[j] = mem_access(temp, 0, 0);
                temp++;
            }
            printAction(mem_idx, cache.blockSize, memoryToCache);
            printAction(addr, 1, cacheToProcessor);
            return cache.blocks[set_idx][i].data[offset];
        }
    }

    int lru = getLRU(set_idx);

    //MISS: not dirty
    if (cache.blocks[set_idx][lru].dirty == 0)
    {
        printAction(cache.blocks[set_idx][lru].memory_location, cache.blockSize, cacheToNowhere);
        for (int i = 0; i < cache.blockSize; ++i)
        {
            cache.blocks[set_idx][lru].data[i] = mem_access(temp, 0, 0);
            temp++;
        }
        cache.blocks[set_idx][lru].memory_location = mem_idx;
        updateLRU(set_idx,lru);
        //cache.blocks[set_idx][lru].lruLabel++;
        cache.blocks[set_idx][lru].tag = tag;
        printAction(mem_idx, cache.blockSize, memoryToCache);
        printAction(addr, 1, cacheToProcessor);
        return cache.blocks[set_idx][lru].data[offset];
    }
    //MISS: dirty
    else
    {
        cache.writebacks++;
        int memory_loc = cache.blocks[set_idx][lru].memory_location;
        printAction(memory_loc, cache.blockSize, cacheToMemory);
        for (int i = 0; i < cache.blockSize; ++i)   //Evict from cache to memory
        {
            mem_access(memory_loc, 1, cache.blocks[set_idx][lru].data[i]);
            memory_loc++;
            cache.blocks[set_idx][lru].data[i] = mem_access(temp, 0, 0);
            temp++;
        }
        cache.blocks[set_idx][lru].tag = tag;
        cache.blocks[set_idx][lru].dirty = 0;
        cache.blocks[set_idx][lru].memory_location = mem_idx;
        updateLRU(set_idx,lru);
        printAction(mem_idx, cache.blockSize, memoryToCache);
        printAction(addr, 1, cacheToProcessor);
        return cache.blocks[set_idx][lru].data[offset];
    }
}

void store(int addr, int writeData)
{
    //printCache();   //MAKE SURE TO GET RID OF
    int set_idx = 0, tag = 0, offset = 0, mem_idx = 0;
    tag = addr / cache.blockSize / cache.numSets;
    set_idx = (addr / cache.blockSize) % cache.numSets;
    offset = addr % cache.blockSize;
    mem_idx = addr - offset;
    int temp = mem_idx;

    //HIT
    for (int i = 0; i < cache.blocksPerSet; ++i)
    {
        if (cache.blocks[set_idx][i].tag == tag)    //tag found
        {
            cache.hits++;
            //cache.blocks[set_idx][i].lruLabel++;
            updateLRU(set_idx, i);
            cache.blocks[set_idx][i].dirty = 1;
            cache.blocks[set_idx][i].data[offset] = writeData;//mem_access(writeData, 0, 0);
            cache.blocks[set_idx][i].memory_location = mem_idx;
            printAction(addr, 1, processorToCache);
            return;
        }
    }

    cache.misses++;

    //MISS: look for empty block
    for (int i = 0; i < cache.blocksPerSet; ++i)
    {
        if (cache.blocks[set_idx][i].valid == 0)
        {
            cache.blocks[set_idx][i].tag = tag;
            cache.blocks[set_idx][i].dirty = 1;
            // cache.blocks[set_idx][i].lruLabel++;
            cache.blocks[set_idx][i].valid = 1;
            updateLRU(set_idx, i);

            for (int j = 0; j < cache.blockSize; ++j)   //can reduce mem_access by 1 by skipping main data block
            {
                cache.blocks[set_idx][i].data[j] = mem_access(temp, 0, 0);
                temp++;
            }
            cache.blocks[set_idx][i].data[offset] = writeData;
            cache.blocks[set_idx][i].memory_location = mem_idx;
            printAction(mem_idx, cache.blockSize, memoryToCache);
            printAction(addr, 1, processorToCache);
            return;
        }
    }

    int lru = getLRU(set_idx);

    //MISS: not dirty
    if (cache.blocks[set_idx][lru].dirty == 0)
    {
        printAction(cache.blocks[set_idx][lru].memory_location, cache.blockSize, cacheToNowhere);

        for (int i = 0; i < cache.blockSize; ++i)
        {
            cache.blocks[set_idx][lru].data[i] = mem_access(temp, 0, 0);
            temp++;
        }
        //cache.blocks[set_idx][lru].lruLabel++;
        updateLRU(set_idx, lru);
        cache.blocks[set_idx][lru].tag = tag;
        cache.blocks[set_idx][lru].memory_location = mem_idx;
        cache.blocks[set_idx][lru].dirty = 1;
        cache.blocks[set_idx][lru].data[offset] = writeData;
        printAction(mem_idx, cache.blockSize, memoryToCache);
        printAction(addr, 1, processorToCache);
    }
    else
    {
        cache.writebacks++;
        int memory_loc = cache.blocks[set_idx][lru].memory_location;
        printAction(memory_loc, cache.blockSize, cacheToMemory);
        for (int i = 0; i < cache.blockSize; ++i)
        {
            mem_access(memory_loc, 1, cache.blocks[set_idx][lru].data[i]);
            memory_loc++;
            cache.blocks[set_idx][lru].data[i] = mem_access(temp, 0, 0);
            temp++;
        }
        //cache.blocks[set_idx][lru].lruLabel++;
        updateLRU(set_idx, lru);
        cache.blocks[set_idx][lru].tag = tag;
        cache.blocks[set_idx][lru].dirty = 1;
        cache.blocks[set_idx][lru].memory_location = mem_idx;
        cache.blocks[set_idx][lru].data[offset] = writeData;

        printAction(mem_idx, cache.blockSize, memoryToCache);
        printAction(addr, 1, processorToCache);
    }
}

int getLRU(int set)
{
    int max = cache.blocks[set][0].lruLabel;
    int idx = 0;
    for (int i = 1; i < cache.blocksPerSet; ++i)
    {
        if (cache.blocks[set][i].lruLabel > max)
        {
            max = cache.blocks[set][i].lruLabel;
            idx = i;
        }
    }
    return idx;
}

void updateLRU(int set,int idx)
{
    for (int i = 0; i < cache.blocksPerSet; ++i)
    {
        if(cache.blocks[set][i].valid == 1)
            cache.blocks[set][i].lruLabel++;
    }
    cache.blocks[set][idx].lruLabel = 0;
}