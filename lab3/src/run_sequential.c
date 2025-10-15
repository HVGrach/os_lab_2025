#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s seed array_size\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        // Дитя: заменить текущий процесс на ./sequential_min_max c двумя аргументами
        execl("./sequential_min_max",
              "./sequential_min_max",
              argv[1], argv[2],
              (char*)NULL);
        // Если мы здесь — exec не удался
        perror("execl");
        _exit(127);
    }

    // Родитель: ждём завершения ребёнка
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("Child exited with code %d\n", code);
        return code;
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Child killed by signal %d\n", sig);
        return 128 + sig;
    } else {
        printf("Child ended unexpectedly\n");
        return 1;
    }
}
