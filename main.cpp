#include "sam.h"
#include "system_setup.h"
#include "SERCOM.h"
#include "API.h"
#include "freertos.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bluetooth.h"
#include "gsm_usart.h"
#include <string>
#include "SendingData.h"


AT_COMMAND_TYPE at_cmd_type = AT_COMMAND_UNKNOWN;

// Global variables
bool startFlag = false;
bool connectedToManager = false;
bool gsmReady = true;
uint8_t length = 0;
uint8_t txbuffer[130];
uint8_t smartmeshData[130];

char responseGsmBuffer[200];
char txGsmBuffer[200];
uint8_t responseLength = 0;
uint8_t responseLengthCopy = 0;

//uint8_t gsmBuffer[200];

UART api_usart = UART(SERCOM0_REGS, 115200);
UART bluetooth = UART(SERCOM1_REGS, 115200);
UART gsm_usart = UART(SERCOM2_REGS, 115200);


Smartmesh_API api = Smartmesh_API(&api_usart);

// RTOS Semaphores and queues
QueueHandle_t bluetoothData = xQueueCreate(64, sizeof(uint8_t));
QueueHandle_t gsmData = xQueueCreate(128, sizeof(uint8_t));



SemaphoreHandle_t dma_in_use = xSemaphoreCreateBinary();
SemaphoreHandle_t bluetoothInUse = xSemaphoreCreateBinary();
SemaphoreHandle_t gsm_in_use = xSemaphoreCreateBinary();


SemaphoreHandle_t dataRecieved = xSemaphoreCreateCounting(10,0);// Data was recieved from network manager
SemaphoreHandle_t gsmDataRecieved = xSemaphoreCreateBinary();// Data was recieved from GSM


SemaphoreHandle_t getNetworkInfo = xSemaphoreCreateBinary();// Network info packet is ready
SemaphoreHandle_t getNetworkConfig = xSemaphoreCreateBinary();
SemaphoreHandle_t moteConfigWasGotFromID = xSemaphoreCreateBinary();// Mote config from id packet is ready

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
		case MGR_HELLO_RESPONSE:
			bluetooth._printf("Manager Connection Initialized!\n");
			api.subscribe(0xFFFFFFFF,0xFFFFFFFF);// Subscribe to notifications
			connectedToManager = true;
			break;
		
		case MGR_HELLO:// No communication was setup
			connectedToManager = false;// No connection has been established
			api.mgr_init();// Send hello packet
			break;
		
		case NOTIF:// Notification packet recieved. TODO: parse the data in a seperate task
			break;
		
		case SET_NTWK_CONFIG:
			xSemaphoreGive(getNetworkConfig);
		case SET_COMMON_JKEY:
			bluetooth._printf("Rebooting...\n");
			api.resetManager();// Reset the system
			break;
		
		case SUBSCRIBE:// Notifications were setup
			bluetooth._printf("Subscribed to notifications!!\n");
			break;
		
		case GET_NETWORK_INFO:// Network info data must be parsed
			network_info info;
			api.parseNetworkInfo(&info, buffer);
			xSemaphoreGive(getNetworkInfo);// Network info has been recieved
			break;
		default:
			break;
	}
	
	vTaskDelete(NULL);// Delete this task once done with parsing
}

void setupParse(void* unused){

	//http_test();
	
	//tcp_write();
	//gsm_usart._printf("AT\r\n");
	//char cmd[] = "AT\r\n";
//		gsm_usart._printf("ate0\r\n");
//vTaskDelay(100);
//	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
//	gsm_usart._printf("ATE0\r\n");
//	vTaskDelay(500);
//	vTaskDelay(50);
//	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
// gsm_usart._printf("AT+CIPSHUT\r\n");
//	
//	
//	//at_send_cmd("AT\r\n", AT_COMMAND_RUN);

//	//at_send_cmd("ATT\r\n", AT_COMMAND_RUN);
//	vTaskDelay(50);
//	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
//	//at_send_cmd("AT\r\n", AT_COMMAND_WRITE);
////	at_send_cmd("AT+CIFSR\r\n", AT_COMMAND_RUN);
//		gsm_usart._printf("AT+CSTT=\"hologram\"\r\n");
//		
//		vTaskDelay(50);
//	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
//	gsm_usart._printf("AT+CIICR\r\n");
//	
//	vTaskDelay(50);
//	xSemaphoreTake(gsm_in_use, portMAX_DELAY);
//	gsm_usart._printf("AT+CIFSR\r\n");
//	at_send_cmd("\r\nAT+CIPSHUT\r\n", AT_COMMAND_RUN);
//	//vTaskDelay(2000);
//	at_send_cmd("AT+CSTT=\"hologram\"\r\n", AT_COMMAND_WRITE);
//	//vTaskDelay(100);
//	at_send_cmd("AT+CIICR\r\n", AT_COMMAND_RUN);
//	//vTaskDelay(100);
//	at_send_cmd("AT+CIFSR\r\n", AT_COMMAND_RUN);
//	//vTaskDelay(100);
//	at_send_cmd("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"\r\n", AT_COMMAND_WRITE);
//	//vTaskDelay(2000);
//	at_send_cmd("AT+CIPSEND=82\r\n", AT_COMMAND_WRITE);
//	//vTaskDelay(100);
//	at_send_cmd("GET https://api.thingspeak.com/update?api_key=XB4GKI5NFDXXS0VU&field2=7&field3=2\r\n",AT_COMMAND_WRITE);
//	//vTaskDelay(2000);
//	bluetooth._printf("done");
	
	while(1){
		xSemaphoreTake(dataRecieved, portMAX_DELAY);
		
		// Create a new instance of smartmesh data parsing task
		xTaskCreate(parseSmartmeshData, "Smart", 256, NULL, 6, NULL);
	}
}

/*
	* Main entry point into the program
	* SERCOM0 for Smartmesh IP
	* SERCOM1 for bluetooth
	* SERCOM2 for GSM module
*/
int main(){
	setup_system();// Setup all peripherals
	xTaskCreate(setupParse, "Parse", 64, NULL, 1, NULL);
	xTaskCreate(sendData, "Parse", 64, NULL, 1, NULL);
	xTaskCreate(bluetoothParse, "BT Parse", 256, NULL, 5, NULL);
	xTaskCreate(setupGsmParse, "GSM Parse", 256, NULL, 1, NULL);
	
	api_usart = UART(SERCOM0_REGS, 115200);
	bluetooth = UART(SERCOM1_REGS, 115200);
	gsm_usart = UART(SERCOM2_REGS, 115200);
	
	api = Smartmesh_API(&api_usart);
	xSemaphoreGive(dma_in_use);// DMA can now be accessed
	xSemaphoreGive(bluetoothInUse);
	xSemaphoreGive(gsm_in_use);
	
//	bluetooth._printf("hello");

	
	vTaskStartScheduler();
		
	for(;;);// SHOULD NOT REACH THIS POINT
}

//************************INTERRUPT HANDLERS**************************//
extern "C"{
	void SERCOM0_Handler(void){// TODO: Move out of interrupt handler and into seperate task
		uint8_t data = SERCOM0_REGS->USART_INT.SERCOM_DATA;
		
		if(!startFlag && data == 0x7E){// Start flag found
			length = 0;
			startFlag = true;
			txbuffer[length] = data;
		}
		else if(startFlag && data == 0x7E){// Checksum to check if end of flag was found
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
		}
		else if(startFlag && data != 0x7E){// Copy to buffer
			txbuffer[length+1] = data;
			length++;
		}
		
		if(length > 133)// Packet failed to be found => reset and search for a new packet
			startFlag = false;
		
		NVIC->ICPR[0] |= (1 << 8);// Clear the interrupt
	}
	
	void SERCOM1_Handler(void){// Bluetooth handler
		uint8_t data = SERCOM1_REGS->USART_INT.SERCOM_DATA;
		xQueueSendFromISR(bluetoothData, &data, NULL);// Send data to the bluetooth queue
		
		NVIC->ICPR[0] |= (1 << 9);// Clear the interrupt
	}
	
	void SERCOM2_Handler(void){// GSM Module handler
		uint8_t data = SERCOM2_REGS->USART_INT.SERCOM_DATA;
		
		txGsmBuffer[responseLength] = data;
		responseLength++;
		
		//process response line by line
		if ((strstr(txGsmBuffer, "\r\n") != NULL))
		{
			responseLengthCopy = responseLength;

			xSemaphoreTakeFromISR(dma_in_use, NULL);
			DMAC_REGS->DMAC_CHID = 0;
			while(DMAC_REGS->DMAC_CHCTRLA != 0);// Check to see if DMA is still running
			uint32_t *desc = (uint32_t*)0x30000000; //Base address
			*desc++ = ((responseLength + 2) << 16)|0x0C01;
			*desc++ = (uint32_t)(txGsmBuffer + responseLength + 2);// Source address
			*desc = (uint32_t)(responseGsmBuffer + responseLength + 2);// Dest address
			DMAC_REGS->DMAC_CHCTRLA = 0x2;// Enable the channel
			DMAC_REGS->DMAC_SWTRIGCTRL |= 0x1;
			xSemaphoreGiveFromISR(dma_in_use, NULL);
				
			clearBuffer(txGsmBuffer);
			xSemaphoreGiveFromISR(gsmDataRecieved, NULL);
		}

		//xQueueSendFromISR(gsmData, &data, NULL);// Send data to the gsm queue
		NVIC->ICPR[0] |= (1 << 10);// Clear the interrupt
	}
}
