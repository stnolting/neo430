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
// # Stephan Nolting, Hannover, Germany                                                 29.04.2018 #
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
inline void clear_irq_buffer(void);
void cpu_delay(uint16_t t);
void cpu_delay_ms(uint16_t ms);
inline void soft_reset(void);
inline void jump_address(uint16_t addr);
inline void call_address(uint16_t addr);
inline uint16_t __bswap(uint16_t a);
inline uint16_t __combine_bytes(uint8_t hi, uint8_t lo);
inline uint16_t __dadd(uint16_t a, uint16_t b);
void __memset(uint8_t *dst, uint8_t data, uint16_t num);
uint8_t __memcmp(uint8_t *dst, uint8_t *src, uint16_t num);
void __memcpy(uint8_t *dst, uint8_t *src, uint16_t num);
uint16_t __bit_rev16(uint16_t x);
uint32_t __xorshift32(void);


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
 * INFO Clear CPU pending IRQ buffer
 * ------------------------------------------------------------ */
inline void clear_irq_buffer(void){

  asm volatile ("bis %0, r2" : : "i" (1<<Q_FLAG));
  // no need to reset the flag as it automatically clears again
}


/* ------------------------------------------------------------
 * INFO Simple wait function
 * PARAM Amount of ~2^16 machine cycles to wait
 * ------------------------------------------------------------ */
void cpu_delay(uint16_t t) {

  register uint16_t i = 0;
  while (t--) {
    for (i=0; i<0xFFFF; i++)
      asm volatile ("nop");
  }
}


/* ------------------------------------------------------------
 * INFO Waits <ms> microseconds (not very precise!)
 * PARAM ms time in microseconds to wait
 * ------------------------------------------------------------ */
void cpu_delay_ms(uint16_t ms) {

  // empirical ;)
  uint32_t a = ((uint32_t)CLOCKSPEED_HI) << 1;
  register uint32_t cnt = a * (uint32_t)ms;

  while(cnt--) {
    asm volatile ("nop");
  }
}


/* ------------------------------------------------------------
 * INFO Perform a soft reset by jumping to beginning of IMEM
 * ------------------------------------------------------------ */
inline void soft_reset(void) {

  asm volatile ("mov #0x0000, r0");
}


/* ------------------------------------------------------------
 * INFO Jump to address
 * PARAM Destination address
 * ------------------------------------------------------------ */
inline void jump_address(uint16_t addr) {

  register uint16_t r = addr;
  asm volatile ("mov %0, r0" : : "r" (r));
}


/* ------------------------------------------------------------
 * INFO Call address and save return address to stack
 * PARAM Destination address
 * ------------------------------------------------------------ */
inline void call_address(uint16_t addr) {

  register uint16_t r = addr;
  asm volatile ("call %0" : : "r" (r));
}


/* ------------------------------------------------------------
 * INFO Perform byte swap of 16-bit word (e.g., for endianness conversion)
 * PARAM 16-bit input word
 * RETURN 16-bit word with swapped bytes
 * ------------------------------------------------------------ */
inline uint16_t __bswap(uint16_t a) {

  register uint16_t r = a;
  asm volatile ("swpb %0, %1" : "=r" (r) : "r" (r));
  return r;
}


/* ------------------------------------------------------------
 * INFO Combine two bytes into one word
 * PARAM hi will be put in result's high byte
 * PARAM lo will be put in result's low byte
 * RETURN 16-bit combined word
 * ------------------------------------------------------------ */
inline uint16_t __combine_bytes(uint8_t hi, uint8_t lo) {

  register uint16_t r = __bswap((uint16_t)hi);
  return r | (uint16_t)lo;
}


/* ------------------------------------------------------------
 * INFO Binary-coded decimal addition
 * WARNING Make sure the DADD unit is syntheszied!!!!
 * PARAM 2x 16-bit BCD operands (4 digits)
 * RETURN 16-bit BCD result (4 digits)
 * ------------------------------------------------------------ */
inline uint16_t __dadd(uint16_t a, uint16_t b) {

  register uint16_t z = a;
  asm volatile ("clrc");
  asm volatile ("dadd %[b], %[z]" : [z] "=r" (z) : "[z]" (z), [b] "r" (b));
  return z;
}


/* ------------------------------------------------------------
 * INFO Memory initialization (byte-wise)
 * PARAM dst: Byte-pointer to beginning of target memory space
 * PARAM data: Init data
 * PARAM num: Number of bytes to initialize
 * ------------------------------------------------------------ */
void __memset(uint8_t *dst, uint8_t data, uint16_t num) {

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
uint8_t __memcmp(uint8_t *dst, uint8_t *src, uint16_t num) {

  while (num--) {
    if (*dst++ != *src++)
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
void __memcpy(uint8_t *dst, uint8_t *src, uint16_t num) {

  while (num--)
    *dst++ = *src++;
}


/* ------------------------------------------------------------
 * INFO 16-bit bit reversal
 * PARAM input operand to be reversed
 * RETURN reversed bit pattern
 * ------------------------------------------------------------ */
uint16_t __bit_rev16(uint16_t x) {

  register uint16_t z = x;
  register uint16_t y = 0;
  uint8_t i = 0;

  for (i=0; i<8; i++) { // two-times unrolled
    asm volatile ("rrc %[a], %[b]" : [b] "=r" (z) : "[b]" (z), [a] "r" (z));
    asm volatile ("rlc %[c], %[d]" : [d] "=r" (y) : "[d]" (y), [c] "r" (y));

    asm volatile ("rrc %[a], %[b]" : [b] "=r" (z) : "[b]" (z), [a] "r" (z));
    asm volatile ("rlc %[c], %[d]" : [d] "=r" (y) : "[d]" (y), [c] "r" (y));
  }
  return y;
}


/* ------------------------------------------------------------
 * INFO Pseudo-random number generator
 * RETURN 32-bit random data
 * ------------------------------------------------------------ */
uint32_t __xorshift32(void) {

  static uint32_t x32 = 314159265;

  x32 ^= x32 << 13;
  x32 ^= x32 >> 17;
  x32 ^= x32 << 5;

  return x32;
}


#endif // neo430_cpu_h
