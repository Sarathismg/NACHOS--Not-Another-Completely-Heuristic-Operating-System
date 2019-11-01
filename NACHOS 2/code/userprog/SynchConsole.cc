//
// Created by snigdha on 12/29/17.
//

#include "SynchConsole.h"
#include "system.h"

static void
ReadAvailThrow (void* arg){
    SynchConsole* s = (SynchConsole*)arg;
    s->ReadAvail();
}

static void WriteDoneThrow(void* arg){
    SynchConsole* s = (SynchConsole*)arg;
    s->WriteDone();
}


void SynchConsole::ReadAvail() { readAvail->V(); }
void SynchConsole::WriteDone() { writeDone->V(); }


SynchConsole::SynchConsole() {
    readAvail = new Semaphore("synch console read", 0);
    writeDone = new Semaphore("synch console write",0);
    lock = new Lock("synch console lock");
    console = new Console(NULL,NULL,ReadAvailThrow,WriteDoneThrow,this);

}

SynchConsole::~SynchConsole() {
    delete readAvail;
    delete writeDone;
    delete console;
    delete lock;
}

int SynchConsole::ReadFromConsoleV(int dataVaddr, int size) {
    lock->Acquire();
    char ch;
    int k;
    for (k=0;k<size;k++) {
        if(k==0) {
            readAvail->P();
        }// wait for character to arrive

        ch = console->GetChar();
        if(ch == EOF)
            break;
        else if(k != 0)
            readAvail->P(); // As the interrupt handler will be called even though semaphore wasn't decreased earlier

        bool isvalid=machine->WriteMem(dataVaddr+k,1,ch);
        if(!isvalid) {
            lock->Release();
            printf("Couldn't be written to memory ..\n");
            return -1;
        }
        //console->PutChar(ch);	// echo it!
        //writeDone->P() ;        // wait for write to finish
        //if (ch == 'q') return;  // if q, quit

    }
    lock->Release();
    return k;
}

void SynchConsole::WriteToConsoleV(int dataVaddr, int size) {
	int tmposize = size;
    char ch[tmposize];
    int k;
    for(k=0;k<tmposize;k++) {
        bool isValid = machine->ReadMem(dataVaddr + k, 1, (int *) &ch[k]);
        if (!isValid){
            printf("Couldn't be read\n");
            return;
    }
    }
    lock->Acquire();
    for(k=0;k<tmposize;k++){
        console->PutChar(ch[k]);
        writeDone->P();
    }
    lock->Release();
}

