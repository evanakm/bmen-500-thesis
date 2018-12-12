// Originally written November 2018
// by Evan Meikleham

#ifndef _THESIS_H_
#define _THESIS_H_

#ifdef __cplusplus
extern "C" {
#endif

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
