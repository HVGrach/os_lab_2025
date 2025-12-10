#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
uint64_t global_result = 1;
uint64_t global_mod = 1;

struct FactArgs {
  uint64_t begin; // включительно
  uint64_t end;   // включительно
};

void *ThreadFact(void *args) {
  struct FactArgs *fa = (struct FactArgs *)args;

  uint64_t local = 1;
  for (uint64_t i = fa->begin; i <= fa->end; i++) {
    local = (local * (i % global_mod)) % global_mod;
  }

  // Критическая секция: обновляем общий результат
  pthread_mutex_lock(&mutex);
  global_result = (global_result * local) % global_mod;
  pthread_mutex_unlock(&mutex);

  return NULL;
}

static void print_usage(const char *prog) {
  printf("Usage: %s -k <num> --pnum=<num> --mod=<num>\n", prog);
}

int main(int argc, char **argv) {
  uint64_t k = 0;
  uint32_t pnum = 0;
  uint64_t mod = 0;

  // Разбор аргументов командной строки
  while (1) {
    static struct option options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 0},
        {"mod", required_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "k:", options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'k':
      k = strtoull(optarg, NULL, 10);
      break;
    case 0:
      if (option_index == 1) {
        pnum = (uint32_t)strtoul(optarg, NULL, 10);
      } else if (option_index == 2) {
        mod = strtoull(optarg, NULL, 10);
      }
      break;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  if (k == 0 || pnum == 0 || mod == 0) {
    print_usage(argv[0]);
    return 1;
  }

  global_mod = mod;
  global_result = 1;

  if (pnum > k)
    pnum = (uint32_t)k; // нет смысла больше потоков, чем чисел

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  struct FactArgs *args = malloc(sizeof(struct FactArgs) * pnum);
  if (!threads || !args) {
    perror("malloc");
    free(threads);
    free(args);
    return 1;
  }

  // Разбиваем отрезок [1, k] на pnum подотрезков
  uint64_t base = k / pnum;
  uint64_t rem = k % pnum;

  uint64_t current = 1;
  for (uint32_t i = 0; i < pnum; i++) {
    uint64_t extra = (i < rem) ? 1 : 0;
    args[i].begin = current;
    args[i].end = current + base + extra - 1;
    current = args[i].end + 1;

    if (pthread_create(&threads[i], NULL, ThreadFact, &args[i]) != 0) {
      perror("pthread_create");
      free(threads);
      free(args);
      return 1;
    }
  }

  for (uint32_t i = 0; i < pnum; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("pthread_join");
      free(threads);
      free(args);
      return 1;
    }
  }

  printf("k! mod mod = %llu (k=%llu, mod=%llu, pnum=%u)\n",
         (unsigned long long)global_result,
         (unsigned long long)k,
         (unsigned long long)mod,
         pnum);

  free(threads);
  free(args);

  return 0;
}