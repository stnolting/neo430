// #################################################################################################
// #  < Test the multiplier/divider unit >                                                         #
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
// #  Stephan Nolting, Hannover, Germany                                               07.12.2017  #
// #################################################################################################

// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200

// Prototypes
uint32_t xorshift32(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  _printf("Multiplier/Divider Test\n\n");

  // check if multiplier/divider unit was synthesized, exit if not
  if (!(SYS_FEATURES & (1<<SYS_MULDIV_EN))) {
    uart_br_print("Error! No multiplier/divider unit synthesized!");
    return 1;
  }

  uint32_t prod, prod_ref;
  int32_t sprod, sprod_ref;
  uint16_t i, a, b, rem, quot, rem_ref, quot_ref;
  int16_t sa, sb, srem, squot, srem_ref, squot_ref;

  for (i=0; i<0xFFFF; i++) {

    // get "random" operands
    a = (uint16_t)xorshift32();
    b = (uint16_t)xorshift32();

    // multiply test
    prod_ref = (uint32_t)a * (uint32_t)b;
    prod = umul32(a, b);
    _printf("UNSIGNED %u: %u * %u  =  %n  vs ref.  %n\n", i, a, b, prod, prod_ref);

    // division test
    if (b == 0) // dont divide by zero
      b = 1;
    quot_ref = a / b;
    rem_ref  = a % b;
    quot = umoddiv16(&rem, a, b);
    _printf("UNSIGNED %u: %u / %u  =  %u & %u  vs ref.  %u & %u\n", i, a, b, quot, rem, quot_ref, rem_ref);


    // get "random" operands
    sa = (int16_t)xorshift32();
    sb = (int16_t)xorshift32();

    // division test - signed
    if (sb == 0) // dont divide by zero
      sb = 1;
    squot_ref = sa / sb;
    srem_ref  = sa % sb;
    squot = moddiv16(&srem, sa, sb);
    _printf("SIGNED %u: %i / %i  =  %i & %i  vs ref.  %i & %i\n", i, sa, sb, squot, srem, squot_ref, srem_ref);

    // multiply test - signed
    sprod_ref = (int32_t)sa * (int32_t)sb;
    sprod = mul32(sa, sb);
    _printf("SIGNED %u: %i * %i  =  %l  vs ref.  %l\n", i, sa, sb, sprod, sprod_ref);


    // all correct?
    if ((rem != rem_ref) || (quot != quot_ref) || (prod != prod_ref) || (srem != srem_ref) || (squot != squot_ref) || (sprod != sprod_ref)) {
      _printf("ERROR in one or more of the four previous operations!");
      while(1);
    }
  }

  _printf("Tests passed!\n");
  while(1);

  return 0;  
}


/* ------------------------------------------------------------
 * INFO Pseudo-random number generator
 * RETURN 32-bit random data
 * ------------------------------------------------------------ */
uint32_t xorshift32(void) {

  static uint32_t x32 = 314159265;

  x32 ^= x32 << 13;
  x32 ^= x32 >> 17;
  x32 ^= x32 << 5;

  return x32;
}
