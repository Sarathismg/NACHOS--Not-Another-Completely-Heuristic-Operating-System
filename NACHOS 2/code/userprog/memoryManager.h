//
// Created by snigdha on 12/27/17.
//

#ifndef CODE_MEMORYMANAGER_H
#define CODE_MEMORYMANAGER_H

#include "bitmap.h"
#include "../threads/synch.h"

class MemoryManager{
private:
    BitMap* bitmap;
    int numPhysMemoryPages;
    Lock* memMngLock;
public:
    /* Create a manager to track the allocation of numPages of physical memory.
   You will create one by calling the constructor with NumPhysPages as
   the parameter.  All physical pages start as free, unallocated pages. */
    MemoryManager(int numPages);

/* Allocate a free page, returning its physical page number or -1
   if there are no free pages available. */
    int AllocPage();

/* Free the physical page and make it available for future allocation. */
    void FreePage(int physPageNum);

/* True if the physical page is allocated, false otherwise. */
    bool PageIsAllocated(int physPageNum);


};

#endif //CODE_MEMORYMANAGER_H
