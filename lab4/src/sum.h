#ifndef SUM_H
#define SUM_H

struct SumArgs {
  int *array;
  int begin; // индекс начала (включительно)
  int end;   // индекс конца (исключительно)
};

int Sum(const struct SumArgs *args);

#endif