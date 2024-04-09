#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 30000
#define MAXLINE 1024

int main() {

  struct sockaddr_in servaddr;
  int sockfd;
  
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  struct timeval timeout;    
  timeout.tv_sec = 0;
  timeout.tv_usec = 500*1000;
  if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)  
    herror("setsockopt failed\n");

  int n, len;
  char* msg;
  char hello[30] = "hello from client !";
  while(1) {
    sendto(sockfd, (char*) hello, sizeof(e), 0, (const struct sockaddr *) &servaddr, len);
    n = recvfrom(sockfd, (char*) msg, MAXLINE, NULL, (struct sockaddr *) &servaddr, &len);
    printf("msg: %s\n", msg);
    sleep(1);
  }

  return 0;
}
