
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#define MAX 80 
#define PORT 8080 
   
int main() 
{ 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    bzero(&servaddr, sizeof(servaddr)); 
   
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
   
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
   
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(EXIT_FAILURE); 
    } 
    len = sizeof(cli); 
   
    connfd = accept(sockfd, (sockaddr*)&cli, &len); 
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(EXIT_FAILURE); 
    }

    char buff[MAX]; 
    int n; 
    
    while(1) { 
        bzero(buff, MAX); 
   
        read(connfd, buff, sizeof(buff)); 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') {}
   
        write(connfd, buff, sizeof(buff)); 
   
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
   
    // After chatting close the socket 
    close(sockfd); 
}
