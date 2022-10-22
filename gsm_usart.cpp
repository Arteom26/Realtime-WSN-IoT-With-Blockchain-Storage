#include "gsm_usart.h"
#include <string>
#include <algorithm>


// This task will parse any data recived thorugh bluetooth
void gsmParse(void* unused){
	uint8_t recieved_data;
	
	while(1){
		xQueueReceive(gsmData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		gsm_usart._printf("%c", recieved_data);
	}
}
