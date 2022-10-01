#include "SERCOM.h"
#include <system_saml21.h>

UART::UART(sercom_registers_t *port, int baudrate){
	UART_port = &port->USART_INT;
	
	if(port == SERCOM0_REGS){
		PORT_REGS->GROUP[0].PORT_PINCFG[8] = 0x1;
		PORT_REGS->GROUP[0].PORT_PINCFG[10] = 0x1;
		PORT_REGS->GROUP[0].PORT_PMUX[4] = 0x2;
		PORT_REGS->GROUP[0].PORT_PMUX[5] = 0x2;
		NVIC->IP[2] |= 192;
		NVIC->ISER[0] |= (1 << 8);
		NVIC->ICPR[0] |= (1 << 8);
	}else if(port == SERCOM1_REGS){
		PORT_REGS->GROUP[0].PORT_PINCFG[16] = 0x1;
		PORT_REGS->GROUP[0].PORT_PINCFG[18] = 0x1;
		PORT_REGS->GROUP[0].PORT_PMUX[8] = 0x3;
		PORT_REGS->GROUP[0].PORT_PMUX[9] = 0x3;
	}

	// UART interface setup w/ proper baud rate
	GCLK_REGS->GCLK_PCHCTRL[18] = 0x42;// Enable UART clock
	UART_port->SERCOM_CTRLA = 0x40010004;// Internal clock => SERCOM_PAD[2] TX, SERCOM_PAD[0] RX(SERCOM0 PA4 and PA6)
	UART_port->SERCOM_CTRLB |= 0x30000;// Enable tx and rx pins
	UART_port->SERCOM_BAUD = 57987;// Always set to 115200 for now
	UART_port->SERCOM_INTENSET |= 0x4;// Enable interrupt rx complete interrupt
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
		while((UART_port->SERCOM_INTFLAG & 0x2) == 0);// Wait for data to be transmitted
		UART_port->SERCOM_INTFLAG |= 0x2;
	}
	
	return 0;
}

void UART::send_array(uint8_t *data, uint8_t length){
	for(int i = 0;i < length;i++){
		UART_port->SERCOM_DATA = *data;
		while((UART_port->SERCOM_INTFLAG & 0x2) != 0);// Wait for data to be transmitted
		UART_port->SERCOM_INTFLAG |= 0x2;
		data++;
	}
}
