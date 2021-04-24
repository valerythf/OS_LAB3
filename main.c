#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <chrono>
#include <thread>

long int * g_size_arr = nullptr;
long int * g_num_threads = nullptr;
long int *arr = nullptr;
long int *arr_single_proc = nullptr;


void merge_sort(long int* arr, long int left, long int right);
void merge(long int* arr, long int left, long int middle, long int right);
void *thread_merge_sort(void* arg);
void merge_sections(long int* arr, long int num_thread, long int size_sub_arr, long int aggregation);
void find_min(long int* arr);

int main(int argc, char *argv[]) {
    g_num_threads = new long int;
    *g_num_threads = atoi(argv[1]);
    g_size_arr = new long int;
    *g_size_arr = 1000000;
    arr = new long int[*g_size_arr];
    arr_single_proc = new long int[*g_size_arr];

    long int size_sub_arr = *g_size_arr / *g_num_threads;
    struct timeval start, end;
    double time_spent, time_single_spent;

    long int i;
    for (i = 0; i < *g_size_arr; ++i) {
        arr[i] = rand() % 1000000;
        arr_single_proc[i] = arr[i];
    }

    pthread_t threads[*g_num_threads];
    gettimeofday(&start, NULL);
    for (i = 0; i < *g_num_threads; ++i) {
        int err = pthread_create(&threads[i], NULL, thread_merge_sort, (void *) i);
        if (err){
            printf("ERROR return code from pthread_create() is %d\n", err);
            exit(-1);
        }
    }
    for(i = 0; i < *g_num_threads; ++i)
        pthread_join(threads[i], NULL);

    merge_sections(arr, *g_num_threads, size_sub_arr, 1);
    gettimeofday(&end, NULL);
    time_spent = ((double) ((double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec)));
    printf("\n\tTime for %ld proccessing: %f s\n", *g_num_threads, time_spent);


    find_min(arr);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));


    gettimeofday(&start, NULL);
    merge_sort(arr_single_proc, 0, *g_size_arr);
    gettimeofday(&end, NULL);
    time_single_spent = ((double) ((double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec)));
    printf("\tTime for single proc: %f s\n", time_single_spent);

    find_min(arr_single_proc);

    // metrics
    printf("\n\n\n\tResults\n");
    printf("\tУскорение S(n)=T(1)/T(n):         %f\n", time_single_spent / time_spent);
    printf("\tЭффективность E(n)=S(n)/n=T(1)/(nT(n)): %f\n\n", time_single_spent / (time_spent * (*g_num_threads)));


    printf("\n");
    delete g_size_arr;
    delete g_num_threads;
    delete[] arr;
    delete[] arr_single_proc;

    return 0;
}

void merge(long int* arr, long int left, long int middle, long int right) {
    long int k = 0;
    long int left_length = middle - left + 1;
    long int right_length = right - middle;
    long int left_array[left_length];
    long int right_array[right_length];
    long int i;
    long int j;
    for (i = 0; i < left_length; ++i)
        left_array[i] = arr[left + i];

    for (j = 0; j < right_length; ++j)
        right_array[j] = arr[middle + 1 + j];

    i = 0;
    j = 0;

    while (i < left_length && j < right_length) {
        if (left_array[i] <= right_array[j]) {
            arr[left + k] = left_array[i];
            ++i;
        } else {
            arr[left + k] = right_array[j];
            ++j;
        }
        ++k;
    }

    while (i < left_length) {
        arr[left + k] = left_array[i];
        ++k;
        ++i;
    }
    while (j < right_length) {
        arr[left + k] = right_array[j];
        ++k;
        ++j;
    }
}

void merge_sort(long int* arr, long int left, long int right) {
    long int middle;
    if (left < right) {
        middle = left + (right - left) / 2;
        merge_sort(arr, left, middle);
        merge_sort(arr, middle + 1, right);
        merge(arr, left, middle, right);
    }
}

void merge_sections(long int* arr, long int num_thread, long int size_sub_arr, long int aggregation) {
    long int left;
    long int right;
    long int middle;
    for(long int i = 0; i < num_thread; i++) {
        left = i * (size_sub_arr * aggregation);
        right = left + ((i + 2) * size_sub_arr * aggregation) - 1;
        middle = left + (size_sub_arr * aggregation) - 1;
        if (right >= *g_size_arr) {
            right = *g_size_arr - 1;
        }
        merge(arr, left, middle, right);
    }
    if (num_thread / 2 >= 1) {
        merge_sections(arr, num_thread / 2, size_sub_arr, aggregation * 2);
    }
}


void *thread_merge_sort(void* arg) {
    long int thread_id = (long int)arg;
    long int left = thread_id * (*g_size_arr / *g_num_threads);
    long int right = (thread_id + 1) * (*g_size_arr / *g_num_threads) - 1;
    if (thread_id == *g_num_threads - 1)
        right += (*g_size_arr % *g_num_threads);
    long int middle = left + (right - left) / 2;
    if (left < right) {
        merge_sort(arr, left, right);
        merge_sort(arr, left + 1, right);
        merge(arr, left, middle, right);
    }
    pthread_exit(nullptr);
}

void find_min(long int* arr) {
    printf("\tMinimum of array is  %ld\n", arr[0]);
}
