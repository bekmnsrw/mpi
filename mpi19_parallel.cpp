#include <iostream>
#include <vector>
#include <algorithm>
#include <mpi.h>
#include <ctime>

void quicksort(std::vector<int> &arr, int start, int end) {
    if (start < end) {
        int pivot = arr[end];
        int i = start - 1;

        for (int j = start; j <= end - 1; j++) {
            if (arr[j] < pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }

        std::swap(arr[i + 1], arr[end]);
        int pivotIndex = i + 1;

        quicksort(arr, start, pivotIndex - 1);
        quicksort(arr, pivotIndex + 1, end);
    }
}

int main(int argc, char **argv) {
    int rank, size;
    double total_time = 0;
    const int num_iterations = 1000;
    int number_of_elements_to_sort = 50000;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int iter = 0; iter < num_iterations; iter++) {
        std::vector<int> arr;

        if (rank == 0) {
            for (int i = 0; i < number_of_elements_to_sort; i++) { arr.push_back(rand()); }
        }

        int chunkSize = number_of_elements_to_sort / size;
        int remainder = number_of_elements_to_sort % size;

        std::vector<int> counts(size, chunkSize);
        std::vector<int> displacements(size, 0);

        for (int i = 0; i < remainder; i++) { counts[i]++; }
        for (int i = 1; i < size; i++) { displacements[i] = displacements[i - 1] + counts[i - 1]; }

        chunkSize = counts[rank];
        std::vector<int> localArr(chunkSize);

        double startTime = MPI_Wtime();

        MPI_Scatterv(
                arr.data(),
                counts.data(),
                displacements.data(),
                MPI_INT,
                localArr.data(),
                chunkSize,
                MPI_INT,
                0,
                MPI_COMM_WORLD
        );

        quicksort(localArr, 0, chunkSize - 1);

        MPI_Gatherv(
                localArr.data(),
                chunkSize,
                MPI_INT,
                arr.data(),
                counts.data(),
                displacements.data(),
                MPI_INT,
                0,
                MPI_COMM_WORLD
        );

        double endTime = MPI_Wtime();
        total_time += endTime - startTime;
    }

    MPI_Finalize();

    if (rank == 0) {
        double average_time = total_time / num_iterations;
        std::cout << "Number of elements: " << number_of_elements_to_sort << std::endl;
        std::cout << "Number of proc: " << size << std::endl;
        std::cout << "Average time: " << average_time << std::endl;
    }

    return EXIT_SUCCESS;
}
