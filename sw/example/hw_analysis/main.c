// #################################################################################################
// #  < Processor hardware analysis tool >                                                         #
// # ********************************************************************************************* #
// #  Prints various information from the system.                                                  #
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
// #  Stephan Nolting, Hannover, Germany                                               25.04.2018  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include <stdlib.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define BAUD_RATE 19200

// Prototypes
void print_state(uint16_t d);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // intro text
  _printf("\nNEO430 Hardware Analysis Tool\n\n");

  // General information
  // --------------------------------------------
  // HW version
  _printf("Hardware version: 0x%x\n", HW_VERSION);

  // HW user code
  _printf("User code:        0x%x\n", USER_CODE);
  
  // Clock speed
  uint32_t clock = CLOCKSPEED_32bit;
  _printf("Clock speed:      %n Hz\n", clock);

  // ROM/IMEM
  _printf("IMEM/ROM:         %u bytes @ 0x0000\n", IMEM_SIZE);

  // RAM/DMEM
  _printf("DMEM/RAM:         %u bytes @ 0x%x\n", DMEM_SIZE, DMEM_BASE);

  // UART baud rate
  uint16_t baud = USI_BAUD & 0x00FF;
  uint16_t prsc;
  switch ((USI_BAUD >> 8) & 0x0007) {
    case 0:  prsc = 2; break;
    case 1:  prsc = 4; break;
    case 2:  prsc = 8; break;
    case 3:  prsc = 64; break;
    case 4:  prsc = 128; break;
    case 5:  prsc = 1024; break;
    case 6:  prsc = 2048; break;
    case 7:  prsc = 4096; break;
    default: prsc = 0; break;
  }
  uint32_t baud_value = clock / (uint32_t)(prsc * baud);
  _printf("UART Baud rate:   %n\n", baud_value);


  // System features
  // --------------------------------------------
  uint16_t ft = SYS_FEATURES;
  _printf("\n\nSystem features\n");
  // CFU
  _printf("- Multiplier/Divider:    ");
  print_state(ft & (1<<SYS_MULDIV_EN));
  // WB32
  _printf("- Wishbone adapter:      ");
  print_state(ft & (1<<SYS_WB32_EN));
  // WDT
  _printf("- Watchdog timer:        ");
  print_state(ft & (1<<SYS_WDT_EN));
  // GPIO
  _printf("- GPIO unit:             ");
  print_state(ft & (1<<SYS_GPIO_EN));
  // TIMER
  _printf("- High-precision timer:  ");
  print_state(ft & (1<<SYS_TIMER_EN));
  // USART
  _printf("- USART:                 ");
  print_state(ft & (1<<SYS_USART_EN));
  // DADD
  _printf("- DADD instruction:      ");
  print_state(ft & (1<<SYS_DADD_EN));
  // Bootloader installed
  _printf("- Internal bootloader:   ");
  print_state(ft & (1<<SYS_BTLD_EN));
  // is IMEM true ROM?
  _printf("- IMEM as true ROM:      ");
  print_state(ft & (1<<SYS_IROM_EN));
  // CRC
  _printf("- CRC16/CRC32:           ");
  print_state(ft & (1<<SYS_CRC_EN));
  // CFU
  _printf("- Custom Functions Unit: ");
  print_state(ft & (1<<SYS_CFU_EN));
  // PWM
  _printf("- PWM Controller:        ");
  print_state(ft & (1<<SYS_PWM_EN));
  // TRNG
  _printf("- True Random Generator: ");
  print_state(ft & (1<<SYS_TRNG_EN));


  // Exit
  // --------------------------------------------
  _printf("\n\nPress any key to return to bootloader.\n");
  while(!uart_char_received());

  if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
    uart_br_print("No bootloader installed!\n");
  else
    asm volatile ("mov #0xF000, r0");

  return 0;
}


/* ------------------------------------------------------------
 * INFO print state
 * ------------------------------------------------------------ */
void print_state(uint16_t d) {

  if (d)
    _printf("enabled\n");
  else
    _printf("DISABLED\n");
}
