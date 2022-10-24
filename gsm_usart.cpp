#include "gsm_usart.h"
#include "bluetooth.h"
#include <string>
#include <algorithm>


// This task will parse any data recived thorugh bluetooth
void gsmParse(void* unused){
	uint8_t recieved_data;
	
	while(1){
		xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		bluetooth._printf("%c", recieved_data);
	}
}
