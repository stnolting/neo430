// #################################################################################################
// #  < Morse code translator >                                                                    #
// # ********************************************************************************************* #
// # Translates a text string into Morse code. The encode can only encode letters.                 #
// # Output via GPIO.output(0) (bootloader status LED).                                            #
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
// #  Stephan Nolting, Hannover, Germany                                               06.10.2017  #
// #################################################################################################


// Libraries
#include <stdint.h>
#include "../../lib/neo430/neo430.h"

// Configuration
#define MAX_STRING_LENGTH 128 // max length of plain morse text
#define LED_PIN 0 // GPIO output pin #0
#define TIME_PRSC 192
#define BAUD_RATE 19200

// Global vars
uint32_t time_base;

// Prototypes
void send_morse(char *s);

// Morse alphabet (ASCII order)
char morse_code[][7] = {
  "--..--", // ,
  "-....-", // -
  ".-.-.-", // .
  "-..-.",  // /

  "-----",  // 0
  ".----",  // 1
  "..---",  // 2
  "...--",  // 3
  "....-",  // 4
  ".....",  // 5
  "-....",  // 6
  "--...",  // 7
  "---..",  // 8
  "----.",  // 9

  "---...", // :
  "-.-.-.", // ;
  "",       // < (not implemented)
  "-...-",  // =
  "",       // > (not implemented)
  "..--..", // ?
  "",       // @ (not implemented)

  ".-",     // A
  "-...",   // B
  "-.-.",   // C
  "-..",    // D
  ".",      // E
  "..-.",   // F
  "--.",    // G
  "....",   // H
  "..",     // I
  ".---",   // J
  "-.-",    // K
  ".-..",   // L
  "--",     // M
  "-.",     // N
  "---",    // O
  ".--.",   // P
  "--.-",   // Q
  ".-.",    // R
  "...",    // S
  "-",      // T
  "..-",    // U
  "...-",   // V
  ".--",    // W
  "-..-",   // X
  "-.--",   // Y
  "--..",   // Z
};

/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  char buffer[MAX_STRING_LENGTH];

  // setup UART
  uart_set_baud(BAUD_RATE);
  USI_CT = (1<<USI_CT_EN);

  // configure time base
  uint32_t clock = CLOCKSPEED_32bit;
  time_base = 0;
  while (clock >= TIME_PRSC) {
    clock = clock - TIME_PRSC;
    time_base++; // time base for a 'Dit'
  }

  uart_br_print("\n--- Morse code translator ---\n");
  uart_br_print("Enter a string to translate it to Morse code.\n");
  uart_br_print("Output via high-active LED at GPIO.out(0) (bootloader status LED).\n");

  // check if GPIO unit was synthesized, exit if no GPIO is available
  if (!(SYS_FEATURES & (1<<SYS_GPIO_EN))) {
    uart_br_print("Error! No GPIO unit synthesized!");
    return 1;
  }

  gpio_port_set(0); // LED off

  while (1) {
    // get string
    uart_br_print("\nEnter text: ");
    uint16_t length = uart_scan(buffer, MAX_STRING_LENGTH);
    uart_br_print("\nSending: ");

    // encode each letter
    uint16_t i = 0;
    for (i=0; i<length; i++) {
      char c = buffer[i];

      // cast to capital letters
      if ((c >= 'a') && (c <= 'z'))
        c -= 32;

      // in valid alphabet?
      if ((c >= ',') && (c <= 'Z')) {
        uint8_t index = c - ',';
        uart_br_print(" ");
        send_morse(morse_code[index]);

        uint32_t time = time_base * 3; // inter-letter pause
        while(time--)
          asm volatile ("nop");

      }
      // user abort?
      if (uart_char_received() != 0) {
        uart_br_print("\nAborted.");
        break;
      }
    }

  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Send morse code via LED pin
 * PARAM *s pointer to source morse symbol string
 * ------------------------------------------------------------ */
void send_morse(char *s){

  char c = 0;
  uint32_t time = 0;

  while ((c = *s++)) {
    
    gpio_pin_set(LED_PIN); // LED on

    if (c == '.')
      time = time_base;
    else if (c == '-')
      time = time_base * 3;
    else
      time = 0;

    uart_putc(c);

    // wait
    while(time--)
      asm volatile ("nop");

    gpio_pin_clr(LED_PIN); // LED off

    // inter-symbol pause
    time = time_base;
    while(time--)
      asm volatile ("nop");
  }
}