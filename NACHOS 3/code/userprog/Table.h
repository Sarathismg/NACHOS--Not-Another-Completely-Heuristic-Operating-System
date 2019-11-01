//
// Created by snigdha on 12/28/17.
//

#ifndef INC__TABLE_H
#define INC__TABLE_H

#include "bitmap.h"
#include "../threads/synch.h"
#include "addrspace.h"


class Table {
private:
    BitMap* bitmap;
    int Size;
    unsigned int no_of_process;
    AddrSpace** processArray;
    Lock* tableLock;
public:

    /* Create a table to hold at most "size" entries. */
    Table(int size);
    ~Table();

/* Allocate a table slot for "object", returning the "index" of the
   allocated entry; otherwise, return -1 if no free slots are available. */
    int Alloc(void *object);

/* Retrieve the object from table slot at "index", or NULL if that
   slot has not been allocated. */
    void *Get(int index);

/* Free the table slot at index. */
    void Release(int index);
};


#endif //INC_1405021_3_TABLE_H
