// #################################################################################################
// #  < Test program for the Watchdog Timer (WDT) >                                                #
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

  // intro text
  neo430_uart_br_print("\n<<< Watchdog Test Program >>>\n\n");

  // check if WDT was synthesized, exit if no WDT is available
  if (!(SYS_FEATURES & (1<<SYS_WDT_EN))) {
    neo430_uart_br_print("Error! No WDT synthesized!");
    return 0;
  }

  neo430_uart_br_print("Cause of last processor reset: ");
  uint16_t wdt_cause = WDT_CT;
  if ((wdt_cause & (1<<WDT_CT_RCAUSE)) == 0) { // reset caused by WDT at all?
    neo430_uart_br_print("EXTERNAL RESET");
  }
  else {
    if ((wdt_cause & (1<<WDT_CT_RPWFAIL)) == 0) { // WDT reset caused by wrong password access?
      neo430_uart_br_print("WATCHDOG Timeout");
    }
    else {
      neo430_uart_br_print("WATCHDOG Access Error");
    }
  }


  neo430_uart_br_print("\n\nWill reset WDT 64 times.\n"
                "A system reset will be executed in the following time out.\n"
                "Press any key to trigger manual WDT hardware reset by WDT access with wrong password.\n"
                "Restart this program after reset to check for the reset cause.\n\n"
                "WDT resets: [................................................................]");
  neo430_uart_bs(65); // back-space terminal cursor by 65 positions

  // activate watchdog: second largest period
  neo430_wdt_enable(WDT_PRSC_2048);


  uint8_t i;
  for (i=0; i<64; i++) {
    neo430_uart_putc('#');
    neo430_wdt_reset(); // reset watchdog
    neo430_cpu_delay_ms(80); // wait some time

    // trigger manual reset if key pressed
    if (neo430_uart_char_received()) {
      neo430_wdt_force_hw_reset(); // access wdt with wrong password
    }
  }

  while (1) { // wait for the watchdog time-out or key press
    if (neo430_uart_char_received()) {
      neo430_wdt_force_hw_reset();
    }
  }

  return 0;
}
