#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#define N 8192

typedef struct thread_args
{
  float *data;
  float *work;
  int index;
  int k;
  int num_of_elements;
} thread_args;

void *merge(void *args)
{
  float *data = ((thread_args *)args)->data;
  float *work = ((thread_args *)args)->work;
  int index = ((thread_args *)args)->index;
  int k = ((thread_args *)args)->k;
  int num_of_elements = ((thread_args *)args)->num_of_elements;

  int l = index * 2 * k;
  int m = fminf(l + k, num_of_elements);
  int r = fminf(l + 2 * k, num_of_elements);

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

void swap(float **a, float **b)
{
  float *tmp = *a;
  *a = *b;
  *b = tmp;
}

void merge_sort(int num_of_elements, float *data)
{
  int max_tasks = (num_of_elements + 1) / 2;
  pthread_t *threads = (pthread_t *)calloc(max_tasks, sizeof(pthread_t));
  thread_args *args = (thread_args *)calloc(max_tasks, sizeof(thread_args));
  float *work = (float *)malloc(num_of_elements * sizeof(float));

  int stride = 0;
  for (int k = 1; k < num_of_elements; k = 2 * k)
  {
    int tasks = (num_of_elements + 2 * k - 1) / (2 * k);
    for (int i = 0; i < tasks; i++)
    {
      args[i].data = data;
      args[i].work = work;
      args[i].index = i;
      args[i].k = k;
      args[i].num_of_elements = num_of_elements;
    }
    for (int i = 0; i < tasks; i++)
      pthread_create(&threads[i], NULL, *merge, &args[i]);
    for (int i = 0; i < tasks; i++)
      pthread_join(threads[i], NULL);
    swap(&data, &work);
    stride += 1;
  }
  if (stride % 2 != 0)
  {
    swap(&data, &work);
    memcpy(data, work, num_of_elements * sizeof(float));
  }
  free(threads);
  free(args);
  free(work);
}