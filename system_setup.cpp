#ifndef __SYSTEM_SETUP_C
#define __SYSTEM_SETUP_C

#include "system_setup.h"
#include "system_saml21.h"
#include "RTC.h"

void setup_system(void){
	uint32_t *nvc = (uint32_t*)0x00806020;
	uint32_t osc32k_cal = (*nvc >> 6)&0x7F;
	
	// 32kHz and 1kHz Oscillator Setup => will output on clock source 0
	OSC32KCTRL_REGS->OSC32KCTRL_OSC32K = 0x407;// Enable the 32kHz internal oscillator => use 18 clock cycles for startup
	OSC32KCTRL_REGS->OSC32KCTRL_OSC32K |= (osc32k_cal << 16);
	OSC32KCTRL_REGS->OSC32KCTRL_RTCCTRL = 0x2;// 1kHz oscillator input source to RTC
	while((OSC32KCTRL_REGS->OSC32KCTRL_STATUS&0x2) == 0);
	PORT_REGS->GROUP[0].PORT_PINCFG[14] |= 0x1;
	PORT_REGS->GROUP[0].PORT_PMUX[7] = 0x7;
	
	// GCLK Setup
	GCLK_REGS->GCLK_GENCTRL[1] = 0x104;// Setup 32kHz oscillator on clock source 1
	GCLK_REGS->GCLK_GENCTRL[2] = 0x106;// Setup internal oscillator on clock source 2
	GCLK_REGS->GCLK_PCHCTRL[1] = 0x40;// Enable DPLL and setup GCLK source => 32kHz oscillator
	GCLK_REGS->GCLK_PCHCTRL[2] = 0x41;// Enable DPLL and setup GCLK source => 32kHz oscillator
	
	// 48MHz main clock setup
	OSCCTRL_REGS->OSCCTRL_DPLLCTRLB |= 0x20;// Use GCLK as the source and setup clock division
	OSCCTRL_REGS->OSCCTRL_DPLLPRESC = 0x0;// Divide output clock by 0
	OSCCTRL_REGS->OSCCTRL_DPLLRATIO = 4;
	while((OSCCTRL_REGS->OSCCTRL_DPLLSYNCBUSY&0x4) != 0);
	OSCCTRL_REGS->OSCCTRL_DPLLCTRLA = 0x2;// Enable PLL Output
	while((OSCCTRL_REGS->OSCCTRL_DPLLSYNCBUSY&0x2) != 0);
	while((OSCCTRL_REGS->OSCCTRL_DPLLSTATUS&0x1) == 0);// Wait for PLL to lock
	
	// 48Mhz DFLL Setup
	uint32_t bits = (*nvc >> 26);// Get the proper bits
	OSCCTRL_REGS->OSCCTRL_DFLLVAL = (bits << 10)|0x292;// Fine frequency control
	OSCCTRL_REGS->OSCCTRL_DFLLCTRL = 0x2;
	
	SystemCoreClock = 16000000;// 32MHz system core clock
	
	GCLK_REGS->GCLK_GENCTRL[0] = 0x906;// Change cpu clock from internal clock to PLL
	OSCCTRL_REGS->OSCCTRL_OSC16MCTRL |= 0xC;// Setup internal oscillator speed to 16MHz
	
	// Real-time clock setup
	setup_rtc();
	
	// DMA setup
}

#endif
