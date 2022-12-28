#include "SendingData.h"

void sendData(void* unused)
{
	//gsm_usart._printf("HELLO");
	do{
		xSemaphoreGive(gsm_in_use);
	at_send_cmd("AT\r\n", AT_COMMAND_RUN);
	vTaskDelay(100);
	}while(xSemaphoreTake(gsm_in_use, 250) == pdFALSE);

	xSemaphoreGive(gsm_in_use);
	gsm_init();
	uint8_t dat[] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
	//tcp_write(dat);
	vTaskDelete(NULL);
}

void gsm_init(void)
{
	//at_send_cmd("AT\r\n", AT_COMMAND_RUN);
	at_send_cmd("ATE0\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+CFUN=0\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CFUN=1\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CIPSHUT\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CNMP=38\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CMNB=1\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CSTT=\"hologram\"\r\n", AT_COMMAND_WRITE);
	at_send_cmd("AT+CIICR\r\n", AT_COMMAND_RUN);
}


void http_test(void)
{
		
		//gsm_usart._printf("AT\r\n");
	//gsm_usart._printf("AT+CNACT=0");
	vTaskDelay(100);
	gsm_usart._printf("AT+CSTT=\"hologram\"\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CIICR\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CNACT=1,\"hologram\"\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CNACT?\r\n");
	vTaskDelay(100);
	
	gsm_usart._printf("AT+SHDISC\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+CSSLCFG=\"sslversion\",1,3\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHSSL=1,\"\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHCONF=\"URL\",\"https://cloudflare-eth.com\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHCONF=\"BODYLEN\",1024\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHCONF=\"HEADERLEN\",350\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHCONN\r\n");
	vTaskDelay(6000);
	gsm_usart._printf("AT+SHCHEAD\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHAHEAD=\"Content-Type\",\"application/json\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHAHEAD=\"Connection\",\"keep-alive\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHAHEAD=\"Accept\",\"*/*\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHAHEAD=\"Cache-control\",\"no-cache\"\r\n");
	vTaskDelay(DELAY);
	gsm_usart._printf("AT+SHBOD=\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"method\\\":\\\"web3_clientVersion\\\",\\\"params\\\":[],\\\"id\\\":\\\"1\\\"}\",68\r\n");
	vTaskDelay(500);
	
	//	std::string x = "\"{\\\"jsonrpc\\\":\\\"2.0\\\",\\\"method\\\":\\\"web3_clientVersion\\\",\\\"params\\\":[],\\\"id\\\":\\\"1\\\"}\"";
//	char y[2];
//	sprintf(y, "%d", x.length());
//	std::string x2 = "AT+SHBOD=" + x + "," + std::string(y) + "\r\n";

////API KEY: 3MFB6LHHEJKS2BU1



	gsm_usart._printf("AT+SHBOD?\r\n");
	vTaskDelay(500);
	gsm_usart._printf("AT+SHREQ=\"/post\",3\r\n");
	vTaskDelay(10000);
	gsm_usart._printf("AT+SHREAD=0,57\r\n");
//	//vTaskDelay(100000);
	
	//for(int i = 0;i < 10000000;i++);
	/*gsm_usart._printf("AT+SHCONF=\"URL\",\"https://rinkeby-light.eth.linkpool.io\"\r\n");
	gsm_usart._printf("AT+SHCONN\r\n");
	for(int i = 0;i < 4000000;i++);
	gsm_usart._printf("AT+SHCHEAD\r\n");
	gsm_usart._printf("AT+SHAHEAD=\"content-type\",\"application/json\"\r\n");*/
}

void https_write(void){
	
}

void tcp_write(uint8_t *data, int field)
{
	char ass[128];
	// Convert to ascii
	uint32_t val = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32)\
			 | (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
	
	int count = 0;
	for(int i = 10000;i >= 1; i /= 10){
		char x = (val - (val % i))/i % 10 + 0x30;
		ass[count] = x;
		count++;
	}
	
	char *buffer = new char[72 + count + 1];
	if(field == 1){
		std::memcpy(buffer, "GET https://api.thingspeak.com/update?api_key=XB4GKI5NFDXXS0VU&field1=", 70);
		std::memcpy(buffer + 70, ass, count + 1);
		std::memcpy(buffer + 70 + count, "\r\n\0", 3);
	}else if(field == 2){
		std::memcpy(buffer, "GET https://api.thingspeak.com/update?api_key=XB4GKI5NFDXXS0VU&field2=", 70);
		std::memcpy(buffer + 70, ass, count + 1);
		std::memcpy(buffer + 70 + count, "\r\n\0", 3);
	}else if(field == 3){
		std::memcpy(buffer, "GET https://api.thingspeak.com/update?api_key=XB4GKI5NFDXXS0VU&field3=", 70);
		std::memcpy(buffer + 70, ass, count + 1);
		std::memcpy(buffer + 70 + count, "\r\n\0", 3);
	}
	
	count = 72+count;
	int assCount = 0;
	for(int i = 10; i >= 1;i /= 10){
		char x = (count - (count % i))/i % 10 + 0x30;
		ass[assCount] = x;
		assCount++;
	}
	
	char *assBuffer = new char[16];
	std::memcpy(assBuffer, "AT+CIPSEND=", 11);
	std::memcpy(assBuffer + 11, ass, assCount);
	std::memcpy(assBuffer + 11 + assCount, "\r\n\0", 3);
	at_send_cmd("\r\nAT+CIPSHUT\r\n", AT_COMMAND_RUN);
	//vTaskDelay(2000);
	at_send_cmd("AT+CSTT=\"hologram\"\r\n", AT_COMMAND_WRITE);
	vTaskDelay(100);
	at_send_cmd("AT+CIICR\r\n", AT_COMMAND_RUN);
	//vTaskDelay(100);
	at_send_cmd("AT+CIFSR\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"\r\n", AT_COMMAND_WRITE);
	vTaskDelay(1000);
	at_send_cmd(assBuffer, AT_COMMAND_WRITE);
	at_send_cmd(buffer,AT_COMMAND_WRITE);
	vTaskDelay(200);
	
	delete[] assBuffer;
	delete[] buffer;
}

void tcp_read(void)
{
	gsm_usart._printf("\r\nAT+CIPSHUT\r\n");
	vTaskDelay(2000);
	gsm_usart._printf("AT+CSTT=\"hologram\"\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CIICR\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CIFSR\r\n");
	vTaskDelay(100);
	gsm_usart._printf("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"\r\n");
	vTaskDelay(2000);
	gsm_usart._printf("AT+CIPSEND=73\r\n");
	vTaskDelay(100);
}

