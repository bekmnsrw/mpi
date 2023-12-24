#include <iostream>
#include <vector>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size;

    int const n = 10;

    int array[n] = {1,2,3,4,5,6,7,8,9,10};
    int *reversed_array = new int[n];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Вычисление смещения и количества данных для каждого процесса
    std::vector<int> sendcounts(size, 0);
    std::vector<int> displacements(size, 0);

    int elements_per_process = n / size;
    int remaining_elements = n % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = elements_per_process + (i < remaining_elements ? 1 : 0);
        if (i > 0) { displacements[i] = displacements[i - 1] + sendcounts[i - 1]; }
    }

    // Вычисление размера локального буфера и выделение памяти
    int local_size = sendcounts[rank];
    std::vector<int> local_data(local_size);

    // Распределение данных с использованием MPI_Scatterv
    MPI_Scatterv(
            array,
            sendcounts.data(),
            displacements.data(),
            MPI_INT,
            local_data.data(),
            local_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD
    );

    int *local_reversed_array = new int[local_size];
    int *recv_displacements = new int[size];

    for (int i = 0; i < local_size; i++) {
        local_reversed_array[i] = local_data[local_size - i - 1];
    }

    recv_displacements[size - 1] = 0;
    for (int i = size - 2; i >= 0; i--) {
        recv_displacements[i] = recv_displacements[i + 1] + sendcounts[i + 1];
    }

    MPI_Gatherv(
            local_reversed_array,
            local_size,
            MPI_INT,
            reversed_array,
            sendcounts.data(),
            recv_displacements,
            MPI_INT,
            0,
            MPI_COMM_WORLD
            );

    if (rank == 0) {
        printf("Reversed array:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", reversed_array[i]);
        }
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}
