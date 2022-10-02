#ifndef __SYSTEM_SETUP_C
#define __SYSTEM_SETUP_C

#include "system_setup.h"
#include "system_saml21.h"
#include "RTC.h"
#include "API.h"

uint16_t verifyP(uint16_t fcs, uint8_t *data, uint16_t len){
	//data++;
	while(len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *data++) & 0xFF];
	
	fcs ^= 0xFFFF;
	
	return fcs;
}

void setup_system(void){
	PM_REGS->PM_PLCFG = 0x2;// Set to high power mode(PL2)
	while(PM_REGS->PM_INTFLAG == 0);
	PM_REGS->PM_STDBYCFG |= 0x40;
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
	GCLK_REGS->GCLK_PCHCTRL[1] = 0x41;// Enable DPLL and setup GCLK source => 32kHz oscillator
	GCLK_REGS->GCLK_PCHCTRL[2] = 0x41;// Enable DPLL and setup GCLK source => 32kHz oscillator
	
	// 48MHz main clock setup
	OSCCTRL_REGS->OSCCTRL_DPLLCTRLB |= 0x20;// Use GCLK as the source and setup clock division
	OSCCTRL_REGS->OSCCTRL_DPLLPRESC = 0x0;// Divide output clock by 1
	OSCCTRL_REGS->OSCCTRL_DPLLRATIO = 750;// Current max value I can get out of PLL
	while((OSCCTRL_REGS->OSCCTRL_DPLLSYNCBUSY&0x4) != 0);
	OSCCTRL_REGS->OSCCTRL_DPLLCTRLA = 0x2;// Enable PLL Output
	while((OSCCTRL_REGS->OSCCTRL_DPLLSYNCBUSY&0x2) != 0);
	while((OSCCTRL_REGS->OSCCTRL_DPLLSTATUS&0x1) == 0);// Wait for PLL to lock
	
	// 48Mhz DFLL Setup
	uint32_t bits = (*nvc >> 26);// Get the proper bits
	OSCCTRL_REGS->OSCCTRL_DFLLVAL = (bits << 10)|0x0;// Fine frequency control(0x3FF max value)
	OSCCTRL_REGS->OSCCTRL_DFLLCTRL = 0x2;
	
	GCLK_REGS->GCLK_GENCTRL[0] = 0x108;// Change cpu clock from internal clock to PLL
	OSCCTRL_REGS->OSCCTRL_OSC16MCTRL |= 0xC;// Setup internal oscillator speed to 16MHz
	
	SystemCoreClock = 26500000;// Current max freq. I could achieve
	
	// Real-time clock setup
	setup_rtc();
	
	// DMA descriptor setup
	uint32_t *desc = (uint32_t*)0x30000000;// Channel for copying data between buffers for Smartmesh IP
	*desc++ = 0x01000C01;// Channel 0 descriptor
	*desc++ = (uint32_t)&SERCOM0_REGS->USART_INT.SERCOM_BAUD;
	*desc++ = 0x30000100;
	*desc++ = 0x00000000;
	for(int i = 0;i < 3;i++){// SERCOM descriptors setup
	*desc++ = 0x01000401;// Channel 1 descriptor
	*desc++ = 0x30000000;
	*desc++ = (uint32_t)&SERCOM0_REGS->USART_INT.SERCOM_DATA;
	*desc++ = 0x00000000;
	}
	
	// DMA initialization and setup
	// Channel 0 => Smartmesh IP data copying
	// Channels 1-3 => UART TX data transfer for SERCOM0-SERCOM2
	// Channel 4 => Misc. Copying tasks
	DMAC_REGS->DMAC_BASEADDR = 0x30000000;
	DMAC_REGS->DMAC_WRBADDR = 0x30000100;
	DMAC_REGS->DMAC_CTRL = 0xF02;
	DMAC_REGS->DMAC_CHCTRLB = 0x60;// Channel 0 config
	DMAC_REGS->DMAC_CHID = 0x1;// Set to channel 1
	DMAC_REGS->DMAC_CHCTRLB = 0x800260;
}
#endif
