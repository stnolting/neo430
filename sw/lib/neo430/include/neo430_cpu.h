// #################################################################################################
// #  < neo430_cpu.h - CPU helper functions >                                                      #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: https://github.com/stnolting/neo430        #
// # Copyright by Stephan Nolting: stnolting@gmail.com                                             #
// #                                                                                               #
// # This source file may be used and distributed without restriction provided that this copyright #
// # statement is not removed from the file and that any derivative work contains the original     #
// # copyright notice and the associated disclaimer.                                               #
// #                                                                                               #
// # This source file is free software; you can redistribute it and/or modify it under the terms   #
// # of the GNU Lesser General Public License as published by the Free Software Foundation,        #
// # either version 3 of the License, or (at your option) any later version.                       #
// #                                                                                               #
// # This source is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
// # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     #
// # See the GNU Lesser General Public License for more details.                                   #
// #                                                                                               #
// # You should have received a copy of the GNU Lesser General Public License along with this      #
// # source; if not, download it from https://www.gnu.org/licenses/lgpl-3.0.en.html                #
// # ********************************************************************************************* #
// # Stephan Nolting, Hannover, Germany                                                 12.02.2020 #
// #################################################################################################

#ifndef neo430_cpu_h
#define neo430_cpu_h

// prototypes
void     neo430_eint(void);                                       // enable global interrupts
void     neo430_dint(void);                                       // disable global interrupts
uint16_t neo430_get_sp(void);                                     // get stack pointer
uint16_t neo430_get_sreg(void);                                   // get status register
void     neo430_set_sreg(uint16_t d);                             // set status register
uint16_t neo430_get_parity(uint16_t d);                           // get parity (EXTENDED ALU OPERATION!)
void     neo430_sleep(void);                                      // set CPU to sleep mode
void     neo430_clear_irq_buffer(void);                           // clear pending IRQs
void     neo430_cpu_delay(uint16_t t);                            // wait cycles
void     neo430_cpu_delay_ms(uint16_t ms);                        // wait ms
void     neo430_soft_reset(void);                                 // perform soft reset
void     neo430_jump_address(uint16_t addr);                      // jump to certain address
void     neo430_call_address(uint16_t addr);                      // call certain address
uint16_t neo430_bswap(uint16_t a);                                // swap bytes in word
uint16_t neo430_combine_bytes(uint8_t hi, uint8_t lo);            // combine two bytes into a single word
void     neo430_memset(uint8_t *dst, uint8_t data, uint16_t num); // set num bytes in memory
uint8_t  neo430_memcmp(uint8_t *dst, uint8_t *src, uint16_t num); // compare num bytes in memory
void     neo430_memcpy(uint8_t *dst, uint8_t *src, uint16_t num); // copy num bytes from memory to memory
uint16_t neo430_bit_rev16(uint16_t x);                            // reverse bit order of word
uint16_t neo430_rotate_right_w(uint16_t x);                       // rotate right word by one bit
uint16_t neo430_rotate_left_w(uint16_t x);                        // rotate left word by one bit
uint8_t  neo430_rotate_right_b(uint8_t x);                        // rotate right byte by one bit
uint8_t  neo430_rotate_left_b(uint8_t x);                         // rotate left byte by one bit
uint32_t neo430_xorshift32(void);                                 // simple PRNG

#endif // neo430_cpu_h
