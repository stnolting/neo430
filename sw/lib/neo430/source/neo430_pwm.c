// #################################################################################################
// #  < neo430_pwm.c - PWM controller helper functions >                                           #
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
// # Stephan Nolting, Hannover, Germany                                                 22.11.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_pwm.h"


/* ------------------------------------------------------------
 * INFO Reset and activate PWM controller
 * PARAM prsc: Clock prescaler for PWM clock
 * PARAM size: 1=use 8-bit counter, 0=use 4-bit counter
 * PARAM gpio_pwm: Use channel 3 for GPIO.output modulation when '1'
 * ------------------------------------------------------------ */
void neo430_pwm_enable(const uint16_t prsc, const uint16_t size, const uint16_t gpio_pwm) {

  PWM_CT = 0; // reset
  PWM_CT = (1<<PWM_CT_EN) | (prsc<<PWM_CT_PRSC0) | (size<<PWM_CT_SIZE_SEL) | (gpio_pwm<<PWM_CT_GPIO_PWM);
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

  uint16_t duty_cycle = 0;

  // get current state
  if (channel & 2) // channel 2 or 3
    duty_cycle = PWM_CH32;
  else // channel 1 or 0
    duty_cycle = PWM_CH10;

  // modify high or low part (even or odd channel)
  if (channel & 1) { // channel 1 or 3
    duty_cycle &= 0x00ff;
    duty_cycle |= ((uint16_t)dc) << 8;
  }
  else { // channel 0 or 2
    duty_cycle &= 0xff00;
    duty_cycle |= ((uint16_t)dc) << 0;
  }

  // write back
  if (channel & 2) // channel 2 or 3
    PWM_CH32 = duty_cycle;
  else // channel 1 or 0
    PWM_CH10 = duty_cycle;
}
