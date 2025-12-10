// tcpclient.c
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  int fd;
  ssize_t nread;
  char *buf = NULL;
  struct sockaddr_in servaddr;

  if (argc < 4) {
    fprintf(stderr, "Usage: %s <IP> <PORT> <BUFSIZE>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *ip_str = argv[1];
  int port = atoi(argv[2]);
  int bufsize = atoi(argv[3]);

  if (port <= 0 || bufsize <= 0) {
    fprintf(stderr, "Port and BUFSIZE must be positive integers\n");
    exit(EXIT_FAILURE);
  }

  buf = malloc(bufsize);
  if (buf == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creating");
    free(buf);
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip_str, &servaddr.sin_addr) <= 0) {
    perror("bad address");
    free(buf);
    close(fd);
    exit(EXIT_FAILURE);
  }

  servaddr.sin_port = htons(port);

  if (connect(fd, (SADDR *)&servaddr, sizeof(servaddr)) < 0) {
    perror("connect");
    free(buf);
    close(fd);
    exit(EXIT_FAILURE);
  }

  write(STDOUT_FILENO, "Input message to send\n", 22);
  while ((nread = read(STDIN_FILENO, buf, bufsize)) > 0) {
    if (write(fd, buf, nread) < 0) {
      perror("write");
      free(buf);
      close(fd);
      exit(EXIT_FAILURE);
    }
  }

  if (nread < 0) {
    perror("read");
  }

  free(buf);
  close(fd);
  return 0;
}