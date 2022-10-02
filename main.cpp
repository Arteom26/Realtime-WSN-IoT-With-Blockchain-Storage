#include "sam.h"
#include "system_setup.h"
#include "SERCOM.h"
#include "API.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Global variables
bool startFlag = false;
bool recievedBTData = false;// Bluetooth flag(TODO: semaphore)
uint8_t length = 0;// TODO: add to queue (possibly)
uint8_t txbuffer[128];// TODO: Convert to a queue
UART usart = UART(SERCOM0_REGS, 115200);
Smartmesh_API api = Smartmesh_API(&usart);

QueueHandle_t managerData = xQueueCreate(1, sizeof(uint8_t));
SemaphoreHandle_t dataRecieved = xSemaphoreCreateBinary();

// Checksum verification for interrupt handler
uint16_t verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len){
	data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	
	return fcs;
}

void parseSmartmeshData(void* unused){
	BaseType_t xStatus;
	uint8_t data;
	uint8_t len;
	bool flag = false;
	
	while(1){
		xSemaphoreTake(dataRecieved, portMAX_DELAY);
	}
}

/*
	* Main entry point into the program
	* SERCOM0 for Smartmesh IP
	* SERCOM1 for bluetooth
	* SERCOM2 for GSM module
*/
int main(){
	xTaskCreate(parseSmartmeshData, "Parse", 256, NULL, 6, NULL);
	
	setup_system();
	usart = UART(SERCOM0_REGS, 115200);
	//api = Smartmesh_API(&usart);
	//api.mgr_init();
	
	usart._printf("Hello World!");
	usart._printf("Hello World!");
	
	vTaskStartScheduler();
		
	for(;;);
	return 0;
}

//************************INTERRUPT HANDLERS**************************//
extern "C"{
	void SERCOM0_Handler(void){// TODO: Move out of interrupt handler and into seperate task
		uint8_t data = SERCOM0_REGS->USART_INT.SERCOM_DATA;
		
		if(!startFlag && data == 0x7E){// Start flag found
			length = 0;
			startFlag = true;
			txbuffer[length] = data;
		}else if(startFlag && data == 0x7E){// Checksum to check if end of flag was found
			uint16_t check = verifyPacket(START_CHECKSUM, txbuffer, length - 2);
			uint16_t currCheck = txbuffer[length - 1] | (txbuffer[length] << 8);
			txbuffer[length+1] = data;
			startFlag = false;
			if(check == currCheck)// Verification passed
				xSemaphoreGiveFromISR(dataRecieved, NULL);
		}else if(startFlag && data != 0x7E){// Copy to buffer
			txbuffer[length+1] = data;
			length++;
		}
		
		if(length > 133)// Packet failed to be found
			startFlag = false;
		
		NVIC->ICPR[0] |= (1 << 8);// Clear the interrupt
	}
	
	void SERCOM1_Handler(void){// Bluetooth handler
		recievedBTData = true;
		
		NVIC->ICPR[0] |= (1 << 9);// Clear the interrupt
	}
	
	void SERCOM2_Handler(void){// GSM Module handler
		NVIC->ICPR[0] |= (1 << 10);// Clear the interrupt
	}
}
