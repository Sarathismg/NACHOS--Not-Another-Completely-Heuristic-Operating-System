/*
 * exectest.c
 *
 * Exec a simple program.  Return the result of Exec.
 */

#include "syscall.h"

int
main()
{
    int b,result = 1000;
    result = Exec("exittest");
    //b = Exec("array");
    //result = b+result;
    Exit(result);
}
