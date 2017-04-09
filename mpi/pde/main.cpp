#include <cstdio>
#include <mpi.h>
#include <functional>
#include <vector>
#include "Domain.h"

double phi(double x)
{
    return (x == 0) ? 1 : 0;
}

double psi(double t)
{
    return 1;
}

double f(double x, double t)
{
    return 0;
}

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
    Domain domain(h, Nt, Nx, rank, size, phi, psi, f);
    
    for (long i = 0; i < Nt; ++i)
        domain.Step(tau);

    domain.GatherDomains(&solution);

    if (rank == 0)
    {
        for (auto u : solution)
            std::cout << u << " ";
        std::cout << "\n";
    }

    MPI_Finalize();
}
