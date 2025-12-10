// udpclient.c
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv) {
  int sockfd, n;
  char *sendline = NULL;
  char *recvline = NULL;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <IPaddress of server> <PORT> <BUFSIZE>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *ip_str = argv[1];
  int port = atoi(argv[2]);
  int bufsize = atoi(argv[3]);

  if (port <= 0 || bufsize <= 0) {
    fprintf(stderr, "Port and BUFSIZE must be positive integers\n");
    exit(EXIT_FAILURE);
  }

  sendline = malloc(bufsize);
  recvline = malloc(bufsize + 1);
  if (sendline == NULL || recvline == NULL) {
    perror("malloc");
    free(sendline);
    free(recvline);
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip_str, &servaddr.sin_addr) < 0) {
    perror("inet_pton problem");
    free(sendline);
    free(recvline);
    exit(EXIT_FAILURE);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem");
    free(sendline);
    free(recvline);
    exit(EXIT_FAILURE);
  }

  write(STDOUT_FILENO, "Enter string\n", 13);

  while ((n = read(STDIN_FILENO, sendline, bufsize)) > 0) {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("sendto problem");
      close(sockfd);
      free(sendline);
      free(recvline);
      exit(EXIT_FAILURE);
    }

    if ((n = recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL)) == -1) {
      perror("recvfrom problem");
      close(sockfd);
      free(sendline);
      free(recvline);
      exit(EXIT_FAILURE);
    }

    recvline[n] = '\0';
    printf("REPLY FROM SERVER= %s\n", recvline);
  }

  if (n < 0) {
    perror("read");
  }

  close(sockfd);
  free(sendline);
  free(recvline);
  return 0;
}