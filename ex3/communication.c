
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "elevator.h"
#include "elevator_io_types.h"
#include "fsm.h"
#include "requests.h"

#define PORT        14000
#define MAXLINE     1024
#define TIMEOUT_MS  150

pthread_mutex_t mtx;
int sockfd;
struct sockaddr_in servaddr;
int n, len;
Elevator e, b;

int standalone = 1, request = -1;

void* startCom(void* args){

    // Initialize the connection
    if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    char* ip = args;

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    struct timeval timeout;    
	timeout.tv_sec = 0;
	timeout.tv_usec = TIMEOUT_MS*1000;
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)  
		herror("setsockopt failed\n");

    int cnt;

    while(1) {

        e = fsm_getElevator();

        if(e.id == -1) {
            pthread_mutex_lock(&mtx);
            e.msg = -1;
            sendto(sockfd, (const Elevator *)&e, sizeof(e), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            n = recvfrom(sockfd, (Elevator *)&b, MAXLINE, MSG_CONFIRM, (struct sockaddr *) &servaddr, &len);
            pthread_mutex_unlock(&mtx);
            
            if(n < 1) {
                standalone = 1;
                fsm_setElevator(-1, "0");
            }
            else if(b.msg == -2){
                perror("Too many elevator to run");
                exit(EXIT_FAILURE);
            }
            else if(b.msg == 2) {
                cnt = 0;
                standalone = 0;
                fsm_setBackup(b);
            }
            else if(b.id != -1) {
                cnt = 0;
                standalone = 0;
                fsm_setElevator(b.id, b.ip);
            }

	    usleep(200*1000);
            continue;
        }

        request = -1;
        pthread_mutex_lock(&mtx);
        n = recvfrom(sockfd, (Elevator *)&b, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        pthread_mutex_unlock(&mtx);
        if(n < 1) {
            switch(errno) {
                case EAGAIN:
                    cnt++;
                break;
                default:
                    printf("error detected : errno from communication.c 2\n");
                break;
            }

            if(cnt > 10){
                standalone = 1;
                cnt = 0;
                continue;
            }
        }
        else {
            standalone = 0;
            cnt = 0;
        }

        switch(b.msg) {

            // too many elevators
            case -2:
                standalone = 1;
            break;

            case -1:
                e = fsm_getElevator();
                pthread_mutex_lock(&mtx);
                e.msg = 5;
                sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
                pthread_mutex_unlock(&mtx);
            break;

            // don't take the request
            case 0:
            break;

            // take the request
            case 1:
                fsm_onRequestButtonPress(b.floor, b.btn_type);
            break;

            case 4:
                fsm_setBackup( requests_clearAtGivenFloor( fsm_getElevator(), b.floor, b.btn_type ) );
            break;

            default:
            break;
        }

        usleep(200*1000);
    }
}


void sendBackup(void) {
    if(standalone) {
        return;
    }
    e = fsm_getElevator();
    pthread_mutex_lock(&mtx);
    e.msg = 0;
    sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    pthread_mutex_unlock(&mtx);
}


int sendRequest(int btn_floor, Button btn_type) {
    e = fsm_getElevator();
    int n, len;
    pthread_mutex_lock(&mtx);
    e.msg = 1;
    e.floor = btn_floor;
    e.btn_type = btn_type;
    sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, (Elevator *)&b, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
    pthread_mutex_unlock(&mtx);

    if(n < 1) {
    	standalone = 1;
    	return 0;
    }
    standalone = 0;
    return 1;
}


void sendClear(int floor, Button btn) {
    if(standalone) {
        return;
    }
    e = fsm_getElevator();
    pthread_mutex_lock(&mtx);
    e.msg = 4;
    e.floor = floor;
    e.btn_type = btn;
    sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    pthread_mutex_unlock(&mtx);
}


void sendObstructed(void) {
    if(standalone) {
        return;
    }
    e = fsm_getElevator();
    pthread_mutex_lock(&mtx);
    e.msg = 2;
    sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    pthread_mutex_unlock(&mtx);
}


void sendFree(void) {
    if(standalone) {
        return;
    }
    e = fsm_getElevator();
    pthread_mutex_lock(&mtx);
    e.msg = 3;
    sendto(sockfd, (const Elevator *)&e, sizeof(e), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    pthread_mutex_unlock(&mtx);
}


int isStandalone(void) {
    return standalone;
}
