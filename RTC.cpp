#include "RTC.h"
#include "SendingData.h"

using namespace std;

// All years will be in reference to 2020
RTC::RTC(){
	RTC_REGS->MODE2.RTC_CTRLA |= 0x9B0A;// Setup RTC and enable the counter
}
		
void RTC::set_timestamp(timestamp times){
	this->time = times;
			
	uint8_t year = times.year - REF_YEAR;
	RTC_REGS->MODE2.RTC_CLOCK = (((times.year - REF_YEAR) << 26))|((times.month) << 22)|((times.day << 17))\
		|((times.hour << 12))|((times.minute << 6))|(times.second);
}
		
timestamp RTC::get_timestamp(){
	timestamp tempTime = this->time;
	tempTime.year = time.year + REF_YEAR;// Readjust the year
			
	return tempTime;
}
		
char* RTC::get_string_timestamp(){
	// Timestamp format mm/dd/yy hh:mm:ss => always in 24hr time
	char* buffer = new char[18];// Null character included
	
	timestamp tempTime = get_timestamp_from_reg();
	tempTime.year = tempTime.year + REF_YEAR;// Readjust the year
	
	char* stuff = int_to_ascii_base10(time.month, 10);
	std::memcpy(buffer, stuff, 2);
	delete stuff;
	
	buffer[2] = '-';
	stuff = int_to_ascii_base10(tempTime.day, 10);
	std::memcpy(buffer+3, stuff, 2);
	delete stuff;
	
	buffer[5] = '-';
	stuff = int_to_ascii_base10(tempTime.year, 1000);
	std::memcpy(buffer+6, stuff+2, 2);
	delete stuff;
	
	buffer[8] = ' ';
	stuff = int_to_ascii_base10(tempTime.hour, 10);
	std::memcpy(buffer+9, stuff, 2);
	delete stuff;
	
	buffer[11] = ':';
	stuff = int_to_ascii_base10(tempTime.minute, 10);
	std::memcpy(buffer+12, stuff, 2);
	delete stuff;
	
	buffer[14] = ':';
	stuff = int_to_ascii_base10(tempTime.second, 10);
	std::memcpy(buffer+15, stuff, 2);
	delete stuff;
	buffer[17] = '\0';
	
	return buffer;
}

timestamp RTC::get_timestamp_from_reg(){
	timestamp currTime;
	
	currTime.year = RTC_REGS->MODE2.RTC_CLOCK >> 26;
	currTime.month = (RTC_REGS->MODE2.RTC_CLOCK >> 22) &0xF;
	currTime.day = (RTC_REGS->MODE2.RTC_CLOCK >> 17) &0x1F;
	currTime.hour = (RTC_REGS->MODE2.RTC_CLOCK >> 12) &0x1F;
	currTime.minute = (RTC_REGS->MODE2.RTC_CLOCK >> 6) &0x3F;
	currTime.second = RTC_REGS->MODE2.RTC_CLOCK & 0x3F;
	
	return currTime;
}
