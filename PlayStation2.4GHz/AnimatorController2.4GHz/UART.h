//-----------------------------------------------------------------------------
//
//  UART_atmega328p.h
//
//  Swallowtail UART Firmware for ATmega328P
//  AVR (ATmega328P) UART Firmware
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

#define BIT_SET(byte, bit) (byte & (1<<bit))

/******************** Includes ***************************/

#include <avr/io.h>

/******************* Globals *****************************/


/******************** Functions **************************/

//Function for setting up the USART communication for the ATmega328P
void USART_init(uint16_t baud){
	/*
		Pin Descriptions:
		PD0: Used for USART TX: Output (1)
		PD1: Used for USART RX: Input (0)
	*/
	DDRD |= (1<<PD0) | (0<<PD1);
	//Set the default values for outputs to zero and inputs to have pull-up resistors
	PORTD |= (0<<PD0) | (1<<PD1);
	//Set the baud rate
	uint16_t UBBR = (uint16_t) ((F_CPU / (baud * 16UL)) - 1);
	UBRR0H = (uint8_t)(UBBR>>8);
	UBRR0L = (uint8_t)UBBR;
	//Enable the receiver with interrupts
	UCSR0B = (1<<RXEN0) | (1<<RXCIE0);
	//Set-up the frame format for the USART communication (8-bits 1 stop bit Parity Disabled)
	UCSR0C = (0<<USBS0) | (3<<UCSZ00);
	return; //Go back to previous location
}

//Function to empty the USART Receiver and Transmit Buffer
uint8_t USART_flush(){
	uint8_t dummy = 0x00; //Dummy variable to clear the buffer with
	//Check if the RXC0 flag is cleared
	while(UCSR0A & (1<<RXC0)){
		dummy = UDR0; //Flush the FIFO buffer
	}
	return dummy; //Go back to previous location
}

//Function to read the UDR0 Buffer from the receive
uint8_t USART_recieve(){
	//Wait for data to be received
	while(!(UCSR0A & (1<<RXC0)));
	//Return received data from the buffer
	return UDR0; //Go back to previous location
}

//Function to send the UDR0 Buffer
void USART_transmit(uint8_t data){
	//Wait for the transmit buffer to empty
	while(!(UCSR0A & (1<<UDRE0)));
	//Return received data from the buffer
	UDR0 = data; //Go back to previous location
}


/******************** Interrupt Service Routines *********/
