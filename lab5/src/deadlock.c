#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

void *Thread1(void *arg) {
  (void)arg;

  printf("Thread1: locking m1\n");
  pthread_mutex_lock(&m1);
  printf("Thread1: m1 locked, sleep...\n");
  sleep(1);

  printf("Thread1: trying to lock m2\n");
  pthread_mutex_lock(&m2);
  printf("Thread1: got m2 (this message will likely never appear)\n");

  pthread_mutex_unlock(&m2);
  pthread_mutex_unlock(&m1);
  return NULL;
}

void *Thread2(void *arg) {
  (void)arg;

  printf("Thread2: locking m2\n");
  pthread_mutex_lock(&m2);
  printf("Thread2: m2 locked, sleep...\n");
  sleep(1);

  printf("Thread2: trying to lock m1\n");
  pthread_mutex_lock(&m1);
  printf("Thread2: got m1 (this message will likely never appear)\n");

  pthread_mutex_unlock(&m1);
  pthread_mutex_unlock(&m2);
  return NULL;
}

int main(void) {
  pthread_t t1, t2;

  if (pthread_create(&t1, NULL, Thread1, NULL) != 0) {
    perror("pthread_create");
    return 1;
  }

  if (pthread_create(&t2, NULL, Thread2, NULL) != 0) {
    perror("pthread_create");
    return 1;
  }

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("Main: done\n");
  return 0;
}