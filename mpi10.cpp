#include <iostream>
#include <vector>
#include <mpi.h>
#include <ctime>

int main(int argc, char **argv) {
    int rank, size;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int const n = 100;

    int *sbuf = new int[n];
    int *rbuf = new int[n];

    int message_buf_size = n * sizeof(int) + MPI_BSEND_OVERHEAD;
    int *buf = (int *) malloc(message_buf_size);

    double t;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            sbuf[i] = rand() % 10000;
        }
    }

    if (rank == 0) {
        // Send
        t = MPI_Wtime();
        MPI_Send(sbuf, n, MPI_INT, 1, 1, MPI_COMM_WORLD);
        MPI_Recv(rbuf, n, MPI_INT, 1, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("n = %d\ntime Send = %f\n\n", n, MPI_Wtime() - t);

        // Ssend - синхронный
        t = MPI_Wtime();
        MPI_Ssend(sbuf, n, MPI_INT, 1, 2, MPI_COMM_WORLD);
        MPI_Recv(rbuf, n, MPI_INT, 1, 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("n = %d\ntime Ssend = %f\n\n", n, MPI_Wtime() - t);
        MPI_Barrier(MPI_COMM_WORLD);

        // Bsend - буферизация
        t = MPI_Wtime();
        MPI_Buffer_attach(buf, message_buf_size);
        MPI_Ssend(sbuf, n, MPI_INT, 1, 3, MPI_COMM_WORLD);
        MPI_Recv(rbuf, n, MPI_INT, 1, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("n = %d\ntime Bsend = %f\n\n", n, MPI_Wtime() - t);
        MPI_Buffer_detach(buf, &message_buf_size);
        free(buf);

        // Rsend - по готовности
        t = MPI_Wtime();
        MPI_Rsend(sbuf, n, MPI_INT, 1, 4, MPI_COMM_WORLD);
        MPI_Recv(rbuf, n, MPI_INT, 1, 4, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        printf("n = %d\ntime Rsend = %f\n\n", n, MPI_Wtime() - t);
    } else {
        MPI_Recv(rbuf, n, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        MPI_Send(sbuf, n, MPI_INT, 0, 1, MPI_COMM_WORLD);

        MPI_Recv(rbuf, n, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        MPI_Ssend(sbuf, n, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Recv(rbuf, n, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        MPI_Buffer_attach(buf, message_buf_size);
        MPI_Bsend(sbuf, n, MPI_INT, 0, 3, MPI_COMM_WORLD);
        MPI_Buffer_detach(buf, &message_buf_size);
        free(buf);

        MPI_Recv(rbuf, n, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
        MPI_Rsend(sbuf, n, MPI_INT, 0, 4, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
