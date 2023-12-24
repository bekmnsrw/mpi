#include <iostream>
#include <vector>
#include <mpi.h>

void func(int num, MPI_Comm comm) {
    int rank, size;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    int *send = new int[1];
    int *recv = new int[1];

    int to = rank + 1;
    int from = rank - 1;

    if (rank == 0) { from = size - 1; }
    if (rank == size - 1) { to = 0; }

    if (rank == 0) {
        send[0] = num;
        MPI_Send(send, 1, MPI_INT, to, to, comm);
        printf("Process %d to Process %d, num = %d\n", rank, to, send[0]);
    }

    if (rank != 0) {
        MPI_Recv(recv, 1, MPI_INT, from, rank, comm, MPI_STATUSES_IGNORE);
        printf("Process %d from Process %d, num = %d\n", rank, from, recv[0]);
        send[0] = recv[0] + 1;

        MPI_Send(send, 1, MPI_INT, to, to, comm);
        printf("Process %d to Process %d, num = %d\n", rank, to, send[0]);
    }

    if (rank == 0) {
        MPI_Recv(recv, 1, MPI_INT, from, 0, comm, MPI_STATUSES_IGNORE);
        printf("Process %d from Process %d, num = %d\n", rank, from, recv[0]);
    }
}

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);

    func(1, MPI_COMM_WORLD);

    MPI_Group MPI_GROUP_WORLD, group;
    MPI_Comm comm;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD);

    int group_size = size / 2;

    int *ranks = new int[group_size];
    for (int i = 0; i < group_size; i++) {
        ranks[i] = i;
    }

    MPI_Group_incl(MPI_GROUP_WORLD, group_size, ranks, &group);
    MPI_Comm_create(MPI_COMM_WORLD, group, &comm);

    if (comm != MPI_COMM_NULL) {
        func(100, comm);
    }

    MPI_Finalize();
    return 0;
}
