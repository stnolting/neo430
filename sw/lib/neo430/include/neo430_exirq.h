// #################################################################################################
// #  < neo430_exirqt.h - External interrupts controler driver functions >                         #
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
// # Stephan Nolting, Hannover, Germany                                                 05.12.2019 #
// #################################################################################################

#ifndef neo430_exirq_h
#define neo430_exirq_h

// data types
struct neo430_exirq_config_t {
  uint16_t address[8];
  uint8_t  enable;
};

// private variables
static uint16_t neo430_exirq_vectors[8] __attribute__((unused)); // do not ouput a warning when this variable is unused

// prototypes
void   neo430_exirq_enable(void);                                // activate EXIRQ controller
void   neo430_exirq_disable(void);                               // deactivate EXIRQ controller
void   neo430_exirq_config(struct neo430_exirq_config_t config); // configure EXIRQ controller
void   neo430_exirq_sw_irq(uint8_t id);                          // trigger interrupt by software
void __attribute__((__interrupt__)) exirq_irq_handler(void);     // EXIRQ IRQ handler

#endif // neo430_exirq_h
