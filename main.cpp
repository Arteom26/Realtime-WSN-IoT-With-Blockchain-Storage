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
bool connectedToManager = false;
uint8_t length = 0;
uint8_t txbuffer[130];
uint8_t smartmeshData[130];
UART api_usart = UART(SERCOM0_REGS, 115200);
Smartmesh_API api = Smartmesh_API(&api_usart);
SemaphoreHandle_t dma_in_use = xSemaphoreCreateBinary();

QueueHandle_t bluetoothData = xQueueCreate(32, sizeof(uint8_t));
SemaphoreHandle_t dataRecieved = xSemaphoreCreateBinary();

// Checksum verification for interrupt handler
uint16_t verifyPacket(uint16_t fcs, uint8_t *data, uint16_t len){
	data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	
	return fcs;
}

// This task will be created spontaneously to parse a recieved packet
void parseSmartmeshData(void* unused){
	uint8_t buffer[130];// Buffer for stored data
	
	// Copy data over from buffer in setupParse to task buffer
	xSemaphoreTake(dma_in_use, portMAX_DELAY);
	DMAC_REGS->DMAC_CHID = 0;
	while(DMAC_REGS->DMAC_CHCTRLA != 0);// Check to see if DMA is still running
	uint32_t *desc = (uint32_t*)0x30000000;
	*desc++ = ((length+2) << 16)|0x0C01;
	*desc++ = (uint32_t)(smartmeshData + smartmeshData[4] + 8);// Source address
	*desc = (uint32_t)(buffer + smartmeshData[4] + 8);// Dest address
	DMAC_REGS->DMAC_CHCTRLA = 0x2;// Enable the channel
	DMAC_REGS->DMAC_SWTRIGCTRL |= 0x1;
	xSemaphoreGive(dma_in_use);
	
	// TODO: finish parsing section
	uint8_t packetType = buffer[2];
	switch(packetType){
		case 0x2:
			api.subscribe(0xFFFFFFFF,0xFFFFFFFF);// Subscribe to notifications
			connectedToManager = true;
			break;
		case 0x3:// No communication was setup
			api.mgr_init();// Send hello packet
			break;
		case 0x14:// Notification packet recieved. TODO: parse the data in a seperate task
			break;
		default:
			break;
	}
	
	vTaskDelete(NULL);// Delete this task once done with parsing
}

void setupParse(void* unused){
	while(1){
		xSemaphoreTake(dataRecieved, portMAX_DELAY);
		
		// Create a new instance of smartmesh data parsing task
		xTaskCreate(parseSmartmeshData, "Smart", 256, NULL, 6, NULL);
	}
}

// This task will parse any data recived thorugh bluetooth
void bluetoothParse(void* unused){
	uint8_t recieved_data;
	
	while(1){
		xQueueReceive(bluetoothData, &recieved_data, portMAX_DELAY);// Wait for data to come in
	}
}

/*
	* Main entry point into the program
	* SERCOM0 for Smartmesh IP
	* SERCOM1 for bluetooth
	* SERCOM2 for GSM module
*/
int main(){
	xTaskCreate(setupParse, "Parse", 128, NULL, 6, NULL);
	xSemaphoreGive(dma_in_use);
	setup_system();
	api_usart = UART(SERCOM0_REGS, 115200);
	api = Smartmesh_API(&api_usart);
	//api_usart._printf("123");
	api.mgr_init();
	
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
			xSemaphoreTakeFromISR(dma_in_use, NULL);
			DMAC_REGS->DMAC_CHID = 0;
			while(DMAC_REGS->DMAC_CHCTRLA != 0);// Check to see if DMA is still running
			uint32_t *desc = (uint32_t*)0x30000000;
			*desc++ = ((length+2) << 16)|0x0C01;
			*desc++ = (uint32_t)(txbuffer + length + 2);// Source address
			*desc = (uint32_t)(smartmeshData + length + 2);// Dest address
			DMAC_REGS->DMAC_CHCTRLA = 0x2;// Enable the channel
			DMAC_REGS->DMAC_SWTRIGCTRL |= 0x1;
			xSemaphoreGiveFromISR(dma_in_use, NULL);
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
		uint8_t data = SERCOM1_REGS->USART_INT.SERCOM_DATA;
		xQueueSendFromISR(bluetoothData, &data, NULL);
		
		NVIC->ICPR[0] |= (1 << 9);// Clear the interrupt
	}
	
	void SERCOM2_Handler(void){// GSM Module handler
		NVIC->ICPR[0] |= (1 << 10);// Clear the interrupt
	}
}
