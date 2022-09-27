#ifndef __SERCOM_H
#define __SERCOM_H

#include "saml21g18b.h"
#include <stdarg.h>
#include <stdio.h>

#define UART_BUFFER_SIZE 128

using namespace std;

class UART{
	public:
		UART(sercom_registers_t* port, int baudrate);
		int _printf(const char *format, ...);
		void send_array(uint8_t *data, uint8_t length);
	private:
		sercom_usart_int_registers_t* UART_port;// UART port to use
};

#endif
