// #################################################################################################
// #  < neo430_exirqt.c - External interrupts controler driver functions >                         #
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
// # Stephan Nolting, Hannover, Germany                                                 28.11.2019 #
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

  // set correct CPU external IRQ interrupt handler address
  IRQVEC_EXT = (uint16_t)(&exirq_irq_handler);

  // configure channel enables and global trigger
  uint16_t trig = (uint16_t)config.trigger;
  uint16_t ena  = (uint16_t)config.enable;

  EXIRQ_CT = (ena<<EXIRQ_CT_IRQ0_EN) | (trig<<EXIRQ_CT_TRIG) ;
}


/* ------------------------------------------------------------
 * INFO Actual external interrupts controller IRQ handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) exirq_irq_handler(void) {

  uint16_t src = EXIRQ_CT & ((8-1)<<EXIRQ_CT_SRC0);
  neo430_call_address(neo430_exirq_vectors[src]);
}

