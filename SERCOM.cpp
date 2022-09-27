#include "SERCOM.h"

UART::UART(sercom_registers_t *port, int baudrate){
	UART_port = &port->USART_INT;
	
	// UART interface setup w/ proper baud rate
	
}

int UART::_printf(const char *format, ...){
	char buffer[UART_BUFFER_SIZE];
	
	va_list args;
	va_start(args, format);
	
	int done = vsprintf(buffer, format, args);
	if(done > UART_BUFFER_SIZE)
		return NULL;
	
	for(int i = 0;i < done;i++){// Send the data
		//current_UART->DR = buffer[i];
		//while((current_UART->SR&0x40) == 0){};
	}
	
	return 0;
}

void UART::send_array(uint8_t *data, uint8_t length){
	for(int i = 0;i < length;i++){
	//	current_UART->DR = *data;
	//	while((current_UART->SR&0x40) == 0);
		data++;
	}
}
