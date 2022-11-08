#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include <stdint.h>
#include "freertos.h"
#include "queue.h"
#include "task.h"
#include "SERCOM.h"
#include "API.h"

extern QueueHandle_t bluetoothData;
extern UART bluetooth;
extern Smartmesh_API api;
extern uint8_t smartmeshData[130];
extern SemaphoreHandle_t getNetworkInfo;
extern SemaphoreHandle_t getNetworkConfig;
extern SemaphoreHandle_t moteConfigWasGotFromID;
extern SemaphoreHandle_t getMoteInfo;
extern SemaphoreHandle_t apiInUse;
extern bool connectedToManager;

void bluetoothParse(void* unused);

#endif
