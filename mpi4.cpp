#include <iostream>
#include <vector>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size;

    int data[] = {0, -10, 2, 3, 10, -1, -2, -10, 1, 6, 8, 9, 0};
    int data_size = 13;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Вычисление смещения и количества данных для каждого процесса
    std::vector<int> sendcounts(size, 0);
    std::vector<int> displacements(size, 0);

    int elements_per_process = data_size / size;
    int remaining_elements = data_size % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = elements_per_process + (i < remaining_elements ? 1 : 0);
        if (i > 0) { displacements[i] = displacements[i - 1] + sendcounts[i - 1]; }
    }

    // Вычисление размера локального буфера и выделение памяти
    int local_size = sendcounts[rank];
    std::vector<int> local_data(local_size);

    // Распределение данных с использованием MPI_Scatterv
    MPI_Scatterv(
            data,
            sendcounts.data(),
            displacements.data(),
            MPI_INT,
            local_data.data(),
            local_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD
    );

    // Вывод локальных данных на каждом процессе
    std::cout << "Local data on Process " << rank << ": ";
    for (int i = 0; i < local_size; i++) {
        std::cout << local_data[i] << " ";
    }
    std::cout << std::endl;

    int local_sum = 0;
    int local_positive_count = 0;

    // Вычисление локальных сумм и количества положительных элементов на каждом процессе
    for (int i = 0; i < local_size; i++) {
        if (local_data[i] > 0) {
            local_sum += local_data[i];
            local_positive_count++;
        }
    }

    // Вывод вычисленных локальных сумм и количества положительных элементов на каждом процессе
    std::cout << "Process " << rank << ": " << "local_sum = " << local_sum << ", local_positive_count = " << local_positive_count << std::endl;

    int global_sum = 0;
    int global_positive_count = 0;

    MPI_Reduce(
            &local_sum,
            &global_sum,
            1,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD
    );

    MPI_Reduce(
            &local_positive_count,
            &global_positive_count,
            1,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD
    );

    if (rank == 0) {
        std::cout << "Average: " << static_cast<double>(global_sum) / global_positive_count << std::endl;
    }

    MPI_Finalize();
    return 0;
}
