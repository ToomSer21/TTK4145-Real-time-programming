
#include "elevator_io_types.h"

void* startCom(void* args);
void sendBackup(void);
int sendRequest(int btn_floor, Button btn_type);
void sendClear(int floor, Button btn);
void sendObstructed(void);
void sendFree(void);
int isStandalone(void);