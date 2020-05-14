// #################################################################################################
// #  < Processor hardware analysis tool >                                                         #
// # ********************************************************************************************* #
// #  Prints various information from the system.                                                  #
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
#include <stdlib.h>
#include <neo430.h>

// Configuration
#define BAUD_RATE 19200

// Prototypes
void print_state(uint16_t d);
void print_state2(uint16_t d);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  // intro text
  neo430_printf("\nNEO430 Processor Hardware Analysis Tool\n\n");

  // General information
  // --------------------------------------------
  // HW version
  neo430_printf("Hardware version: 0x%x\n", HW_VERSION);

  // HW user code
  neo430_printf("User code:        0x%x\n", USER_CODE);
  
  // Clock speed
  uint32_t clock = CLOCKSPEED_32bit;
  neo430_printf("Clock speed:      %n Hz\n", clock);

  // ROM/IMEM
  neo430_printf("IMEM/ROM:         %u bytes @ 0x%x\n", IMEM_SIZE, IMEM_ADDR_BASE);

  // RAM/DMEM
  neo430_printf("DMEM/RAM:         %u bytes @ 0x%x\n", DMEM_SIZE, DMEM_ADDR_BASE);

  // UART baud rate
  neo430_printf("UART Baud rate:   %n\n", neo430_uart_get_baudrate());


  // System features
  // --------------------------------------------
  uint16_t ft = SYS_FEATURES;
  neo430_printf("\nSystem features\n");

  // CFU
  neo430_printf("- Multiplier/Divider:    ");
  print_state(ft & (1<<SYS_MULDIV_EN));

  // WB32
  neo430_printf("- Wishbone Adapter:      ");
  print_state(ft & (1<<SYS_WB32_EN));

  // WDT
  neo430_printf("- Watchdog Timer:        ");
  print_state(ft & (1<<SYS_WDT_EN));

  // GPIO
  neo430_printf("- GPIO Unit:             ");
  print_state(ft & (1<<SYS_GPIO_EN));

  // TIMER
  neo430_printf("- High-Precision Timer:  ");
  print_state(ft & (1<<SYS_TIMER_EN));

  // UART
  neo430_printf("- UART:                  ");
  print_state(ft & (1<<SYS_UART_EN));

  // SPI
  neo430_printf("- SPI:                   ");
  print_state(ft & (1<<SYS_SPI_EN));

  // Bootloader installed
  neo430_printf("- Internal Bootloader:   ");
  print_state(ft & (1<<SYS_BTLD_EN));

  // is IMEM true ROM?
  neo430_printf("- IMEM as True ROM:      ");
  print_state2(ft & (1<<SYS_IROM_EN));

  // CRC
  neo430_printf("- CRC16/CRC32:           ");
  print_state(ft & (1<<SYS_CRC_EN));

  // CFU
  neo430_printf("- Custom Functions Unit: ");
  print_state(ft & (1<<SYS_CFU_EN));

  // PWM
  neo430_printf("- PWM Controller:        ");
  print_state(ft & (1<<SYS_PWM_EN));

  // TWI
  neo430_printf("- Two Wire Interface:    ");
  print_state(ft & (1<<SYS_TWI_EN));

  // TRNG
  neo430_printf("- True Random Generator: ");
  print_state(ft & (1<<SYS_TRNG_EN));

  // EXIRQ
  neo430_printf("- External IRQs Ctrl.:   ");
  print_state(ft & (1<<SYS_EXIRQ_EN));


  // Advanced/experimental features
  // --------------------------------------------
  uint16_t nx = NX_FEATURES;
  neo430_printf("\nAdvanced/experimental (NX) features\n");

  // DSP for multiplication
  neo430_printf("- Using embedded DSP.mul:   ");
  print_state2(nx & (1<<NX_DSP_MUL_EN));

  // extended ALU functions
  neo430_printf("- Extended ALU functions:   ");
  print_state2(nx & (1<<NX_XALU_EN));

  // low-power implementation
  neo430_printf("- Low-Power Implementation: ");
  print_state2(nx & (1<<NX_LOWPOWER_EN));


  // Exit
  // --------------------------------------------
  neo430_printf("\n\nPress any key to return to bootloader.\n");
  while(!neo430_uart_char_received());

  if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
    neo430_printf("No bootloader installed!\n");
  else
    asm volatile ("mov #0xF000, r0");

  return 0;
}


/* ------------------------------------------------------------
 * INFO print state
 * ------------------------------------------------------------ */
void print_state(uint16_t d) {

  if (d)
    neo430_printf("synthesized\n");
  else
    neo430_printf("-\n");
}


/* ------------------------------------------------------------
 * INFO print state 2
 * ------------------------------------------------------------ */
void print_state2(uint16_t d) {

  if (d)
    neo430_printf("true\n");
  else
    neo430_printf("false\n");
}
