// #################################################################################################
// #  < Software PWM generating a "heartbeat" >                                                    #
// # ********************************************************************************************* #
// # Generates a heartbeat with software PWM using the internal timer.                             #
// # PWM refresh rate: 1MHz.                                                                       #
// # Output via GPIO.out(0) (bootloader status LED)                                                #
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
// # Stephan Nolting, Hannover, Germany                                                04.07.2018 #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <neo430.h>

// Function prototypes
void __attribute__((__interrupt__)) timer_irq_handler(void);

// Global variables
volatile uint8_t pwm_cnt;
volatile uint8_t led_brightness;

// Configuration
#define LED_PIN 0   // GPIO output pin #0
#define MIN_VAL 5   // minimum intensity (0 = 0%)
#define MAX_VAL 255 // maximum intensity (255 = 100%)
#define BAUD_RATE 19200


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  neo430_uart_br_print("\nSoftware PWM demo.\n");

  // check if TIMER unit was synthesized, exit if no TIMER is available
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    neo430_uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }

  // check if GPIO unit was synthesized, exit if no GPIO is available
  if (!(SYS_FEATURES & (1<<SYS_GPIO_EN))) {
    neo430_uart_br_print("Error! No GPIO unit synthesized!");
    return 1;
  }

  // deactivate all LEDs
  neo430_gpio_port_set(0);

  // set address of timer IRQ handler
  IRQVEC_TIMER = (uint16_t)(&timer_irq_handler);

  // set timer configuration:
  // PWM frequency = 10kHz
  // f_tick := 10kHz @ PRSC := 128
  // f_tick = 10kHz = f_clock / (PRSC * (TMR_THRES + 1))
  // TMR_THRES = f_clock / (f_tick * PRSC) - 1
  //           = f_clock / (10000 * 128) - 1
  //           = f_clock / 1280000 - 1
  uint32_t f_clock = ((uint32_t)CLOCKSPEED_HI<<16) | (uint32_t)CLOCKSPEED_LO;
  TMR_THRES = (uint16_t)((f_clock / 1280000) - 1);

  // configure timer operation
  TMR_CT = (1<<TMR_CT_EN) |   // enable timer
           (1<<TMR_CT_ARST) | // auto reset on threshold match
           (1<<TMR_CT_IRQ) |  // enable IRQ
           (4<<TMR_CT_PRSC0); // 4 -> PRSC = 128

  // beat
  uint16_t beat = 0;
  beat = (uint16_t)(f_clock / 10000);

  // init IRQ variables
  pwm_cnt = 0;
  led_brightness = MIN_VAL;

  // enable global IRQs
  neo430_eint();

  // generate heartbeat
  uint8_t up_down = 0; // start with decreasing intensity
  while (1) {
    // min/max reached?
    if ((led_brightness == MAX_VAL) || (led_brightness == MIN_VAL))
      up_down = ~up_down;

    if (up_down) // increase brightness
      led_brightness++;
    else // decrease brightness
      led_brightness--;

    uint16_t i = 0;
    for (i=0; i<beat; i++) // wait a moment
      asm volatile ("nop");
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Timer interrupt handler - update PWM
 * ------------------------------------------------------------ */
void __attribute__((__interrupt__)) timer_irq_handler(void) {

  if (pwm_cnt <= led_brightness)
    neo430_gpio_pin_set(LED_PIN); // LED on
  else
    neo430_gpio_pin_clr(LED_PIN); // LED off

  if (pwm_cnt == 256)
    pwm_cnt = 0;

  pwm_cnt++;
}
