#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              fprintf(stderr, "seed must be a positive integer\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              fprintf(stderr, "array_size must be a positive integer\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              fprintf(stderr, "pnum must be a positive integer\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files|-f]\n",
           argv[0]);
    return 1;
  }

  if (pnum > array_size) {
    // не запрещаем, но предупредим — часть процессов получит пустые диапазоны
    fprintf(stderr, "Warning: pnum (%d) > array_size (%d); some chunks will be empty.\n",
            pnum, array_size);
  }

  int *array = malloc(sizeof(int) * array_size);
  if (!array) {
    perror("malloc array");
    return 1;
  }
  GenerateArray(array, array_size, seed);

  // заранее создадим пайпы (если нужен pipe-режим)
  int (*pipefd)[2] = NULL;
  if (!with_files) {
    pipefd = malloc(sizeof(int[2]) * (size_t)pnum);
    if (!pipefd) { perror("malloc pipefd"); free(array); return 1; }
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipefd[i]) == -1) {
        perror("pipe");
        free(pipefd);
        free(array);
        return 1;
      }
    }
  }

  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // разобьём массив на pnum кусков: первые 'rem' кусков на 1 элемент длиннее
  int chunk = array_size / pnum;
  int rem = array_size % pnum;

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      if (child_pid == 0) {
        // child process
        int extra = (i < rem) ? 1 : 0;
        int begin = i * chunk + (i < rem ? i : rem);
        int end = begin + chunk + extra;

        struct MinMax mm = GetMinMax(array, (unsigned)begin, (unsigned)end);

        if (with_files) {
          char fname[256];
          snprintf(fname, sizeof(fname), "minmax_%d.txt", i);
          FILE *f = fopen(fname, "w");
          if (!f) { perror("fopen"); _exit(1); }
          // пишем два числа в текстовом виде
          fprintf(f, "%d %d\n", mm.min, mm.max);
          fclose(f);
        } else {
          // pipe: ребёнок пишет, родитель читает
          // закрыть лишний конец
          close(pipefd[i][0]); // не читаем в ребёнке
          if (write(pipefd[i][1], &mm.min, sizeof(mm.min)) != sizeof(mm.min)) _exit(1);
          if (write(pipefd[i][1], &mm.max, sizeof(mm.max)) != sizeof(mm.max)) _exit(1);
          close(pipefd[i][1]);
        }
        _exit(0);
      } else {
        // parent
        active_child_processes += 1;
        if (!with_files) {
          // родителю не нужен write-end
          close(pipefd[i][1]);
        }
      }

    } else {
      printf("Fork failed!\n");
      if (!with_files && pipefd) {
        for (int k = 0; k < i; k++) close(pipefd[k][0]);
        free(pipefd);
      }
      free(array);
      return 1;
    }
  }

  while (active_child_processes > 0) {
    int status = 0;
    if (wait(&status) == -1) {
      perror("wait");
      break;
    }
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char fname[256];
      snprintf(fname, sizeof(fname), "minmax_%d.txt", i);
      FILE *f = fopen(fname, "r");
      if (!f) { perror("fopen read"); free(array); if (pipefd) free(pipefd); return 1; }
      if (fscanf(f, "%d %d", &min, &max) != 2) {
        fprintf(stderr, "failed to read results from %s\n", fname);
        fclose(f);
        free(array);
        if (pipefd) free(pipefd);
        return 1;
      }
      fclose(f);
      // по желанию можно удалить файл: remove(fname);
    } else {
      // читаем два int из пайпа i
      ssize_t r1 = read(pipefd[i][0], &min, sizeof(min));
      ssize_t r2 = read(pipefd[i][0], &max, sizeof(max));
      close(pipefd[i][0]);
      if (r1 != sizeof(min) || r2 != sizeof(max)) {
        fprintf(stderr, "pipe read failed for chunk %d\n", i);
        free(array);
        free(pipefd);
        return 1;
      }
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  if (pipefd) free(pipefd);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
