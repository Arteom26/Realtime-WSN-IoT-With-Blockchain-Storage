#include "SendingData.h"
#include "RTC.h"
#include <Math.h>

SemaphoreHandle_t sendingData = xSemaphoreCreateBinary();

// Converts integer to ascii
char* int_to_ascii_base10(int number, int places){
	int buffer_size = (int)log10f((float)places);
	char* buffer = new char[buffer_size];
	
	int dataCount = 0;
	for(int i = places; i >=1; i/=10){
		char x = (number - (number % i))/i % 10 + 0x30;
		buffer[dataCount] = x;
		dataCount++;
	}
	
	return buffer;// Use sizeof to get character length
}

void sendData(void* unused)
{
	xSemaphoreGive(sendingData);
	do{
		xSemaphoreGive(gsm_in_use);
	at_send_cmd("AT\r\n", AT_COMMAND_RUN);
	vTaskDelay(100);
	}while(xSemaphoreTake(gsm_in_use, 250) == pdFALSE);

	xSemaphoreGive(gsm_in_use);
	gsm_init();
	uint8_t dat[] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF};
	//tcp_write(dat);
	//send_to_firebase(dat, dat, 1);
	vTaskDelete(NULL);
}

void gsm_init(void)
{
	//at_send_cmd("AT\r\n", AT_COMMAND_RUN);
	at_send_cmd("ATE1\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+CFUN=0\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CFUN=1\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CIPSHUT\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CNMP=38\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CMNB=1\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CSTT=\"hologram\"\r\n", AT_COMMAND_WRITE);
	at_send_cmd("AT+CIICR\r\n", AT_COMMAND_RUN);
}

void send_to_firebase(uint8_t *data, uint8_t* mac, int position){
	xSemaphoreTake(sendingData, portMAX_DELAY);
	//at_send_cmd("AT+CCLK=\"23/01/09,14:35:00-24\"\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CSSLCFG=\"sslversion\",1,3\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+SHSSL=1,\"\"\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+CNACT=0\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CNACT=1\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+CNACT?\r\n", AT_COMMAND_RUN);
	
	at_send_cmd("AT+SHDISC\r\n", AT_COMMAND_RUN);// Make sure it is not connected to anything
	at_send_cmd("AT+SHCONF=\"URL\",\"https://wsn-iot-default-rtdb.firebaseio.com\"\r\n", AT_COMMAND_RUN);
	// These two commmands most likely are not needed
	at_send_cmd("AT+SHCONF=\"BODYLEN\",1024\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+SHCONF=\"HEADERLEN\",350\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+SHCONF?\r\n", AT_COMMAND_RUN);

	at_send_cmd("AT+SHCONN\r\n", AT_COMMAND_RUN);// Connect to server
	
	// Setup the header
	at_send_cmd("AT+SHCHEAD\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+SHAHEAD=\"Content-Type\",\"application/json\"\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+SHAHEAD=\"Cache-control\",\"no-cache\"\r\n", AT_COMMAND_RUN);
	//at_send_cmd("AT+SHAHEAD=\"Accept\",\"*/*\"\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+SHAHEAD=\"Connection\",\"keep-alive\"\r\n", AT_COMMAND_RUN);// Currently the only one needed
	
	//char* d = int_to_ascii_base10(10, 1000000);
	
	// Convert data to ascii
	uint32_t val = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) | ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32)\
			 | (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
	char asciiData[10];
	int dataCount = 0;
	for(int i = 100000;i >= 1; i /= 10){
		char x = (val - (val % i))/i % 10 + 0x30;
		asciiData[dataCount] = x;
		dataCount++;
	}
	
	// Setup the body
	// First need to convert position to ascii
	/*char dat[6];
	int ass = 0;
	for(int i = 1000;i >= 1; i /= 10){
		char x = (position - (position % i))/i % 10 + 0x30;
		dat[ass] = x;
		ass++;
	}*/
	
	char* dat = rtc.get_string_timestamp();
	int ass = 17;
	
	// Setup main body command with JSON embedded
	char buffer[128];
	std::memcpy(buffer, "AT+SHBOD=\"{\\\"", 13);
	std::memcpy(buffer+13, dat , ass);
	std::memcpy(buffer+13+ass, "\\\":\\\"", 5);
	std::memcpy(buffer+18+ass, asciiData, dataCount);
	std::memcpy(buffer+18+ass+dataCount, "\\\"}\",", 5);
	
	delete dat;
	
	char cntChar[2];
	int charCount = 7 + ass + dataCount;
	int countPos = 0;
	for(int i = 10;i >= 1; i /= 10){
		char x = (charCount - (charCount % i))/i % 10 + 0x30;
		cntChar[countPos] = x;
		countPos++;
	}
	
	std::memcpy(buffer+23+ass+dataCount, cntChar, 2);
	std::memcpy(buffer+25+ass+dataCount, "\r\n\0", 3);
	at_send_cmd(buffer, AT_COMMAND_RUN);
	// Send the data and disconnect from server
	// Convert mac address to ascii format
	char mac_addy[23];
	int byt = 0;
	for(int i = 0;i < 8;i++){
		for(int j = 1;j >= 0;j--){
			byt = mac[i];
			byt = (byt >> 4*j)&0xF;
			if(byt < 10)
				mac_addy[i*3+(1-j)] = byt + 0x30;
			else
				mac_addy[i*3+(1-j)] = byt + 0x37;
		}
		if(i == 7) break;
		mac_addy[2+i*3] = ':';
	}
	
	ass = 0;
	std::memcpy(buffer, "AT+SHREQ=\"/", 11);
	std::memcpy(buffer+11, mac_addy, 23);
	std::memcpy(buffer+34, ".json\",4\r\n\0",11);
	
	at_send_cmd(buffer, AT_COMMAND_RUN);// Runs a PUT command
	//at_send_cmd("AT+SHREQ=\"/namse.json\",4\r\n", AT_COMMAND_RUN);// Runs a PUT command
	//at_send_cmd("AT+SHREQ=\"/names.json\",1\r\n", AT_COMMAND_RUN); // For the GET command
	at_send_cmd("AT+SHDISC\r\n", AT_COMMAND_RUN);// Disconnect from the server
	
	xSemaphoreGive(sendingData);
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

