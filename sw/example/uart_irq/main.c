// #################################################################################################
// #  < UART interrupt example >                                                                   #
// # ********************************************************************************************* #
// # UART RECEIVE is conducted using the UART RX interrupt                                         #
// # UART TRANSMIT is conducted by using the timer interrupt                                       #
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
// # Stephan Nolting, Hannover, Germany                                                 10.12.2019 #
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
