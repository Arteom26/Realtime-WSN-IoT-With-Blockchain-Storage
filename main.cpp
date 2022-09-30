#include "sam.h"
#include "system_setup.h"
#include "SERCOM.h"

void mainTask(void* unused){
	while(1);
}

/*
	* Main entry point into the program
	* SERCOM0 for Smartmesh IP
	* SERCOM1 for bluetooth
	* SERCOM2 for GSM module
*/
UART usart = UART(SERCOM0_REGS, 115200);;

int main(){
	setup_system();
	usart = UART(SERCOM0_REGS, 115200);
	
	char *name = "ARTEOM KATKOV!";
	usart._printf("Hello World");
	//usart.send_array((uint8_t*)name, 14);
		
	for(;;);
	return 0;
}

//************************INTERRUPT HANDLERS**************************//
extern "C"{
	void SERCOM0_Handler(void){
		uint8_t data = SERCOM0_REGS->USART_INT.SERCOM_DATA;
		usart._printf("%c", data);
		NVIC->ICPR[0] |= (1 << 8);
	}
}
