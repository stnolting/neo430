// #################################################################################################
// #  < neo430_gpio.c - PIO controller helper functions >                                          #
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
// # Stephan Nolting, Hannover, Germany                                                 13.03.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_gpio.h"


/* ------------------------------------------------------------
 * INFO Set output pin
 * PARAM Pin number to be set (0..15)
 * ------------------------------------------------------------ */
void neo430_gpio_pin_set(uint8_t p) {

  GPIO_OUTPUT = GPIO_OUTPUT | (uint16_t)(1 << p);
}


/* ------------------------------------------------------------
 * INFO Clear output pin
 * PARAM Pin number to be cleared (0..15)
 * ------------------------------------------------------------ */
void neo430_gpio_pin_clr(uint8_t p) {

  GPIO_OUTPUT = GPIO_OUTPUT & ~((uint16_t)(1 << p));
}


/* ------------------------------------------------------------
 * INFO Set output port
 * PARAM Output data
 * ------------------------------------------------------------ */
void neo430_gpio_port_set(uint16_t d) {

  GPIO_OUTPUT = d;
}


/* ------------------------------------------------------------
 * INFO Toggle output port
 * PARAM Toggle output data
 * ------------------------------------------------------------ */
void neo430_gpio_port_toggle(uint16_t d) {

  GPIO_OUTPUT = GPIO_OUTPUT ^ d;
}


/* ------------------------------------------------------------
 * INFO Toggle output pin
 * PARAM Pin number (0..15)
 * ------------------------------------------------------------ */
void neo430_gpio_pin_toggle(uint8_t p) {

  GPIO_OUTPUT = GPIO_OUTPUT ^ (uint16_t)(1 << p);
}


/* ------------------------------------------------------------
 * INFO Get input pin state
 * PARAM Pin number (0..15)
 * RETURN 0 if pin cleared, pin set otherwise
 * ------------------------------------------------------------ */
uint16_t neo430_gpio_pin_get(uint8_t p) {

  return GPIO_INPUT & (uint16_t)(1 << p);
}


/* ------------------------------------------------------------
 * INFO Get input port
 * RETURN Input port state
 * ------------------------------------------------------------ */
uint16_t neo430_gpio_port_get(void) {

  return GPIO_INPUT;
}
