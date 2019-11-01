//
// Created by snigdha on 12/8/17.
//

#ifndef NACHOS_64BIT_3_CONSUMER_H
#define NACHOS_64BIT_3_CONSUMER_H


#include "synch.h"

class consumer {
private:
    int id;
    int perConsumerLimit;
    int tableLimit;

public:
    consumer(int idd,int limit,int perLimit);
    ~consumer();
    static void consume(void* pointer);

};


#endif //NACHOS_64BIT_3_CONSUMER_H
