#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 20000
#define MAXLINE 1024

int main() {
  int sockfd;
	struct sockaddr_in servaddr, cliaddr;

  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

  if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

  int n, len;
  char* msg;
  char hello[30] = "hello from server !";
  while(1) {
    n = recvfrom(sockfd, (char*) msg, MAXLINE, NULL, ( struct sockaddr *) &cliaddr, &len);
    printf("received: %s\n", msg);
    sendto(sockfd, (char*) hello, sizeof(e), 0, (const struct sockaddr *) &cliaddr, len);
    sleep(1)
  }

}
