#include <cstdio>
#include <cmath>
#include <mpi.h>
#include <functional>
#include <vector>

class Domain
{
public:
    Domain(double tau, double h, long xNum, int rank, int size);
    void Run(long tNum);
    void GatherDomains(std::vector<double> *field);

    static double phi(double x);
    static double psi(double t);
    static double f(double x, double t);

private:
    double tau, h, xNum;
    int rank, size;
    long segmLen, segmStart;
    std::vector<double> u;
};

Domain::Domain(double tau, double h, long xNum, int rank, int size)
    :tau(tau),
    h(h),
    xNum(xNum),
    rank(rank),
    size(size),
    segmLen(xNum / size),
    segmStart(segmLen * rank),
    u(std::vector<double>(xNum / size))
{
    /* Initialize field by u(0,x) = phi(x) */
    long i = segmStart;
    for (auto &u_i : u)
        u_i = phi(h * i++);
}

void Domain::GatherDomains(std::vector<double> *field)
{
    MPI_Gather(u.data(), segmLen, MPI_DOUBLE, field->data(), 
        segmLen, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void Domain::Run(long tNum)
{
    double t = 0;
    for (long k = 0; k < tNum; ++k)
    {
        if (rank != size - 1)
        MPI_Send(&u[segmLen - 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);

        double uLeft;
        MPI_Status status;
        if (rank == 0)
            uLeft = psi(t);
        else
            MPI_Recv(&uLeft, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);

        for (size_t i = segmLen - 1; i > 0; --i)
            u[i] = u[i] + tau*(f((segmStart+i)*h, t) - (u[i] - u[i - 1])/h);

        u[0] = u[0] + tau*(f((segmStart)*h, t) - (u[0] - uLeft)/h);

        t += tau;
    }
}

double Domain::phi(double x)
{
    return (x == 0) ? 1 : 0;
}

double Domain::psi(double t)
{
    return 1;
}

double Domain::f(double x, double t)
{
    return 0;
}