// #################################################################################################
// #  < neo430_aux.h - Handy helper functions ;) >                                                 #
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
// #  Stephan Nolting, Hannover, Germany                                               29.12.2017  #
// #################################################################################################

#ifndef neo430_aux_h
#define neo430_aux_h

// prototypes
void __memset(uint8_t *dst, uint8_t data, uint16_t num);
uint8_t __memcmp(uint8_t *dst, uint8_t *src, uint16_t num);
void __memcpy(uint8_t *dst, uint8_t *src, uint16_t num);
uint16_t __bit_rev16(uint16_t x);
uint32_t __xorshift32(void);
uint8_t config_timer_period(uint32_t f_timer);
uint32_t hexstr_to_uint(char *buffer, uint8_t length);



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


/* ------------------------------------------------------------
 * INFO Configure Timer period
 * PARAM Timer frequency in Hz (1Hz ... F_CPU/2)
 * RETURN 0 if successful, -1 if error
 * ------------------------------------------------------------ */
uint8_t config_timer_period(uint32_t f_timer) {

  uint32_t clock = CLOCKSPEED_32bit;
  uint32_t ticks = (clock / (f_timer*2)) >> 1; // divide by lowest prescaler (=2)

  uint8_t prsc = 0;

  if (ticks == 0)
    return -1; // frequency too high!

  // find prescaler
  while(prsc < 8) {
    if (ticks <= 0x0000ffff)
      break;
    else {
      if ((prsc == 2) || (prsc == 4))
        ticks >>= 3;
      else
        ticks >>= 1;
      prsc++;
    }
  }

  TMR_THRES = (uint16_t)ticks;
  TMR_CT &= ~(7<<TMR_CT_PRSC0);
  TMR_CT |= (prsc<<TMR_CT_PRSC0);
  TMR_CNT = 0;

  return 0;
}


/* ------------------------------------------------------------
 * INFO Convert N hex chars into uint32
 * PARAM Pointer to buffer with hex chars
 * PARAM Number of hex chars to convert (1..8)
 * RETURN Conversion result
 * ------------------------------------------------------------ */
uint32_t hexstr_to_uint(char *buffer, uint8_t length) {

  uint32_t res = 0, d = 0;
  char c = 0;

  while (length--) {
    c = *buffer++;

    if ((c >= '0') && (c <= '9'))
      d = (uint32_t)(c - '0');
    else if ((c >= 'a') && (c <= 'f'))
      d = (uint32_t)((c - 'a') + 10);
    else if ((c >= 'A') && (c <= 'F'))
      d = (uint32_t)((c - 'A') + 10);
    else
      d = 0;

    res = res + (d << (length*4));
  }

  return res;
}


#endif // neo430_aux_h
