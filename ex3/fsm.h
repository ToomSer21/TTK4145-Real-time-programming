#pragma once

#include "elevator.h"
#include "elevator_io_types.h"

void fsm_onInitBetweenFloors(void);
void fsm_onRequestButtonPress(int btn_floor, Button btn_type);
void fsm_onFloorArrival(int newFloor);
void fsm_onDoorTimeout(void);
void fsm_onDoorObstruction(void);
Elevator fsm_getElevator(void);
void fsm_setElevator(int id, char *ip);
void fsm_setBackup(Elevator b);
