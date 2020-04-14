// #################################################################################################
// #  < neo430_timer.c - Timer helper functions >                                                  #
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
#include "neo430_timer.h"


/* ------------------------------------------------------------
 * INFO Activate Timer
 * ------------------------------------------------------------ */
void neo430_timer_enable(void) {

  TMR_CT |= (1<<TMR_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Deactivate (and reset) Timer
 * ------------------------------------------------------------ */
void neo430_timer_disable(void) {

  TMR_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Start Timer
 * ------------------------------------------------------------ */
void neo430_timer_run(void) {

  TMR_CT |= (1<<TMR_CT_RUN);
}


/* ------------------------------------------------------------
 * INFO Pause Timer
 * ------------------------------------------------------------ */
void neo430_timer_pause(void) {

  TMR_CT &= ~(1<<TMR_CT_RUN);
}


/* ------------------------------------------------------------
 * INFO Configure timer period
 * PARAM Timer frequency in Hz (1 ... F_CPU/2), uint16_t pointer to store computed THRES value
 * RETURN 0 if successful, 0xff if error
 * ------------------------------------------------------------ */
uint8_t neo430_timer_config_freq(uint32_t f_timer, uint16_t *thres) {

  uint32_t clock = CLOCKSPEED_32bit;
  uint32_t ticks = (clock / f_timer) >> 1; // divide by lowest prescaler (= f/2)

  uint8_t prsc = 0;

  if (ticks == 0)
    return 0xff; // frequency too high!

  // find prescaler
  while(prsc < 8) {
    if (ticks <= 0x0000ffff) {
      break;
    }
    else {
      if ((prsc == 2) || (prsc == 4)) {
        ticks >>= 3;
      }
      else {
        ticks >>= 1;
      }
      prsc++;
    }
  }

  TMR_THRES = (uint16_t)ticks;
  *thres = (uint16_t)ticks;
  TMR_CT &= ~(7<<TMR_CT_PRSC0); // clear prsc bits
  TMR_CT |= (prsc<<TMR_CT_PRSC0);

  return 0;
}
