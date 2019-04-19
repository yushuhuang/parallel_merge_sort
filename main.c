#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 8192

void merge_sort(int num_of_elements, float *data);

void printArr(float *arr)
{
    for (int i = 0; i < N; i++)
        printf("%.3f ", arr[i]);
    printf("\n");
}

int compare_function(const void *a, const void *b)
{
    if (*((float *)a) < *((float *)b))
        return -1;
    else if (*((float *)a) > *((float *)b))
        return 1;
    else
        return 0;
}

float float_rand()
{
    float scale = rand() / (float)RAND_MAX;
    return -100 + scale * 200;
}

int main(void)
{
    float data[N];
    for (int i = 0; i < N; i++)
        data[i] = float_rand();

    merge_sort(N, data);

    // Verify
    float output_data[N];
    memcpy(output_data, data, N * sizeof(float));
    qsort(data, N, sizeof(float), compare_function);

    int i = 0;
    for (i = 0; i < N; i++)
    {
        if (output_data[i] != data[i])
        {
            printf("Fail at %d\n", i);
            break;
        }
    }
    if (i == N)
        printf("Pass!\n");
    return 0;
}