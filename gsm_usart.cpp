#include "gsm_usart.h"
#include "bluetooth.h"
#include <string>
#include <algorithm>
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



bool at_send_cmd(const char *cmd, AT_COMMAND_TYPE cmd_type)
{
	xSemaphoreTake(gsm_in_use, portMAX_DELAY); // wait for GSM data to be parsed
	
	gsm_usart._printf("%s",cmd);
	at_cmd_type = cmd_type;
	
	gsmReady = false;
	
	return true;
}

bool isValidIPAddress(const char *s)
{
    int len = strlen(s);

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
	bluetooth._printf("%c", recieved_data);
	
	vTaskDelete(NULL);
}

// This task will parse any data recived thorugh bluetooth
void gsmParse(void* unused){
	
	
	while(1){
		xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		//bluetooth._printf("%c", recieved_data);
		

		while (recieved_data != '\n' && recieved_data != '\r') 
		{
			gsmBuffer[count] = recieved_data;
			count++;
			xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		}
		gsmBuffer[count+1] = NULL;


		if (at_cmd_type == AT_COMMAND_RUN) 
		{
			if (at_OK(gsmBuffer) || isValidIPAddress(gsmBuffer)) 
			{
				bluetooth._printf("\r\nrun ok response: %s\r\n", gsmBuffer);
				gsmReady = true;
				at_cmd_type = AT_COMMAND_UNKNOWN;
			}
			if (at_ERROR(gsmBuffer)) 
			{
				bluetooth._printf("\r\nrun error response: %s\r\n", gsmBuffer);
				gsmReady = true;
				at_cmd_type = AT_COMMAND_UNKNOWN;
			}
			count = 0;
		}
		
		if (at_cmd_type == AT_COMMAND_WRITE) 
		{
			xSemaphoreTake(bluetoothInUse, portMAX_DELAY);
			if (at_OK(gsmBuffer)) 
			{
				bluetooth._printf("\r\nwrite ok response: %s\r\n", gsmBuffer);
				at_cmd_type = AT_COMMAND_UNKNOWN;
				gsmReady = true;
			}
			if (at_ERROR(gsmBuffer)) 
			{
				bluetooth._printf("\r\nwrite error response: %s\r\n", gsmBuffer);
				at_cmd_type = AT_COMMAND_UNKNOWN;
			}
			xSemaphoreGive(bluetoothInUse);
			count = 0;
		}

		//xSemaphoreTake(gsmDataRecieved, portMAX_DELAY);
		//xTaskCreate(parseGSMData, "GSM Data Parse", 256, NULL, 7, NULL);
		
//		if (at_cmd_type == AT_COMMAND_READ) 
//		{
//			bluetooth._printf("read cmd\r\n");
//			at_cmd_type = AT_COMMAND_UNKNOWN;
//			gsmReady = true;
//		}		
//		
//		if (at_cmd_type == AT_COMMAND_TEST) 
//		{
//			bluetooth._printf("test cmd\r\n");
//			at_cmd_type = AT_COMMAND_UNKNOWN;
//			gsmReady = true;
//		}
//		
//		if (at_cmd_type == AT_COMMAND_WRITE) 
//		{
//			bluetooth._printf("write cmd\r\n");
//			at_cmd_type = AT_COMMAND_UNKNOWN;
//			gsmReady = true;
//		}
		
					//for(int i = 0; i <= count; i++) bluetooth._printf("%c", gsmBuffer[i]);
//			gsmBuffer[count] = '\0';
//			char* s = gsmBuffer;
//		
//		if (at_cmd_ERROR(gsmBuffer))
//		{
//			gsmBuffer[count] = '\0';
//			char* s = gsmBuffer;
//			bluetooth._printf("%s\r\n", s);
//		}
//		
//		//bluetooth._printf("%c", recieved_data);
//		count = 0;
		xSemaphoreGive(gsm_in_use); // GSM parsing done
	}
}


