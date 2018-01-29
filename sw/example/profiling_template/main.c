// #################################################################################################
// #  < Profiling Template >                                                                       #
// # ********************************************************************************************* #
// #  You can use this template to perform a timing profiling (i.e. runtime) of your application.  #
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
// #  Stephan Nolting, Hannover, Germany                                               08.11.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200

// Timer prescaler
#define TIMER_PRSC_CLK2    TMR_PRSC_2
#define TIMER_PRSC_CLK4    TMR_PRSC_4
#define TIMER_PRSC_CLK8    TMR_PRSC_8
#define TIMER_PRSC_CLK64   TMR_PRSC_64
#define TIMER_PRSC_CLK128  TMR_PRSC_128
#define TIMER_PRSC_CLK1024 TMR_PRSC_1024
#define TIMER_PRSC_CLK2048 TMR_PRSC_2048
#define TIMER_PRSC_CLK4096 TMR_PRSC_4096
// actual timer prescaler configuration:
#define TIMER_PRSC TIMER_PRSC_CLK2

// Function prototypes
inline void __attribute__((always_inline)) start_profiling(const uint16_t prsc);
inline uint16_t __attribute__((always_inline)) stop_profiling(uint32_t *cpu_cycles, const uint16_t prsc);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  _printf("\r\nProfiling template\r\n");

  // check if TIMER unit was synthesized, exit if no TIMER is available
  if (!(SYS_FEATURES & (1<<SYS_TIMER_EN))) {
    uart_br_print("Error! No TIMER unit synthesized!");
    return 1;
  }

  _printf("Starting profiling...\r\n");

  // start runtime profiling
  start_profiling(TIMER_PRSC);



  // **************************************************************
  // YOUR APPLIhttps://smile.amazon.de/gp/video/detail/B00J9OBDU4/ref=pd_cbs_318_20CATION YOU WANT TO PROFILE REGARDING REAL RUNTIME
  // The code below is just a compute-intensive exemplary dummy...
  volatile uint32_t a = 0x87654321;
  volatile uint32_t b = 0x12345678;
  volatile uint32_t c = (a%b)/27; // use *SW* division and multiplication here
  GPIO_OUT = (uint16_t)c; // store result to a read-only register so the compiler does not complain -> dismiss result ;)
  // **************************************************************



  // stop runtime profiling
  uint32_t runtime = 0;
  if (stop_profiling(&runtime, TIMER_PRSC) != 0) { // use SAME prescaler as in "start_time"
    _printf("Timer overflow! Use a greater prescaler!\r\n\r\n");
  }


  // print results
  _printf("Elapsed CPU cycles: %n\r\n", runtime);
  _printf("Re-run the measurement with a smaller prescaler to increase precision.\r\n");

  return 0;
}



/* ------------------------------------------------------------
 * INFO Start cycle counter
 * PARAM Prescaler 0..7
 * 0: CLK/2
 * 1: CLK/4
 * 2: CLK/8
 * 3: CLK/64
 * 4: CLK/128
 * 5: CLK/1024
 * 6: CLK/2048
 * 7: CLK/4096
 * ------------------------------------------------------------ */
inline void __attribute__((always_inline)) start_profiling(const uint16_t prsc) {

  // activate timer, no auto-reset, no IRQ, prsc set by user
  TMR_CT = (1<<TMR_CT_EN) | (0<<TMR_CT_ARST) | (0<<TMR_CT_IRQ) | ((prsc & 7) << TMR_CT_PRSC0);
  TMR_THRES = 0xFFFF; // max threshold

  TMR_CNT = 0; // reset counter and start counting
}


/* ------------------------------------------------------------
 * INFO Get elapsed CPU cycles
 * PARAM Prescaler 0..7
 * 0: CLK/2
 * 1: CLK/4
 * 2: CLK/8
 * 3: CLK/64
 * 4: CLK/128
 * 5: CLK/1024
 * 6: CLK/2048
 * 7: CLK/4096
 * PARAM Pointer to 32-bit value to store elapsed cycles to
 * RETURN 0 if success, 1 if timer overflow (= prescaler too small)
 * ------------------------------------------------------------ */
inline uint16_t __attribute__((always_inline)) stop_profiling(uint32_t *cpu_cycles, const uint16_t prsc) {

  uint16_t timer = TMR_CNT; // read timer counter value

  if (timer == 0xFFFF) // overflow!!!
    return 1;

  uint16_t prescaler_val;
  switch (prsc & 7) {
    case 0: prescaler_val = 2; break;
    case 1: prescaler_val = 4; break;
    case 2: prescaler_val = 8; break;
    case 3: prescaler_val = 64; break;
    case 4: prescaler_val = 128; break;
    case 5: prescaler_val = 1024; break;
    case 6: prescaler_val = 2048; break;
    case 7: prescaler_val = 4096; break;
    default: prescaler_val = 0; break; // invalid
  }
  *cpu_cycles = (uint32_t)timer * (uint32_t)prescaler_val;

  return 0; // success!
}
