// #################################################################################################
// #  < External Interrupts Controller test program >                                              #
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


// Libraries
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200

// Prototypes
void ext_irq_ch0_handler(void);
void ext_irq_ch1_handler(void);
void ext_irq_ch2_handler(void);
void ext_irq_ch3_handler(void);

void ext_irq_ch7_handler(void);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_uart_br_print("\n<<< EXIRQ Test >>>\n");

  // check if EXIRQ was synthesized, exit if not available
  if (!(SYS_FEATURES & (1<<SYS_EXIRQ_EN))) {
    neo430_uart_br_print("Error! No EXIRQ synthesized!");
    return 1;
  }

  neo430_uart_br_print("\nTrigger the external interrupt pin (set high) or perform a manual\n"
                       "triggering (sw interrupt) by pressing key 0 to 7.\n");

  // clear output port
  neo430_gpio_port_set(0);


  // use this predefined struct for configuring the EXIRQ controller
  struct neo430_exirq_config_t exirq_config;

  // initialise handler addresses
  exirq_config.address[0] = (uint16_t)(&ext_irq_ch0_handler);
  exirq_config.address[1] = (uint16_t)(&ext_irq_ch1_handler);
  exirq_config.address[2] = (uint16_t)(&ext_irq_ch2_handler);
  exirq_config.address[3] = (uint16_t)(&ext_irq_ch3_handler);
  exirq_config.address[4] = 0; // set unused vectors to zero
  exirq_config.address[5] = 0;
  exirq_config.address[6] = 0;
  exirq_config.address[7] = (uint16_t)(&ext_irq_ch7_handler);

  // only enable the actually used IRQ channels
  exirq_config.enable = 0b10001111; // each bit represents the according channel

  // program configuration and activate EXIRQ controller
  neo430_exirq_config(exirq_config);
  neo430_exirq_enable();

  // enable global interrupts
  neo430_eint();

  // trigger EXIRQ channel 0 IRQ by software just for fun
  neo430_exirq_sw_irq(0);

  // wait for key input
  while(1) {
    char c = neo430_uart_getc();
    if ((c >= '0') && (c <= '7')) {
      c = c - '0';
      neo430_exirq_sw_irq((uint8_t)c); // trigger according IRQ by software
    }
  }

  return 0;
}


// handler functions for the external interrupt channels:
// - must not have parameters nor a return value
// - should not use the interrupt attribute, as they can be normal functions called by the actual interrupt handler

void ext_irq_ch0_handler(void) {

  neo430_gpio_pin_toggle(0);
}


void ext_irq_ch1_handler(void) {

  neo430_gpio_pin_toggle(1);
}


void ext_irq_ch2_handler(void) {

  neo430_gpio_pin_toggle(2);
}


void ext_irq_ch3_handler(void) {

  neo430_gpio_pin_toggle(3);
}


void ext_irq_ch7_handler(void) {

  neo430_gpio_pin_toggle(7);
}

