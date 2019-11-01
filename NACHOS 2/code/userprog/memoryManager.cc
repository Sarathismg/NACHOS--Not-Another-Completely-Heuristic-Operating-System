//
// Created by snigdha on 12/27/17.
//

#include "memoryManager.h"
#include "../machine/machine.h"

MemoryManager::MemoryManager(int numPages) {
    numPhysMemoryPages = numPages;
    bitmap = new BitMap(numPhysMemoryPages);
    memMngLock = new Lock("Memory Manager Lock Universal");

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
