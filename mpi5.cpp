#include <iostream>
#include <vector>
#include <mpi.h>

int main(int argc, char **argv) {
    int rank, size;

    double vector_a[] = {1.5, 2.4, 3.3, 4.2, 5.1};
    double vector_b[] = {5.5, 4.4, 3.3, 2.2, 1.1};

    int VECTOR_CONST = 5;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Вычисление смещения и количества данных для каждого процесса
    std::vector<int> sendcounts(size, 0);
    std::vector<int> displacements(size, 0);

    int elements_per_process = VECTOR_CONST / size;
    int remaining_elements = VECTOR_CONST % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = elements_per_process + (i < remaining_elements ? 1 : 0);
        if (i > 0) { displacements[i] = displacements[i - 1] + sendcounts[i - 1]; }
    }

    // Вычисление размера локального буфера и выделение памяти
    int local_vector_a_size = sendcounts[rank];
    std::vector<double> local_vector_a(local_vector_a_size);

    int local_vector_b_size = sendcounts[rank];
    std::vector<double> local_vector_b(local_vector_a_size);

    // Распределение данных с использованием MPI_Scatterv
    MPI_Scatterv(
            vector_a,
            sendcounts.data(),
            displacements.data(),
            MPI_DOUBLE,
            local_vector_a.data(),
            local_vector_a_size,
            MPI_DOUBLE,
            0,
            MPI_COMM_WORLD
    );

    MPI_Scatterv(
            vector_b,
            sendcounts.data(),
            displacements.data(),
            MPI_DOUBLE,
            local_vector_b.data(),
            local_vector_b_size,
            MPI_DOUBLE,
            0,
            MPI_COMM_WORLD
    );

    // Вывод локальных данных на каждом процессе
    std::cout << "Vector a, Local data on Process " << rank << ": ";
    for (int i = 0; i < local_vector_a_size; i++) {
        std::cout << local_vector_a[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Vector b, Local data on Process " << rank << ": ";
    for (int i = 0; i < local_vector_b_size; i++) {
        std::cout << local_vector_b[i] << " ";
    }
    std::cout << std::endl;

    double local_scalar_product = 0;

    // Вычисление локального скалярного произведения на каждом процессе
    for (int i = 0; i < local_vector_a_size; i++) {
        local_scalar_product += (local_vector_a[i] * local_vector_b[i]);
    }

    // Вывод скалярного произведения на каждом процессе
    std::cout << "Process " << rank << ": " << "local_scalar_product = " << local_scalar_product << std::endl;

    double global_scalar_product = 0;;

    // Вычисление глобального скалярного произведения
    MPI_Reduce(
            &local_scalar_product,
            &global_scalar_product,
            1,
            MPI_DOUBLE,
            MPI_SUM,
            0,
            MPI_COMM_WORLD
    );

    if (rank == 0) {
        std::cout << "Scalar product: " << global_scalar_product << std::endl;
    }

    MPI_Finalize();
    return 0;
}
