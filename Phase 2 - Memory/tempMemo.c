#include "stdio.h";

struct PCB
{
    int id;
    int num;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int priority; //(low)10>=priorit>=(high)0
    int endTime;  //0->not finished
    int state;    //-1->finished,0->stopped,1->running
    int runTime;
    struct PCB *nextProcess;
    //waiting time=endTime-arrivalTime-totalTime
    //running time=getClk()-arrivalTime
    /*For memory*/
    int memoSize;
    int startAddress; //-1->if not assigned
};

struct memoryBlock
{
    int state;                     //0->empty,1->full
    int start;                     //start address
    int size;                      //size of the block
    struct memoryBlock *nextBlock; //next memory block
};
struct memoryBlock *root;   //this pointer to first segment in memory
struct memoryBlock *memory; //this pointer to first empty segment in memory (neded for Last Fit)
void initMemory()
{
    root = memory = (struct memoryBlock *)malloc(sizeof(struct memoryBlock));
    memory->state = 0;
    memory->start = 0;
    memory->size = 1024;
    memory->nextBlock = NULL;
}
void checkNeighboring() // if there is neighbors segments empty then it will merge them
{
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL)
    {
        if (currentBlock->state == 0 && currentBlock->nextBlock->state == 0)
        {
            currentBlock->size += currentBlock->nextBlock->size;
            struct memoryBlock *temp = currentBlock->nextBlock;
            currentBlock->nextBlock = currentBlock->nextBlock->nextBlock;
            free(currentBlock->nextBlock);
        }
        currentBlock = currentBlock->nextBlock;
    }
}

void freeMemory(int start)
{ // it set to specific memory segment 0 (empty seg.)
    struct memoryBlock *currentBlock = root;
    struct memoryBlock *lastBlock;

    while (currentBlock != NULL)
    {
        if (currentBlock->start == start && currentBlock->state == 1)
        {
            currentBlock->state == 0;
            checkNeighboring();
            return;
        }
        lastBlock = currentBlock;
        currentBlock = currentBlock->nextBlock;
    }
}
int allocateMemory(int start, int size, struct memoryBlock *currentBlock)
{ /*-1->no memory avalible, 0->the operation has done*/
    if (currentBlock != NULL)
    { // while isn't needed
        if (currentBlock->start == start)
        {
            if (currentBlock->state == 1)
            {
                return -1;
            }
            if (currentBlock->size < size)
            {
                return -1;
            }
            currentBlock->state = 1;
            if (currentBlock->nextBlock == NULL && currentBlock->size - size == 0) //this case that I found the allocate space in the end of memory
            {
                memory = root;
                return 0;
            }
            else if (currentBlock->size - size == 0) //this case that I found the allocate space in the middel of memory
            {
                memory = currentBlock -> nextBlock;
                return 0;
            }
            struct memoryBlock *newBlock = (struct memoryBlock *)malloc(sizeof(struct memoryBlock));
            newBlock->state = 0;
            newBlock->start = currentBlock->start + size;
            newBlock->size = currentBlock->size - size;
            currentBlock->size = size;
            newBlock->nextBlock = currentBlock->nextBlock;
            currentBlock->nextBlock = newBlock;
            memory = newBlock;
            return 0;
        }
    }
    return -1;
}
int FirstFit(int size)
{ //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL)
    {
        if (allocateMemory(currentBlock->start, size, currentBlock) != -1)
        { // currentBlock -> start => 0
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    }
    return -1;
}

int LastFit(int size)
{ //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = memory;
    do{
        if(currentBlock == NULL) // to complete the chain
            currentBlock = root;
        if (allocateMemory(currentBlock->start, size, currentBlock) != -1)
        {
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    }
    while (currentBlock != memory);
    return -1;
}

int BestFit(int size)
{ //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = root;
    struct memoryBlock *choosenBlock = NULL;
    int bestsize = __INT_MAX__;
    while (currentBlock != NULL)
    {
        if (currentBlock->state == 0 && currentBlock->size > size && currentBlock->size < bestsize)
        {
            bestsize = currentBlock->size;
            choosenBlock = currentBlock;
        }
        currentBlock = currentBlock->nextBlock;
    }
    if (choosenBlock != NULL)
    {
        allocateMemory(choosenBlock->start, size, choosenBlock);
        return choosenBlock->start;
    }
    return -1;
}
void Buddy_System_Allocation(int size)
{ //-1->if not avalible, start address->avalible
    if (size < 8) //2^3
        size = 8;
    else if (size < 16) //2^4
        size = 16;
    else if (size < 32) //2^5
        size = 32;
    else if (size < 64) //2^6
        size = 64;
    else if (size < 128) //2^7
        size = 128;
    else if (size < 256) //2^8
        size = 256;
    else if (size < 512) //2^9
        size = 512;
    else if (size < 1024) //2^10
        size = 1024;

    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL)
    {
        if (currentBlock->state == 0 && currentBlock->size == size)
        {
            allocateMemory(currentBlock->start, size, currentBlock);
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    }

    currentBlock = root;
    int bestsize = __INT_MAX__;
    while (currentBlock != NULL)
    {
        if (currentBlock->state == 0 && currentBlock->size == size)
        {
            allocateMemory(currentBlock->start, size, currentBlock);
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    }
    return -1;
}

int main()
{
    initMemory();
}