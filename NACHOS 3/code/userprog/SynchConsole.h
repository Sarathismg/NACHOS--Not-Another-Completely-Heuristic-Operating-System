//
// Created by snigdha on 12/29/17.
//

#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "synch.h"
#include "console.h"


class SynchConsole {
public:
    SynchConsole();    	// Initialize a synchronous console,
    // by initializing the raw Disk.
    ~SynchConsole();

    int ReadFromConsoleV(int data, int size);
    void WriteToConsoleV(const int data, int size);
    void ReadAvail();
    void WriteDone();

private:
    Console *console;
    Semaphore *readAvail;
    Semaphore *writeDone;
    Lock* lock;


};


#endif //SYNCHCONSOLE_H
