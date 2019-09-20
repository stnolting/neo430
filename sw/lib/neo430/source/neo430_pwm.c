// #################################################################################################
// #  < neo430_pwm.h - PWM controller helper functions >                                           #
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
// # Stephan Nolting, Hannover, Germany                                                 12.05.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_pwm.h"


/* ------------------------------------------------------------
 * INFO Reset and activate PWM controller
 * PARAM prsc: Clock prescaler for PWM clock
 * PARAM size: Actual bit width of PWM counter (1..8)
 * ------------------------------------------------------------ */
void neo430_pwm_enable(uint8_t prsc, uint8_t size) {

  PWM_CT = 0; // reset
  PWM_CT = (1<<PWM_CT_EN) | (((uint16_t)prsc)<<PWM_CT_PRSC0) | (((uint16_t)size-1)<<PWM_CT_SIZE0);
}


/* ------------------------------------------------------------
 * INFO Disable PWM controller
 * ------------------------------------------------------------ */
void neo430_pwm_disable(void) {

  PWM_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Set duty cycle of channel
 * PARAM channel 0..3
 * PARAM 8-bit duty cycle
 * ------------------------------------------------------------ */
void neo430_pwm_set(uint8_t channel, uint8_t dc) {

  uint16_t data = 0;

  // get current state
  if (channel & 2) // channel 2 or 3
    data = PWM_CH32;
  else // channel 1 or 0
    data = PWM_CH10;

  // modify high or low part (even or odd channel)
  if (channel & 1) { // channel 1 or 3
    data &= 0x00ff;
    data |= ((uint16_t)dc) << 8;
  }
  else { // channel 0 or 2
    data &= 0xff00;
    data |= ((uint16_t)dc) << 0;
  }

  // write back
  if (channel & 2) // channel 2 or 3
    PWM_CH32 = data;
  else // channel 1 or 0
    PWM_CH10 = data;
}
