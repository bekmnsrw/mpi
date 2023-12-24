#include <cstdio>
#include <cstdlib>
#include "mpi.h"

/*
 * Задача:
 *     Максимум массива (2 балла)
 * Спецификация задачи:
 *     Уметь использовать что-то из send, recv, bcast, reduce
 *
 *     mpiexec -n 3 ./cmake-build-debug/mpi.exe
*/

int main(int argc, char **argv) {
    int rank;
    int size;

    int array_size = 16;
    int array[array_size];

    int elements_per_process;
    int number_of_elements_received;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("%d\n", size);

    if (rank == 0) {
        /* Filling the array with random numbers */
        for (int i = 0; i < array_size; i++) {
            array[i] = rand() % 100;
            printf("%d ", array[i]);
        }
        printf("\n");

        int index, i;

        elements_per_process = array_size / size;

        if (size > 1) {
            /* Send subarrays to all other processes */
            for (i = 1; i < size - 1; i++) {
                index = i * elements_per_process;
                MPI_Send(&elements_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Send(&array[index], elements_per_process, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

            /* Extending last subarray if necessary */
            index = i * elements_per_process;
            int elements_left = array_size - index;
            printf("Elements left: %d\n", elements_left);

            MPI_Send(&elements_left, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&array[index], elements_left, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int global_max = -1;
        int local_max = -1;

        /* Process 0 local max */
        for (i = 0; i < elements_per_process; i++) {
            if (array[i] > global_max) {
                global_max = array[i];
                printf("Master process local max: %d\n", global_max);
            }
        }

        /* Collect local max from other processes */
        for (i = 1; i < size; i++) {
            MPI_Recv(&local_max, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            if (local_max > global_max) {
                global_max = local_max;
                printf("Other process local max: %d\n", global_max);
            }
        }

        printf("GLOBAL_MAX: %d\n", global_max);
    } else {
        /* Receiving data from process 0 */
        MPI_Recv(&number_of_elements_received, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&array, number_of_elements_received, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        /* Compute local max */
        int local_max = -1;
        for (int i = 0; i < number_of_elements_received; i++) {
            if (array[i] > local_max) {
                local_max = array[i];
            }
        }

        /* Send local max to process 0 */
        MPI_Send(&local_max, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
