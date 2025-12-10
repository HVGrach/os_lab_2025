#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    // Дочерний процесс быстро завершается
    printf("Child: my pid = %d, exiting now\n", getpid());
    _exit(0);
  } else {
    // Родитель НИКОГДА не вызывает wait().
    // Пока родитель жив и не вызвал wait, дочерний процесс висит в состоянии Z (zombie).
    printf("Parent: my pid = %d, child pid = %d\n", getpid(), pid);
    printf("Now sleep 30 seconds.\n");
    sleep(30);
    printf("Parent: exiting without wait(). Zombie will be reapeddd by init.\n");
  }

  return 0;
}