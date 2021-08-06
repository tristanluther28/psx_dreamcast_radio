//-----------------------------------------------------------------------------
//
//  nRF24L01.h
//
//  Swallowtail nRF24L01/+ Firmware
//  nRF24L01/+ Interface Firmware
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

// Memory Map
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17

// Bit Mnemonics
#define MASK_RX_DR  0x06
#define MASK_TX_DS  0x05
#define MASK_MAX_RT 0x04
#define EN_CRC      0x03
#define CRCO        0x02
#define PWR_UP      0x01
#define PRIM_RX     0x00
#define ENAA_P5     0x05
#define ENAA_P4     0x04
#define ENAA_P3     0x03
#define ENAA_P2     0x02
#define ENAA_P1     0x01
#define ENAA_P0     0x00
#define ERX_P5      0x05
#define ERX_P4      0x04
#define ERX_P3      0x03
#define ERX_P2      0x02
#define ERX_P1      0x01
#define ERX_P0      0x00
#define AW          0x00
#define ARD         0x04
#define ARC         0x00
#define PLL_LOCK    0x04
#define RF_DR       0x03
#define RF_PWR      0x01
#define LNA_HCURR   0x00
#define RX_DR       0x06
#define TX_DS       0x05
#define MAX_RT      0x04
#define RX_P_NO     0x01
#define TX_FULL     0x00
#define PLOS_CNT    0x04
#define ARC_CNT     0x00
#define TX_REUSE    0x06
#define FIFO_FULL   0x05
#define TX_EMPTY    0x04
#define RX_FULL     0x01
#define RX_EMPTY    0x00

// Instruction Mnemonics
#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

//Read and Write for the CE pin
#define WRITE 1
#define READ 0

//RX and TX Select
#define RX 0x1F
#define TX 0x1E

#define DDR_nRF24L01 DDRC
#define PORT_nRF24L01 PORTC
#define BIT_SET(byte, bit) (byte & (1<<bit))

//Physical Pin Configuration
#define CE PC1 //Chip enable (Indicates RX or TX Mode)
#define CSN PC0 //Chip Select
#define IRQ PC2 //Mask-able interrupt (Active Low)

/******************** Includes ***************************/

#include <avr/io.h>
#include <util/delay.h>
#include "SPI.h"

/******************* Globals *****************************/


/******************** Functions **************************/

//CSN enabled
void nRF24L01_Enable(){
	//CSN must be held low - nRF starts to listen for a command
	PORT_nRF24L01 &= ~(1<<CSN);
	return;
}

//CSN disables
void nRF24L01_Disable(){
	//CSN must be held high - nRF is no longer listening
	PORT_nRF24L01 |= (1<<CSN);
	return;
}

//Writes the byte into the device or returns a byte array if reading
uint8_t *nRF24L01_Transfer(uint8_t rwt, uint8_t reg, uint8_t *buffer, uint8_t length){
	//If the user wants to write add the correct prefix to the register
	if(rwt == WRITE){
		reg = W_REGISTER + reg;
	}
	//Create the return buffer
	static uint8_t returnBuff[32];
	
	_delay_us(10); //Be sure the previous command has finished executing
	nRF24L01_Enable();
	_delay_us(10);
	SPI_Transfer(reg); //Set the nRF starts to listen for command
	_delay_us(10);
	
	uint8_t i;
	for(i=0; i<length; i++){
		//If the user wants to read from the receiver
		if(rwt == READ && reg != W_TX_PAYLOAD){
			returnBuff[i] = SPI_Transfer(NOP); //Send dummy bytes to read the data
			_delay_us(10);
		}
		//Send the write data out to the buffer
		else{
			SPI_Transfer(buffer[i]); //Send the commands to the nRF one at a time
			_delay_us(10);
		}
	}
	nRF24L01_Disable();
	//Return 1 to indicate success
	return returnBuff;
}

void nRF24L01_Reset(){
	_delay_us(10);
	PORT_nRF24L01 &= ~(1<<CSN);
	_delay_us(10);
	SPI_Transfer(W_REGISTER + STATUS); //Write to the status registry
	_delay_us(10);
	SPI_Transfer(0x70);	//Reset all IRQ in STATUS registry
	_delay_us(10);
	PORT_nRF24L01 |= (1<<CSN);
}

uint8_t nRF24L01_ReadRegister(uint8_t reg){
	_delay_us(10); //Be sure the previous command has finished executing
	nRF24L01_Enable();
	_delay_us(10);
	SPI_Transfer(R_REGISTER + reg); //R_REGISTER set the nRF to reading mode (reg is the register to be read)
	_delay_us(10);
	reg = SPI_Transfer(NOP); //Send a no operation to receive the register contents
	_delay_us(10);
	nRF24L01_Disable();
	return reg; //Return the read register
}

//Initialize the USI on the ATmega168/328 for Three-Wire Operation
void nRF24L01_init(uint8_t mode, uint8_t *RX_Address, uint8_t *TX_Address){
	//Initialize the physical output
	DDR_nRF24L01 |= (1<<CE)|(1<<CSN)|(0<<IRQ);
	PORT_nRF24L01 |= (1<<CE)|(1<<CSN)|(0<<IRQ);
	//Initialize the SPI Connection
	SPI_init();
	
	uint8_t buffer[1]; //Buffer for holding set-up data (1 byte wide only needed for set-up)
	
	//Enable auto-acknowledgment (EN_AA) - Transmitter gets an auto response from the receiver when the transmission is successful (Must have same RF Address and it's channel)
	buffer[0] = 0x01;
	nRF24L01_Transfer(WRITE, EN_AA, buffer, 1);
	
	//Set-up the number of retry attempts and number of retry delay
	buffer[0] = 0x2F; //0b0010 1111 '2' sets up a 750us delay between every retry 'F' is the number of retries (15)
	nRF24L01_Transfer(WRITE, SETUP_RETR, buffer, 1);
	
	//Selects the number of enabled data pipes (1-5)
	buffer[0] = 0x01;
	nRF24L01_Transfer(WRITE, EN_RXADDR, buffer, 1); //Enable data pipe 0
	
	//RF Address width setup (Number of bytes wide is the the receiver address, keep it varied so other controllers don't get on your channel (1-5))
	buffer[0] = 0x03; //Address is 5-bytes wide (0b0000 0011 = 5 bytes)
	nRF24L01_Transfer(WRITE, SETUP_AW, buffer, 1);
	
	//RF Channel Set-up - Choose a Frequency 2.400 - 2.527GHz with 1MHz/bit-step
	buffer[0] = 0x01; //0b0000 0001 = 2.401GHz (must be the same on TX and RX)
	nRF24L01_Transfer(WRITE, RF_CH, buffer, 1);
	
	//RF Set-up - Power Mode and Data Speed (Different for the (+) version of the nRF)
	buffer[0]=0x07; //0b0000 0111 bit: 3='0' 1Mbps = longer range / bit: 2-1 power mode ('11' = -0dB, '00'= -18dB) // Make to 0x27 for the + model (allows for 250kbps mode)
	nRF24L01_Transfer(WRITE, RF_SETUP, buffer, 1);
	
	//Set-up the RX RF Address 5 bytes - Set the receiver address
	nRF24L01_Transfer(WRITE, RX_ADDR_P0, RX_Address, 5);
	
	//Set-up the TX RF Address 5 bytes - Set the transmitter address
	nRF24L01_Transfer(WRITE, TX_ADDR, TX_Address, 5);
	
	//Payload width set-up - Can be between 1-32 bytes (how many bytes to send per transmission)
	buffer[0] = 0x05; //Must be same for TX and RX
	nRF24L01_Transfer(WRITE, RX_PW_P0, buffer, 1);
	
	//CONFIG set-up - Boot the nRF and choose if it is suppose to be TX or RX
	//If this is a transmitter
	if(mode == TX){
		buffer[0] = 0x7E; //0b0001 1110 - bit: 0='0':transmitter or '1':receiver, bit: 1='1':power up
	}
	else{
		//Otherwise this is a receiver
		buffer[0] = 0x7F;
	}
	nRF24L01_Transfer(WRITE, CONFIG, buffer, 1);
	//Give the device 1.5ms to reach standby mode (CE=low)
	_delay_ms(100);
	return; //Return to call point
}

//Transmit the buffer given (32 bytes wide)
void nRF24L01_Transmit(uint8_t *buffer){
	//Flush the current transmit buffer
	nRF24L01_Transfer(READ, FLUSH_TX, buffer, 0);
	//Sends the data in buffer to the nRF
	nRF24L01_Transfer(READ, W_TX_PAYLOAD, buffer, 5); //User read operation since W_TX_PAYLOAD is on the highest byte level in thr nRF
	
	//Give 10ms after loading the nRF with payload before TX
	_delay_ms(10);
	//Pull CE high to transmit the data
	PORT_nRF24L01 |= (1<<CE);
	//Delay a bit for that data to go where it needs to
	_delay_us(20);
	//Stop transmitting
	PORT_nRF24L01 &= ~(1<<CE);
	//Long delay before next operation
	_delay_ms(10);
}

uint8_t *nRF24L01_Recieve(){
	uint8_t *buffer = 0x00; //Receive buffer
	//Set CE high to listen for data
	PORTB |= (1<<CE);
	_delay_ms(1000); //Listen for a second
	PORTB &= ~(1<<CE); //Stop listening
	//Read out the received message
	buffer = nRF24L01_Transfer(READ, R_RX_PAYLOAD, buffer, 5);
	nRF24L01_Reset();
	//return the received buffer
	return buffer;
}

/******************** Interrupt Service Routines *********/
