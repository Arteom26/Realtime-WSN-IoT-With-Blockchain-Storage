#ifndef __GSM_USART_H
#define __GSM_USART_H

#include <string>
#include <stdint.h>
#include "freertos.h"
#include "queue.h"
#include "task.h"
#include "SERCOM.h"

typedef enum
{
    AT_COMMAND_READ,
    AT_COMMAND_WRITE,
    AT_COMMAND_TEST,
    AT_COMMAND_RUN,
		AT_COMMAND_UNKNOWN
} AT_COMMAND_TYPE;


extern char responseGsmBuffer[200];
extern char txGsmBuffer[200];
extern uint8_t responseLength;
extern uint8_t responseLengthCopy;


extern QueueHandle_t gsmData;
extern SemaphoreHandle_t gsmDataRecieved;
extern UART gsm_usart;
extern AT_COMMAND_TYPE at_cmd_type;
extern bool gsmReady;

void setupGsmParse(void* unused);
void clearBuffer(char *buf);
bool read_resp(const char *buf);
bool at_OK(const char *buf);
bool at_ERROR(const char *buf);

bool at_send_cmd(const char *cmd, AT_COMMAND_TYPE cmd_type);

bool isValidIPAddress(const char *ip);





#define AT_HEAD "AT"
#define AT_SUCCESS "OK"
#define AT_ERROR "ERROR"
#define AT_CLOSED "CLOSED"
#define AT_CME_ERROR "+CME ERROR:"




#endif
