#include "SERCOM.h"
#include <system_saml21.h>

UART::UART(sercom_registers_t *port, int baudrate){
	UART_port = &port->USART_INT;
	
	// UART interface setup w/ proper baud rate
	UART_port->SERCOM_CTRLA |= 0x10004;// Internal clock => SERCOM_PAD[2] TX, SERCOM_PAD[0] RX
	UART_port->SERCOM_CTRLB |= 0x30000;// Enable tx and rx pins
	UART_port->SERCOM_BAUD |= (uint16_t)(65536*(1-16*(115200.0/SystemCoreClock)));
	UART_port->SERCOM_INTENSET |= 0x4;// Enable interrupt complete interrupt
	UART_port->SERCOM_CTRLA |= 0x2;// Enable the UART port
}

int UART::_printf(const char *format, ...){
	char buffer[UART_BUFFER_SIZE];
	
	va_list args;
	va_start(args, format);
	
	int done = vsprintf(buffer, format, args);
	if(done > UART_BUFFER_SIZE)
		return NULL;
	
	for(int i = 0;i < done;i++){// Send the data
		UART_port->SERCOM_DATA = buffer[i];
		while((UART_port->SERCOM_INTFLAG & 0x2) != 0);// Wait for data to be transmitted
	}
	
	return 0;
}

void UART::send_array(uint8_t *data, uint8_t length){
	for(int i = 0;i < length;i++){
		UART_port->SERCOM_DATA = *data;
		while((UART_port->SERCOM_INTFLAG & 0x2) != 0);// Wait for data to be transmitted
		data++;
	}
}
