// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
 *
 * Atmel Corporation
 *
 * File              : USI_TWI_Slave.c
 * Compiler          : IAR EWAAVR 3.20c
 * Revision          : $Revision: 1.14 $
 * Date              : $Date: Friday, December 09, 2005 17:25:38 UTC $
 * Updated by        : $Author: jtyssoe $
 *
 * Support mail      : avr@atmel.com
 *
 * Supported devices : All device with USI module can be used.
 *
 * AppNote           : AVR312 - Using the USI module as a I2C slave
 *
 * Description       : Example showing how to use the USI_TWI drivers;
 *                     Loops back received data.
 *
 *
 ****************************************************************************/
// Chip type           : ATtiny44
// Clock frequency     : 1,638400 MHz


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include "USI_TWI_Slave.h"

// TWI transmission commands
#define TWI_CMD_SET_OH_UH 0x00
#define TWI_CMD_SET_OH    0x01
#define TWI_CMD_SET_UH    0x02
#define TWI_CMD_SET_FAN   0x03

/*! Local variables
 */
static uint8_t TWI_RxBuf[TWI_RX_BUFFER_SIZE];
static uint8_t TWI_TxBuf[TWI_TX_BUFFER_SIZE];

/* \Brief The main function.
 * The program entry point. Initates TWI and enters eternal loop, waiting for data.
 */
int main(void)
/* should be void and noreturn ... */
__attribute__((naked));

int main(void) {
	unsigned char TWI_slaveAddress;
	int8_t RX_start, TX_start = 0;

	clock_prescale_set(clock_div_2);

	// Input/Output Ports initialization
	// Port A initialization
	// Func7=Out Func6=In Func5=Out Func4=In Func3=In Func2=In Func1=In Func0=In
	// State7=1 State6=T State5=1 State4=T State3=P State2=P State1=P State0=P
	PORTA = 0xAF;
	DDRA = 0xA0;

	// Port B initialization
	// Func3=In Func2=Out Func1=In Func0=In
	// State3=T State2=0 State1=T State0=T
	PORTB = 0x00;
	DDRB = 0x04;

	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 25,600 kHz
	// Mode: Fast PWM top=FFh
	// OC0A output: Disconnected
	// OC0B output: Disconnected
	TCCR0A = 0x03;
	TCCR0B = 0x03;
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;

	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: 1,600 kHz
	// Mode: Fast PWM top=ICR1
	// OC1A output: Discon.
	// OC1B output: Discon.
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer 1 Overflow Interrupt: Off
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	TCCR1A = 0x02;
	TCCR1B = 0x1D;
	//TCCR1B=0x1A;

	TCNT1H = 0x00;
	TCNT1L = 0x00;
	ICR1H = 0x0F;
	ICR1L = 0xFF;
	OCR1AH = 0x00;
	OCR1AL = 0x00;
	OCR1BH = 0x00;
	OCR1BL = 0x00;
	TIFR1 = 0x27;
	TIMSK1 = 0x02; // enable timer1 interrupts

	// Analog Comparator initialization
	// Analog Comparator: Off
	// Analog Comparator Input Capture by Timer/Counter 1: Off
	ACSR = 0x80;
	ADCSRB = 0x00;

	// Own TWI slave address
	TWI_slaveAddress = 0x50;
	USI_TWI_Slave_Initialise(TWI_slaveAddress, TWI_RxBuf, TWI_TxBuf);

	sei();
	// This loop runs forever. If the TWI Transceiver is busy the execution will just continue doing other operations.
	TWI_TxBuf[0] = 1;
	TWI_TxBuf[1] = 1;
	TWI_TxBuf[2] = 1;
	TWI_TxBuf[3] = 1;
	TWI_TxBuf[4] = 1;
	TWI_TxBuf[5] = 1;
	TWI_TxBuf[6] = 1;
	TWI_TxBuf[7] = 1;
	USI_TWI_Set_TX_Start(TX_start);

	for (;;) {

		if ((RX_start = USI_TWI_Data_In_Receive_Buffer()) != -1) {
			switch (TWI_RxBuf[RX_start]) {
			case 0:
				OCR1B = (TWI_RxBuf[RX_start + 1] << 4) + 0x0F;
				OCR1A = (TWI_RxBuf[RX_start + 2] << 4) + 0x0F;
				//TCNT1=0x0FF0;
				if (TWI_RxBuf[RX_start + 1])
					TCCR1A = 0x32;
				else
					TCCR1A = 0x02;
				if (TWI_RxBuf[RX_start + 2]) {
					TIFR1 = 0x03;
					TIMSK1 = 0x03;
				} else {
					TIMSK1 = 0x02;
					PORTA |= (1 << PA7);
				}
				break;

			case 1:
				OCR1B = (TWI_RxBuf[RX_start + 1] << 4) + 0x0F;
				if (TWI_RxBuf[RX_start + 1])
					TCCR1A = 0x32;
				else
					TCCR1A = 0x02;
				break;

			case 2:
				OCR1A = (TWI_RxBuf[RX_start + 1] << 4) + 0x0F;
				if (TWI_RxBuf[RX_start + 1]) {
					TIFR1 = 0x03;
					TIMSK1 = 0x03;
				} else {
					TIMSK1 = 0x02;
					PORTA |= (1 << PA7);
				}
				break;

			case 3:
				OCR0A = TWI_RxBuf[RX_start + 1];
				if (OCR0A)
					TCCR0A = 0x83;
				else
					TCCR0A = 0x03;
				break;

			default:
				break;
			}

			//RX_start = temp;
			TX_start = (TX_start + 4) & TWI_TX_BUFFER_MASK;
			TWI_TxBuf[TX_start] = 10;
			TWI_TxBuf[TX_start + 1] = 0;
			TWI_TxBuf[TX_start + 2] = 11;
			TWI_TxBuf[TX_start + 3] = 0;
			USI_TWI_Set_TX_Start(TX_start);
		}
		// Do something else while waiting for the TWI transceiver to complete.
		// Put own code here.
	}
}

/*! \mainpage
 * \section Intro Introduction
 * This documents data structures, functions, variables, defines, enums, and
 * typedefs in the software for application note AVR312.
 *
 * \section CI Compilation Info
 * This software was written for the IAR Embedded Workbench 4.11A.
 *
 * To make project:
 * <ol>
 * <li> Add the file main.c and USI_TWI_Slave to project.
 * <li> Under processor configuration, select any AVR device with USI module.
 * <li> Enable bit definitions in I/O include files
 * <li> High optimization on size is recommended for lower code size.
 * </ol>
 *
 * \section DI Device Info
 * The included source code is written for all devices with with a USI module.
 *
 * \section TS Target setup
 * This example is made to work together with the AVR310 USI TWI Master application note. In adition to connecting the TWI
 * pins, also connect PORTB to the LEDS. The code reads a message as a TWI slave and acts according to if it is a
 * general call, or an address call. If it is an address call, then the first byte is considered a command byte and
 * it then responds differently according to the commands.
 *
 */
ISR(TIM1_OVF_vect, ISR_NAKED)
{
	PORTA &= ~(1 << PA7); // results in CBI which does not affect SREG
	reti();
}

ISR(TIM1_COMPA_vect, ISR_NAKED)
{
	PORTA |= (1 << PA7); // results in SBI which does not affect SREG
	reti();
}

