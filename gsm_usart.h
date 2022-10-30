#ifndef __GSM_USART_H
#define __GSM_USART_H

#include <string>
#include <stdint.h>
#include "freertos.h"
#include "queue.h"
#include "task.h"
#include "SERCOM.h"

extern QueueHandle_t gsmData;
extern UART gsm_usart;
void gsmParse(void* unused);


bool read_resp(char *buf);
bool at_cmd_OK(char *buf);
bool at_cmd_ERROR(char *buf);


#define AT_HEAD "AT"
#define AT_SUCCESS "OK"
#define AT_ERROR "ERROR"
#define AT_CME_ERROR "+CME ERROR:"




#endif
