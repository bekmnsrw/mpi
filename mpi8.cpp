#include <cstdio>
#include <mpi.h>
#include <vector>
#include <iostream>
#include <ctime>

int main(int argc, char *argv[]) {
    int rank, size;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int const n = 9;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int partition = n / size;

    int *array = new int[n];
    int *buf = new int[partition];
    int *result_array = new int[n];

    if (rank == 0) {
        // Заполнение массива
        printf("Initial array:\n");
        for (int i = 0; i < n; i++) {
            array[i] = rand() % 100;
            printf("%d ", array[i]);
        }
        printf("\n\n");

        // Отправка с 0 процесса подмассивов остальным процессам
        for (int i = 0; i < size; i++) {
            int *send_buf = new int[partition];

            for (int j = 0; j < partition; j++) {
                send_buf[j] = array[i * partition + j];
            }

            if (i == 0) {
                buf = send_buf;
            } else {
                MPI_Send(
                        send_buf,
                        partition,
                        MPI_INT,
                        i,
                        i + 100,
                        MPI_COMM_WORLD
                );
            }
        }
    }

    // Получение подмассивов на остальных процессах
    if (rank != 0) {
        MPI_Recv(
                buf,
                partition,
                MPI_INT,
                0,
                rank + 100,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
        );
    }

    printf("Rank: %d\n", rank);
    for (int i = 0; i < partition; i++) {
        printf("%d ", buf[i]);
    }
    printf("\n\n");

    if (rank != 0) {
        // Отправка со всех процессов подмассивов на процесс 0
        MPI_Send(
                buf,
                partition,
                MPI_INT,
                0,
                rank + 1000,
                MPI_COMM_WORLD
        );
    } else {
        // Заполнение на процессе 0 результирующего массива из буфера 0 процесса
        for (int i = 0; i < partition; i++) {
            result_array[i] = buf[i];
        }
    }

    if (rank == 0) {
        // Получение на 0 процессе подмассивов остальных процессов
        for (int i = 1; i < size; i++) {
            MPI_Recv(
                    buf,
                    partition,
                    MPI_INT,
                    i,
                    i + 1000,
                    MPI_COMM_WORLD,
                    MPI_STATUSES_IGNORE
            );

            for (int j = 0; j < partition; j++) {
                result_array[i * partition + j] = buf[j];
            }
        }

        // Вывод на 0 процессе результирующего массива
        printf("Result array:\n");
        for (int i = 0; i < n; i++) {
            printf("%d ", result_array[i]);
        }
        printf("\n\n");
    }

    MPI_Finalize();
    return 0;
}
