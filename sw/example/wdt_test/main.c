// #################################################################################################
// #  < Test program for the Watchdog Timer (WDT) >                                                #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
// # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
// # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
// # ********************************************************************************************* #
// #  Stephan Nolting, Hannover, Germany                                               22.09.2016  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN_C)  | (5<<USI_CT_SPIPRSC0_C) | (63<<USI_CT_SPICS0_C);

  // intro text
  uart_br_print("\n<<< Watchdog test program. >>>\n\n");

  // check if WDT was synthesized, exit if no WDT is available
  if (!(SYS_FEATURES & (1<<SYS_WDT_EN_C))) {
    uart_br_print("Error! No WDT synthesized!");
    return 0;
  }

  uart_br_print("Cause of last processor reset: ");
  if ((WDT_CTRL & (1<<WDT_RCAUSE_C)) == 0)
    uart_br_print("EXTERNAL RESET");
  else
    uart_br_print("WATCHDOG TIMEOUT");

  uart_br_print("\n\nWill reset WDT 64 times.\n"
                "A system reset will be executed in the following time out.\n"
                "[----------------------------------------------------------------]\n ");

  // init watchdog: enable, third largest period
  WDT_CTRL = (WDT_PASSWORD_C<<8) | (1<<WDT_ENABLE_C) | 5;

  uint8_t i;
  for (i=0; i<64; i++) {
    uart_putc('.');
    WDT_CTRL = (WDT_PASSWORD_C<<8) | (1<<WDT_ENABLE_C) | 5; // reset WDT by re-writing config data
    cpu_delay(4); // wait some time
  }

  while (1) // wait for the watchdog time-out
    asm volatile ("nop");

  return 0;
}
