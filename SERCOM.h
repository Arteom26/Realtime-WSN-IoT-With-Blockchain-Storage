#ifndef __SERCOM_H
#define __SERCOM_H

#include "saml21g18b.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "freertos.h"
#include "semphr.h"
#include <string>

#define UART_BUFFER_SIZE 256

extern SemaphoreHandle_t dma_in_use;
extern SemaphoreHandle_t bluetoothInUse;
extern SemaphoreHandle_t gsm_in_use;

class UART{
	public:
		UART(sercom_registers_t* port, int baudrate);
		int _printf(const char *format, ...);
		void send_array(uint8_t *data, uint8_t length);
	private:
		sercom_usart_int_registers_t* UART_port;// UART port to use
		uint8_t dma_channel_id;
};

#endif
