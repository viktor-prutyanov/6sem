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
#include <cerrno>
#include "BigUInt.h"
#include <mpi.h>

unsigned long int get_ulong_opt(char *optarg, const char *name)
{
    char *endptr;
    errno = 0;
                
    unsigned long int arg = strtoul(optarg, &endptr, 10);

    if (errno == ERANGE)
        fprintf(stderr, "Args error: %s is out of range.\n", name);
    else if (*endptr != '\0')
    {
        fprintf(stderr, "Args error: invalid %s.\n", name);
        errno = EINVAL;
    }
            
    return arg;
}

int main(int argc, char * argv[])
{  
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank );

    uint32_t num = get_ulong_opt(argv[1], "number");

    BigUInt f(1);

    for (uint32_t i = rank + 1; i <= num; i += size)
    {
        f *= i;
    }

    f.Print("\n");
/*
    if (rank == 0)
    {
        printf("%d %d\n", sizeof(unsigned long int), sizeof(unsigned int));
        BigUInt f(1);
        for (uint32_t i = 1; i <= 10; ++i)
            f *= i;
        BigUInt s = f * f;
        s.Print("\n");
    }
*/
    MPI_Finalize();
    return 0;
}
