// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"
#include "machine.h"
#include "../threads/system.h"
#include "../machine/translate.h"
#include "../machine/stats.h"

void ProcessStart(void* adrspace){
    /*printf("New thread starts\n");*/
    AddrSpace* space = (AddrSpace*) adrspace;
    currentThread->space = space;
    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    machine->Run();
}

void ExitProcess(int status){
    //printf("abc");
    printf("Process is exiting with exit code %d;\n--Page faults till now: %d \n",status,stats->numPageFaults);
    //printf("def");
    processTable->Release(currentThread->space->processId);
    //printf("ghi");
    currentThread->Finish();
    //printf("jkl");
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
    else if((which == SyscallException) && (type == SC_Exec)){
        DEBUG('a', "Execute , initiated by user program.\n");
        int nameptrVaddrs = machine->ReadRegister(4);
        //printf("nametsf: %d\n",nameptrVaddrs);
        bool another ;
        bool isInsideLimit = false;
        char* filename = new char[100];//some extra to be safe

        //Taken that reasonable namesize is 100
        for(int k=0;k<100;k++){
            int temp;
            bool SuccessRead=machine->ReadMem(nameptrVaddrs,1,&temp);
            if(!SuccessRead){
                SuccessRead = machine->ReadMem(nameptrVaddrs,1,&temp);
                if(!SuccessRead){
                    //printf("Error in reading Frm Mem %s\n",filename);
                    /*if(!another){
                        another = true;
                        k--;
                        continue;
                    }*/
                    machine->WriteRegister(2,-1);
                    int pcAfter = machine->registers[NextPCReg] + 4;
                    machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
                    // are jumping into lala-land
                    machine->registers[PCReg] = machine->registers[NextPCReg];
                    machine->registers[NextPCReg] = pcAfter;
                    return;
                }
            }

            filename[k]=temp;
            if( filename[k]== '\0'){
                isInsideLimit = true;
                break;
            }
            nameptrVaddrs++;
        }

        if((isInsideLimit)==false){
            printf("Out of limit\n");
            printf("%s",filename);
            machine->WriteRegister(2,-1);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;
        }
        //printf("File name is %s",filename);
        OpenFile *executable = fileSystem->Open(filename);
        AddrSpace *space;

            if (executable == NULL) {
            //printf("Unable to open file %s\n", filename);
            machine->WriteRegister(2,-2);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;

        }
        space = new AddrSpace();
        space = space->Initialize(executable);
        if(space == NULL){
            printf("memory allocation Error, not enough Space available\n");
            machine->WriteRegister(2,-3);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            delete(space);
            return;
        }

        int givenindex = processTable->Alloc(space);
        if(givenindex == -1){
            //printf("Maximum number of processes Reached.Process couldn't be started..\n");
            machine->WriteRegister(2,-4);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            delete(space);
            return;
        }

        space->processId=givenindex;
        Thread *t = new Thread(filename);
        t->Fork(ProcessStart,space);
        //currentThread->space = space;
        //delete executable;			// close file
        machine->WriteRegister(2,givenindex);
        int pcAfter = machine->registers[NextPCReg] + 4;
        machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
        // are jumping into lala-land
        machine->registers[PCReg] = machine->registers[NextPCReg];
        machine->registers[NextPCReg] = pcAfter;
        return;

    }
    else if((which == SyscallException) && (type == SC_Exit)) {
        int status = machine->ReadRegister(4);
        ExitProcess(status);
        printf("mno");
    }

    else if((which == SyscallException) && (type == SC_Read)){
        int buf = machine->ReadRegister(4);
        int siz = machine->ReadRegister(5);
        OpenFileId filid = machine->ReadRegister(6);
        if(filid == ConsoleInput){
            int actuallyRead=globalConsole->ReadFromConsoleV(buf,siz);
            machine->WriteRegister(2,actuallyRead);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;
        }
        else{
            printf("Unknown flieId\n.Request couldn't be served\n");
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;
        }
    }

    else if((which == SyscallException) && (type == SC_Write)){
        int buf = machine->ReadRegister(4);
        int siz = machine->ReadRegister(5);
        OpenFileId filid = machine->ReadRegister(6);
        if(filid == ConsoleOutput){
            globalConsole->WriteToConsoleV(buf,siz);
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;
        }
        else{
            printf("Unknown flieId\n.Request couldn't be served\n");
            int pcAfter = machine->registers[NextPCReg] + 4;
            machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
            // are jumping into lala-land
            machine->registers[PCReg] = machine->registers[NextPCReg];
            machine->registers[NextPCReg] = pcAfter;
            return;
        }
    }

    else if (which == PageFaultException)  {
        stats->numPageFaults++;
        int addr = machine->ReadRegister(39);
        int vpn = addr / PageSize;
        int physicalPage= memoryManager->Alloc(currentThread->space->processId,machine->pageTable[vpn]);
        if (physicalPage == -1){
            //force free
            //printf("Forced\n");
            //temporarily updating
            physicalPage = memoryManager->AllocByForce();
            TranslationEntry* e=memoryManager->getTranslationEntry(physicalPage);
            int procno = memoryManager->getProcessNo(physicalPage);
            //get addrspace of evicted page
            AddrSpace* obj = (AddrSpace*)processTable->Get(procno);

            if(e->dirty || !(obj->isSwapPageExists(e->virtualPage)))
                obj->saveIntoSwapSpace(e->virtualPage);


            e->valid=false;
            e->physicalPage=-1;
            e->valid = false;
            e->use = false;
            e->dirty = false;
            e->readOnly = false;
        }

        currentThread->space->loadIntoFreePage(addr, physicalPage);
        memoryManager->setTranslationEntry(physicalPage,machine->pageTable[vpn]);
        memoryManager->setProcessNo(physicalPage,currentThread->space->processId);
        //ExitProcess(-1);
    }

    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(false);
    }
}
