//
// Created by snigdha on 12/27/17.
//

#include "memoryManager.h"
#include "../machine/machine.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


MemoryManager::MemoryManager(int numPages) {
    numPhysMemoryPages = numPages;
    srand (time(NULL));
    bitmap = new BitMap(numPhysMemoryPages);
    memMngLock = new Lock("Memory Manager Lock Universal");
    processMap = new int[numPages];

    entries = new TranslationEntry*[numPages];

    for(int k=0;k<numPages;k++){
        processMap[k]=-1;
        entries[k]=NULL;
    }

}

int MemoryManager::AllocPage() {
    memMngLock->Acquire();
    int retval = bitmap->Find();
    memMngLock->Release();
    return retval;
}

void MemoryManager::FreePage(int physPageNum) {
    memMngLock->Acquire();
    bitmap->Clear(physPageNum);
    memMngLock->Release();
}

bool MemoryManager::PageIsAllocated(int physPageNum) {
    memMngLock->Acquire();
    bool res =  bitmap->Test(physPageNum);
    memMngLock->Release();
    return res;

}

int MemoryManager::Alloc(unsigned int& processNo, TranslationEntry& entry) {
    memMngLock->Acquire();
    int retval = bitmap->Find();
    if(retval != -1){
        processMap[retval]=processNo;
        entries[retval] = &entry;
    }
    //printf("inside retval of memory man %d\n",retval);
    memMngLock->Release();
    return retval;
}

int MemoryManager::AllocByForce() {
    int ind;

    //for random replacement or LRU replacement

    memMngLock->Acquire();
    //ind = rand()%numPhysMemoryPages;
    ind = getLRUvalue();
    //printf("MISS in paging %d\n",ind);
    /*                   */
    memMngLock->Release();
    return ind;
}

int MemoryManager::getProcessNo(int pageno) {
    return processMap[pageno];
    return 0;
}

TranslationEntry* MemoryManager::getTranslationEntry(int pageno) {
    return entries[pageno];
}

void MemoryManager::setTranslationEntry(int& pageno, TranslationEntry& entry) {
    memMngLock->Acquire();
    entries[pageno]=&entry;
    memMngLock->Release();

}

void MemoryManager::setProcessNo(int &pageno, int processNo) {
    memMngLock->Acquire();
    processMap[pageno]=processNo;
    memMngLock->Release();
}

int MemoryManager::getLRUvalue() {
    int lowestval,lowestindex;
    lowestval = entries[0]->accessTime;
    lowestindex = 0;
    for(int k=0;k<numPhysMemoryPages;k++){
        if(entries[k]->accessTime < lowestval){
            //update lowest val and index
            lowestval = entries[k]->accessTime;
            lowestindex = k;
        }
    }
    return lowestindex;
}
