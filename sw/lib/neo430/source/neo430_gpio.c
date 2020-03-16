// #################################################################################################
// #  < neo430_gpio.c - PIO controller helper functions >                                          #
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
