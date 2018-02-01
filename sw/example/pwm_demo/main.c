// #################################################################################################
// #  < PWM controller demo program >                                                              #
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
// # Stephan Nolting, Hannover, Germany                                                 02.01.2018 #
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
  USI_CT |= (1<<USI_CT_EN);

  uart_br_print("\n<<< PWM controller demo >>>\n");

  // check if PWM unit was synthesized, exit if no PWM controller is available
  if (!(SYS_FEATURES & (1<<SYS_PWM_EN))) {
    uart_br_print("Error! No PWM controller synthesized!");
    return 1;
  }

  // enable pwm controller in fast mode
  pwm_enable_fast_mode();

  uint8_t pwm = 0;
  uint8_t up = 1;
  uint8_t ch = 0;

  // clear all channels
  pwm_set_ch0(0);
  pwm_set_ch1(0);
  pwm_set_ch2(0);

  // animate!
  while(1) {
  
    // update duty cycle
    if (up) {
      if (pwm == 127) // up to half intensity
        up = 0;
      else
        pwm++;
    }
    else {
      if (pwm == 0) {
        ch = (ch + 1) & 3;
        up = 1;
        cpu_delay_ms(200);
      }
      else
        pwm--;
    }
  
    // output new duty cycle
    if (ch == 0)
      pwm_set_ch0(pwm);
    if (ch == 1)
      pwm_set_ch1(pwm);
    if (ch == 2)
      pwm_set_ch2(pwm);
  
    // wait 5ms
    cpu_delay_ms(5);
  }

  return 0;
}
