
#include <arpa/inet.h> 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> 
#include <sys/socket.h>
#include <unistd.h> 

#define MAX 80
#define PORT 8080
 
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
 
    if (connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection failed\n");
        exit(EXIT_FAILURE);
    }

    char buff[MAX];
    int n;
    while(1) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n') {}
            
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sockfd, buff, sizeof(buff));
        printf("From Server : %s", buff);
        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
 
    // close the socket
    close(sockfd);
}
