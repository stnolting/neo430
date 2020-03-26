// #################################################################################################
// #  < neo430_muldiv.c - Multiplier/Divider function >                                            #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2020, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The NEO430 Processor - https://github.com/stnolting/neo430                                    #
// #################################################################################################

#include "neo430.h"
#include "neo430_muldiv.h"

// private prototypes
static inline void neo430_muldiv_set_mul(void);
static inline void neo430_muldiv_set_div(void);


/* ------------------------------------------------------------
 * INFO Unsigned 16x16-bit multiplication
 * PARAM 16-bit factor a
 * PARAM 16-bit factor b
 * RETURN 32-bit product
 * ------------------------------------------------------------ */
uint32_t neo430_umul32(uint16_t a, uint16_t b) {

  neo430_muldiv_set_mul();
  MULDIV_OPA_CTRL = a;
  MULDIV_OPB = b;

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

  neo430_muldiv_set_mul();
  MULDIV_OPA_CTRL = (uint16_t)a;
  MULDIV_OPB = (uint16_t)b;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = dividend;
  MULDIV_OPB = divisor;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = (uint16_t)dividend;
  MULDIV_OPB = (uint16_t)divisor;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = dividend;
  MULDIV_OPB = divisor;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = (uint16_t)dividend_int;
  MULDIV_OPB = (uint16_t)divisor;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = dividend;
  MULDIV_OPB = divisor;

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

  neo430_muldiv_set_div();
  MULDIV_OPA_CTRL = (uint16_t)dividend_int;
  MULDIV_OPB = (uint16_t)divisor;

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


/* ------------------------------------------------------------
 * INFO Configure MULDIV for multiplication
 * ------------------------------------------------------------ */
static inline void neo430_muldiv_set_mul(void) {

  MULDIV_OPA_CTRL = 0x0000; // reset
  MULDIV_OPA_CTRL = MULDIV_CONFIG_MUL; // configure for multiplication
}


/* ------------------------------------------------------------
 * INFO Configure MULDIV for division
 * ------------------------------------------------------------ */
static inline void neo430_muldiv_set_div(void) {

  MULDIV_OPA_CTRL = 0x0000; // reset
  MULDIV_OPA_CTRL = MULDIV_CONFIG_DIV; // configure for division
}

