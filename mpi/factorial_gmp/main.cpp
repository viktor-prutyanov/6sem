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
#include <mpi.h>
#include <string>
#include <gmpxx.h> 
#include <iostream>
#include <vector>

#define SET_BIT(p,n) ((p) | (1 << (n)))
#define CLEAR_BIT(p,n) ((p) & (~(1 << (n))))
#define CHECK_BIT(p,n) (((p) >> (n)) & 1)

bool isSender(int rank, int size, unsigned long level)
{
    if (CHECK_BIT(rank, level))
    {
        for (unsigned i = 0; i < level; ++i)
            if (CHECK_BIT(rank, i))
                return false;
    }
    else
        return false;

    return true;
}

bool isReceiver(int rank, int size, unsigned int level)
{
    if (!CHECK_BIT(rank, level))
    {
        for (unsigned i = 0; i < level; ++i)
            if (CHECK_BIT(rank, i))
                return false;
    }
    else
        return false;

    return ((rank + (1 << level)) < size) && true;
}

unsigned long int get_ulong_opt(char *optarg)
{
    char *endptr;
    errno = 0;
                
    unsigned long int arg = strtoul(optarg, &endptr, 10);

    if (*endptr != '\0')
        errno = EINVAL;
   
    return arg;
}

mpz_class treeMultiply(unsigned int start, unsigned int end)
{
    if (start > end)
        return 1;
    else if (start == end)
        return start;
    else if (end - start == 1)
        return (mpz_class)end * start;
    int middle = (start + end) / 2;
    return treeMultiply(start, middle) * treeMultiply(middle + 1, end);
}

int main(int argc, char *argv[])
{  
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank );

    if (argc != 2)
    {
        if (rank == 0)
            fprintf(stderr, "usage: %s [number]\n", argv[0]);
        MPI_Finalize();
        return -1;
    }

    uint32_t num = get_ulong_opt(argv[1]);
    if (errno)
    {
        if (rank == 0)
            perror(NULL);
        MPI_Finalize();
        return -1;
    }
    else if (num < size)
    {
        if (rank == 0)
            fprintf(stderr, "Number is too small\n"); 
        MPI_Finalize();
        return -1;
    }

    unsigned int start = 0;
    unsigned int end = 0;
    int remain_num = num;
    for (int i = 0; i <= rank; ++i)
    {
        start = end + 1;
        end = start + remain_num / (size - i) - 1;
        remain_num -= remain_num / (size - i);
    }

    fprintf(stderr, "%d : %u;%u\n", rank, start, end);

    mpz_class a = treeMultiply(start, end);

    std::vector<char> sendBuf;
    std::vector<char> recvBuf;

    unsigned int level = 0;
    unsigned int distance = 0;
    MPI_Status status;
    while (distance * 2 < size)
    {
        distance = 1 << level;
        if (isSender(rank, size, level))
        {
            double time = MPI_Wtime();
            size_t sendBufSize = (mpz_sizeinbase(a.get_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
            sendBuf.resize(sendBufSize);
            mpz_export(&sendBuf[0], &sendBufSize, 1, 1, 0, 0, a.get_mpz_t());
            sendBuf.resize(sendBufSize);
            MPI_Send(&sendBufSize, 1, MPI_UNSIGNED_LONG, rank - distance, 0, MPI_COMM_WORLD);
            MPI_Send(&sendBuf[0], sendBufSize, MPI_CHAR, rank - distance, 0, MPI_COMM_WORLD);
            time = MPI_Wtime() - time;
            printf("%d : sent len = %d to %d time = %lg\n", rank, sendBufSize, CLEAR_BIT(rank, level), time);
        }
        else if (isReceiver(rank, size, level))
        {
            double time1 = MPI_Wtime();
            size_t recvBufSize;
            MPI_Recv(&recvBufSize, 1, MPI_UNSIGNED_LONG, rank + distance, 0, MPI_COMM_WORLD, &status);
            recvBuf.resize(recvBufSize);
            MPI_Recv(&recvBuf[0], recvBufSize, MPI_CHAR, rank + distance, 0, MPI_COMM_WORLD, &status);
            mpz_class b;
            mpz_import(b.get_mpz_t(), recvBufSize, 1, 1, 0, 0, &recvBuf[0]);
            time1 = MPI_Wtime() - time1;
            double time2 = MPI_Wtime();
            a = a * b;
            time2 = MPI_Wtime() - time2;
            printf("%d : received len = %d from %d time1 = %lg time2 = %lg\n", rank, recvBufSize, SET_BIT(rank, level), time1, time2);
        }
        ++level;
    }

    if (rank == 0)
    {
        FILE *out_file = fopen("answer.txt", "wb");
        gmp_fprintf(out_file, "%Zx\n", a.get_mpz_t(), 8);
        fclose(out_file);
    }

    MPI_Finalize();
    return 0;
}
