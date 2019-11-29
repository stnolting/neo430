// #################################################################################################
// #  < neo430_uart.h - Internal UART driver functions >                                           #
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
// # Stephan Nolting, Hannover, Germany                                                 27.11.2019 #
// #################################################################################################

#ifndef neo430_uart_h
#define neo430_uart_h

// Libs required by functions
#include <stdarg.h>

// prototypes
void neo430_uart_setup(uint32_t baudrate);                                 // activate and configure UART
void neo430_uart_putc(char c);                                             // send single char
char neo430_uart_getc(void);                                               // wait and read single char
uint16_t neo430_uart_char_received(void);                                  // test if a char has been received
char neo430_uart_char_read(void);                                          // get a received char
void neo430_uart_print(char *s);                                           // print a text string
void neo430_uart_br_print(char *s);                                        // print a text string and allow easy line breaks
uint16_t neo430_uart_scan(char *buffer, uint16_t max_size, uint16_t echo); // read several chars into buffer
void neo430_uart_print_hex_char(char c);                                   // print byte as a hex char
void neo430_uart_print_hex_byte(uint8_t b);                                // print byte as 2 hex numbers
void neo430_uart_print_hex_word(uint16_t w);                               // print word as 4 hex numbers
void neo430_uart_print_hex_dword(uint32_t dw);                             // print double word as 8 hex numbers
void neo430_uart_print_bin_byte(uint8_t b);                                // print byte in binary form
void neo430_uart_print_bin_word(uint16_t w);                               // print word in binary form
void neo430_uart_print_bin_dword(uint32_t dw);                             // print double word in binary form
void neo430_itoa(uint32_t x, const uint16_t leading_zeros);                // convert double word to decimal number
void neo430_printf(char *format, ...);                                     // print format string
void neo430_fp_print(int32_t num, const uint16_t fp);                      // print fixed point number
uint32_t neo430_hexstr_to_uint(char *buffer, uint8_t length);              // convert hex string to number
void neo430_uart_bs(uint16_t n);                                           // return terminal cursor n positions

#endif // neo430_uart_h
