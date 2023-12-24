#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
    int rank, size;

    int const ROWS_COUNT = 5;
    int const COLUMN_COUNT = 3;

    int matrix[ROWS_COUNT][COLUMN_COUNT] = {
            {16, 21, 32},
            {23, 51, 20},
            {45, 10, 29},
            {45, 50, 60},
            {45, 10, 60}
    };

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<int> sendcounts(size, 0);
    std::vector<int> displacements(size, 0);

    int rows_per_process = ROWS_COUNT / size;
    int remaining_rows = ROWS_COUNT % size;

    for (int i = 0; i < size; i++) {
        sendcounts[i] = rows_per_process * COLUMN_COUNT + (i < remaining_rows ? COLUMN_COUNT : 0);
        if (i > 0) { displacements[i] = displacements[i - 1] + sendcounts[i - 1]; }
    }

    int receive_size = sendcounts[rank];
    std::vector<int> received_elements(receive_size);

    MPI_Scatterv(
            matrix,
            sendcounts.data(),
            displacements.data(),
            MPI_INT,
            received_elements.data(),
            receive_size,
            MPI_INT,
            0,
            MPI_COMM_WORLD
    );

    std::vector<int> local_minimums(receive_size / COLUMN_COUNT);
//    std::vector<int> local_maximums(receive_size / COLUMN_COUNT);

    int local_min = INT_MAX;
//    int local_max = INT_MIN;

    int current_row = -1;

    for (int i = 0; i < receive_size; i++) {
        if (i % COLUMN_COUNT == 0) {
            current_row++;

            if (current_row != 0) {
                local_minimums[current_row - 1] = local_min;
//                local_maximums[current_row - 1] = local_max;
                /* if (rank == 0) { printf("Save min: %d for row %d\n", local_min, current_row - 1); } */
            }

            local_min = received_elements[i];
//            local_max = received_elements[i];
            /* if (rank == 0) { printf("Current row index: %d, local min: %d\n", current_row, local_min); } */
        } else {
            if (received_elements[i] < local_min) {
                local_min = received_elements[i];
                /* if (rank == 0) { printf("Current row index: %d, update local min: %d\n", current_row, local_min); } */
            }

//            if (received_elements[i] > local_max) {
//                local_max = received_elements[i];
//            }
        }
    }

    local_minimums[current_row] = local_min;
//    local_maximums[current_row] = local_max;
    /* if (rank == 0) { printf("Save min: %d for row %d\n", local_min, current_row); } */

    std::cout << "Local mins for process " << rank << ": ";
    for (int min: local_minimums) { std::cout << min << " "; }
    std::cout << std::endl;

//    std::cout << "Local maxes for process " << rank << ": ";
//    for (int max: local_maximums) { std::cout << max << " "; }
//    std::cout << std::endl;

    int local_maxmin = INT_MIN;
//    int local_minmax = INT_MAX;

    int global_maxmin = INT_MIN;
//    int global_minmax = INT_MAX;

    for (int min: local_minimums) {
        if (min > local_maxmin) {
            local_maxmin = min;
        }
    }

//    for (int max: local_maximums) {
//        if (max < local_minmax) {
//            local_minmax = max;
//        }
//    }

    MPI_Reduce(
            &local_maxmin,
            &global_maxmin,
            1,
            MPI_INT,
            MPI_MAX,
            0,
            MPI_COMM_WORLD
    );

//    MPI_Reduce(
//            &local_minmax,
//            &global_minmax,
//            1,
//            MPI_INT,
//            MPI_MIN,
//            0,
//            MPI_COMM_WORLD
//    );

    if (rank == 0) {
        printf("Global maxmin: %d\n", global_maxmin);
//        printf("Global minmax: %d\n", global_minmax);
    }

    MPI_Finalize();
    return 0;
}
