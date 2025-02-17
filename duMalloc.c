#include<stdio.h>
#include "duMalloc.h"
#include <string.h>


#define HEAP_SIZE (128*8)
#define FIRST_FIT 0
#define BEST_FIT 1

int globalFit = 0;

unsigned char* managedList[HEAP_SIZE/8];
int managedListSize;
int managedIndex;

unsigned char heap[3][HEAP_SIZE];

//make current heap a pointer to the first heap
unsigned char* currentHeap = heap[0];
int heapIndex = 0;
unsigned char* oldHeap = heap[2];


typedef struct memoryBlockHeader {
    int free; // 0 - used, 1 = free
    int size; // size of the reserved block
    int managedIndex; // the unchanging index in the managed array
    int survivalAmt; // the number of times the block has moved between young heaps
    struct memoryBlockHeader* next;
} memoryBlockHeader;



memoryBlockHeader* freeListHead;
memoryBlockHeader* oldFreeListHead;


void duInitMalloc();


void* duMalloc(int size, int heapNum);
void duFree(void* ptr);
void printFreelist();

void duInitMalloc(int fit){
    printf("\n");
    printf("duInitMalloc\n");
    printf("\n");
    for(int i = 0; i < HEAP_SIZE; i++){
        currentHeap[i] = 0;
    }

    for(int i = 0; i < HEAP_SIZE; i++){
        heap[2][i] = 0;
    }
    
    if(fit==BEST_FIT){
        printf("Best Fit\n");
        globalFit = 1;
    } else {
        printf("First Fit\n");
    }

    
    memoryBlockHeader* currentBlock = (memoryBlockHeader*)currentHeap;
    currentBlock->size = HEAP_SIZE - sizeof(memoryBlockHeader);
    currentBlock->next = NULL;
    currentBlock->free = 1;
    freeListHead = currentBlock;

    memoryBlockHeader* oldBlock = (memoryBlockHeader*)oldHeap;
    oldBlock->size = HEAP_SIZE - sizeof(memoryBlockHeader);
    oldBlock->next = NULL;
    oldBlock->free = 1;
    oldFreeListHead = oldBlock;


    
}

void printFreelist(){
    memoryBlockHeader* currentBlock = freeListHead;
    while(currentBlock != NULL){
        printf("Block at %p, size: %d, freedom: %d\n", currentBlock, currentBlock->size, currentBlock->free);
        currentBlock = currentBlock->next;
    }
}

void printOldFreeList(){
    memoryBlockHeader* currentBlock = oldFreeListHead;
    while(currentBlock != NULL){
        printf("Block at %p, size: %d, freedom: %d\n", currentBlock, currentBlock->size, currentBlock->free);
        currentBlock = currentBlock->next;
    }
}

void memoryBlock(){
    memoryBlockHeader* currentBlock = oldFreeListHead;
    memoryBlockHeader* previousBlock = NULL;
    memoryBlockHeader* nextBlock = NULL;
    memoryBlockHeader* tempBlock = NULL;
    while(currentBlock != NULL){
        nextBlock = currentBlock->next;
        while(nextBlock != NULL){
            if(currentBlock > nextBlock){
                if(previousBlock == NULL){
                    freeListHead = nextBlock;
                } else {
                    previousBlock->next = nextBlock;
                }
                tempBlock = nextBlock->next;
                nextBlock->next = currentBlock->next;
                currentBlock->next = tempBlock;
                nextBlock->next = currentBlock;
                currentBlock = nextBlock;
            }
            nextBlock = nextBlock->next;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }


    currentBlock = freeListHead;
    previousBlock = NULL;
    nextBlock = NULL;
    tempBlock = NULL;
    while(currentBlock != NULL){
        nextBlock = currentBlock->next;
        while(nextBlock != NULL){
            if(currentBlock > nextBlock){
                if(previousBlock == NULL){
                    freeListHead = nextBlock;
                } else {
                    previousBlock->next = nextBlock;
                }
                tempBlock = nextBlock->next;
                nextBlock->next = currentBlock->next;
                currentBlock->next = tempBlock;
                nextBlock->next = currentBlock;
                currentBlock = nextBlock;
            }
            nextBlock = nextBlock->next;
        }
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
    }


    currentBlock = (memoryBlockHeader*)currentHeap;
    while((unsigned char*)currentBlock < currentHeap + HEAP_SIZE){
        if(currentBlock->free == 0){
            printf("Used block at %p, size: %d, survival: %d\n", currentBlock, currentBlock->size, currentBlock->survivalAmt);
        } else {
            printf("Free block at %p, size: %d\n", currentBlock, currentBlock->size);
        }
        currentBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + currentBlock->size + sizeof(memoryBlockHeader));
    }



    for(int i = 0; i < HEAP_SIZE; i=i+8){
            if(currentHeap[i] == 0){
                printf("a");
            } else {
                printf("A");
            }
        
    }
    printf("\n");


}

void printManagedList(){
    for(int i = 0; i < managedListSize; i++){
        if(managedList[i] != NULL){
            printf("Managedlist[%d]: %p\n", i, managedList[i]);
        }else{
            printf("ManagedList[%d]: null\n", i);
        }
    }
    
}

void printOldHeap(){
    memoryBlockHeader* currentBlock = (memoryBlockHeader*)oldHeap;
    while((unsigned char*)currentBlock < oldHeap + HEAP_SIZE){
        if(currentBlock->free == 0){
            printf("Used block at %p, size: %d\n", currentBlock, currentBlock->size);
        } else {
            printf("Free block at %p, size: %d\n", currentBlock, currentBlock->size);
        }
        currentBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + currentBlock->size + sizeof(memoryBlockHeader));
    }
}



void duMemoryDump(){

    printf("MEMORY DUMP\n");
    printf("current heap: %d\n", heapIndex);
    printf("young heap\n");
    memoryBlock();
    printf("young free list\n");
    printFreelist();
    printf("managed list\n");
    printManagedList();
    printf("old heap\n");
    printOldHeap();
    printf("old free list\n");
    printOldFreeList();


}

void* duMalloc(int size, int heapNum){

    if(heapNum <2){
    
        memoryBlockHeader* currentBlock = freeListHead;
        
        memoryBlockHeader* previousBlock = NULL;
        int bestSize = HEAP_SIZE;
        
        if (globalFit == BEST_FIT){
            while(currentBlock != NULL){
                if(currentBlock->size >= size + sizeof(memoryBlockHeader) && currentBlock->size <= bestSize){
                    bestSize = currentBlock->size;
                }
                previousBlock = currentBlock;
                currentBlock = currentBlock->next;
            }
        }

        currentBlock = freeListHead;

        previousBlock = NULL;

        while(currentBlock != NULL){
            if(currentBlock->size >= size && currentBlock->size <= bestSize){
                if(currentBlock->size > size + sizeof(memoryBlockHeader)){
                    memoryBlockHeader* newBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + size + sizeof(memoryBlockHeader));
                    newBlock->size = currentBlock->size - size - sizeof(memoryBlockHeader);
                    newBlock->free = 1;
                    newBlock->next = currentBlock->next;
                    currentBlock->size = size;
                    currentBlock->free = 0;
                    currentBlock->next = newBlock;
                    currentBlock->survivalAmt=0;
                } else {
                    currentBlock->free = 0;
                }
                if(previousBlock == NULL){
                    freeListHead = currentBlock->next;
                } else {
                    previousBlock->next = currentBlock->next;
                }
                return (unsigned char*)currentBlock + sizeof(memoryBlockHeader);
            }
            previousBlock = currentBlock;
            currentBlock = currentBlock->next;
        }

        return NULL;
    }else{

        memoryBlockHeader* currentBlock = oldFreeListHead;
        
        memoryBlockHeader* previousBlock = NULL;

        while(currentBlock != NULL){
            if(currentBlock->size > size + sizeof(memoryBlockHeader)){
                memoryBlockHeader* newBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + size + sizeof(memoryBlockHeader));
                newBlock->size = currentBlock->size - size - sizeof(memoryBlockHeader);
                newBlock->free = 1;
                newBlock->next = currentBlock->next;
                currentBlock->size = size;
                currentBlock->free = 0;
                currentBlock->next = newBlock;
                currentBlock->survivalAmt=0;
            } else {
                currentBlock->free = 0;
            }
            if(previousBlock == NULL){
                oldFreeListHead = currentBlock->next;
            } else {
                previousBlock->next = currentBlock->next;
            }
            return (unsigned char*)currentBlock + sizeof(memoryBlockHeader);
        
        previousBlock = currentBlock;
        currentBlock = currentBlock->next;
        }

        return NULL;
    }
            
        
    
    }

void duFree(void* ptr){

    memoryBlockHeader* block = (memoryBlockHeader*)((unsigned char*)ptr - sizeof(memoryBlockHeader));
    
    block->free = 1;
    
    if(block<(memoryBlockHeader*)(currentHeap + HEAP_SIZE)){
        printf("Block freed from young heap\n");
        block->next = freeListHead;
        freeListHead = block;
    }else{
        printf("Block freed from old heap\n");
        block->next = oldFreeListHead;
        oldFreeListHead = block;
    
    }


}

void duManagedInitMalloc(int searchType){
    duInitMalloc(searchType);
    managedListSize=1;
    managedIndex=0;
    for(int i = 0; i < HEAP_SIZE/8; i++){
        managedList[i] = NULL;
    }
    
}

void** duManagedMalloc(int size){
    void** mptr = (void**)duMalloc(size,heapIndex);
    if(mptr == NULL){
        return NULL;
    }
    managedList[managedIndex] = (unsigned char*)mptr;
    managedIndex++;
    managedListSize++;
    return (void**)&managedList[managedIndex-1];
    


}

void duManagedFree(void** mptr){
    duFree((void*)*mptr);
    for(int i = 0; i < managedIndex; i++){
        if((void*)managedList[i] == (void*)*mptr){
            managedList[i] = NULL;
        }
    }
    
    

}

void minorCollection(){
    printf("\n");
    printf("Minor Collection\n");

    memoryBlockHeader* otherHeap = (memoryBlockHeader*) currentHeap;
    heapIndex = !heapIndex;
    currentHeap = heap[heapIndex];
    

    duInitMalloc(globalFit);

    

    for(int i=0;i<managedIndex;i++){
        if(managedList[i] != NULL){
            if((memoryBlockHeader*)managedList[i] < (memoryBlockHeader*)heap[2]){

                memoryBlockHeader* block = (memoryBlockHeader*)((unsigned char*)managedList[i] - sizeof(memoryBlockHeader));
                printf("Managed block at %p, size: %d, survival: %d\n", block, block->size, block->survivalAmt);

            if(block->survivalAmt<2){
                printf("block moving to young heap\n");
                
                memoryBlockHeader* newBlock = duMalloc(block->size,heapIndex);
                
                memcpy(newBlock, block, block->size);

                memoryBlockHeader* newBlockHeader = (memoryBlockHeader*)((unsigned char*)newBlock - sizeof(memoryBlockHeader));
                newBlockHeader->managedIndex = block->managedIndex;
                newBlockHeader->survivalAmt = block->survivalAmt + 1;

                managedList[i] = (unsigned char*)newBlock;

                
                
                printf("New block at %p, size: %d, survival: %d\n", newBlock, newBlock->size, newBlockHeader->survivalAmt);

            }else{
                printf("block moving to old heap\n");
                memoryBlockHeader* newBlock = duMalloc(block->size,2);
                
                memcpy(newBlock, block, block->size);

                memoryBlockHeader* newBlockHeader = (memoryBlockHeader*)((unsigned char*)newBlock - sizeof(memoryBlockHeader));
                newBlockHeader->managedIndex = block->managedIndex;
                newBlockHeader->survivalAmt = block->survivalAmt + 1;

                managedList[i] = (unsigned char*)newBlock;

                
                
                
                printf("New block at %p, size: %d, survival: %d\n", newBlock, newBlock->size, newBlockHeader->survivalAmt);

            }
            
            


            }
            
        }
    }

}


void majorCollection(){
    printf("Major Collection\n");
    printf("Old heap\n");
    printOldHeap();


    memoryBlockHeader* firstFreeBlock = NULL;
    memoryBlockHeader* currentBlock = (memoryBlockHeader*)oldHeap;
    while((unsigned char*)currentBlock < oldHeap + HEAP_SIZE){
        if(currentBlock->free == 1){
            firstFreeBlock = currentBlock;
            break;
        }
        currentBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + currentBlock->size + sizeof(memoryBlockHeader));
    }

    printf("made it this far 1\n");

    if(firstFreeBlock == NULL){
        printf("No free blocks in old heap\n");
        return;
    }

    printf("made it this far 2\n");

    currentBlock = firstFreeBlock;
    while((unsigned char*)currentBlock < oldHeap + HEAP_SIZE){
        if(currentBlock == firstFreeBlock){
            printf("first free block\n");
        }
        else if(currentBlock->free == 0){
            printf("Used block\n");
            //switch this block with the first free block
            firstFreeBlock->size = firstFreeBlock->size + currentBlock->size + sizeof(memoryBlockHeader);
            firstFreeBlock->next = currentBlock->next;
            currentBlock->size = 0;
            currentBlock->free = 1;
            currentBlock->next = firstFreeBlock->next;
            currentBlock = firstFreeBlock;
            firstFreeBlock = (memoryBlockHeader*)((unsigned char*)firstFreeBlock + firstFreeBlock->size + sizeof(memoryBlockHeader));
            


            
        } else {
            printf("Free block\n");
            //combine this block with the first free block
            firstFreeBlock->size = firstFreeBlock->size + currentBlock->size + sizeof(memoryBlockHeader);
            firstFreeBlock->next = currentBlock->next;
            currentBlock->size = 0;
            currentBlock->free = 1;
            currentBlock->next = firstFreeBlock->next;
            currentBlock = firstFreeBlock;
            firstFreeBlock = (memoryBlockHeader*)((unsigned char*)firstFreeBlock + firstFreeBlock->size + sizeof(memoryBlockHeader));
            

        }
        currentBlock = (memoryBlockHeader*)((unsigned char*)currentBlock + currentBlock->size + sizeof(memoryBlockHeader));
    
    }
    

}