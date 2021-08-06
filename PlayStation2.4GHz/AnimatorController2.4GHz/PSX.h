//-----------------------------------------------------------------------------
//
//  PSX.h
//
//  Swallowtail PSX Firmware
//  PSX Interface Firmware
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

//Button Access Macros
#define PSX_SLCT 8
#define PSX_R3 9
#define PSX_L3 10
#define PSX_STRT 11
#define PSX_UP 12
#define PSX_RGHT 13
#define PSX_DOWN 14
#define PSX_LEFT 15
#define PSX_L2 0
#define PSX_R2 1
#define PSX_L1 2
#define PSX_R1 3
#define PSX_TRGL 4
#define PSX_CIRC 5
#define PSX_X 6
#define PSX_SQR 7

#define ATT_DELAY_US 0

#define ACK PB6
#define MOTOR PB4
#define DDR_PSX DDRB
#define PORT_PSX PORTB
#define BIT_SET(byte, bit) (byte & (1<<bit))

/******************** Includes ***************************/

#include <avr/io.h>
#include "SPI.h"

/******************* Globals *****************************/

//Struct for holding the status of the standard controller
typedef struct PSXControllerStatus {
	uint8_t id; // Controller's ID
	uint16_t buttons; // digital buttons bitfield (little endian)
	uint8_t joylx; // left analogue joystick X (0-255)
	uint8_t joyly; // left analogue joystick Y (0-255)
	uint8_t joyrx; // right second analogue joystick X (0-255)
	uint8_t joyry; // right second analogue joystick Y (0-255)
} PSXControllerStatus;

/******************** Functions **************************/

//Initialize the USI on the ATmega168/328 for Three-Wire Operation
void PSX_init(){
	//Initialize the output for SPI Master Mode
	DDR_PSX |= (0<<ACK);
	//Set the default values for outputs to zero and inputs to have pull-up resistors
	PORT_PSX |= (1<<ACK);
	//Initialize the SPI Connection
	SPI_init();
	return; //Return to call point
}

//Writes the byte into the device
uint8_t PSX_Read(PSXControllerStatus *controller){
	//Wake up the controller with the ATT (Attention Line)
	SPI_Enable();
	//Send 0x01 to receive the controller ID
	SPI_Transfer(0x01);
	_delay_us(ATT_DELAY_US);
	//Transfer returns controller ID
	//Send 0x42 and if we received a 0x5A, the controller is ready for send data
	controller->id = SPI_Transfer(0x42);
	_delay_us(ATT_DELAY_US);
	SPI_Transfer(0x00);
	_delay_us(ATT_DELAY_US);
	//Data is ready and sending the two bytes for button status
	uint8_t upper = ~SPI_Transfer(0xFF); //First Byte
	_delay_us(ATT_DELAY_US);
	controller->buttons = (uint16_t)(upper << 8) | (~SPI_Transfer(0xFF) & 0xFF);
	_delay_us(ATT_DELAY_US);
	controller->joyrx = ~SPI_Transfer(0xFF);
	_delay_us(ATT_DELAY_US);
	controller->joyry = ~SPI_Transfer(0xFF);
	_delay_us(ATT_DELAY_US);
	controller->joylx = ~SPI_Transfer(0xFF);
	_delay_us(ATT_DELAY_US);
	controller->joyly = ~SPI_Transfer(0xFF);
	_delay_us(ATT_DELAY_US);
	SPI_Disable();
	//Buttons are active low but inverted to appear as active high
	//Return 1 to indicate success
	return 1;
}

/******************** Interrupt Service Routines *********/
