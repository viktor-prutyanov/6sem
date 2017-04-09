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
    int active, inactive;
    std::vector<double> uField[2];
    void swap()
    {
        int temp;
        temp = active;    
        active = inactive;
        inactive = temp;
    }
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
    uField({std::vector<double>(xNum / size), std::vector<double>(xNum / size)}),
    psi(psi),
    f(f),
    active(0),
    inactive(1)
{
    /* Initialize field by u(0,x) = phi(x) */
    long i = segmStart;
    for (auto &u : uField[inactive])
        u = phi(h * i++);
}

void Domain::GatherDomains(std::vector<double> *field)
{
    MPI_Gather(uField[inactive].data(), segmLen, MPI_DOUBLE, (*field).data(), 
        segmLen, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void Domain::Step(double tau)
{
    if (rank != size - 1)
        MPI_Send(&uField[inactive][segmStart + segmLen], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);

    double uLeft;
    MPI_Status status;
    if (rank == 0)
        uLeft = psi(t);
    else
        MPI_Recv(&uLeft, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);

    uField[active][0] = uField[inactive][0] + tau * (f(t, 0) - (uField[inactive][0] - uLeft) / h);
    for (long i = 1; i < segmLen; ++i)
        uField[active][i] = uField[inactive][i] 
            + tau * (f(t, segmStart + i * h) - (uField[inactive][i] - uField[inactive][i - 1]) / h);

    t += tau;
    swap();
}
