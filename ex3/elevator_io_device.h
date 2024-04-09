#pragma once

#include "elevator_io_types.h"

void elev_init(char *port);
ElevInputDevice elevio_getInputDevice(void);
ElevOutputDevice elevio_getOutputDevice(void);