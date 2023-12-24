#include <cstdio>
#include <random>
#include <ctime>
#include "mpi.h"

bool is_point_in_circle(double x, double y) {
    if ((x - 1.0f) * (x - 1.0f) + (y - 1.0f) * (y - 1.0f) <= 1.0f) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char **argv) {
    int rank;
    int size;

    int sequence_length = 100000000;

    int elements_per_process;
    int number_of_elements_received;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf("Number of processors: %d\n", size);

        int i;

        elements_per_process = sequence_length / size;

        if (size > 1) {
            /* Send to all other processes how many numbers each of them should generate */
            for (i = 1; i < size - 1; i++) {
                MPI_Send(&elements_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }

            /* Extending elements_per_process for last process if necessary */
            int elements_left = sequence_length - i * elements_per_process;
            MPI_Send(&elements_left, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        int global_counter = 0;
        int local_counter = 0;

        /* Process 0 local counter */
        for (i = 0; i < elements_per_process; i++) {
            double x = static_cast<double>(std::rand() % 2001) / 1000.0;
            double y = static_cast<double>(std::rand() % 2001) / 1000.0;

            /* Check if point inside the circle */
            if (is_point_in_circle(x, y)) { global_counter++; }
        }

        /* Collect local counters from other processes */
        for (i = 1; i < size; i++) {
            MPI_Recv(&local_counter, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            global_counter += local_counter;
        }

        printf("Number of points inside the circle: %d\n", global_counter);
        printf("Pi: %f\n", (double)(4 * global_counter) / sequence_length);
    } else {
        /* Receiving data from process 0 */
        MPI_Recv(&number_of_elements_received, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        /* Compute local counter */
        int local_counter = 0;

        for (int i = 0; i < number_of_elements_received; i++) {
            double x = static_cast<double>(std::rand() % 2001) / 1000.0;
            double y = static_cast<double>(std::rand() % 2001) / 1000.0;

            /* Check if point inside the circle */
            if (is_point_in_circle(x, y)) { local_counter++; }
        }

        /* Send local counter to process 0 */
        MPI_Send(&local_counter, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
