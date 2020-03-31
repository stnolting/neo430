// #################################################################################################
// #  < Nested IRQs example >                                                                      #
// # ********************************************************************************************* #
// # Generates a run time clock using the timer IRQ.                                               #
// # Whenever a char via the UART is received, the according ISR show the current time.            #
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
#include <stdint.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE  19200

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);
void __attribute__((__interrupt__)) uart_irq_handler(void);

// Variable
volatile uint64_t time_ms;


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);


  // check if TIMER unit was synthesized, exit if no TIMER is available
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    neo430_uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }


  // reset time
  time_ms = 0;


  // intro text
//neo430_uart_br_print("\nClock example. Press any key to show the current time.\n");


  // init TIMER IRQ
  // ------------------------------------------------------

  // set address of timer IRQ handler
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // configure timer frequency
  neo430_timer_disable();
  uint16_t timer_thres;
  if (neo430_timer_config_freq(1000, &timer_thres)) { // 1kHz to increment every 1ms
    neo430_uart_br_print("Invalid TIMER frequency!\n");
  }

  neo430_printf("THR: %x, CTR: %x\n", TMR_THRES, TMR_CT);
  TMR_CT |= (1<<TMR_CT_EN) | (1<<TMR_CT_ARST) | (1<<TMR_CT_IRQ) | (1<<TMR_CT_RUN); // enable timer, auto-reset, irq enabled, start timer
  neo430_printf("THR: %x, CTR: %x\n", TMR_THRES, TMR_CT);


  // init UART RX IRQ
  // ------------------------------------------------------

  // set address of UART IRQ handler
  IRQVEC_SERIAL = (uint16_t)(&uart_irq_handler);

  // activate UART RX interrupt
  UART_CT |= (1<<UART_CT_RX_IRQ);


  // enable global IRQs
  neo430_eint();


  // do nothing
  while (1) {
    neo430_sleep();
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  time_ms++;
}


/* ------------------------------------------------------------
 * INFO UART interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) uart_irq_handler(void) {

  // reactive IRQs to allow nested interrupts
  neo430_eint();

  // show time
  uint32_t current_time = time_ms; // in seconds
  uint16_t hour     = (uint16_t)( (((current_time/10000)/60)/3600)%24 );
  uint16_t minute   = (uint16_t)( ((current_time/1000)/60)%60 );
  uint16_t second   = (uint16_t)( (current_time/1000)%60 );
  uint16_t m_second = (uint16_t)( (current_time%1000)%1000 );
  neo430_printf("Current runtime: %u:%u:%u:%u\n", hour, minute, second, m_second);
}

