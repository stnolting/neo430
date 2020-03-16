// #################################################################################################
// #  < neo430_pwm.c - PWM controller helper functions >                                           #
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

#include "neo430.h"
#include "neo430_pwm.h"


/* ------------------------------------------------------------
 * INFO Reset and activate PWM controller
 * PARAM prsc: Clock prescaler for PWM clock
 * PARAM size: 1=use 8-bit counter, 0=use 4-bit counter
 * PARAM gpio_pwm: Use channel 3 for GPIO.output modulation when 1
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
