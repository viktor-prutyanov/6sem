#include <stdio.h>
#include <mpi.h>
#include <gmpxx.h>
#include <cstdio>
#include <vector>
#include <numeric>
#include <iostream>

uint64_t initial_factorial(int n)
{
	uint64_t fact = 1;
	for (int i = 2; i <= n; i++)
		fact *= i;

	return fact;
}

int main(int argc, char *argv[])
{
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc != 3)
    {
        if (rank == 0)
            std::cerr << "usage: " << argv[0] << " [number of summands] [number of digits]\n";
    	return -1;
    }

    int n = atoi(argv[1]);
    int precision = atoi(argv[2]);
    int len = 0;

    if (n % size == 0)
    	len = n / size;
    else
    {
        if (rank == 0)
            std::cerr << "Number of summands must be a size divisor.\n";
    	return -1;
    }

    std::vector<mpq_class> v(len);

    v[0].get_num() = 1;
    v[0].get_den() = initial_factorial(rank + 1);
    
    for (int i = 1; i < len; i++)
    {
    	v[i] = v[i - 1];
    	for (int j = 2 + rank + size * (i - 1); j <= size * i + rank + 1; j++)
    	{	
    		v[i] /= j;
    	}
    }

    mpq_class total = std::accumulate(v.begin(), v.end(), 0_mpq);

    if (rank == 0)
    {
        size_t recvBufSize = 0;
        std::vector<char> recvBuf;
        MPI_Status status;
        mpq_class subtotal;
    	for (int i = 1; i < size; i++)
    	{
            MPI_Recv(&recvBufSize, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, &status);
            recvBuf.resize(recvBufSize);
            MPI_Recv(&recvBuf[0], recvBufSize, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
            mpz_import(subtotal.get_num_mpz_t(), recvBufSize, 1, 1, 0, 0, &recvBuf[0]);
            
            MPI_Recv(&recvBufSize, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, &status);
            recvBuf.resize(recvBufSize);
            MPI_Recv(&recvBuf[0], recvBufSize, MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
            mpz_import(subtotal.get_den_mpz_t(), recvBufSize, 1, 1, 0, 0, &recvBuf[0]);
            
            total += subtotal;
    	}
    	
    }
    else
    {
        size_t sendBufSize = 0;
        std::vector<char> sendBuf;
        
        sendBufSize = (mpz_sizeinbase(total.get_num_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        sendBuf.resize(sendBufSize);
        mpz_export(&sendBuf[0], &sendBufSize, 1, 1, 0, 0, total.get_num_mpz_t());
        sendBuf.resize(sendBufSize);
        MPI_Send(&sendBufSize, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&sendBuf[0], sendBufSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

        sendBufSize = (mpz_sizeinbase(total.get_den_mpz_t(), 2) + CHAR_BIT - 1) / CHAR_BIT;
        sendBuf.resize(sendBufSize);
        mpz_export(&sendBuf[0], &sendBufSize, 1, 1, 0, 0, total.get_den_mpz_t());
        sendBuf.resize(sendBufSize);
        MPI_Send(&sendBufSize, 1, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&sendBuf[0], sendBufSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}

    mpf_class f(0, precision * 4); //это подгон, чтобы точность соответствовала количеству знаков после запятой
    f = 1 + total;

    if (rank == 0)
        gmp_printf("%.*Ff\n", precision, f.get_mpf_t());

    MPI_Finalize(); 

	return 0;
}
