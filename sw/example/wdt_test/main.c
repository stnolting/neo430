// #################################################################################################
// #  < Test program for the Watchdog Timer (WDT) >                                                #
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
