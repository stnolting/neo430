// #################################################################################################
// #  < neo430_trng.c - True random number generator helper functions >                            #
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
#include "neo430_trng.h"


/* ------------------------------------------------------------
 * INFO Enable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_enable(uint16_t tap_mask) {

  TRNG_CT = 0; // reset

  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");

  TRNG_CT = (1<<TRNG_CT_EN) | tap_mask;

  asm volatile("nop");
  asm volatile("nop");
  asm volatile("nop");
}


/* ------------------------------------------------------------
 * INFO Disable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_disable(void) {

  TRNG_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Get TRNG data and status information
 * PARAM byte-pointer to store random data
 * RETURN is 0 when data is VALID
 * ------------------------------------------------------------ */
uint16_t neo430_trng_get(uint8_t *data) {

  uint16_t trng_ct = TRNG_CT;
  *data = (uint8_t)trng_ct; // actual TRNG data
  if (trng_ct & (1<<TRNG_CT_VALID)) // output data valid?
    return 0;
  else
    return 1;
}
