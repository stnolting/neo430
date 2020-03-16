// #################################################################################################
// #  < TWI bus explorer >                                                                         #
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
#include <string.h>
#include <neo430.h>

// Prototypes
void scan_twi(void);
void set_speed(void);
void send_twi(void);

// Configuration
#define BAUD_RATE 19200

// Global variables
uint16_t bus_claimed;


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  // setup UART
  neo430_uart_setup(BAUD_RATE);

  char buffer[8];
  uint16_t length = 0;

  neo430_uart_br_print("\n---------------------------------\n"
                         "--- TWI Bus Explorer Terminal ---\n"
                         "---------------------------------\n\n");

  // check if TWI unit was synthesized, exit if no TWI is available
  if (!(SYS_FEATURES & (1<<SYS_TWI_EN))) {
    neo430_uart_br_print("Error! No TWI synthesized!");
    return 1;
  }

  neo430_uart_br_print("This program allows to create TWI transfers by hand.\n"
                       "Type 'help' to see the help menu.\n\n");

  // init TWI
  // SCL clock speed = f_cpu / (4 * PRSC)
  neo430_twi_enable(TWI_PRSC_2048); // second slowest
  bus_claimed = 0; // no active bus session

  // Main menu
  for (;;) {
    neo430_uart_br_print("TWI_EXPLORER:> ");
    length = neo430_uart_scan(buffer, 8, 1);
    neo430_uart_br_print("\n");

    if (!length) // nothing to be done
     continue;

    // decode input and execute command
    if (!strcmp(buffer, "help")) {
      neo430_uart_br_print("Available commands:\n"
                           " help  - show this text\n"
                           " scan  - scan bus for devices\n"
                           " start - generate START condition\n"
                           " stop  - generate STOP condition\n"
                           " send  - write & read single byte to/from bus\n"
                           " speed - select bus clock\n"
                           " reset - perform soft-reset\n"
                           " exit  - exit program and return to bootloader\n\n"
                           "Start a new transmission by generating a START condition. Next, transfer the 7-bit device address\n"
                           "and the R/W flag. After that, transfer your data to be written or send a 0xFF if you want to read\n"
                           "data from the bus. Finish the transmission by generating a STOP condition.\n");
    }
    else if (!strcmp(buffer, "start")) {
      neo430_twi_generate_start(); // generate START condition
      bus_claimed = 1;
    }
    else if (!strcmp(buffer, "stop")) {
      if (bus_claimed == 0) {
        neo430_uart_br_print("No active I2C transmission.\n");
        continue;
      }
      neo430_twi_generate_stop(); // generate STOP condition
      bus_claimed = 0;
    }
    else if (!strcmp(buffer, "scan")) {
      scan_twi();
    }
    else if (!strcmp(buffer, "speed")) {
      set_speed();
    }
    else if (!strcmp(buffer, "send")) {
      send_twi();
    }
    else if (!strcmp(buffer, "reset")) {
      while ((UART_CT & (1<<UART_CT_TX_BUSY)) != 0); // wait for current UART transmission
      neo430_twi_disable();
      neo430_soft_reset();
    }
    else if (!strcmp(buffer, "exit")) {
      if (!(SYS_FEATURES & (1<<SYS_BTLD_EN)))
        neo430_uart_br_print("No bootloader installed!\n");
      else
        asm volatile ("mov #0xF000, r0");
    }
    else {
      neo430_uart_br_print("Invalid command. Type 'help' to see all commands.\n");
    }
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Set TWI speed
 * ------------------------------------------------------------ */
void set_speed(void) {

  char terminal_buffer[2];

  neo430_uart_br_print("Select new clock prescaler (0..7): ");
  neo430_uart_scan(terminal_buffer, 2, 1); // 1 hex char plus '\0'
  uint8_t prsc = (uint8_t)neo430_hexstr_to_uint(terminal_buffer, strlen(terminal_buffer));
  if ((prsc >= 0) && (prsc < 8)) { // valid?
    TWI_CT = 0; // reset
    TWI_CT = (1 << TWI_CT_EN) | (prsc << TWI_CT_PRSC0);
    neo430_uart_br_print("\nDone.\n");
  }
  else {
    neo430_uart_br_print("\nInvalid selection!\n");
    return;
  }

  // print new clock frequency
  uint32_t clock = CLOCKSPEED_32bit;
  switch (prsc) {
    case 0: clock = clock / 2; break;
    case 1: clock = clock / 4; break;
    case 2: clock = clock / 8; break;
    case 3: clock = clock / 64; break;
    case 4: clock = clock / 128; break;
    case 5: clock = clock / 1024; break;
    case 6: clock = clock / 2048; break;
    case 7: clock = clock / 4096; break;
    default: clock = 0; break;
  }
  neo430_printf("New I2C clock: %n Hz\n", clock);
}


/* ------------------------------------------------------------
 * INFO Scan 7-bit TWI address space
 * ------------------------------------------------------------ */
void scan_twi(void) {

  neo430_uart_br_print("Scanning TWI bus...\n");
  uint8_t i, num_devices = 0;
  for (i=0; i<128; i++) {
    uint8_t twi_ack = neo430_twi_start_trans((uint8_t)(2*i+1));
    neo430_twi_generate_stop();

    if (twi_ack == 0) {
      neo430_uart_br_print("+ Found device at write-address 0x");
      neo430_uart_print_hex_byte(2*i);
      neo430_uart_br_print("\n");
      num_devices++;
    }
  }

  if (!num_devices) {
    neo430_uart_br_print("No devices found.\n");
  }
}


/* ------------------------------------------------------------
 * INFO Read/write 1 byte from/to bus
 * ------------------------------------------------------------ */
void send_twi(void) {

  char terminal_buffer[4];

  if (bus_claimed == 0) {
    neo430_uart_br_print("No active I2C transmission. Generate a START condition first.\n");
    return;
  }

  // enter data
  neo430_uart_br_print("Enter TX data (2 hex chars): ");
  neo430_uart_scan(terminal_buffer, 3, 1); // 2 hex chars for address plus '\0'
  uint8_t tmp = (uint8_t)neo430_hexstr_to_uint(terminal_buffer, strlen(terminal_buffer));
  uint8_t res = neo430_twi_trans(tmp);
  neo430_uart_br_print("\nRX data:  0x");
  neo430_uart_print_hex_byte(neo430_twi_get_data());
  neo430_uart_br_print("\nResponse: ");
  if (res == 0)
    neo430_uart_br_print("ACK\n");
  else
    neo430_uart_br_print("NACK\n");

}

