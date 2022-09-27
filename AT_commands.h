#ifndef __AT_COMMANDS_H
#define __AT_COMMANDS_H

/*send AT, response OK*/
void GSM_ready(void); //AT

void get_IMEI(void); //AT+GSN
void is_SIM_ready(void); //AT+CPIN?
void get_SIM_identity(void); // AT+CIMI , AT+CCID
void select_network (void); //- Automatic AT+COPS=0, Manual AT+COPS=1
void get_network_status(void); // AT+CREG?
void set_APN(void); //AT+CGDCONT
void check_signal_strength(void); //AT+CSQ
void send_Data(void); //AT+CGDATA
void set_SMS_format(void); //AT+CMGF
void show_errors(void); //AT+CMEE=2
void reset_module(void);
//"AT+CPSI?" show mode(CAT-M1, NB-IoT, etc), as well as the cellular band
//"AT+CNSMOD?" Configure CAT-M or NB-IoT Band (GSM/EGPRS/CAT-M1/NB-IoT)
//"AT+CGMR" Check Firmware Version "B01" is the first version.
#endif
