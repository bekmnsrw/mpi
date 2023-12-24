#include <mpi.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <ctime>

int ProcRank;
int ProcNum;

void ProcessInitialization(double** pProcData, int* ProcDataSize) {
    *ProcDataSize = 10000;
    *pProcData = new double[*ProcDataSize];

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int i = 0; i < *ProcDataSize; ++i) {
        (*pProcData)[i] = rand();
    }
}

void ProcessTermination(const double* pProcData, int ProcDataSize) {
//    std::cout << "Sorted Array: ";
//    for (int i = 0; i < ProcDataSize; ++i) {
//        std::cout << pProcData[i] << " ";
//    }
//    std::cout << std::endl;

    delete[] pProcData;
}

// Последовательный алгоритм QuickSort для LocalDataSort
void QuickSort(double arr[], int low, int high) {
    if (low < high) {
        double pivot = arr[high];
        int i = (low - 1);

        for (int j = low; j <= high - 1; j++) {
            if (arr[j] <= pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }

        std::swap(arr[i + 1], arr[high]);
        int partitionIndex = i + 1;

        QuickSort(arr, low, partitionIndex - 1);
        QuickSort(arr, partitionIndex + 1, high);
    }
}

void LocalDataSort(double* pData, int DataSize) {
    QuickSort(pData, 0, DataSize - 1);
}

int GetProcDataDivisionPos(const double* pData, int DataSize, double Pivot) {
    int pos = 0;
    for (int i = 0; i < DataSize; ++i) {
        if (pData[i] <= Pivot) {
            ++pos;
        }
    }

    return pos;
}

void DataMerge(double* pResult, const double* pData1, int Size1, const double* pData2, int Size2) {
    int i = 0, j = 0, k = 0;

    while (i < Size1 && j < Size2) {
        if (pData1[i] <= pData2[j]) {
            pResult[k++] = pData1[i++];
        } else {
            pResult[k++] = pData2[j++];
        }
    }

    while (i < Size1) {
        pResult[k++] = pData1[i++];
    }

    while (j < Size2) {
        pResult[k++] = pData2[j++];
    }
}

void PivotDistribution(const double* pProcData, int ProcDataSize, int Dim, int Mask, int Iter, double* pPivot) {
    MPI_Group WorldGroup;
    MPI_Group SubcubeGroup; // Группа процессов - подгиперкуб
    MPI_Comm SubcubeComm; // Коммуникатор подгиперкуба

    int j = 0;
    int GroupNum = ProcNum / (int)pow(2, Dim - Iter);
    int* ProcRanks = new int[GroupNum];

    // Формирование списка рангов процессов для гиперкуба
    int StartProc = ProcRank - GroupNum;
    if (StartProc < 0) StartProc = 0;
    int EndProc = ProcRank + GroupNum;
    if (EndProc > ProcNum) EndProc = ProcNum;

    for (int proc = StartProc; proc < EndProc; proc++) {
        if ((ProcRank & Mask) >> (Iter) == (proc & Mask) >> (Iter)) {
            ProcRanks[j++] = proc;
        }
    }

    // Объединение процессов подгиперкуба в одну группу
    MPI_Comm_group(MPI_COMM_WORLD, &WorldGroup);
    MPI_Group_incl(WorldGroup, GroupNum, ProcRanks, &SubcubeGroup);
    MPI_Comm_create(MPI_COMM_WORLD, SubcubeGroup, &SubcubeComm);

    // Поиск и рассылка ведущего элемента всем процессам подгиперкуба
    if (ProcRank == ProcRanks[0]) *pPivot = pProcData[ProcDataSize / 2];
    MPI_Bcast(pPivot, 1, MPI_DOUBLE, 0, SubcubeComm);

    MPI_Group_free(&SubcubeGroup);
    MPI_Comm_free(&SubcubeComm);
    delete[] ProcRanks;
}

void ParallelHyperQuickSort(double* pProcData, int ProcDataSize) {
    MPI_Status status;
    int CommProcRank; // Ранг процессора, с которым выполняется взаимодействие
    double* pData, // Часть блока, остающаяся на процессоре
    * pSendData, // Часть блока, передаваемая процессору CommProcRank
    * pRecvData, // Часть блока, получаемая от процессора CommProcRank
    * pMergeData; // Блок данных, получаемый после слияния
    int DataSize, SendDataSize, RecvDataSize, MergeDataSize;
    int HypercubeDim = (int)ceil(log(ProcNum) / log(2)); // Размерность гиперкуба
    int Mask = ProcNum;
    double Pivot;

    LocalDataSort(pProcData, ProcDataSize);

    // Итерации обобщенной быстрой сортировки
    for (int i = HypercubeDim; i > 0; i--) {
        // Определение ведущего значения и его рассылка всем процессорам
        PivotDistribution(pProcData, ProcDataSize, HypercubeDim, Mask, i, &Pivot);
        Mask = Mask >> 1;

        // Определение границы разделения блока
        int pos = GetProcDataDivisionPos(pProcData, ProcDataSize, Pivot);

        // Разделение блока на части
        if (((ProcRank & Mask) >> (i - 1)) == 0) { // Старший бит = 0
            pSendData = &pProcData[pos + 1];
            SendDataSize = ProcDataSize - pos - 1;
            if (SendDataSize < 0) SendDataSize = 0;
            CommProcRank = ProcRank + Mask;
            pData = &pProcData[0];
            DataSize = pos + 1;
        } else { // Старший бит = 1
            pSendData = &pProcData[0];
            SendDataSize = pos + 1;
            if (SendDataSize > ProcDataSize) SendDataSize = pos;
            CommProcRank = ProcRank - Mask;
            pData = &pProcData[pos + 1];
            DataSize = ProcDataSize - pos - 1;
            if (DataSize < 0) DataSize = 0;
        }

        // Пересылка размеров частей блоков данных
        MPI_Sendrecv(
                &SendDataSize,
                1,
                MPI_INT,
                CommProcRank,
                0,
                &RecvDataSize,
                1,
                MPI_INT,
                CommProcRank,
                0,
                MPI_COMM_WORLD,
                &status
        );

        // Пересылка частей блоков данных
        pRecvData = new double[RecvDataSize];
        MPI_Sendrecv(
                pSendData,
                SendDataSize,
                MPI_DOUBLE,
                CommProcRank,
                0,
                pRecvData,
                RecvDataSize,
                MPI_DOUBLE,
                CommProcRank,
                0,
                MPI_COMM_WORLD,
                &status
        );

        MergeDataSize = DataSize + RecvDataSize;
        pMergeData = new double[MergeDataSize];
        DataMerge(pMergeData, pData, DataSize, pRecvData, RecvDataSize);
        delete[] pProcData;
        delete[] pRecvData;
        pProcData = pMergeData;
        ProcDataSize = MergeDataSize;
    }
}

int main(int argc, char* argv[]) {
    double* pProcData;
    int ProcDataSize;

    double total_time = 0;

    MPI_Init(&argc, &argv);
    int ProcRank, ProcNum;
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);

    for (int iter = 0; iter < 1000; iter++) {
        ProcessInitialization(&pProcData, &ProcDataSize);

        double startTime = MPI_Wtime();

        ParallelHyperQuickSort(pProcData, ProcDataSize);

        double endTime = MPI_Wtime();
        total_time += endTime - startTime;

        if (ProcRank == 0) ProcessTermination(pProcData, ProcDataSize);
    }

    MPI_Finalize();

    if (ProcRank == 0) std::cout << "Average time: " << total_time / 1000 << std::endl;

    return EXIT_SUCCESS;
}
