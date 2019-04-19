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

void *sub_merge_sort(void *args)
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

void merge_sort(int num_of_elements, float *data)
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
    pthread_create(&threads[i], NULL, *sub_merge_sort, &args[i]);

  for (int i = 0; i < num_of_threads; i++)
    pthread_join(threads[i], NULL);

  // final merge
  // for (int i = 1; i < num_of_threads; i = i * 2)
  //   for (int l = 0; l < num_of_elements; l += 2 * i * items)
  //     merge(l, fminf(l + i * items, num_of_elements), fminf(l + 2 * i * items, num_of_elements), data);

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
}