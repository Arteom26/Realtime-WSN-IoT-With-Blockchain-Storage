#include "AT_commands.h"
#include "UART_setup.h"

void GSM_ready(void)
{
	serial_write("AT\r\n");
}

void get_IMEI(void)
{
	serial_write("AT+GSN\r\n");
}

void is_SIM_ready(void)
{
	serial_write("AT+CPIN?\r\n");
}

void get_SIM_identity(void)
{
	serial_write("AT+CCID\r\n"); // AT+CIMI , AT+CCID
}

void select_network(void) //change to choice later
{
	serial_write("AT+COPS=0\r\n"); //- Automatic AT+COPS=0, Manual AT+COPS=1
}

void get_network_status(void)
{
	serial_write("AT+CREG?\r\n"); 
}

void set_APN(void)
{
	serial_write("AT+CGDCONT\r\n"); 
}

void check_signal_strength(void) 
{
	serial_write("AT+CSQ\r\n"); 
}

void send_Data(void) 
{
	serial_write("AT+CGDATA\r\n"); 
}

void set_SMS_format(void)
{
	serial_write("AT+CMGF\r\n");
}

void show_errors(void)
{
	serial_write("AT+CMEE=2\r\n");  //cmee = 0: display ERROR, 1: Show numerical error, 2: show verbose Error
}

void reset_module(void)
{
	serial_write("AT&F0\r\n"); //reset to factory settings
}

