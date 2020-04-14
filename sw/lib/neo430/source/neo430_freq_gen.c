// #################################################################################################
// #  < neo430_freq_gen.c - Frequency Generator helper functions >                                 #
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
#include "neo430_freq_gen.h"

// Private function prototypes
static uint32_t neo430_freq_gen_nco_real_output(uint16_t tuning_word, uint16_t prsc_shift);


/* ------------------------------------------------------------
 * INFO Enable programmable frequency output channel ch (0..2)
 * ------------------------------------------------------------ */
void neo430_freq_gen_enable_ch(uint16_t ch) {

  if (ch > 2) {
    return;
  }

  FREQ_GEN_CT |= (1 << (FREQ_GEN_CT_CH0_EN+ch));
}


/* ------------------------------------------------------------
 * INFO Disable programmable frequency output channel ch (0..2)
 * ------------------------------------------------------------ */
void neo430_freq_gen_disable_ch(uint16_t ch) {

  if (ch > 2) {
    return;
  }

  FREQ_GEN_CT &= ~(1 << (FREQ_GEN_CT_CH0_EN+ch));
}


/* ------------------------------------------------------------
 * INFO Disable all programmable frequency outputs
 * ------------------------------------------------------------ */
void neo430_freq_gen_disable(void) {

  register uint16_t ct = FREQ_GEN_CT;
  ct &= ~(1 << FREQ_GEN_CT_CH0_EN);
  ct &= ~(1 << FREQ_GEN_CT_CH1_EN);
  ct &= ~(1 << FREQ_GEN_CT_CH2_EN);
  FREQ_GEN_CT = ct;
}


/* ------------------------------------------------------------
 * INFO Disable all programmable frequency outputs and reset unit
 * ------------------------------------------------------------ */
void neo430_freq_gen_reset(void) {

  FREQ_GEN_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Set frequency programmable frequency output
 * INFO f_out = ((f_cpu / nco_prsc) * tuning_word[15:0]) / 2^17
 * WARNING Imprecise due to rounding/truncation errors!
 * PARAM ch channel to configure (0..2)
 * PARAM frequency: output frequency in Hz (no fractions possible here)
 * RETURN the actual output frequency
 * ------------------------------------------------------------ */
uint32_t neo430_freq_gen_set_freq(uint16_t ch, uint32_t frequency) {

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
    freq_real = neo430_freq_gen_nco_real_output(tuning_word+1, prsc_shift);

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


  // set tuning word and prescaler
  neo430_freq_gen_set(ch, tuning_word_best, prsc_best);

  return freq_real_best;
}


/* ------------------------------------------------------------
 * INFO Compute actual NCO output frequency based on tuning word and prescaler
 * RETURN the actual output frequency in Hz
 * ------------------------------------------------------------ */
static uint32_t neo430_freq_gen_nco_real_output(uint16_t tuning_word, uint16_t prsc_shift) {

  // f_out = ((f_cpu/nco_prsc) * tuning_word[15:0]) / 2^17

  uint32_t f_cpu = CLOCKSPEED_32bit;
  uint64_t f_out = (uint64_t)f_cpu;
  f_out = f_out * tuning_word;
  f_out = f_out >> (17 + prsc_shift); // divide by 2^17 * PRSC

  return (uint32_t)f_out;
}


/* ------------------------------------------------------------
 * INFO Set HW configuration
 * PARAM ch channel 0,1,2
 * PARAM 16-bit tuning word
 * PARAM 3-bit prescaler selector (0,...,7)
 * ------------------------------------------------------------ */
void neo430_freq_gen_set(uint16_t ch, uint16_t tuning_word, uint16_t prsc) {

  // set tuning word and prescaler
  register uint16_t ctrl = FREQ_GEN_CT;
  if (ch == 0) {
    FREQ_GEN_TW_CH0 = tuning_word;
    ctrl &= ~(0b111 << FREQ_GEN_CT_CH0_PRSC0); // clear old prescaler config
    ctrl |=  (prsc  << FREQ_GEN_CT_CH0_PRSC0); // set new prescaler config
  }
  else if (ch == 1) {
    FREQ_GEN_TW_CH1 = tuning_word;
    ctrl &= ~(0b111 << FREQ_GEN_CT_CH1_PRSC0); // clear old prescaler config
    ctrl |=  (prsc  << FREQ_GEN_CT_CH1_PRSC0); // set new prescaler config
  }
  else if (ch == 2) {
    FREQ_GEN_TW_CH2 = tuning_word;
    ctrl &= ~(0b111 << FREQ_GEN_CT_CH2_PRSC0); // clear old prescaler config
    ctrl |=  (prsc  << FREQ_GEN_CT_CH2_PRSC0); // set new prescaler config
  }
  FREQ_GEN_CT = ctrl;
}
