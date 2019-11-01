//
// Created by snigdha on 12/8/17.
//

#include "producer.h"
#include <time.h>
#include <stdlib.h>
#include "synch.h"
//#include "threadtest.cc"


extern int* sharedBuffer;
extern Lock* mutualLock;
extern Condition* mutualCondition;


producer::producer(int idd, int limit,int perProdlim) {
    id = idd;
    //sharedBuffer= sharedBuff;
    //condition = cndtn;
    //tableLock = tbllck;
    tableLimit = limit;
    //printf("aaa %s is tihsjksdfjkls",tableLock->getName());

    perProducerLimit = perProdlim;
    DEBUG('t', "Entering Producer");

    Thread *t = new Thread("forked thread");
    //printf("%d ",this->sharedBuffer);
    t->Fork(produce, this);
}

void producer::produce(void* pointer) {

    int iter,i,j,k;
    producer* prod = (producer*) pointer;

    for(iter = 0;iter<prod->perProducerLimit;iter++){
        mutualLock->Acquire();
        while(*sharedBuffer==prod->tableLimit){
            mutualCondition->Wait(mutualLock);
        }

        *sharedBuffer=*sharedBuffer+1;
        printf("Producer %d has produced %d th food--currently in table %d\n",prod->id,iter,(*sharedBuffer));
        if(*(sharedBuffer) == 1)
            mutualCondition->Broadcast(mutualLock);

        for(i=0;i<16635;i++){
            i=i;
        }

        mutualLock->Release();

        for(i=0;i<12869;i++){
            i=i;
        }

    }
    printf("Finishing Producer %d\n\n",prod->id);

}

producer::~producer() {

}


