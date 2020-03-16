// #################################################################################################
// #  < UART interrupt example >                                                                   #
// # ********************************************************************************************* #
// # UART RECEIVE is conducted using the UART RX interrupt                                         #
// # UART TRANSMIT is conducted by using the timer interrupt                                       #
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
#define BAUD_RATE 19200
#define UART_FIFO_SIZE 512 // must be a power of two!
#define UART_FIFO_MASK (UART_FIFO_SIZE-1)

// Types
struct uart_fifo {
  uint8_t data[UART_FIFO_SIZE];
  uint16_t get_pnt;
  uint16_t put_pnt;
};

// Global vars
volatile struct uart_fifo uart_rtx_fifo;

// Function prototypes
void __attribute__((__interrupt__)) uart_irq_handler(void);
void __attribute__((__interrupt__)) timer_irq_handler(void);
void __attribute__((__interrupt__)) gpio_irq_handler(void);

void fifo_put_string(volatile struct uart_fifo *fifo, char *s);
uint8_t fifo_put(volatile struct uart_fifo *fifo, uint8_t c);
uint8_t fifo_get(volatile struct uart_fifo *fifo, uint8_t *c);

/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);


  // init fifo
  uart_rtx_fifo.get_pnt = 0;
  uart_rtx_fifo.put_pnt = 0;

  // deactivate all LEDs
  GPIO_OUTPUT = 0;

  // set address of UART, TIMER and GPIO IRQ handlers
  IRQVEC_SERIAL = (uint16_t)(&uart_irq_handler);
  IRQVEC_TIMER  = (uint16_t)(&timer_irq_handler);
  IRQVEC_GPIO   = (uint16_t)(&gpio_irq_handler);

  // configure GPIO pin-change interrupt
  GPIO_IRQMASK = 0xFFFF; // use all input pins as trigger

  // configure UART RX interrupt
  UART_CT |= (1<<UART_CT_RX_IRQ);

  // configure TIMER period
  neo430_timer_disable();
  TMR_THRES = 1;

  // configure timer operation
  TMR_CT = (1<<TMR_CT_EN)   | // enable timer
           (1<<TMR_CT_ARST) | // auto reset on threshold match
           (1<<TMR_CT_IRQ)  | // enable IRQ
           (1<<TMR_CT_RUN)  | // make timer run
           (TMR_PRSC_4096<<TMR_CT_PRSC0);


  // write string to buffer
  fifo_put_string(&uart_rtx_fifo, "\r\nUART IRQ FIFO Echo Test\r\n");

  // enable global IRQs
  neo430_eint();

  while(1) {
    neo430_sleep();
  }

  return 0;
}



/* ------------------------------------------------------------
 * INFO UART interrupt handler
 * INFO Put received char into buffer
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) uart_irq_handler(void) {

  fifo_put(&uart_rtx_fifo, (uint8_t)neo430_uart_char_read());
}



/* ------------------------------------------------------------
 * INFO Timer interrupt handler
 * INFO Send char from buffer if available
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  uint8_t c;

  // UART transceiver idle?
  if ((UART_CT & (1<<UART_CT_TX_BUSY)) == 0) {
    // char in buffer available?
    if (fifo_get(&uart_rtx_fifo, &c) == 0) {
      UART_RTX = (uint16_t)c;
    }
  }
}

/* ------------------------------------------------------------
 * INFO Write string to buffer, blocking!
 * ------------------------------------------------------------ */
void fifo_put_string(volatile struct uart_fifo *fifo, char *s) {

  uint8_t c = 0;
  while ((c = (uint8_t)*s++))
    fifo_put(fifo, c);
}


/* ------------------------------------------------------------
 * INFO Write to buffer, return 0 if success
 * ------------------------------------------------------------ */
uint8_t fifo_put(volatile struct uart_fifo *fifo, uint8_t c) {

  uint16_t next = ((fifo->put_pnt + 1) & UART_FIFO_MASK);

  if (fifo->get_pnt == next)
    return 1; // fifo full

  fifo->data[fifo->put_pnt & UART_FIFO_MASK] = c;
  fifo->put_pnt = next;

  return 0;
}


/* ------------------------------------------------------------
 * INFO Read from buffer, returns 0 if success
 * ------------------------------------------------------------ */
uint8_t fifo_get(volatile struct uart_fifo *fifo, uint8_t *c) {

  if (fifo->get_pnt == fifo->put_pnt)
    return 1; // fifo empty

  *c = fifo->data[fifo->get_pnt];

  fifo->get_pnt = (fifo->get_pnt+1) & UART_FIFO_MASK;

  return 0;
}



/* ------------------------------------------------------------
 * INFO GPIO pin-change interrupt handler
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) gpio_irq_handler(void) {

  GPIO_OUTPUT = (GPIO_OUTPUT + 1) & 0x00FF; // increment LED counter
}
