// #################################################################################################
// #  < neo430_trng.c - True random number generator helper functions >                            #
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
// # Stephan Nolting, Hannover, Germany                                                 09.01.2020 #
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
