//
// Created by snigdha on 12/28/17.
//

#include "Table.h"


Table::Table(int size) {
    bitmap = new BitMap(size);
    tableLock = new Lock("process table lock");
    no_of_process = 0;
    processArray= new AddrSpace*[size];
    Size = size;
}

Table::~Table(){

    for(int k=0;k<Size;k++){
        if(this->Get(k)!=NULL)
            this->Release(k);
    }
    delete bitmap;
    delete tableLock;
    delete processArray;
}

int Table::Alloc(void *object) {
    tableLock->Acquire();
    int retval = bitmap->Find();
    if(retval!=-1){
        processArray[retval]=(AddrSpace*)object;
    }
    tableLock->Release();

    return retval;
}

void *Table::Get(int index) {
    tableLock->Acquire();
    bool a=bitmap->Test(index);
    if(!a)
        return NULL;
    AddrSpace* ret = processArray[index];
    tableLock->Release();
    return ret;
}

void Table::Release(int index) {
    tableLock->Acquire();
    bitmap->Clear(index);
    delete processArray[index];
    tableLock->Release();
}
