#ifndef __RTC_H
#define __RTC_H

#include "saml21g18b.h"
#include <stdint.h>

#define REF_YEAR 2020

// Timestamp struct
typedef struct{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}timestamp;

class RTC{
	public:
		RTC();
		void set_timestamp(timestamp);
		timestamp get_timestamp();
		char* get_string_timestamp();
	
	private:
		timestamp time;
	
		timestamp get_timestamp_from_reg();
};

extern RTC rtc;

#endif
