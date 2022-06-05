from ltypes import i32, f32
import random
# from numpy import empty

def _swap(arr: f32[500000], i: i32, j: i32):
    tmp: f32
    tmp = arr[i]
    arr[i] = arr[j]
    arr[j] = tmp

def Partition(arr: f32[500000], low: i32, high: i32) -> i32:
    i: i32
    i = low - 1
    pivot: f32
    pivot = arr[high]
    j: i32
    for j in range(low, high):
        if (arr[j] <= pivot):
            i += 1
            _swap(arr, i, j)
    _swap(arr, high, i+1)
    return i+1

def QuickSort(arr: f32[500000], low: i32, high: i32):
    if (low < high):
        pivot: i32
        pivot = Partition(arr, low, high)
        QuickSort(arr, low, pivot-1)
        QuickSort(arr, pivot+1, high)

def main():
    arr: f32[500000]
    # arr = empty(500000)
    i: i32
    for i in range(500000):
        arr[i] = random.random()
    QuickSort(arr, 0, 500000-1)
    print("Array sorted using Quick Sort: ")
    for i in range(500000):
        print(arr[i])

main()
