
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "con_load.h"
#include "elevator_io_device.h"
#include "elevator.h"
#include "fsm.h"
#include "timer.h"
#include "communication.h"


int main(int argc, char *argv[]){
    printf("Started!\n");

    // Initialize the elevator hardware
    // Doesn't verify if the port given is valid
    char* ip;
    if(argc == 1) {
        elev_init("15657");
        strcpy(ip, "127.0.0.1");
    }
    else if(argc == 2 || argc == 4 || argc > 5 || strcmp(argv[1], "--port") != 0 || strcmp(argv[3], "--ip") != 0 || strcmp(argv[1], "--ip") != 0) {
        printf("Usage: ./elevator --port <port> --ip <IP>\n or \n./elevator --ip <IP>");
        exit(EXIT_FAILURE);
    }
    else {
    
    	if(argc == 5) {
    	    elev_init(argv[2]);
    	    strcpy(ip, argv[4]);
    	}
    	else if(argc == 3 && strcmp(argv[1], "--port") == 0) {
    	    elev_init(argv[2]);
    	    strcpy(ip, "127.0.0.1");
    	}
    	else {
    	    elev_init("15657");
    	    strcpy(ip, argv[2]);
    	}
    }
    
    int inputPollRate_ms = 25;
    con_load("elevator.con",
        con_val("inputPollRate_ms", &inputPollRate_ms, "%d")
    )
    
    // Initialize the elevator
    ElevInputDevice input = elevio_getInputDevice();    
    if(input.floorSensor() == -1){
        fsm_onInitBetweenFloors();
    }
    fsm_setElevator(-1, "0");

    // Initialize the communication thread
    pthread_t com;
    pthread_create(&com, NULL, startCom, ip);
    printf("com ");
    
    while(1){

        { // Send current state for backup
            sendBackup();
        }

        { // Request button
            static int prev[N_FLOORS][N_BUTTONS], sent[N_FLOORS][N_BUTTONS], cnt[N_FLOORS][N_BUTTONS];

            for(int f = 0; f < N_FLOORS; f++) {
                for(int b = 0; b < N_BUTTONS; b++) {

                    int v = input.requestButton(f, b);
                    if(v  &&  v != prev[f][b]){

                        if(b != B_Cab && !isStandalone()) {
                            sent[f][b] = sendRequest(f, b);
                            if(!sent[f][b]) {
                                fsm_onRequestButtonPress(f, b);
                            }
                        }
                        else {
                            fsm_onRequestButtonPress(f, b);
                            sent[f][b] = 1;
                        }
                    }
                    prev[f][b] = v;

                    // Resend the hall request 5 times if the server fails to respond
                    if(!sent[f][b] && cnt[f][b] < 5 && b != B_Cab) {
                        cnt[f][b]++;
                        sent[f][b] = sendRequest(f, b);
                    }
                    else {
                        cnt[f][b] = 0;
                        sent[f][b] = 1;
                    }
                }
            }
        }
        
        { // Floor sensor
            static int prev = -1;
            int f = input.floorSensor();
            if(f != -1  &&  f != prev){
                fsm_onFloorArrival(f);
            }
            prev = f;
        }
        
        { // Timer + Obstruction sensor
            int f = input.obstruction();
            if(f){
                sendObstructed();
                fsm_onDoorObstruction();
            }
            else if(timer_timedOut()){
                sendFree();
                timer_stop();
                fsm_onDoorTimeout();
            }
        }
        
        usleep(inputPollRate_ms*1000);
    }
}
