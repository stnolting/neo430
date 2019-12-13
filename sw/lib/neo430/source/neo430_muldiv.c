// #################################################################################################
// #  < neo430_muldiv.c - Multiplier/Divider function >                                            #
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
// # Stephan Nolting, Hannover, Germany                                                 13.03.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_muldiv.h"


/* ------------------------------------------------------------
 * INFO Unsigned 16x16-bit multiplication
 * PARAM 16-bit factor a
 * PARAM 16-bit factor b
 * RETURN 32-bit product
 * ------------------------------------------------------------ */
uint32_t neo430_umul32(uint16_t a, uint16_t b) {

  MULDIV_OPA = a;
  MULDIV_OPB_MUL = b;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  return MULDIV_R32bit;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16-bit multiplication
 * PARAM 16-bit factor a
 * PARAM 16-bit factor b
 * RETURN 32-bit product
 * ------------------------------------------------------------ */
int32_t neo430_mul32(int16_t a, int16_t b) {

  int16_t sign = a ^ b;

  // make positive / unsigned
  if (a < 0)
    a = 0 - a;
  if (b < 0)
    b = 0 - b;

  MULDIV_OPA = (uint16_t)a;
  MULDIV_OPB_MUL = (uint16_t)b;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  int32_t r = (int32_t)MULDIV_R32bit;

  if (sign < 0)
    return 0 - r;
  else
    return r;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * RETURN 16-bit quotient
 * ------------------------------------------------------------ */
uint16_t neo430_udiv16(uint16_t dividend, uint16_t divisor) {

  MULDIV_OPA = dividend;
  MULDIV_OPB_DIV = divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  return MULDIV_RESX;
}


/* ------------------------------------------------------------
 * INFO Signed 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * RETURN 16-bit quotient
 * ------------------------------------------------------------ */
int16_t neo430_div16(int16_t dividend, int16_t divisor) {

  // make positive / unsigned
  if (dividend < 0)
    dividend = 0 - dividend;
  if (divisor < 0)
    divisor = 0 - divisor;

  MULDIV_OPA = (uint16_t)dividend;
  MULDIV_OPB_DIV  = (uint16_t)divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  int16_t r = (int16_t)MULDIV_RESX;

  if (dividend < 0)
    return 0 - r;
  else
    return r;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * RETURN 16-bit remainder
 * ------------------------------------------------------------ */
uint16_t neo430_umod16(uint16_t dividend, uint16_t divisor) {

  MULDIV_OPA = dividend;
  MULDIV_OPB_DIV = divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  return MULDIV_RESY;
}


/* ------------------------------------------------------------
 * INFO Signed 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * RETURN 16-bit remainder
 * ------------------------------------------------------------ */
int16_t neo430_mod16(int16_t dividend, int16_t divisor) {

  int16_t dividend_int = dividend;

  // make positive / unsigned
  if (dividend_int < 0)
    dividend_int = 0 - dividend_int;
  if (divisor < 0)
    divisor = 0 - divisor;

  MULDIV_OPA = (uint16_t)dividend_int;
  MULDIV_OPB_DIV  = (uint16_t)divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  int16_t r = (int16_t)MULDIV_RESY;

  if (dividend < 0)
    return 0 - r;
  else
    return r;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * PARAM Pointer to store 16-bit remainder
 * RETURN 16-bit quotient
 * ------------------------------------------------------------ */
uint16_t neo430_umoddiv16(uint16_t *remainder, uint16_t dividend, uint16_t divisor) {

  MULDIV_OPA = dividend;
  MULDIV_OPB_DIV = divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  *remainder = MULDIV_RESY;
  return MULDIV_RESX;
}


/* ------------------------------------------------------------
 * INFO Signed 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * PARAM Pointer to store 16-bit remainder
 * RETURN 16-bit quotient
 * ------------------------------------------------------------ */
int16_t neo430_moddiv16(int16_t *remainder, int16_t dividend, int16_t divisor) {

  int16_t sign = dividend ^ divisor;
  int16_t dividend_int = dividend;

  // make positive / unsigned
  if (dividend_int < 0)
    dividend_int = 0 - dividend_int;
  if (divisor < 0)
    divisor = 0 - divisor;

  MULDIV_OPA = (uint16_t)dividend_int;
  MULDIV_OPB_DIV  = (uint16_t)divisor;

  // HW processing delay
  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  int16_t q = (int16_t)MULDIV_RESX;
  int16_t r = (int16_t)MULDIV_RESY;

  if (dividend < 0)
    *remainder = 0 - r;
  else
    *remainder = r;

  if (sign < 0)
    return 0 - q;
  else
    return q;
}
