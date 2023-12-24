#include <cstdio>
#include <mpi.h>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
    int rank, size;

    // Количество столбцов матрицы == количеству строк вектора
    int const ROWS_COUNT = 3;
    int const COLUMN_COUNT = 4;

    int matrix[ROWS_COUNT][COLUMN_COUNT] = {
            {17, 21, 32, 21},
            {23, 52, 20, 21},
            {45, 10, 23, 21},
    };

    int vector[COLUMN_COUNT][1] = {
            {1},
            {5},
            {3},
            {3}
    };

    int transp_matrix[COLUMN_COUNT][ROWS_COUNT] = {};

    for (int i = 0; i < COLUMN_COUNT; i++) {
        for (int j = 0; j < ROWS_COUNT; j++) {
            transp_matrix[i][j] = matrix[j][i];
        }
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> sendcounts(size, 0);
    std::vector<int> displacements(size, 0);

    std::vector<int> sendcounts_vec(size, 0);
    std::vector<int> displacements_vec(size, 0);

    int columns_per_process = COLUMN_COUNT / size;
    int remaining_columns = COLUMN_COUNT % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = columns_per_process * ROWS_COUNT + (i < remaining_columns ? ROWS_COUNT : 0);
        sendcounts_vec[i] = sendcounts[i] / ROWS_COUNT;
        if (i > 0) {
            displacements[i] = displacements[i - 1] + sendcounts[i - 1];
            displacements_vec[i] = displacements_vec[i - 1] + sendcounts_vec[i - 1];
        }
    }

    int receive_size = sendcounts[rank];
    std::vector<int> received_elements(receive_size);

    int receive_size_vec = sendcounts_vec[rank];
    std::vector<int> received_elements_vec(receive_size_vec);

    MPI_Scatterv(
            transp_matrix,
            sendcounts.data(),
            displacements.data(),
            MPI_INT,
            received_elements.data(),
            receive_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD
    );

    MPI_Scatterv(
            vector,
            sendcounts_vec.data(),
            displacements_vec.data(),
            MPI_INT,
            received_elements_vec.data(),
            receive_size_vec,
            MPI_INT,
            0,
            MPI_COMM_WORLD
    );

//    if (rank == 0) {
//        std::cout << "Local mins for process " << rank << ": ";
//        for (int min: received_elements) { std::cout << min << " "; }
//        std::cout << std::endl;
//
//        std::cout << "Vector for process " << rank << ": ";
//        for (int min: received_elements_vec) { std::cout << min << " "; }
//        std::cout << std::endl;
//    }

//    if (rank == 1) {
//        std::cout << "Local mins for process " << rank << ": ";
//        for (int min: received_elements) { std::cout << min << " "; }
//        std::cout << std::endl;
//
//        std::cout << "Vector for process " << rank << ": ";
//        for (int min: received_elements_vec) { std::cout << min << " "; }
//        std::cout << std::endl;
//    }

    std::vector<int> local_sum_vec(ROWS_COUNT);
    std::vector<int> res_sum_vec(ROWS_COUNT);

    for (int i = 0; i < receive_size_vec; i++) {
        int start_index = i * ROWS_COUNT;
        int end_index = start_index + ROWS_COUNT;
        int index = 0;

        for (int j = start_index; j < end_index; j++) {
            local_sum_vec[index] += (received_elements_vec[i] * received_elements[j]);
            index++;
        }
    }

//    if (rank == 0) {
//        printf("1\n");
//        for (int i = 0; i < COLUMN_COUNT; i++) {
//            printf("%d ", local_sum_vec[i]);
//        }
//        printf("\n");
//    }
//
//    if (rank == 1) {
//        printf("2\n");
//        for (int i = 0; i < COLUMN_COUNT; i++) {
//            printf("%d ", local_sum_vec[i]);
//        }
//        printf("\n");
//    }

    MPI_Reduce(
            &local_sum_vec[0],
            &res_sum_vec[0],
            ROWS_COUNT,
            MPI_INT,
            MPI_SUM,
            0,
            MPI_COMM_WORLD
    );

    if (rank == 0) {
        for (int i = 0; i < ROWS_COUNT; i++) {
            printf("%d\n", res_sum_vec[i]);
        }
    }

    MPI_Finalize();
    return 0;
}
