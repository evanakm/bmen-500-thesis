// Originally written November 2018
// by Evan Meikleham

#ifndef _THESIS_H_
#define _THESIS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
Pins 8 to 13 (on Master and Slave) are going to be the bus.
For now we're only using the 4 LSB, since a byte requires two
read cycles anyway.
*/
#define ENABLE_WRITE DDRB |= 0x3f
#define ENABLE_READ DDRB &= 0xc0

#define WRITE_BUS(x) PORTB = (x)
#define READ_BUS inputByte = PINB

// Commands for both Master and Slave

#define NOP __asm__("nop\n\t")

enum command { 	CMD_WRITE  = 0b00000000,
				CMD_READ_1 = 0b00000010,
				CMD_READ_N = 0b00000011,
				CMD_RESET  = 0b00001111};

#ifdef __cplusplus
} // extern "C"
#endif

#endif //include guard
