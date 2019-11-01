//
// Created by snigdha on 12/8/17.
//

#ifndef NACHOS_64BIT_3_PRODUCER_H
#define NACHOS_64BIT_3_PRODUCER_H


#include "synch.h"

class producer {


private:
    int id;
    //int *sharedBuffer;
    //Condition* condition;
    //Lock* tableLock;
    int tableLimit;
    int perProducerLimit;
public:
    producer(int idd,int limit,int perprodlim);

    ~producer();
    static void produce(void* pointer);




};


#endif //NACHOS_64BIT_3_PRODUCER_H
