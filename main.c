#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#include <memory.h>

static const long Num_To_Sort = 50000000;//original 1000000000
const int number_of_threads = 100;

//Sequential version of your sort: QUICKSORT
//Name: Bonaventure Biko
//Date: 4/20/2019
/*
 * Function to print elements of an array
 * */
void printArray(int arr[], int size)
{
    int i;
    for (i=0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}
/*
 * swap function is used to switch a's position to b's and vice versa
 * */
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
// If you're implementing the PSRS algorithm, you may ignore this section
/*
 * QUICKSORT
 * Algorithm and code referenced from this site: https://www.geeksforgeeks.org/quick-sort/
 * and
 * https://github.com/eduardlopez/quicksort-parallel/blob/master/quicksort-omp.h
 * */

void sort_s(int *arr, int low, int high)//Quicksort sequential algorithm
{

/* make low the Starting index; make high the Ending index */
    int i = low;
    int j = high;
    int pivot = arr[(low + high) / 2];//make pivot near-median element

    //partition code start
    while (i <= j)
    {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            swap(&arr[i], &arr[j]);
            i++;
            j--;
        }
    }
    //partition code end
    if (low < j){ sort_s(arr, low, j);  }//sort elememts in lower partition
    if (i< high){ sort_s(arr, i, high); }//sort elements in upper partition
}
// Parallel version of your sort
/*
 * Algorithm referenced from these sites:
 * https://www.geeksforgeeks.org/quick-sort/
 * and
 * https://github.com/eduardlopez/quicksort-parallel/blob/master/quicksort-omp.h
 * */
/*
 * THIS FUNCTION WILL RUN THE SEQUENTIAL CODE USING 2 TASKS RUNNING IN PARALLEL
 * */
void quickSort_p_internal(int *arr, int low,int high, int cutoff)
{
    int i = low;
    int j = high;
    int pivot = arr[(low + high) / 2];//make pivot near-median element

    //partition code start
    while (i <= j)
    {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            swap(&arr[i], &arr[j]);
            i++;
            j--;
        }
    }
    //partition code end
    if ( (high-low)<cutoff )
    {
        if (low < j){ quickSort_p_internal(arr, low, j, cutoff); }//sort elements in lower partition
        if (i < high){ quickSort_p_internal(arr, i, high, cutoff); }//sort elements in upper partition
    }
    else
    {
#pragma omp task
        { quickSort_p_internal(arr, low, j, cutoff); }
#pragma omp task
        { quickSort_p_internal(arr, i, high, cutoff); }
    }
}
/*
 * Main function for quicksort version in parallel
 * */
void sort_p(int *arr, int this_array_length, int numThreads)
{
    int cutoff = 1000;

#pragma omp parallel num_threads(numThreads)
    {
#pragma omp single nowait
        {
            quickSort_p_internal(arr, 0, this_array_length-1, cutoff);
        }
    }
}


//Main function
int main() {

    int *arr_s = malloc(sizeof(int) * Num_To_Sort);

    long chunk_size = Num_To_Sort / omp_get_max_threads();//original: omp_get_max_threads

    //printf("Initial_array_size::%d\n",*arr_s);

#pragma omp parallel num_threads(omp_get_max_threads())
    {
        int p = omp_get_thread_num();
        unsigned int seed = (unsigned int) time(NULL) + (unsigned int) p;
        long chunk_start = p * chunk_size;
        long chunk_end = chunk_start + chunk_size;

        //printf("chunk_size::%d\n",chunk_size);
        int loop = 0;
        for (long i = chunk_start; i < chunk_end; i++) {
            arr_s[i] = rand_r(&seed);
        }
    }

    //start of my code
    int array_size = Num_To_Sort;//sizeof(arr_s)/sizeof(arr_s[0]);
    int low = 0;
    int high = array_size-1;

    // Copy the array so that the sorting function can operate on it directly.
    // Note that this doubles the memory usage.
    // You may wish to test with slightly smaller arrays if you're running out of memory.
    int *arr_p = malloc(sizeof(int) * Num_To_Sort);
    memcpy(arr_p, arr_s, sizeof(int) * Num_To_Sort);

    struct timeval start, end;
    printf("Array Size:%d\n",array_size);
    printf("Timing sequential...\n");
    gettimeofday(&start, NULL);
    printf("Before Sorting:\n");
    printArray(arr_s, 20);
    sort_s(arr_s,low,high);
    printf("After Sorting:\n");
    printArray(arr_s, 20);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    free(arr_s);

    printf("Timing parallel...\n");
    gettimeofday(&start, NULL);
    printf("Before Sorting:\n");
    printArray(arr_p,20);
    sort_p(arr_p,Num_To_Sort,number_of_threads);
    printf("After Sorting:\n");
    printArray(arr_p, 20);
    gettimeofday(&end, NULL);
    printf("Took %f seconds\n\n", end.tv_sec - start.tv_sec + (double) (end.tv_usec - start.tv_usec) / 1000000);

    free(arr_p);

    return 0;
}

