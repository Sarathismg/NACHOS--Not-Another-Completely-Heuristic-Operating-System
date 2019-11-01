// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "producer.h"
#include "consumer.h"
//#include "producer.h"
//#include "consumer.h"

//#include "producer.cc"
//#include "consumer.cc"
#define NO_OF_PRODUCERS 5
#define NO_OF_CONSUMERS 10
#define PER_CONSUMER_LIMIT 5
#define PER_PRODUCER_LIMIT 10
#define TABLE_LIMIT 3

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

Lock* mutualLock;
Condition* mutualCondition;
int* sharedBuffer=new int;


void SimpleThread(int which)
{
    /*int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }*/
    int i;

    for(i=0;i<NO_OF_PRODUCERS;i++){
        new producer(i,TABLE_LIMIT,PER_PRODUCER_LIMIT);;
    }


    for(i=0;i<NO_OF_CONSUMERS;i++){
        new consumer(i,TABLE_LIMIT,PER_CONSUMER_LIMIT);
    }
    //printf("getting out");
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    int i,j,k;
    int SharedBufferSpace=0;
    *sharedBuffer = 0;
    mutualCondition = new Condition("Prodcer_consumer_condition");
    mutualLock =new Lock("Producer_Consumer_lock");
    DEBUG('t', "Entering SimpleTest");;

    Thread *t = new Thread("forked thread");
    int p = 2; //dummy
    t->Fork((VoidFunctionPtr)SimpleThread, (void*)p);;
    //SimpleThread(0);
}


