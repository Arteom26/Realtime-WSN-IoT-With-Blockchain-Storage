#include "gsm_usart.h"
#include "bluetooth.h"

#define DELAY 200
void gsm_init(void);
void http_test(void);
void tcp_write(uint8_t*, int = 1);
void tcp_read(void);
void send_to_firebase(uint8_t*, uint8_t*, int);

void sendData(void* unused);

extern bool isError;


