#include <iostream>
#include <vector>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size;

    int num = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *send = new int[1];
    int *recv = new int[1];

    int receiver = rank + 1;
    int sender = rank - 1;

    if (rank == 0) { sender = size - 1; }
    if (rank == size - 1) { receiver = 0; }

    if (rank == 0) {
        send[0] = num;
        MPI_Send(send, 1, MPI_INT, receiver, receiver, MPI_COMM_WORLD);
        printf("Process %d to Process %d, num = %d\n", rank, receiver, send[0]);
    }

    if (rank != 0) {
        MPI_Recv(recv, 1, MPI_INT, sender, rank, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("Process %d from Process %d, num = %d\n", rank, sender, recv[0]);
        send[0] = recv[0] + 1;

        MPI_Send(send, 1, MPI_INT, receiver, receiver, MPI_COMM_WORLD);
        printf("Process %d to Process %d, num = %d\n", rank, receiver, send[0]);
    }

    if (rank == 0) {
        MPI_Recv(recv, 1, MPI_INT, sender, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("Process %d from Process %d, num = %d\n", rank, sender, recv[0]);
    }

    MPI_Finalize();
    return 0;
}
