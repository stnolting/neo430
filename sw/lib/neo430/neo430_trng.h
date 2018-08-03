// #################################################################################################
// #  < neo430_trng.h - True random number generator helper functions >                            #
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
// # Stephan Nolting, Hannover, Germany                                                 04.07.2018 #
// #################################################################################################

#ifndef neo430_trng_h
#define neo430_trng_h

// prototypes
void neo430_trng_enable(void);
void neo430_trng_disable(void);
uint8_t neo430_trng_get_byte(void);


/* ------------------------------------------------------------
 * INFO Enable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_enable(void) {

  TRNG_CT = (1<<TRNG_CT_ENABLE);
}


/* ------------------------------------------------------------
 * INFO Disable TRNG
 * ------------------------------------------------------------ */
void neo430_trng_disable(void) {

  TRNG_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Get one random data byte from TRNG
 * RETURN Random data byte
 * ------------------------------------------------------------ */
uint8_t neo430_trng_get_byte(void) {

  return (uint8_t)TRNG_DATA;
}


#endif // neo430_trng_h
