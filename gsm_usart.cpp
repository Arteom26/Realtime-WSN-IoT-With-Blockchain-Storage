#include "gsm_usart.h"
#include "bluetooth.h"
#include <string>
#include <algorithm>
//#include "GSM.h"

char gsmBuffer[] = "";
bool new_line = false;

bool at_cmd_OK(char *buf)
{
	if (strncmp (buf,AT_SUCCESS,2) == 0)
	{
		return true;
	}
	return false;
}



bool read_resp(char *buf)
{
	if (strncmp (buf,"+xx",1) == 0)
	{
		
		return true;
	}
	return false;
}



bool at_cmd_ERROR(char *buf)
{
	if (strncmp (buf,AT_ERROR,5) == 0)
	{
		return true;
	}
	return false;
}

// This task will parse any data recived thorugh bluetooth
void gsmParse(void* unused){
	uint8_t recieved_data;
	int count = 0;
	
	while(1){
		xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		bluetooth._printf("%c", recieved_data);
		
//		while (recieved_data != '\n' && recieved_data != '\r') 
//		{
//			gsmBuffer[count] = recieved_data;
//			//bluetooth._printf("%c", gsmBuffer[count]);
//			//bluetooth._printf("%d",count);
//			count++;
//			xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
//		}

//		
//		//bluetooth._printf("%s",gsmBuffer);
//		if (at_cmd_OK(gsmBuffer)) 
//		{
//			//for(int i = 0; i <= count; i++) bluetooth._printf("%c", gsmBuffer[i]);
//			gsmBuffer[count] = '\0';
//			char* s = gsmBuffer;
//			bluetooth._printf("%s\r\n", s);
//		}
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

	}
}
