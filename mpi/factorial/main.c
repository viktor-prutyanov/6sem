/**
*   Parallel factorial computation with big arithmetics
*   
*   @file main.c
*
*   @date 02.2016
*   
*   @copyright GNU GPL v2.0
*
*   @author Viktor Prutyanov mailto:viktor.prutyanov@phystech.edu 
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <mpi.h>
#include <bigMultiply.h>

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

    size_t ans_len;
    uint32_t *ans = treeMultiply(&ans_len, start, end);

    unsigned int level = 0;
    unsigned int distance = 0;
    MPI_Status status;
    while (distance * 2 < size)
    {
        distance = 1 << level;
        if (isSender(rank, size, level))
        {
            double time = MPI_Wtime();
            MPI_Send(&ans_len, 1, MPI_UNSIGNED_LONG, rank - distance, 0, MPI_COMM_WORLD);
            MPI_Send(ans, ans_len, MPI_UNSIGNED, rank - distance, 0, MPI_COMM_WORLD);
            time = MPI_Wtime() - time;
            
            printf("%d : sent len = %d to %d time = %lg\n", rank, ans_len, CLEAR_BIT(rank, level), time);
        }
        else if (isReceiver(rank, size, level))
        {
            double time1 = MPI_Wtime();
            size_t recvBufSize;
            MPI_Recv(&recvBufSize, 1, MPI_UNSIGNED_LONG, rank + distance, 0, MPI_COMM_WORLD, &status);
            uint32_t *recvBuf = (uint32_t *)malloc(recvBufSize * sizeof(uint32_t));
            MPI_Recv(recvBuf, recvBufSize, MPI_UNSIGNED, rank + distance, 0, MPI_COMM_WORLD, &status);
            time1 = MPI_Wtime() - time1;
            
            double time2 = MPI_Wtime();
            size_t prod_ans_len;
            uint32_t *prod_ans = bigMultiply(&prod_ans_len, ans, recvBuf, ans_len, recvBufSize);
            free(ans);
            ans_len = prod_ans_len;
            ans = prod_ans;
            time2 = MPI_Wtime() - time2;
            
            printf("%d : received len = %d from %d time1 = %lg time2 = %lg\n", rank, recvBufSize, SET_BIT(rank, level), time1, time2);
        }
        ++level;
    }

    if (rank == 0)
    {
        FILE *out_file = fopen("answer.txt", "wb");
        fdump(out_file, ans, ans_len);
        fclose(out_file);
    }

    free(ans);

    MPI_Finalize();
    return 0;
}
