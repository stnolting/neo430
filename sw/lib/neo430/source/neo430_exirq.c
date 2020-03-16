// #################################################################################################
// #  < neo430_exirqt.c - External interrupts controler driver functions >                         #
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
#include "neo430_exirq.h"


/* ------------------------------------------------------------
 * INFO Enable external interrupts controller
 * ------------------------------------------------------------ */
void neo430_exirq_enable(void) {

  EXIRQ_CT |= (1<<EXIRQ_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Disable external interrupts controller
 * ------------------------------------------------------------ */
void neo430_exirq_disable(void) {

  EXIRQ_CT &= ~(1<<EXIRQ_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Configure external interrupts controller
 * PARAM Struct containing the handler functions addresses and the channel enable bits
 * ------------------------------------------------------------ */
void neo430_exirq_config(struct neo430_exirq_config_t config) {

  // reset controller
  EXIRQ_CT = 0;

  // get handler function address
  neo430_exirq_vectors[0] = config.address[0];
  neo430_exirq_vectors[1] = config.address[1];
  neo430_exirq_vectors[2] = config.address[2];
  neo430_exirq_vectors[3] = config.address[3];
  neo430_exirq_vectors[4] = config.address[4];
  neo430_exirq_vectors[5] = config.address[5];
  neo430_exirq_vectors[6] = config.address[6];
  neo430_exirq_vectors[7] = config.address[7];

  // set correct CPU external interrupts request handler address
  IRQVEC_EXT = (uint16_t)(&exirq_irq_handler);

  // configure channel enables
  uint16_t enable = (uint16_t)config.enable;
  EXIRQ_CT = (enable<<EXIRQ_CT_IRQ0_EN);
}


/* ------------------------------------------------------------
 * INFO Trigger IRQ channel by software
 * PARAM id (0..7) indicating which (ENABLED!) channel shall be triggered
 * ------------------------------------------------------------ */
void neo430_exirq_sw_irq(uint8_t id) {

  uint16_t irq_sel = (uint16_t)(id & 7);

  // apply sw irq enable bit and according irq select
  EXIRQ_CT |= (1<<EXIRQ_CT_SW_IRQ) | (irq_sel<<EXIRQ_CT_SW_IRQ_SEL0);
}


/* ------------------------------------------------------------
 * INFO Actual external interrupts controller IRQ handler
 * INFO This function is automatically installed
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) exirq_irq_handler(void) {

  uint16_t src = EXIRQ_CT & (7<<EXIRQ_CT_SRC0); // get IRQ source
  neo430_call_address(neo430_exirq_vectors[src]); // call according handler
}

