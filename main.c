#include <windows.h>
#include <stdio.h>


typedef struct{
    int n;
    int m;
    int *matrix;
    int curr_row; // текущий ряд для горизонт среза
    int sums_rows; //сколько суммировать
    int sums_cols; //сколько столбцов складывается на ряд
    int batch_size; // сколько в одной коллонне
} Data;

typedef struct{
    int m;
    int *matrix;
    int curr_row;
    int sums_rows;
    int curr_col;
    int batch_size;
    int *res_row;
} MiniData;

DWORD WINAPI VerticalThreads(void* minidata) {
    MiniData* MiniThreadData = (MiniData*)minidata;
    printf("MINI Flow started with: %d\n", MiniThreadData->curr_col);
    
    // Имитация работы
    Sleep(2000);
    
    printf("MINI Flow ended.\n");
    return 0;
}


DWORD WINAPI HorizontalThreads(void* data) {
    Data* ThreadData = (Data*)data;
    printf("Flow started with: %d\n", ThreadData->curr_row);
    int res_row[ThreadData->m];
    memset(res_row, 0, ThreadData->m * sizeof(int));
    HANDLE minithreads[ThreadData->sums_cols];
    DWORD minithreadsId[ThreadData->sums_cols];
    for (int i = 0; i < ThreadData->sums_cols; i+=1){
        MiniData *args = malloc(sizeof(MiniData));
        *args = (MiniData){ThreadData->m, ThreadData->matrix, ThreadData->curr_row, ThreadData->sums_rows, i, ThreadData->batch_size,  res_row};
        minithreads[i] = CreateThread(NULL, 0, VerticalThreads, (void*)args, 0, &minithreadsId[i]);
        if (minithreads[i] == NULL) {
            printf("Error of connecting MINI flow: %lu\n", GetLastError());
            return 1;
        }
    }
    Sleep(2000);

    for (int i = 0; i < ThreadData->sums_cols; i++){
        WaitForSingleObject(minithreads[i], INFINITE);
    }
    
    printf("Flow ended.\n");
    return 0;
}




int main(void){
    static int arr[4][15] = {
        {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2},
        {3, 3, 2, 2, 1, 1, 0, 0, 3, 2, 1, 0, 3, 2, 1},
        {1, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3},
        {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2},
    };

    int *ptr = (int*)arr;
    int maxThreads = 8;
    int arrNum = 4;
    int arrLen = 15;
    int Batch = 5; // сколько чисел в срезе
    int miniThreadsN = arrLen / Batch; // сколько мини потоков на ряд
    int largeThreadsN = maxThreads / miniThreadsN; // сколько больших потоков
    int ThreadsN = miniThreadsN * largeThreadsN; // сколько всего потоков
    int RowsInThread = arrNum / largeThreadsN;  // сколько рядов на 1 большой поток

    HANDLE threads[largeThreadsN];
    DWORD threadsId[largeThreadsN];
    

    for (int i = 0; i < arrNum; i+=largeThreadsN){
        Data *args = malloc(sizeof(Data));
        *args = (Data){ arrNum, arrLen, ptr, i, RowsInThread, miniThreadsN, Batch};
        threads[i] = CreateThread(NULL, 0, HorizontalThreads, (void*)args, 0, &threadsId[i]);
        if (threads[i] == NULL) {
            printf("Error of connecting flow: %lu\n", GetLastError());
            return 1;
        }
    }

    for (int i = 0; i < largeThreadsN; i++){
        WaitForSingleObject(threads[i], INFINITE);
    }

    printf("all flows finished.\n");
    return 0;
}