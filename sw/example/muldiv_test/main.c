// #################################################################################################
// #  < Test the multiplier/divider unit >                                                         #
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

// Libraries
#include <stdint.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  neo430_printf("Multiplier/Divider Test\n\n");

  // check if multiplier/divider unit was synthesized, exit if not
  if (!(SYS_FEATURES & (1<<SYS_MULDIV_EN))) {
    neo430_uart_br_print("Error! No multiplier/divider unit synthesized!");
    return 1;
  }

  uint32_t prod, prod_ref;
  int32_t sprod, sprod_ref;
  uint16_t i, a, b, rem, quot, rem_ref, quot_ref;
  int16_t sa, sb, srem, squot, srem_ref, squot_ref;

  for (i=0; i<0xFFFF; i++) {

    // get "random" operands
    a = (uint16_t)neo430_xorshift32();
    b = (uint16_t)neo430_xorshift32();

    // multiply test
    prod_ref = (uint32_t)a * (uint32_t)b;
    prod = neo430_umul32(a, b);
    neo430_printf("UNSIGNED %u: %u * %u  =  P: %n  vs ref.  P: %n\n", i, a, b, prod, prod_ref);

    // division test
    if (b == 0) // dont divide by zero
      b = 1;
    quot_ref = a / b;
    rem_ref  = a % b;
    quot = neo430_umoddiv16(&rem, a, b);
    neo430_printf("UNSIGNED %u: %u / %u  =  Q: %u & R: %u  vs ref.  Q: %u & R: %u\n", i, a, b, quot, rem, quot_ref, rem_ref);


    // get "random" operands
    sa = (int16_t)neo430_xorshift32();
    sb = (int16_t)neo430_xorshift32();

    // division test - signed
    if (sb == 0) // dont divide by zero
      sb = 1;
    squot_ref = sa / sb;
    srem_ref  = sa % sb;
    squot = neo430_moddiv16(&srem, sa, sb);
    neo430_printf("SIGNED   %u: %i / %i  =  Q: %i & R: %i  vs ref.  Q: %i & R: %i\n", i, sa, sb, squot, srem, squot_ref, srem_ref);

    // multiply test - signed
    sprod_ref = (int32_t)sa * (int32_t)sb;
    sprod = neo430_mul32(sa, sb);
    neo430_printf("SIGNED   %u: %i * %i  =  P: %l  vs ref.  P: %l\n", i, sa, sb, sprod, sprod_ref);


    // all correct?
    if ((rem != rem_ref) || (quot != quot_ref) || (prod != prod_ref) || (srem != srem_ref) || (squot != squot_ref) || (sprod != sprod_ref)) {
      neo430_printf("ERROR in one or more of the four previous operations!");
      while(1);
    }
  }

  neo430_printf("Tests passed!\n");
  while(1);

  return 0;  
}

