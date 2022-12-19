#include "gsm_usart.h"
#include "bluetooth.h"
#include <string>
#include <algorithm>
#include <ctype.h> 
#include "SendingData.h"
//#include "GSM.h"


int count = 0;
char gsmBuffer[128];
bool new_line = false;
uint8_t recieved_data;

bool at_OK(const char *buf)
{
	if (strstr(buf, "OK") != NULL)
	{
		return true;
	}
	return false;
}

bool read_resp(const char *buf)
{
	if (strncmp (buf,"+xx",1) == 0)
	{
		
		return true;
	}
	return false;
}



bool at_ERROR(const char *buf)
{
	if (strstr(buf, "ERROR") != NULL)
	{
		return true;
	}
	return false;
}

void clearBuffer(char *buf)
{
	for(int i = 0; i < 200; i++)
	{
		responseLength = 0;
		buf[i] = '\0';
	}
}

bool at_send_cmd(const char *cmd, AT_COMMAND_TYPE cmd_type)
{
	vTaskDelay(10);
	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
	
	gsm_usart._printf(cmd);
	at_cmd_type = cmd_type;
	
	gsmReady = false;
	
	return true;
}

bool isValidIPAddress(const char *s)
{
		
    int len = strlen(s);
		int count = 0;
		
		//wait till a number is recieved
		while(!isdigit(s[count])) count++;
		len = len - (count+1);
		
    if (len < 7 || len > 15)
        return 0;

    char tail[16];
    tail[0] = 0;

    unsigned int d[4];
    int c = sscanf(s, "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail);

    if (c != 4 || tail[0])
        return 0;

    for (int i = 0; i < 4; i++)
        if (d[i] > 255)
            return 0;

    return 1;
}


void parseGSMData(void* unused)
{
	char tempBuffer[200];

	
	xSemaphoreTakeFromISR(dma_in_use, NULL);
	DMAC_REGS->DMAC_CHID = 0;
	while(DMAC_REGS->DMAC_CHCTRLA != 0);// Check to see if DMA is still running
	uint32_t *desc = (uint32_t*)0x30000000;
	*desc++ = ((responseLengthCopy + 2) << 16)|0x0C01;
	*desc++ = (uint32_t)(responseGsmBuffer + responseLengthCopy + 2);// Source address
	*desc = (uint32_t)(tempBuffer + responseLengthCopy + 2);// Dest address
	DMAC_REGS->DMAC_CHCTRLA = 0x2;// Enable the channel
	DMAC_REGS->DMAC_SWTRIGCTRL |= 0x1;
	xSemaphoreGiveFromISR(dma_in_use, NULL);
		
	responseLengthCopy = 0;
	//clearBuffer(responseGsmBuffer);
	
	//if (isdigit(tempBuffer[0]) && !isValidIPAddress(tempBuffer)) 
		//bluetooth._printf(tempBuffer);
		
	if(strstr(tempBuffer, "OK") != NULL)
	{
		//bluetooth._printf("OK\n\r");
		xSemaphoreGive(gsm_in_use); // GSM parsing done
	}
	else if(strstr(tempBuffer, "ERROR") != NULL)
	{
		//bluetooth._printf("ERROR\n\r");
		xSemaphoreGive(gsm_in_use); // GSM parsing done
	}
	else if( isValidIPAddress(tempBuffer) ) 
	{
		//bluetooth._printf("good\n\r");
		xSemaphoreGive(gsm_in_use); // GSM parsing done
	}
	xSemaphoreGive(gsm_in_use);
	vTaskDelete(NULL);
}

// This task will parse any data recived thorugh bluetooth
void setupGsmParse(void* unused){

	
	while(1)
		{
		//xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		//bluetooth._printf("%c", recieved_data);
		//xSemaphoreGive(gsm_in_use);
			
		
		xSemaphoreTake(gsmDataRecieved, portMAX_DELAY);
			
		xTaskCreate(parseGSMData, "parse gsm data", 256, NULL, 11, NULL);

		

	}
}
