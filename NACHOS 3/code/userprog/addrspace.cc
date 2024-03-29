// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "../machine/machine.h"
#include "../threads/utility.h"
#include "../threads/system.h"
#include "../machine/stats.h"
//#include "../bin/noff.h"


//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace(Changed to Initialize)
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace() {
    acquiredPages = 0;
}

AddrSpace* AddrSpace::Initialize(OpenFile *executabl)
{
    //NoffHeader noffH;
    //noff = &noffH;
    unsigned int i, size;
    this->executable = executabl;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    swapPageNo= new int[numPages];

    for(int it=0;it<numPages;it++){
        swapPageNo[it]=-1;  //reset all swapPage Mapping
    }

    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	//pageTable[i].physicalPage = memoryManager->AllocPage();
        pageTable[i].physicalPage = -1;
    /*if(pageTable[i].physicalPage == -1) {
        //memory release will be done by destructor
        return NULL;
    }*/
    acquiredPages++;
	pageTable[i].valid = false;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    pageTable[i].accessTime = 0; // for giving a start value
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    //bzero(machine->mainMemory, size);
    /*for(i = 0;i< numPages;i++){
        //&machine->mainMemory[3 * PageSize]
        bzero(&(machine->mainMemory[pageTable[i].physicalPage * PageSize]),PageSize);
    }*/


// then, copy in the code and data segments into memory
    /*if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }*/

    //changed copying data and code segment
    //extremely messy section for avoiding issues with page boundary, in code segment

    /*if(noffH.code.size > 0){
        int sizeRemaining = noffH.code.size;
        int fileoff = noffH.code.inFileAddr; //start point
        int virt_addr = noffH.code.virtualAddr;
        int vpn = virt_addr / PageSize;
        int ppn = pageTable[vpn].physicalPage;
        if(virt_addr%PageSize != 0){
            int startoff = virt_addr%PageSize;
            int rem = PageSize-virt_addr%PageSize;
            //machine->WriteMem(fileoff,1,&())
            int physAddr = ppn * PageSize + startoff;
            executable->ReadAt(&(machine->mainMemory[physAddr]),rem, fileoff);
            fileoff = fileoff + rem;
            sizeRemaining=sizeRemaining-rem;
            virt_addr += rem;
        }

        while(sizeRemaining>=PageSize){
            vpn = virt_addr/PageSize;
            ppn = pageTable[vpn].physicalPage;
            int physAddr = ppn*PageSize;
            executable->ReadAt(&(machine->mainMemory[physAddr]),PageSize,fileoff);
            virt_addr+=PageSize;
            sizeRemaining-=PageSize;
            fileoff+=PageSize;
        }

        if(sizeRemaining != 0){
            vpn = virt_addr/PageSize;
            ppn = pageTable[vpn].physicalPage;
            int physAddr = ppn*PageSize;
            executable->ReadAt(&(machine->mainMemory[physAddr]),sizeRemaining,fileoff);
        }
    }



    //messy section for avoiding issues with page boundary in data segment.

    if(noffH.initData.size > 0){
        int sizeRemaining = noffH.initData.size;
        int fileoff = noffH.initData.inFileAddr; //start point
        int virt_addr = noffH.initData.virtualAddr;
        int vpn = virt_addr / PageSize;
        int ppn = pageTable[vpn].physicalPage;
        if(virt_addr%PageSize != 0){
            int startoff = virt_addr%PageSize;
            int rem = PageSize-virt_addr%PageSize;
            //machine->WriteMem(fileoff,1,&())
            int physAddr = ppn * PageSize + startoff;
            executable->ReadAt(&(machine->mainMemory[physAddr]),rem, fileoff);
            fileoff = fileoff + rem;
            sizeRemaining=sizeRemaining-rem;
            virt_addr += rem;
        }

        while(sizeRemaining>=PageSize){
            vpn = virt_addr/PageSize;
            ppn = pageTable[vpn].physicalPage;
            int physAddr = ppn*PageSize;
            executable->ReadAt(&(machine->mainMemory[physAddr]),PageSize,fileoff);
            virt_addr+=PageSize;
            sizeRemaining-=PageSize;
            fileoff+=PageSize;
        }

        if(sizeRemaining != 0){
            vpn = virt_addr/PageSize;
            ppn = pageTable[vpn].physicalPage;
            int physAddr = ppn*PageSize;
            executable->ReadAt(&(machine->mainMemory[physAddr]),sizeRemaining,fileoff);
        }
    }*/

    //end of tough precise code
    return this;


}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   for(int k=0;k<numPages;k++){
       if(pageTable[k].valid) {
           memoryManager->FreePage(pageTable[k].physicalPage);
       }
        if(swapPageNo[k]!=-1) {
            swapMemoryManager->FreePage(swapPageNo[k]);
        }
   }
   delete pageTable;
   delete executable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}


int AddrSpace::loadIntoFreePage(int addr, int physicalPageNo) {
    int codeVbase = noffH.code.virtualAddr;
    int codeVend = codeVbase+noffH.code.size - 1;

    int initVbase = noffH.initData.virtualAddr;
    int initVend = initVbase+noffH.initData.size - 1;

    int uninitVbase = noffH.uninitData.virtualAddr;
    int uninitVend = uninitVbase+noffH.uninitData.size -1;

    //printf("code: %d %d, init: %d %d, uninit: %d %d\n",codeVbase,codeVend,initVbase,initVend,uninitVbase,uninitVend);

    pageTable[addr/PageSize].valid=true;
    pageTable[addr/PageSize].physicalPage=physicalPageNo;
    pageTable[addr/PageSize].accessTime = stats->totalTicks;

    //Check if exists in swapspace. If yes then get the content from there. Otherwise get it from file.
    if(isSwapPageExists(addr/PageSize)){
        //printf("Found in swap space\n");
        loadFromSwapSpace(addr/PageSize);
        return 0;
    }

    //else case
    addr/=PageSize;
    addr*=PageSize;


    int pivot = addr;
    int sizeRem = PageSize;
    int physAddr = physicalPageNo*PageSize;
    //printf("pivot %d\n",pivot);

    if(pivot >= codeVbase && pivot<=codeVend){
        //int vpn = addr/PageSize;
        int off = pivot - codeVbase;
        int d ;
        if(sizeRem < (codeVend-pivot+1))
            d = sizeRem;
        else
            d = (codeVend-pivot+1);
        int debuzg = executable->ReadAt(&(machine->mainMemory[physAddr]),d,noffH.code.inFileAddr+off);
        sizeRem = sizeRem - d;
        //printf("%d code sizeremm\n",off);
        physAddr+=d;
        pivot += d;
    }


    if(sizeRem > 0)
        if(pivot >= initVbase && pivot<=initVend){
            int vpn = addr/PageSize;
            pageTable[vpn].physicalPage=physicalPageNo;
            //pageTable[vpn].valid = true;
            int off = pivot - initVbase;
            int d ;
            if(sizeRem < (initVend-pivot+1))
                d = sizeRem;
            else
                d = (initVend-pivot+1);
            executable->ReadAt(&(machine->mainMemory[physAddr]),d,noffH.initData.inFileAddr+off);
            sizeRem = sizeRem - d;
            //printf("%d init sizeremm\n",off);
            physAddr+=d;
            pivot += d;
        }

    if(sizeRem > 0)
        bzero(&(machine->mainMemory[physAddr]),sizeRem);

    //saveIntoSwapSpace(addr/PageSize);
    return 0;
}

int AddrSpace::saveIntoSwapSpace(int vpn) {
    //Assumed that it is called when it is sure to save in swap space
    //Checking has to be done manually before getting in

    int index= swapPageNo[vpn];
    if(index == -1){ //not yet allocated
        //printf("Thrashed\n");

        index = swapMemoryManager->Alloc(this->processId,this->pageTable[vpn]);
        DEBUG('a', "Thrashed in process %d, allocating location of swap page no %d, called by Thread %s with no %d.\n",this->processId,index,currentThread->getName(),currentThread->space->processId);

        if(index == -1)
            return -1;
        swapPageNo[vpn]= index; //saving the page location

    }
    int physaddrs= (pageTable[vpn].physicalPage)*PageSize;
    int numb=index/30;
    index=index%30;
    int m=fileSystem->SwapFile[numb]->WriteAt(&(machine->mainMemory[physaddrs]),PageSize,index*PageSize);
    //printf("Wrote total %d bytes\n",m);

    return m;
}

int AddrSpace::loadFromSwapSpace(int vpn) {
    //Physical address must be set manually in the translationentry before entering here.
    //Otherwise it won't know which physicalPage it gets

    int physaddrs = (pageTable[vpn].physicalPage)*PageSize;
    int index = swapPageNo[vpn];
    if(index == -1)
        return -1;
    int numb=index/30;
    index=index%30;

    int retv = fileSystem->SwapFile[numb]->ReadAt(&(machine->mainMemory[physaddrs]),PageSize,index*PageSize);
    return retv;

}

bool AddrSpace::isSwapPageExists(int vpn) {
    if(swapPageNo[vpn]==-1)
        return false;
    else
        return true;
}
