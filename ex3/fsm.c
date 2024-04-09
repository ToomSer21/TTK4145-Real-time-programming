
#include "fsm.h"

#include <stdio.h>

#include "con_load.h"
#include "elevator.h"
#include "elevator_io_device.h"
#include "requests.h"
#include "timer.h"

static Elevator             elevator;
static ElevOutputDevice     outputDevice;


static void __attribute__((constructor)) fsm_init(){
    elevator = elevator_uninitialized();
    
    con_load("elevator.con",
        con_val("doorOpenDuration_s", &elevator.config.doorOpenDuration_s, "%lf")
        con_enum("clearRequestVariant", &elevator.config.clearRequestVariant,
            con_match(CV_All)
            con_match(CV_InDirn)
        )
    )
    
    outputDevice = elevio_getOutputDevice();
}

static void setAllLights(Elevator es){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            outputDevice.requestButtonLight(floor, btn, es.requests[floor][btn]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
    outputDevice.motorDirection(D_Down);
    elevator.dirn = D_Down;
    elevator.behaviour = EB_Moving;
    elevator.msg = -1;
    elevator.id = -1;
    strcpy(elevator.ip, "0");
}


void fsm_onRequestButtonPress(int btn_floor, Button btn_type){
    printf("\n\n%s(%d, %s)\n", __FUNCTION__, btn_floor, elevio_button_toString(btn_type));
    elevator_print(elevator);
    
    switch(elevator.behaviour){
    case EB_DoorOpen:
        if(requests_shouldClearImmediately(elevator, btn_floor, btn_type)){
            timer_start(elevator.config.doorOpenDuration_s);
        } else {
            elevator.requests[btn_floor][btn_type] = 1;
        }
        break;

    case EB_Moving:
        elevator.requests[btn_floor][btn_type] = 1;
        break;
        
    case EB_Idle:    
        elevator.requests[btn_floor][btn_type] = 1;
        DirnBehaviourPair pair = requests_chooseDirection(elevator);
        elevator.dirn = pair.dirn;
        elevator.behaviour = pair.behaviour;
        switch(pair.behaviour){
        case EB_DoorOpen:
            outputDevice.doorLight(1);
            timer_start(elevator.config.doorOpenDuration_s);
            elevator = requests_clearAtCurrentFloor(elevator);
            break;

        case EB_Moving:
            outputDevice.motorDirection(elevator.dirn);
            break;
            
        case EB_Idle:
            break;
        }
        break;
    }
    
    setAllLights(elevator);
    
    printf("\nNew state:\n");
    elevator_print(elevator);
}




void fsm_onFloorArrival(int newFloor){
    printf("\n\n%s(%d)\n", __FUNCTION__, newFloor);
    elevator_print(elevator);
    
    elevator.floor = newFloor;
    elevator.prev_floor = newFloor;
    
    outputDevice.floorIndicator(elevator.floor);
    
    switch(elevator.behaviour){
    case EB_Moving:
        if(requests_shouldStop(elevator)){
            outputDevice.motorDirection(D_Stop);
            outputDevice.doorLight(1);
            elevator = requests_clearAtCurrentFloor(elevator);
            timer_start(elevator.config.doorOpenDuration_s);
            setAllLights(elevator);
            elevator.behaviour = EB_DoorOpen;
        }
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator); 
}




void fsm_onDoorTimeout(void){
    printf("\n\n%s()\n", __FUNCTION__);
    elevator_print(elevator);
    
    switch(elevator.behaviour){
    case EB_DoorOpen:;
        DirnBehaviourPair pair = requests_chooseDirection(elevator);
        elevator.dirn = pair.dirn;
        elevator.behaviour = pair.behaviour;
        
        switch(elevator.behaviour){
        case EB_DoorOpen:
            timer_start(elevator.config.doorOpenDuration_s);
            elevator = requests_clearAtCurrentFloor(elevator);
            setAllLights(elevator);
            break;
        case EB_Moving:
        case EB_Idle:
            outputDevice.doorLight(0);
            outputDevice.motorDirection(elevator.dirn);
            break;
        }
        
        break;
    default:
        break;
    }

    printf("\nNew state:\n");
    elevator_print(elevator);
}




void fsm_onDoorObstruction(void){
    printf("\n\n%s()\n", __FUNCTION__);
    elevator_print(elevator);

    switch(elevator.behaviour){
    case EB_DoorOpen:
        timer_start(elevator.config.doorOpenDuration_s);
        break;

    case EB_Idle:
    case EB_Moving:
        break;
    default:
        break;
    }

    printf("\nNew state:\n");
    elevator_print(elevator);
}


Elevator fsm_getElevator(void) {
    return elevator;
}


void fsm_setElevator(int id, char ip[20]) {
    elevator.id = id;
    printf("char %d\n", id);
    strcpy(elevator.ip, ip);
}


void fsm_setBackup(Elevator b) {
    elevator.id = b.id;
    strcpy(elevator.ip, b.ip);
    elevator.floor = b.floor;
    elevator.config = b.config;
    elevator.dirn = b.dirn;
    elevator.behaviour = b.behaviour;

    for(int i = 0; i < N_FLOORS; i++){
        for(int j = 0; j < N_BUTTONS; j++){
            elevator.requests[i][j] = b.requests[i][j];
        }
    }
}
