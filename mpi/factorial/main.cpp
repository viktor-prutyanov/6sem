/**
*   Parallel factorial computation with big arithmetics
*   
*   @file main.cpp
*
*   @date 02.2016
*   
*   @copyright GNU GPL v2.0
*
*   @author Viktor Prutyanov mailto:viktor.prutyanov@phystech.edu 
*/

#include <cstdio>
#include "BigUInt.h"

int main(int argc, char * argv[])
{   
    BigUInt c(1);
    for (uint32_t i = 1; i <= 100; ++i)
        c *= i;
    c.Print("\n");
    return 0;
}
