#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/time.h>
#include <pthread.h>

#include "sum.h"

// ЛОКАЛЬНАЯ реализация GenerateArray (аналог из ЛР3)
void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
  srand(seed);
  for (unsigned int i = 0; i < array_size; i++) {
    // Можно сделать числа поменьше по модулю, чтобы не было переполнений при сумме,
    // но для ТЗ это не критично.
    array[i] = rand();
  }
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  int sum = Sum(sum_args);
  // Костыльный, но простой способ вернуть int через void*
  return (void *)(size_t)sum;
}

static void print_usage(const char *prog) {
  printf("Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n",
         prog);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  // Разбор аргументов командной строки
  while (true) {
    static struct option options[] = {
        {"threads_num", required_argument, 0, 0},
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    if (c == 0) {
      switch (option_index) {
      case 0:
        threads_num = (uint32_t)atoi(optarg);
        if (threads_num == 0) {
          fprintf(stderr, "threads_num must be > 0\n");
          return 1;
        }
        break;
      case 1:
        seed = (uint32_t)atoi(optarg);
        if ((int)seed <= 0) {
          fprintf(stderr, "seed must be > 0\n");
          return 1;
        }
        break;
      case 2:
        array_size = (uint32_t)atoi(optarg);
        if (array_size == 0) {
          fprintf(stderr, "array_size must be > 0\n");
          return 1;
        }
        break;
      default:
        break;
      }
    } else {
      print_usage(argv[0]);
      return 1;
    }
  }

  if (threads_num == 0 || array_size == 0 || seed == 0) {
    print_usage(argv[0]);
    return 1;
  }

  if (threads_num > array_size) {
    fprintf(stderr,
            "Warning: threads_num (%u) > array_size (%u); some chunks may be empty.\n",
            threads_num, array_size);
  }

  // Генерация массива (НЕ входит в замер времени)
  int *array = malloc(sizeof(int) * array_size);
  if (!array) {
    perror("malloc array");
    return 1;
  }

  GenerateArray(array, array_size, seed);

  // Подготовка структур для потоков
  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  if (!threads) {
    perror("malloc threads");
    free(array);
    return 1;
  }

  struct SumArgs *args =
      malloc(sizeof(struct SumArgs) * threads_num);
  if (!args) {
    perror("malloc args");
    free(threads);
    free(array);
    return 1;
  }

  // Разбиение массива на куски
  int chunk = (int)(array_size / threads_num);
  int rem = (int)(array_size % threads_num);

  int current = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    int extra = (i < (uint32_t)rem) ? 1 : 0;
    args[i].array = array;
    args[i].begin = current;
    args[i].end = current + chunk + extra;
    current = args[i].end;
  }

  // Замер времени только суммирования
  struct timeval start_time;
  struct timeval finish_time;

  gettimeofday(&start_time, NULL);

  // Создание потоков
  for (uint32_t i = 0; i < threads_num; i++) {
    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i]) != 0) {
      perror("pthread_create");
      free(args);
      free(threads);
      free(array);
      return 1;
    }
  }

  int total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    void *res = NULL;
    if (pthread_join(threads[i], &res) != 0) {
      perror("pthread_join");
      free(args);
      free(threads);
      free(array);
      return 1;
    }
    int sum = (int)(size_t)res;
    total_sum += sum;
  }

  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  printf("Total: %d\n", total_sum);
  printf("Elapsed time: %f ms\n", elapsed_time);

  free(args);
  free(threads);
  free(array);

  return 0;
}