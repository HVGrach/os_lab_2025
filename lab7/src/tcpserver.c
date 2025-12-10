// tcpserver.c
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  const size_t kSize = sizeof(struct sockaddr_in);

  int lfd, cfd;
  ssize_t nread;
  char *buf = NULL;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <PORT> <BUFSIZE>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[1]);
  int bufsize = atoi(argv[2]);

  if (port <= 0 || bufsize <= 0) {
    fprintf(stderr, "Port and BUFSIZE must be positive integers\n");
    exit(EXIT_FAILURE);
  }

  buf = malloc(bufsize);
  if (buf == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    free(buf);
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, kSize);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(lfd, (SADDR *)&servaddr, kSize) < 0) {
    perror("bind");
    free(buf);
    close(lfd);
    exit(EXIT_FAILURE);
  }

  if (listen(lfd, 5) < 0) {
    perror("listen");
    free(buf);
    close(lfd);
    exit(EXIT_FAILURE);
  }

  while (1) {
    unsigned int clilen = kSize;

    if ((cfd = accept(lfd, (SADDR *)&cliaddr, &clilen)) < 0) {
      perror("accept");
      free(buf);
      close(lfd);
      exit(EXIT_FAILURE);
    }
    printf("connection established\n");

    while ((nread = read(cfd, buf, bufsize)) > 0) {
      if (write(STDOUT_FILENO, buf, nread) < 0) {
        perror("write");
        break;
      }
    }

    if (nread < 0) {
      perror("read");
      close(cfd);
      free(buf);
      close(lfd);
      exit(EXIT_FAILURE);
    }
    close(cfd);
  }

  // формально недостижимо, но оставим
  free(buf);
  close(lfd);
  return 0;
}