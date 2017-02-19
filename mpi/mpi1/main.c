#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank );
    double data = (rank == 0) ? 1337.0 : 0.0;

    int count = atoi(argv[1]);

    MPI_Status status;
    for (int i = 0; i < count; ++i)
    {
        if (rank == 0)
        {
            if (i == 0)
            {
                MPI_Send(&data, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
                fprintf(stderr, "Sent data = %lg in %d/%d\n", data, rank, size);
            }
            else
            {
                MPI_Recv(&data, 1, MPI_DOUBLE, size - 1, 0, MPI_COMM_WORLD, &status);
                fprintf(stderr, "Received data = %lg in %d/%d\n", data++, rank, size);
                MPI_Send(&data, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
            }
        }
        else if (rank == size - 1)
        {
            MPI_Recv(&data, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
            fprintf(stderr, "Received data = %lg in %d/%d\n", data++, rank, size);
            if (i < count - 1)
                MPI_Send(&data, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&data, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &status);
            fprintf(stderr, "Received data = %lg in %d/%d\n", data++, rank, size);
            MPI_Send(&data, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
