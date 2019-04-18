#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#define N 8192
#define num_of_threads 8

typedef struct thread_args
{
  float *data;
  int start;
  int items;
} thread_args;

typedef struct merge_args
{
  int index;
  float *data;
  float *work;
  int interval;
  int num_of_elements;
} merge_args;

void merge(int l, int m, int r, float *data)
{
  int first = l;
  int second = m;
  float *work = (float *)malloc(sizeof(float) * (r - l));
  for (int i = 0; i < r - l; i++)
  {
    if (first < m && (second >= r || data[first] <= data[second]))
    {
      work[i] = data[first];
      first += 1;
    }
    else
    {
      work[i] = data[second];
      second += 1;
    }
  }
  memcpy(&data[l], work, (r - l) * sizeof(float));
  free(work);
}

void *final_merge(void *args)
{
  int index = ((merge_args *)args)->index;
  float *data = ((merge_args *)args)->data;
  float *work = ((merge_args *)args)->work;
  int interval = ((merge_args *)args)->interval;
  int num_of_elements = ((merge_args *)args)->num_of_elements;

  int l = index * 2 * interval;
  int m = fminf(l + interval, num_of_elements);
  int r = fminf(l + 2 * interval, num_of_elements);

  int first = l;
  int second = m;

  for (int i = l; i < r; i++)
  {
    if (first < m && (second >= r || data[first] <= data[second]))
    {
      work[i] = data[first];
      first += 1;
    }
    else
    {
      work[i] = data[second];
      second += 1;
    }
  }
  return NULL;
}

void *merge_sort(void *args)
{
  float *data = ((thread_args *)args)->data;
  int start = ((thread_args *)args)->start;
  int items = ((thread_args *)args)->items;

  for (int k = 1; k < items; k = 2 * k)
    for (int l = start; l < start + items; l += 2 * k)
      merge(l, fminf(l + k, start + items), fminf(l + 2 * k, start + items), data);
  return NULL;
}

void swap(float **a, float **b)
{
  float *tmp = *a;
  *a = *b;
  *b = tmp;
}

int pthread_sort(int num_of_elements, float *data)
{
  int items = (num_of_elements + num_of_threads - 1) / num_of_threads;
  // set args
  thread_args args[num_of_threads];
  int start = 0;
  for (int i = 0; i < num_of_threads; i++)
  {
    args[i].data = data;
    args[i].start = start;
    args[i].items = fminf(items, num_of_elements - start);
    start += items;
  }

  // Split into num_of_threads chunks of data and run in parallel
  pthread_t threads[num_of_threads];
  for (int i = 0; i < num_of_threads; i++)
    pthread_create(&threads[i], NULL, *merge_sort, &args[i]);

  for (int i = 0; i < num_of_threads; i++)
    pthread_join(threads[i], NULL);

  // final merge in parallel
  int max_tasks = (num_of_threads + 1) / 2;
  pthread_t *merge_threads = (pthread_t *)calloc(max_tasks, sizeof(pthread_t));
  merge_args *m_args = (merge_args *)calloc(max_tasks, sizeof(merge_args));
  float *work = (float *)malloc(num_of_elements * sizeof(float));

  int stride = 0;
  for (int k = 1; k < num_of_threads; k = k * 2)
  {
    int tasks = (num_of_threads + 2 * k - 1) / (2 * k);
    int interval = k * items;
    for (int i = 0; i < tasks; i++)
    {
      m_args[i].index = i;
      m_args[i].data = data;
      m_args[i].work = work;
      m_args[i].interval = interval;
      m_args[i].num_of_elements = num_of_elements;
    }
    for (int i = 0; i < tasks; i++)
      pthread_create(&merge_threads[i], NULL, *final_merge, &m_args[i]);
    for (int i = 0; i < tasks; i++)
      pthread_join(merge_threads[i], NULL);
    swap(&data, &work);
    stride += 1;
  }
  if (stride % 2 != 0)
  {
    swap(&data, &work);
    memcpy(data, work, num_of_elements * sizeof(float));
  }
  free(merge_threads);
  free(m_args);
  free(work);
  return 0;
}

// Test
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
  pthread_sort(N, data);

  // Verify
  float output_data[N];
  memcpy(output_data, data, N * sizeof(float));
  // printArr(output_data);
  qsort(data, N, sizeof(float), compare_function);
  // printArr(data);
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