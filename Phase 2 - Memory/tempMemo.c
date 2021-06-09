#include <stdlib.h>

#include "stdio.h"

struct PCB {
    int id;
    int num;
    int totalTime;
    int arrivalTime;
    int remainingTime;
    int priority;  //(low)10>=priorit>=(high)0
    int endTime;   //0->not finished
    int state;     //-1->finished,0->stopped,1->running
    int runTime;
    struct PCB *nextProcess;
    //waiting time=endTime-arrivalTime-totalTime
    //running time=getClk()-arrivalTime
    /*For memory*/
    int memoSize;
    int startAddress;  //-1->if not assigned
};

struct memoryBlock {
    int state;                      //0->empty,1->full
    int start;                      //start address
    int size;                       //size of the block
    struct memoryBlock *nextBlock;  //next memory block
};

struct memoryBlock *root;  //this pointer to first segment in memory

struct memoryBlock *memory;  //this pointer to first empty segment in memory (neded for Last Fit)

struct PCB *waitingListHead = NULL;

struct PCB *waitingListTail = NULL;

void enQueue(struct PCB *processe) {
    if (waitingListHead == NULL) {
        waitingListHead = waitingListTail = processe;
        return;
    }
    waitingListTail->nextProcess = processe;
}

void initMemory() {
    root = memory = (struct memoryBlock *)malloc(sizeof(struct memoryBlock));
    memory->state = 0;
    memory->start = 0;
    memory->size = 1024;
    memory->nextBlock = NULL;
}

void checkNeighboring()  // if there is neighbors segments empty then it will merge them
{
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL && currentBlock->nextBlock != NULL) {
        if (currentBlock->state == 0 && currentBlock->nextBlock->state == 0) {
            currentBlock->size += currentBlock->nextBlock->size;
            struct memoryBlock *temp = currentBlock->nextBlock;
            currentBlock->nextBlock = currentBlock->nextBlock->nextBlock;
            free(currentBlock->nextBlock);
        }
        currentBlock = currentBlock->nextBlock;
    }
}

void freeMemory(int start) {  // it set to specific memory segment 0 (empty seg.)
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL) {
        if (currentBlock->start == start && currentBlock->state == 1) {
            currentBlock->state = 0;
            checkNeighboring();
            return;
        }
        currentBlock = currentBlock->nextBlock;
    }
}

int allocateMemory(int start, int size, struct memoryBlock *currentBlock) { /*-1->no memory avalible, 0->the operation has done*/
    if (currentBlock != NULL) {                                             // while isn't needed
        if (currentBlock->start == start) {
            if (currentBlock->state == 1) {
                return -1;
            }
            if (currentBlock->size < size) {
                return -1;
            }
            currentBlock->state = 1;
            if (currentBlock->nextBlock == NULL && currentBlock->size - size == 0)  //this case that I found the allocate space in the end of memory
            {
                memory = root;
                return 0;
            } else if (currentBlock->size - size == 0)  //this case that I found the allocate space in the middel of memory
            {
                memory = currentBlock->nextBlock;
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

int FirstFit(int size) {  //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL) {
        if (allocateMemory(currentBlock->start, size, currentBlock) != -1) {  // currentBlock -> start => 0
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    }
    return -1;
}

int LastFit(int size) {  //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = memory;
    do {
        if (currentBlock == NULL)  // to complete the chain
            currentBlock = root;
        if (allocateMemory(currentBlock->start, size, currentBlock) != -1) {
            return currentBlock->start;
        }
        currentBlock = currentBlock->nextBlock;
    } while (currentBlock != memory);
    return -1;
}

int BestFit(int size) {  //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = root;
    struct memoryBlock *choosenBlock = NULL;
    int bestsize = __INT_MAX__;
    while (currentBlock != NULL) {
        if (currentBlock->state == 0 && currentBlock->size >= size && currentBlock->size < bestsize) {
            bestsize = currentBlock->size;
            choosenBlock = currentBlock;
        }
        currentBlock = currentBlock->nextBlock;
    }
    if (choosenBlock != NULL) {
        allocateMemory(choosenBlock->start, size, choosenBlock);
        return choosenBlock->start;
    }
    return -1;
}

int nearestPowerOfTwo(int size) {  //helpler to Buddy_System_Allocation
    if (256 < size && size <= 1024) {
        return 1024;
    } else if (128 < size && size <= 256) {
        return 256;
    } else if (64 < size && size <= 128) {
        return 128;
    } else if (32 < size && size <= 64) {
        return 64;
    } else if (16 < size && size <= 32) {
        return 32;
    } else if (8 < size && size <= 16) {
        return 16;
    } else if (4 < size && size <= 8) {
        return 8;
    } else if (2 < size && size <= 4) {
        return 4;
    } else if (1 < size && size <= 2) {
        return 2;
    } else if (0 < size && size <= 1) {
        return 1;
    }
}
void freeMemory_Buddy(int start) {
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL) {
        if (currentBlock->start == start) {
            currentBlock->state = 0;
            break;
        }
    }
    currentBlock = root;
    struct memoryBlock *lastBlock;
    while (currentBlock != NULL) {
        if (currentBlock->state == 0) {
            if (currentBlock->start % 2 == 0 && currentBlock->nextBlock != NULL && currentBlock->nextBlock->state == 0) {
                currentBlock->size *= 2;
                struct memoryBlock *tempNext = currentBlock->nextBlock;
                currentBlock->nextBlock = tempNext->nextBlock;
                free(tempNext);
                freeMemory_Buddy(currentBlock->start);
            } else if (currentBlock->start % 2 == 1 && lastBlock->state == 0) {
                lastBlock->size *= 2;
                struct memoryBlock *tempNext = currentBlock;
                lastBlock->nextBlock = tempNext->nextBlock;
                free(tempNext);
                freeMemory_Buddy(lastBlock->start);
            }
            return;
        }
        lastBlock = currentBlock;
        currentBlock = currentBlock->nextBlock;
    }
}
int Buddy_System_Allocation(int size) {  //-1->if not avalible, start address->avalible
    struct memoryBlock *currentBlock = root;
    while (currentBlock != NULL) {
        if (currentBlock->state != 1) {
            int roundedSize = nearestPowerOfTwo(size);
            while (currentBlock->size > roundedSize) {
                struct memoryBlock *newBlock = (struct memoryBlock *)malloc(sizeof(struct memoryBlock));
                newBlock->state = 0;
                currentBlock->size = currentBlock->size / 2;
                newBlock->size = currentBlock->size;
                newBlock->start = currentBlock->start + newBlock->size;
                newBlock->nextBlock = currentBlock->nextBlock;
                currentBlock->nextBlock = newBlock;
            }
            if (currentBlock->size == roundedSize) {
                allocateMemory(currentBlock->start, currentBlock->size, currentBlock);
                return 0;
            }
        }
        currentBlock = currentBlock->nextBlock;
    }
    return -1;
}

// void Print_memo() for testing
// {
//     struct memoryBlock *currentBlock = root;
//     while (currentBlock != NULL)
//     {
//         printf("start -> %d, size -> %d, and state -> %d\n", currentBlock->start, currentBlock->size, currentBlock->state);
//         currentBlock = currentBlock -> nextBlock;
//     }
//     return;
// }

int main() {
    initMemory();
    struct PCB *coming_process;
    /*assign the process*/
    struct memoryBlock *for_new_process;
    /*assign memory for new process*/

    // assigning process to waiting list
    // if (waitingList == NULL)
    //     waitingList = waitingListTail = for_new_process;
    // else {
    //     waitingListTail->nextBlock = for_new_process;
    //     waitingListTail = waitingListTail->nextBlock;
    // }

    // // run process from waiting list
    // if (waitingList != NULL) {
    //     int result;  // = /*run needed algorithm with size = waitingList -> size*/
    //     if (result != -1) {
    //         struct memoryBlock *temp = waitingList;
    //         waitingList = waitingList->nextBlock;
    //         free(temp);
    //     }
    // }

    /* for Mohammed this test case failing (is it impossible case?)
    Buddy_System_Allocation(5);
    Print_memo();
    printf("\n");
    Buddy_System_Allocation(20);
    Print_memo();
    printf("\n");
    freeMemory(0);
    Print_memo();
    printf("\n");
    Buddy_System_Allocation(996);
    Print_memo();
    printf("\n");
    Buddy_System_Allocation(5);
    Print_memo();
    printf("\n");
*/
}