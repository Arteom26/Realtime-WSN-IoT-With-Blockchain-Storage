#include "bluetooth.h"
#include "gsm_usart.h"
#include <string>
#include <algorithm>

template <typename T>
T littleToBigEndian(T value){
	union{
		T val;
		uint8_t bytes[sizeof(T)];
	}src, dst;
	
	src.val = value;
	std::reverse_copy(src.bytes, src.bytes+sizeof(T), dst.bytes);
	
	return dst.val;
}

// This task will parse any data recived thorugh bluetooth
void bluetoothParse(void* unused){
	uint8_t recieved_data;
	
		while(1){
		xQueueReceive(bluetoothData, &recieved_data, portMAX_DELAY);// Wait for data to come in
		bluetooth._printf("%c", recieved_data);
			
	}
}
