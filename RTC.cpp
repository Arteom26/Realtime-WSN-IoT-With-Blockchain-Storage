#include "RTC.h"

void setup_rtc(){
	//RTC_REGS->MODE2.RTC_CTRLA |= 0x1B0A;// Setup RTC and enable the counter
}

// Set a new RTC timestamp
void set_timestamp(timestamp* current_time){
	uint8_t year = current_time->year - REF_YEAR;
	RTC_REGS->MODE2.RTC_CLOCK = ((year << 26)&0x3F)|((current_time->month) << 22&0xF)|((current_time->day << 17)&0x1F)\
	|((current_time->hour << 12)&0x1F)|((current_time->minute << 6)&0x3F)|(current_time->second&0x3F);
}

// Get the current RTC timestamp
void get_timestamp(timestamp* time){
	
}
