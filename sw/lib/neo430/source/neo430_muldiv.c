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

// Macros
#define muldiv_processing_delay {asm volatile("nop"); asm volatile("nop"); asm volatile("nop");}


/* ------------------------------------------------------------
 * INFO Unsigned 16x16-bit multiplication
 * PARAM 16-bit factor a
 * PARAM 16-bit factor b
 * RETURN 32-bit product
 * ------------------------------------------------------------ */
uint32_t neo430_umul32(uint16_t a, uint16_t b) {

  MULDIV_OPA_RESX = a;
  MULDIV_OPB_UMUL_RESY = b;

  muldiv_processing_delay // HW processing delay

  return MULDIV_R32bit;
}


/* ------------------------------------------------------------
 * INFO Unsigned 32x32-bit multiplication (large!)
 * PARAM 32-bit factor a
 * PARAM 32-bit factor b
 * RETURN 64-bit product
 * ------------------------------------------------------------ */
uint64_t neo430_umul64(uint32_t a, uint32_t b) {

union uint32_words {
  uint32_t num32;
  uint16_t word16[sizeof(uint32_t)/2];
} tmp32;

union uint64_words {
  uint64_t num64;
  uint16_t word16[sizeof(uint64_t)/2];
} tmp64;

  // get input words (16-bit)
  tmp32.num32 = a;
  register uint16_t a_lo = tmp32.word16[0];
  register uint16_t a_hi = tmp32.word16[1];

  tmp32.num32 = b;
  register uint16_t b_lo = tmp32.word16[0];
  register uint16_t b_hi = tmp32.word16[1];

  // compute partial results and arrange in 64-bit word
  MULDIV_OPA_RESX      = a_lo;
  MULDIV_OPB_UMUL_RESY = b_lo;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = MULDIV_OPA_RESX;
  tmp64.word16[1] = MULDIV_OPB_UMUL_RESY;
  tmp64.word16[2] = 0;
  tmp64.word16[3] = 0;
  uint64_t res64 = tmp64.num64;

//MULDIV_OPA_RESX      = a_lo;
  MULDIV_OPB_UMUL_RESY = b_hi;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = MULDIV_OPA_RESX;
  tmp64.word16[2] = MULDIV_OPB_UMUL_RESY;
  tmp64.word16[3] = 0;
  res64 += tmp64.num64;

  MULDIV_OPA_RESX      = a_hi;
  MULDIV_OPB_UMUL_RESY = b_lo;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = MULDIV_OPA_RESX;
  tmp64.word16[2] = MULDIV_OPB_UMUL_RESY;
  tmp64.word16[3] = 0;
  res64 += tmp64.num64;

//MULDIV_OPA_RESX      = a_hi;
  MULDIV_OPB_UMUL_RESY = b_hi;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = 0;
  tmp64.word16[2] = MULDIV_OPA_RESX;
  tmp64.word16[3] = MULDIV_OPB_UMUL_RESY;
  res64 += tmp64.num64;

  return res64;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16-bit multiplication
 * PARAM 16-bit factor a
 * PARAM 16-bit factor b
 * RETURN 32-bit product
 * ------------------------------------------------------------ */
int32_t neo430_mul32(int16_t a, int16_t b) {

  MULDIV_OPA_RESX = (uint16_t)a;
  MULDIV_OPB_SMUL = (uint16_t)b;

  muldiv_processing_delay // HW processing delay

  int32_t r = (int32_t)MULDIV_R32bit;

  return r;
}


/* ------------------------------------------------------------
 * INFO Signed 32x32-bit multiplication (large!)
 * PARAM 32-bit factor a
 * PARAM 32-bit factor b
 * RETURN 64-bit product
 * ------------------------------------------------------------ */
int64_t neo430_mul64(int32_t a, int32_t b) {

union int32_words {
  int32_t num32;
  int16_t word16[sizeof(int32_t)/2];
} tmp32;

union int64_words {
  int64_t num64;
  int16_t word16[sizeof(int64_t)/2];
} tmp64;

  // get input words (16-bit)
  tmp32.num32 = a;
  register int16_t a_lo = tmp32.word16[0];
  register int16_t a_hi = tmp32.word16[1];

  tmp32.num32 = b;
  register int16_t b_lo = tmp32.word16[0];
  register int16_t b_hi = tmp32.word16[1];

  // compute partial results and arrange in 64-bit word
  MULDIV_OPA_RESX = (uint16_t)a_lo;
  MULDIV_OPB_SMUL = (uint16_t)b_lo;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = MULDIV_OPA_RESX;
  tmp64.word16[1] = MULDIV_OPB_UMUL_RESY;
  tmp64.word16[2] = 0;
  tmp64.word16[3] = 0;
  int64_t res64 = tmp64.num64;

//MULDIV_OPA_RESX = (uint16_t)a_lo;
  MULDIV_OPB_SMUL = (uint16_t)b_hi;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = MULDIV_OPA_RESX;
  tmp64.word16[2] = MULDIV_OPB_UMUL_RESY;
  if (tmp64.word16[2] < 0) // sign extension
    tmp64.word16[3] = 0xffff;
  else
    tmp64.word16[3] = 0;
  res64 += tmp64.num64;

  MULDIV_OPA_RESX = (uint16_t)a_hi;
  MULDIV_OPB_SMUL = (uint16_t)b_lo;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = MULDIV_OPA_RESX;
  tmp64.word16[2] = MULDIV_OPB_UMUL_RESY;
  if (tmp64.word16[2] < 0) // sign extension
    tmp64.word16[3] = 0xffff;
  else
    tmp64.word16[3] = 0;
  res64 += tmp64.num64;

//MULDIV_OPA_RESX = (uint16_t)a_hi;
  MULDIV_OPB_SMUL = (uint16_t)b_lo;
  muldiv_processing_delay // HW processing delay
  tmp64.word16[0] = 0;
  tmp64.word16[1] = 0;
  tmp64.word16[2] = MULDIV_OPA_RESX;
  tmp64.word16[3] = MULDIV_OPB_UMUL_RESY;
  res64 += tmp64.num64;

  return res64;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16 by 16-bit division
 * PARAM 16-bit dividend
 * PARAM 16-bit divisor
 * RETURN 16-bit quotient
 * ------------------------------------------------------------ */
uint16_t neo430_udiv16(uint16_t dividend, uint16_t divisor) {

  MULDIV_OPA_RESX = dividend;
  MULDIV_OPB_UDIV = divisor;

  muldiv_processing_delay // HW processing delay

  return MULDIV_OPA_RESX;
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

  MULDIV_OPA_RESX = (uint16_t)dividend;
  MULDIV_OPB_UDIV = (uint16_t)divisor;

  muldiv_processing_delay // HW processing delay

  int16_t r = (int16_t)MULDIV_OPA_RESX;

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

  MULDIV_OPA_RESX = dividend;
  MULDIV_OPB_UDIV = divisor;

  muldiv_processing_delay // HW processing delay

  return MULDIV_OPB_UMUL_RESY;
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

  MULDIV_OPA_RESX = (uint16_t)dividend_int;
  MULDIV_OPB_UDIV = (uint16_t)divisor;

  muldiv_processing_delay // HW processing delay

  int16_t r = (int16_t)MULDIV_OPB_UMUL_RESY;

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

  MULDIV_OPA_RESX = dividend;
  MULDIV_OPB_UDIV = divisor;

  muldiv_processing_delay // HW processing delay

  *remainder = MULDIV_OPB_UMUL_RESY;
  return MULDIV_OPA_RESX;
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

  MULDIV_OPA_RESX = (uint16_t)dividend_int;
  MULDIV_OPB_UDIV = (uint16_t)divisor;

  muldiv_processing_delay // HW processing delay

  int16_t q = (int16_t)MULDIV_OPA_RESX;
  int16_t r = (int16_t)MULDIV_OPB_UMUL_RESY;

  if (dividend < 0)
    *remainder = 0 - r;
  else
    *remainder = r;

  if (sign < 0)
    return 0 - q;
  else
    return q;
}
