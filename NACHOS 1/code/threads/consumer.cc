//
// Created by snigdha on 12/8/17.
//

#include "consumer.h"
#include "synch.h"


extern int* sharedBuffer;
extern Lock* mutualLock;
extern Condition* mutualCondition;


consumer::consumer(int idd,int limit, int perLimit) {
    id = idd;
    perConsumerLimit = perLimit;
    tableLimit = limit;

    DEBUG('t', "Entering Consumer");

    Thread *t = new Thread("forked thread");

    t->Fork(consume, this);
}

void consumer::consume(void* pointer) {
    int iter,i,j,k;
    consumer* consum= (consumer*)pointer;

    for(iter=0;iter<consum->perConsumerLimit;iter++){
        mutualLock->Acquire();
        while(*sharedBuffer==0){
            mutualCondition->Wait(mutualLock);
        }

        *sharedBuffer = *sharedBuffer-1;
        printf("Consumer %d has consumed %d th food--currently in table %d\n",consum->id,iter,*sharedBuffer);
        if(*sharedBuffer == consum->tableLimit -1){
            mutualCondition->Broadcast(mutualLock);
        }

        for(i=0;i<16635;i++){

        }


        mutualLock->Release();

        for(i=0;i<12869;i++){

        }

    }

    printf("Finishing Consumer %d\n\n",consum->id);
}

consumer::~consumer() {

}
