// #################################################################################################
// #  < neo430_uart.h - Internal UART driver functions >                                           #
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

#ifndef neo430_uart_h
#define neo430_uart_h

// Libs required by functions
#include <stdarg.h>

// prototypes
void neo430_uart_setup(uint32_t baudrate);                                              // activate and configure UART
void neo430_uart_disable(void);                                                         // deactivate uart
uint32_t neo430_uart_get_baudrate(void);                                                // compute actual baudrate using UART's current configuration
void neo430_uart_putc(char c);                                                          // send single char
char neo430_uart_getc(void);                                                            // wait and read single char
uint16_t neo430_uart_char_received(void);                                               // test if a char has been received
char neo430_uart_char_read(void);                                                       // get a received char
void neo430_uart_print(char *s);                                                        // print a text string
void neo430_uart_br_print(char *s);                                                     // print a text string and allow easy line breaks
uint16_t neo430_uart_scan(char *buffer, uint16_t max_size, uint16_t echo);              // read several chars into buffer
void neo430_uart_print_hex_char(char c);                                                // print byte as a hex char
void neo430_uart_print_hex_byte(uint8_t b);                                             // print byte as 2 hex numbers
void neo430_uart_print_hex_word(uint16_t w);                                            // print word as 4 hex numbers
void neo430_uart_print_hex_dword(uint32_t dw);                                          // print double word as 8 hex numbers
void neo430_uart_print_hex_qword(uint64_t qw);                                          // print quad word as 16 hex numbers
void neo430_uart_print_bin_byte(uint8_t b);                                             // print byte in binary form
void neo430_uart_print_bin_word(uint16_t w);                                            // print word in binary form
void neo430_uart_print_bin_dword(uint32_t dw);                                          // print double word in binary form
void neo430_itoa(uint32_t x, uint16_t leading_zeros, char *res);                        // convert double word to decimal number
void neo430_printf(char *format, ...);                                                  // print format string
uint32_t neo430_hexstr_to_uint(char *buffer, uint8_t length);                           // convert hex string to number
void neo430_uart_bs(uint16_t n);                                                        // return terminal cursor n positions
void neo430_uart_print_fpf_32(int32_t num, uint16_t fpf_c, uint16_t num_frac_digits_c); // print fixed-point number

#endif // neo430_uart_h
