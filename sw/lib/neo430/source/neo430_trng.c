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
// # Stephan Nolting, Hannover, Germany                                                 27.11.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_trng.h"


/* ------------------------------------------------------------
 * INFO Enable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_enable(void) {

  TRNG_CT = 0; // reset
  TRNG_CT = (1<<TRNG_CT_EN);

  // wait for unit to get oscillating
  asm volatile ("nop");
  asm volatile ("nop");
}


/* ------------------------------------------------------------
 * INFO Disable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_disable(void) {

  TRNG_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Get TRNG data
 * RETURN TRNG random data byte
 * ------------------------------------------------------------ */
uint8_t neo430_trng_get(void) {

  asm volatile ("nop"); // make sure TRNG has enough time to sample a new number
  return (uint8_t)(TRNG_CT & 0xFF);
}
