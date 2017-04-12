#include <iostream>
#include <fstream>
#include <mpi.h>
#include <functional>
#include <vector>
#include "Domain.h"

int main(int argc, char *argv[])
{
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc != 5)
    {
        if (rank == 0)
            std::cerr << "usage: "
                << argv[0] <<  " [tau] [h] [number of time steps] [number of points]\n";
        return -1;
    }

    double tau = atof(argv[1]);
    double h = atof(argv[2]);
    long Nt = atol(argv[3]);
    long Nx = atol(argv[4]);

    if ((tau <= 0) || (h <= 0) || (Nt <= 0) || (Nx <= 0) || (Nx % size != 0))
    {
        if (rank == 0)
            std::cerr << "Invalid aruments.\n";
        return -1;        
    }

    std::vector<double> solution;

    if (rank == 0)
        solution.resize(Nx);

    /* Domain decomposition */
    Domain domain(tau, h, Nx, rank, size);
    
    double t1 = MPI_Wtime();
    domain.Run(Nt);
    double t2 = MPI_Wtime(); 
    
    domain.GatherDomains(&solution);
       
    std::cout << (t2 - t1) << std::endl;

/*
    if (rank == 0)
    {
        std::ofstream data_file("answer.txt", std::ios::out | std::ios::trunc);
        if (!data_file.is_open())
        {
            std::cout << "Unable to open file.\n";
            return -1;
        }

        data_file << std::fixed;
        for (auto u : solution)
            data_file << u << "\n";

        data_file.close();
    }
*/
    MPI_Finalize();
}
