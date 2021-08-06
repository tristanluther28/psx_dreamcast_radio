//-----------------------------------------------------------------------------
//
//  main.c
//
//  Swallowtail Animator Controller 2.4GHz Firmware
//  AVR (ATmega328P) Animator Controller 2.4GHz Firmware
//
//  Copyright (c) 2021 Swallowtail Electronics
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sub-license,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//  Web:    http://tristanluther.com
//  Email:  tristanluther28@gmail.com
//
//-----------------------------------------------------------------------------

/******************** Macros *****************************/
#ifndef F_CPU
#define F_CPU 1000000UL //Set clock speed to 1MHz (Internal 8MHz with CLK/8)
#endif

#define BIT_SET(byte, bit) (byte & (1<<bit))

/******************** Includes ***************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

/******************* Globals *****************************/

//Must use the static keyword or the compiler will welcome itself to overwrite these memory locations in SRAM
//Must use volatile keyword or the compiler will optimize out any variables that are not seen in main (only appear in ISR)


/******************* Local Includes **********************/
#include "PSX.h"
#include "nRF24L01.h"

/******************** Functions **************************/


/********** Interrupt Service Routines *******************/


/******************** Main *******************************/
int main(void)
{
	//nRF RX Address (5 bytes wide)
	static uint8_t rx_address[5] = {0x12, 0x12, 0x12, 0x12, 0x12};
	//nRF TX Address (5 bytes wide)
	static uint8_t tx_address[5] = {0x12, 0x12, 0x12, 0x12, 0x12};
		
	//Buffer for transmitting data
	static uint8_t tx_buffer[5];
	
	//Initialize the debug output
	DDRB |= (1<<PB0);
	//Set the default values for outputs to zero and inputs to have pull-up resistors
	PORTB |= (0<<PB0);
	
	//Initialize the PS1 Communications
	PSX_init();
	//Initialize the nRF24L01 Communications as a transmitter
	nRF24L01_init(TX, rx_address, tx_address);
	
	//Set interrupts
	sei();
	
	/* State machine loop */
	while (1)
	{
		//Get the current controller status
		static PSXControllerStatus controller;
		PSX_Read(&controller);
		//Put controller data into the tx_buffer to be transmitted
		tx_buffer[0] = ((uint16_t)controller.buttons >> 0) & 0xFF; //First byte of digital buttons
		tx_buffer[1] = ((uint16_t)controller.buttons >> 8) & 0xFF; //Second byte of digital buttons
		tx_buffer[2] = controller.joylx; //Left joystick for direction (X-coord)
		tx_buffer[3] = controller.joyly; //Left joystick for direction (Y-coord)
		tx_buffer[4] = 0x00;
		
		//Transmit the controller data
		nRF24L01_Transmit(tx_buffer);
		
		//TODO improve buffer overflow issue
		//Check that the transmission was successful (If MAX_RT is 1 then the transmission failed)
		if((nRF24L01_ReadRegister(STATUS) & (1<<MAX_RT)) != 0){
			PORTB |= (1<<PB0);
			//Reset the nRF
			nRF24L01_Reset();
			PORTB &= ~(1<<PB0);
		}
		_delay_ms(20);
		
	}
}



