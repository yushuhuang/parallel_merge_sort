#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <pthread.h>

#define N 8192

void merge(float *data, int l, int m, int r)
{
  int first = l;
  int second = m;
  float *work = (float *)calloc(r - l, sizeof(float));

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

void merge_sort(int num_of_elements, float *data)
{
  for (int k = 1; k < num_of_elements; k = 2 * k)    // ceil(log2(num_of_elements))
    for (int l = 0; l < num_of_elements; l += 2 * k) // (num_of_elements + 2 * k - 1) / (2 * k)
      merge(data, l, fminf(l + k, num_of_elements), fminf(l + 2 * k, num_of_elements));
}
