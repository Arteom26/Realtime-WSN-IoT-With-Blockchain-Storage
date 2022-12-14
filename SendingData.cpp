#include "SendingData.h"

//void sendData(void* unused)
//{
//	
//}

void gsm_init(void)
{
	at_send_cmd("AT\r\n", AT_COMMAND_RUN);
	at_send_cmd("AT+CFUN=1\r\n", AT_COMMAND_RUN);
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

void tcp_write(void)
{
	at_send_cmd("\r\nAT+CIPSHUT\r\n", AT_COMMAND_RUN);
	//vTaskDelay(2000);
	at_send_cmd("AT+CSTT=\"hologram\"\r\n", AT_COMMAND_WRITE);
	//vTaskDelay(100);
	at_send_cmd("AT+CIICR\r\n", AT_COMMAND_RUN);
	//vTaskDelay(100);
	at_send_cmd("AT+CIFSR\r\n", AT_COMMAND_RUN);
	//vTaskDelay(100);
	at_send_cmd("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"\r\n", AT_COMMAND_WRITE);
	vTaskDelay(1000);
	at_send_cmd("AT+CIPSEND=84\r\n", AT_COMMAND_WRITE);
	//vTaskDelay(100);
	at_send_cmd("GET https://api.thingspeak.com/update?api_key=XB4GKI5NFDXXS0VU&field2=22&field3=44\r\n",AT_COMMAND_WRITE);
	//vTaskDelay(2000);
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

