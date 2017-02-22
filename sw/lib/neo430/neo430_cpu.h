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
// #  Stephan Nolting, Hannover, Germany                                               16.02.2017  #
// #################################################################################################

#ifndef neo430_cpu_h
#define neo430_cpu_h

// prototypes
inline void eint(void);
inline void dint(void);
inline uint16_t get_sp(void);
inline uint16_t get_sreg(void);
inline void set_sreg(uint16_t d);
inline void sleep(void);
void cpu_delay(uint16_t t);
void _memset(uint8_t *dst, uint8_t data, uint16_t num);
uint8_t _memcmp(uint8_t *dst, uint8_t *src, uint16_t num);
void _memcpy(uint8_t *dst, uint8_t *src, uint16_t num);
void soft_reset(void);


/* ------------------------------------------------------------
 * INFO Enable global interrupt flag
 * ------------------------------------------------------------ */
inline void eint(void){

  asm volatile ("eint");
}


/* ------------------------------------------------------------
 * INFO Disable global interrupt flag
 * ------------------------------------------------------------ */
inline void dint(void){

  asm volatile ("dint");
  asm volatile ("nop");
}


/* ------------------------------------------------------------
 * INFO Read stack pointer (for debugging only)
 * RETURN current stack pointer
 * ------------------------------------------------------------ */
inline uint16_t get_sp(void){

  register uint16_t d;
  asm volatile ("mov r1, %0" : "=r" (d));
  uint16_t r = d;
  return r;
}


/* ------------------------------------------------------------
 * INFO Read status register
 * RETURN current status register
 * ------------------------------------------------------------ */
inline uint16_t get_sreg(void){

  register uint16_t d;
  asm volatile ("mov r2, %0" : "=r" (d));
  uint16_t r = d;
  return r;
}


/* ------------------------------------------------------------
 * INFO Set status register
 * PARAM d new value for status register
 * ------------------------------------------------------------ */
inline void set_sreg(uint16_t d){

  register uint16_t r = d;
  asm volatile ("mov %0, r2" : : "r" (r));
}


/* ------------------------------------------------------------
 * INFO Set CPU to sleep mode
 * ------------------------------------------------------------ */
inline void sleep(void){

  asm volatile ("bis %0, r2" : : "i" (1<<S_FLAG));
}


/* ------------------------------------------------------------
 * INFO Simple wait function
 * PARAM Amount of ~2^16 cycles to wait
 * ------------------------------------------------------------ */
void cpu_delay(uint16_t t) {

  register uint16_t i = 0;
  while (t--) {
    for (i=0; i<0xFFFF; i++)
      asm volatile ("nop");
  }
}


/* ------------------------------------------------------------
 * INFO Memory initialization
 * PARAM dst: Pointer to beginning of target memory space
 * PARAM data: Init data
 * PARAM num: Number of bytes to initialize
 * ------------------------------------------------------------ */
void _memset(uint8_t *dst, uint8_t data, uint16_t num) {

  while (num--)
    *dst++ = data;
}


/* ------------------------------------------------------------
 * INFO Compare memory to memory
 * PARAM dst: Pointer to beginning of first memory space
 * PARAM src: Pointer to beginning of second memory space
 * PARAM num: Number of bytes to compare
 * RETURN 0 if src == dst
 * ------------------------------------------------------------ */
uint8_t _memcmp(uint8_t *dst, uint8_t *src, uint16_t num) {

  while (num--) {
    if ((*dst++ - *src++) != 0)
      return 1;
  }
  return 0;
}


/* ------------------------------------------------------------
 * INFO Copy memory space SRC to DST (byte by byte)
 * PARAM dst: Pointer to beginning destination memory space
 * PARAM src: Pointer to beginning source memory space
 * PARAM num: Number of bytes to copy
 * ------------------------------------------------------------ */
void _memcpy(uint8_t *dst, uint8_t *src, uint16_t num) {

  while (num--)
    *dst++ = *src++;
}


/* ------------------------------------------------------------
 * INFO Perform a soft reset by jumping to beginning of IMEM
 * ------------------------------------------------------------ */
void soft_reset(void) {

  asm volatile ("mov #0x0000, r0");
}


#endif // neo430_cpu_h
