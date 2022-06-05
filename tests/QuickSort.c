#include<stdio.h>
#include <stdlib.h>
#include <time.h>

void swap (float* a, float* b)
{
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

int Partition(float arr[], int low, int high){
    int i = low - 1;
    float pivot = arr[high];
    for(int j = low; j < high; j++) {
        if(arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[high], &arr[i+1]);
    return i+1;
}

void QuickSort(float arr[], int low, int high){
    if(low < high) {
        int pivot = Partition(arr, low, high);
        QuickSort(arr, low, pivot-1);
        QuickSort(arr, pivot+1, high);
    }
}

int main() {
    srand(time(0));
    int size = 500000;
    float arr[size];
    for (int i = 0; i < size; i++)
        arr[i] = (rand() / (double) RAND_MAX);
    QuickSort(arr, 0, size-1);
    printf("Array sorted using Quick Sort: ");
    for (int i = 0; i < size; i++)
        printf("%f ", arr[i]);
    printf("\n");
    return 0;
}
