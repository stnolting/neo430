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

// Private function prototypes
static uint32_t neo430_timer_nco_real_output(uint16_t tuning_word, uint16_t prsc_shift);


/* ------------------------------------------------------------
 * INFO Activate Timer
 * ------------------------------------------------------------ */
void neo430_timer_enable(void) {

  TMR_CT |= (1<<TMR_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Dectivate Timer
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
 * INFO Stop Timer
 * ------------------------------------------------------------ */
void neo430_timer_pause(void) {

  TMR_CT &= ~(1<<TMR_CT_RUN);
}


/* ------------------------------------------------------------
 * INFO Configure timer period
 * PARAM Timer frequency in Hz (1Hz ... F_CPU/2)
 * RETURN 0 if successful, !=0 if error
 * ------------------------------------------------------------ */
uint8_t neo430_timer_config_freq(uint32_t f_timer) {

  uint32_t clock = CLOCKSPEED_32bit;
  uint32_t ticks = (clock / f_timer) >> 1; // divide by lowest prescaler (= f/2)

  uint8_t prsc = 0;

  if (ticks == 0)
    return 0xff; // frequency too high!

  // find prescaler
  while(prsc < 8) {
    if (ticks <= 0x0000ffff)
      break;
    else {
      if ((prsc == 2) || (prsc == 4))
        ticks >>= 3;
      else
        ticks >>= 1;
      prsc++;
    }
  }

  TMR_THRES = (uint16_t)ticks;
  TMR_CT &= ~(7<<TMR_CT_PRSC0); // clear prsc bits
  TMR_CT |= (prsc<<TMR_CT_PRSC0);

  return 0;
}


/* ------------------------------------------------------------
 * INFO Enable programmable frequency output (NCO)
 * ------------------------------------------------------------ */
void neo430_timer_nco_enable(void) {

  TMR_CT |= (1<<TMR_CT_NCO_EN);
}


/* ------------------------------------------------------------
 * INFO Disable programmable frequency output (NCO)
 * ------------------------------------------------------------ */
void neo430_timer_nco_disable(void) {

  TMR_CT &= ~(1<<TMR_CT_NCO_EN);
}


/* ------------------------------------------------------------
 * INFO Set frequency programmable frequency output
 * INFO f_out = ((f_cpu / nco_prsc) * tuning_word[15:0]) / 2^17
 * PARAM frequency: output frequency in Hz (no fractions possible here)
 * RETURN the actual output frequency
 * ------------------------------------------------------------ */
uint32_t neo430_timer_nco_set(uint32_t frequency) {

  // tuning_word = (f_out * 2^17) / (f_cpu / nco_prsc)

  uint32_t f_cpu = CLOCKSPEED_32bit;

  int16_t i;
  uint16_t prsc_shift = 12; // start with highest prescaler (4096 => 12)

  if (frequency > (f_cpu/4)) {
    return 0;
  }

  uint64_t freq_tmp;
  uint32_t freq_real;
  uint32_t freq_diff;

  uint32_t freq_diff_best = 0xffffffff; // max
  uint16_t tuning_word_best = 0;
  uint16_t prsc_best = 0;
  uint32_t freq_real_best = 0;

  // check all possible prescaler
  for(i=7; i>=0; i--) {

    freq_tmp = (uint64_t)frequency;
    freq_tmp = freq_tmp << (17 + prsc_shift); // multiply via bit shifts
    freq_tmp = freq_tmp / f_cpu;

    uint16_t tuning_word = (uint16_t)(freq_tmp);

    // add 1 to tuning word (for rounding issues)
    freq_real = neo430_timer_nco_real_output(tuning_word+1, prsc_shift);

    freq_diff = freq_real - frequency;
    if ((int32_t)freq_diff < 0) {
      freq_diff = 0 - freq_diff;
    }

    // best result yet?
    if (freq_diff < freq_diff_best) {
      tuning_word_best = tuning_word;
      prsc_best = i;
      freq_diff_best = freq_diff;
      freq_real_best = freq_real;
    }

    // compute next prescaler
    if ((i == 5) || (i == 3)) {
      prsc_shift = prsc_shift - 3;
    }
    else {
      prsc_shift = prsc_shift - 1;
    }
  }

  // write config to NCO
  uint16_t timer_ctrl = TMR_CT;
  timer_ctrl &= ~(0b111 << TMR_CT_NCO_PRSC0); // clear old prescaler config
  TMR_CT = timer_ctrl | (prsc_best << TMR_CT_NCO_PRSC0); // set new prescaler config
  TMR_NCO = tuning_word_best; // set timer's NCO tuning word

  return freq_real_best;
}


/* ------------------------------------------------------------
 * INFO Compute actual NCO output frequency based on tuning word and prescaler
 * RETURN the actual output frequency in Hz
 * ------------------------------------------------------------ */
static uint32_t neo430_timer_nco_real_output(uint16_t tuning_word, uint16_t prsc_shift) {

  // f_out = ((f_cpu/nco_prsc) * tuning_word[15:0]) / 2^17

  uint32_t f_cpu = CLOCKSPEED_32bit;
  uint64_t f_out = (uint64_t)f_cpu;
  f_out = f_out * tuning_word;
  f_out = f_out >> (17 + prsc_shift); // divide by 2^17 * PRSC

  return (uint32_t)f_out;
}
