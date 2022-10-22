#ifndef __GSM_USART_H
#define __GSM_USART_H

#include <stdint.h>
#include "freertos.h"
#include "queue.h"
#include "task.h"
#include "SERCOM.h"

extern QueueHandle_t gsmData;
extern UART gsm_usart;
void gsmParse(void* unused);

#endif
