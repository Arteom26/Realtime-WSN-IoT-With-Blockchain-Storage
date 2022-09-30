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

void setup_rtc();
void set_timestamp(timestamp* current_time);
void get_timestamp(timestamp* time);

#endif
