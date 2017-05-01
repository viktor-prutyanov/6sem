#include <cstdio>
#include <cmath>
#include <mpi.h>
#include <functional>
#include <vector>

class Domain
{
public:
    Domain(double h, long tNum, long xNum, int rank, int size, 
        std::function<double(double)> phi, 
        std::function<double(double)> psi, 
        std::function<double(double, double)> f);
    void Step(double tau);
    void GatherDomains(std::vector<double> *field);

private:
    double t, h, tNum, xNum;
    int rank, size;
    long segmLen, segmStart;
    std::function<double(double)> psi;
    std::function<double(double, double)> f;
    std::vector<double> u;
};

Domain::Domain(double h, long tNum, long xNum, int rank, int size,
        std::function<double(double)> phi, 
        std::function<double(double)> psi, 
        std::function<double(double, double)> f)
    :t(0),
    h(h),
    tNum(tNum),
    xNum(xNum),
    rank(rank),
    size(size),
    segmLen(xNum / size),
    segmStart(segmLen * rank),
    u(std::vector<double>(xNum / size)),
    psi(psi),
    f(f)
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

void Domain::Step(double tau)
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
