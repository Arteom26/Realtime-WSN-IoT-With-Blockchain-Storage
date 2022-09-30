#include "sam.h"
#include "system_setup.h"
#include "SERCOM.h"
#include "API.h"
#include "freertos.h"
#include "task.h"

// Global variables
bool startFlag = false;
bool recievedBTData = false;// Bluetooth flag(TODO: semaphore)
uint8_t length = 0;// TODO: add to queue (possibly)
uint8_t txbuffer[128];// TODO: Convert to a queue
UART usart = UART(SERCOM0_REGS, 115200);

void mainTask(void* unused){
	while(1);
}

/*
	* Main entry point into the program
	* SERCOM0 for Smartmesh IP
	* SERCOM1 for bluetooth
	* SERCOM2 for GSM module
*/
int main(){
	//setup_system();
	//usart = UART(SERCOM0_REGS, 115200);

	//usart._printf("Hello World");
	
	BaseType_t x = xTaskCreate(mainTask, "Main Task", 64, NULL, 1, NULL);
	
	if(x == pdPASS)
		vTaskStartScheduler();
		
	for(;;);
	return 0;
}

// Checksum verification for interrupt handler
uint16_t verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len){
	data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	
	return fcs;
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
			if(check == currCheck){
				startFlag = false;
				//BaseType_t xHigherPriorityTaskWoken = pdTRUE;// Do not allow to jump directly to the task
				//xSemaphoreGiveFromISR(packetRecieved, &xHigherPriorityTaskWoken);
			}
		}else if(startFlag && data != 0x7E){// Copy to buffer
			txbuffer[length+1] = data;
			length++;
		}
		
		if(length > 133)// Packet failed to be found
			startFlag = false;
		
		NVIC->ICPR[0] |= (1 << 8);// Clear the interrupt
	}
	
	void SERCOM1_Handler(void){// Bluetooth handler
		NVIC->ICPR[0] |= (1 << 9);// Clear the interrupt
	}
	
	void SERCOM2_Handler(void){// GSM Module handler
		NVIC->ICPR[0] |= (1 << 10);// Clear the interrupt
	}
}
