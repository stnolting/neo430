// #################################################################################################
// #  < neo430_usart.c - Internal UARt driver functions >                                          #
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

#include "neo430.h"
#include "neo430_uart.h"


/* ------------------------------------------------------------
 * INFO Reset UART, set the Baud rate of UART transceiver
 * INFO UART_BAUD reg (8 bit) = f_main/(prsc*desired_BAUDRATE)
 * INFO PRSC (Baud register bits 10..8):
 *  0: CLK/2
 *  1: CLK/4
 *  2: CLK/8
 *  3: CLK/64
 *  4: CLK/128
 *  5: CLK/1024
 *  6: CLK/2048
 *  7: CLK/4096
 * PARAM actual baudrate to be used
 * ------------------------------------------------------------ */
void neo430_uart_setup(uint32_t baudrate){

  // raw baud rate prescaler
  uint32_t clock = CLOCKSPEED_32bit;
  uint16_t i = 0; // BAUD rate divisor
  uint8_t p = 0; // prsc = CLK/2
  while (clock >= 2*baudrate) {
    clock -= 2*baudrate;
    i++;
  }

  // find clock prsc
  while (i >= 256) {
    if ((p == 2) || (p == 4))
      i >>= 3;
    else
      i >>= 1;
    p++;
  }

  UART_CT = 0;
  UART_CT = (1<<UART_CT_EN) | ((uint16_t)p << UART_CT_PRSC0) | (i << UART_CT_BAUD0);
}


/* ------------------------------------------------------------
 * INFO Disable UART
 * ------------------------------------------------------------ */
void neo430_uart_disable(void){

  UART_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Get current UARt baud rate
 * ------------------------------------------------------------ */
uint32_t neo430_uart_get_baudrate(void) {

  // Clock speed
  uint32_t clock = CLOCKSPEED_32bit;

  // prescaler
  uint16_t prsc;
  switch ((UART_CT >> 8) & 0x0007) {
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

  uint16_t baud = UART_CT & 0x00FF;
  uint32_t baud_value = clock / (uint32_t)(prsc * baud);

  return baud_value;
}


/* ------------------------------------------------------------
 * INFO Send single char via internal UART
 * PARAM c char to send
 * ------------------------------------------------------------ */
void neo430_uart_putc(char c){

  // wait for previous transfer to finish
  while ((UART_CT & (1<<UART_CT_TX_BUSY)) != 0);
  UART_RTX = (uint16_t)c;
}


/* ------------------------------------------------------------
 * INFO Read single char from internal UART (wait until data available)
 * INFO This function is blocking!
 * RETURN received char
 * ------------------------------------------------------------ */
char neo430_uart_getc(void){

  uint16_t d = 0;
  while (1) {
    d = UART_RTX;
    if ((d & (1<<UART_RTX_AVAIL)) != 0) { // char received?
      return (char)d;
    }
  }
}


/* ------------------------------------------------------------
 * INFO Returns value !=0 if a char has received
 * RETURN 0 if no char available
 * ------------------------------------------------------------ */
uint16_t neo430_uart_char_received(void){

  return UART_RTX & (1<<UART_RTX_AVAIL);
}


/* ------------------------------------------------------------
 * INFO Returns char from UART RX unit
 * INFO Check if char present with <uart_char_received>
 * INFO This is the base for a non-blocking read access
 * RETURN received char
 * ------------------------------------------------------------ */
char neo430_uart_char_read(void){

  return (char)UART_RTX;
}


/* ------------------------------------------------------------
 * INFO Print zero-terminated string of chars via internal UART
 * PARAM *s pointer to source string
 * ------------------------------------------------------------ */
void neo430_uart_print(char *s){

  char c = 0;
  while ((c = *s++)) {
    neo430_uart_putc(c);
  }
}


/* ------------------------------------------------------------
 * INFO Print zero-terminated string of chars via internal UART
 * Prints true line break "\r\n" for every '\n'
 * PARAM *s pointer to source string
 * ------------------------------------------------------------ */
void neo430_uart_br_print(char *s){

  char c = 0;
  while ((c = *s++)) {
    if (c == '\n') {
      neo430_uart_putc('\r');
    }
    neo430_uart_putc(c);
  }
}


/* ------------------------------------------------------------
 * INFO Get string via UART, string is automatically zero-terminated.
 * Input is acknowledged by ENTER, local echo, chars can be deleted using BACKSPACE.
 * PARAM buffer to store string to
 * PARAM size of buffer (=max string length incl. zero-termination)
 * PARAM activate local echo when =! 0
 * RETURN Length of string (without zero-termination character)
 * ------------------------------------------------------------ */
uint16_t neo430_uart_scan(char *buffer, uint16_t max_size, uint16_t echo) {

  char c = 0;
  uint16_t length = 0;

  while (1) {
    c = neo430_uart_getc();
    if (c == '\b') { // BACKSPACE
      if (length != 0) {
        if (echo) {
          neo430_uart_print("\b \b"); // delete last char in console
        }
        buffer--;
        length--;
      }
    }
    else if (c == '\r') // carriage return
      break;
    else if ((c >= ' ') && (c <= '~') && (length < (max_size-1))) {
      if (echo) {
        neo430_uart_putc(c); // echo
      }
      *buffer++ = c;
      length++;
    }
  }
  *buffer = '\0'; // terminate string

  return length;
}


/* ------------------------------------------------------------
 * INFO Print single (capital) hexadecimal value (1 digit)
 * PARAM char to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_hex_char(char c) {

  char d = c & 15;
  if (d < 10)
    d += '0';
  else
    d += 'a'-10;
  neo430_uart_putc(d);
}


/* ------------------------------------------------------------
 * INFO Print 8-bit hexadecimal value (2 digits)
 * PARAM uint8_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_hex_byte(uint8_t b) {

  neo430_uart_print_hex_char((char)(b >> 4));
  neo430_uart_print_hex_char((char)(b >> 0));
}


/* ------------------------------------------------------------
 * INFO Print 16-bit hexadecimal value (4 digits)
 * PARAM uint16_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_hex_word(uint16_t w) {

  union uint16_u tmp;
  tmp.uint16 = w;

  neo430_uart_print_hex_byte(tmp.uint8[1]);
  neo430_uart_print_hex_byte(tmp.uint8[0]);
}


/* ------------------------------------------------------------
 * INFO Print 32-bit hexadecimal value (8 digits)
 * PARAM uint32_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_hex_dword(uint32_t dw) {

  union uint32_u tmp;
  tmp.uint32 = dw;

  neo430_uart_print_hex_byte(tmp.uint8[3]);
  neo430_uart_print_hex_byte(tmp.uint8[2]);
  neo430_uart_print_hex_byte(tmp.uint8[1]);
  neo430_uart_print_hex_byte(tmp.uint8[0]);
}


/* ------------------------------------------------------------
 * INFO Print 64-bit hexadecimal value (16 digits)
 * PARAM uint64_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_hex_qword(uint64_t qw) {

  union uint64_u tmp;
  tmp.uint64 = qw;

  neo430_uart_print_hex_byte(tmp.uint8[7]);
  neo430_uart_print_hex_byte(tmp.uint8[6]);
  neo430_uart_print_hex_byte(tmp.uint8[5]);
  neo430_uart_print_hex_byte(tmp.uint8[4]);
  neo430_uart_print_hex_byte(tmp.uint8[3]);
  neo430_uart_print_hex_byte(tmp.uint8[2]);
  neo430_uart_print_hex_byte(tmp.uint8[1]);
  neo430_uart_print_hex_byte(tmp.uint8[0]);
}


/* ------------------------------------------------------------
 * INFO Print 8-bit binary value (8 digits)
 * PARAM uint8_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_bin_byte(uint8_t b) {

  uint8_t i;
  for (i=0x80; i!=0; i>>=1) {
    if (b & i)
      neo430_uart_putc('1');
    else
      neo430_uart_putc('0');
  }
}


/* ------------------------------------------------------------
 * INFO Print 16-bit binary value (16 digits)
 * PARAM uint16_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_bin_word(uint16_t w) {

  union uint16_u tmp;
  tmp.uint16 = w;

  neo430_uart_print_bin_byte(tmp.uint8[1]);
  neo430_uart_print_bin_byte(tmp.uint8[0]);
}


/* ------------------------------------------------------------
 * INFO Print 32-bit binary value (32 digits)
 * PARAM uint32_t value to be printed
 * ------------------------------------------------------------ */
void neo430_uart_print_bin_dword(uint32_t dw) {

  union uint32_u tmp;
  tmp.uint32 = dw;

  neo430_uart_print_bin_byte(tmp.uint8[3]);
  neo430_uart_print_bin_byte(tmp.uint8[2]);
  neo430_uart_print_bin_byte(tmp.uint8[1]);
  neo430_uart_print_bin_byte(tmp.uint8[0]);
}


/* ------------------------------------------------------------
 * INFO Print 32-bit number as decimal number (10 digits)
 * INFO Slow custom version of itoa
 * PARAM 32-bit value to be printed as decimal number
 * PARAM show #leading_zeros leading zeros
 * PARAM pointer to array (11 elements!!!) to store conversion result string
 * ------------------------------------------------------------ */
void neo430_itoa(uint32_t x, uint16_t leading_zeros, char *res) {

  const char numbers[10] = "0123456789";
  char buffer1[11];
  uint16_t i, j;

  buffer1[10] = '\0';
  res[10] = '\0';

  // convert
  for (i=0; i<10; i++) {
    buffer1[i] = numbers[x%10];
    x /= 10;
  }

  // delete 'leading' zeros
  for (i=9; i!=leading_zeros; i--) {
    if (buffer1[i] == '0')
      buffer1[i] = '\0';
    else
      break;
  }

  // reverse
  j = 0;
  do {
    if (buffer1[i] != '\0')
      res[j++] = buffer1[i];
  } while (i--);

  res[j] = '\0'; // terminate result string
}


/* ------------------------------------------------------------
 * INFO Embedded version of the printf function with reduced functionality
 * INFO Only use this function if it is really required!
 * INFO It is large and slow... ;)
 * INFO Original from http://forum.43oh.com/topic/1289-tiny-printf-c-version/
 * PARAM Argument string
 * ------------------------------------------------------------ */
void neo430_printf(char *format, ...) {

  char c, string_buf[11];
  int32_t n;

  va_list a;
  va_start(a, format);

  while ((c = *format++)) {
    if (c == '%') {
      c = *format++;
      switch (c) {
        case 's': // string
          neo430_uart_print(va_arg(a, char*));
          break;
        case 'c': // char
          neo430_uart_putc((char)va_arg(a, int));
          break;
        case 'b': // unsigned 16-bit binary
          neo430_uart_print_bin_word(va_arg(a, unsigned int));
          break;
        case 'i': // 16-bit integer
          n = (int32_t)va_arg(a, int);
          if (n < 0) {
            n = -n;
            neo430_uart_putc('-');
          }
          neo430_itoa((uint32_t)n, 0, string_buf);
          neo430_uart_br_print(string_buf);
          break;
        case 'u': // 16-bit unsigned
          neo430_itoa((uint32_t)va_arg(a, unsigned int), 0, string_buf);
          neo430_uart_br_print(string_buf);
          break;
        case 'l': // 32-bit signed long
          n = (int32_t)va_arg(a, int32_t);
          if (n < 0) {
            n = -n;
            neo430_uart_putc('-');
          }
          neo430_itoa((uint32_t)n, 0, string_buf);
          neo430_uart_br_print(string_buf);
          break;
        case 'n': // 32-bit unsigned long
          neo430_itoa(va_arg(a, uint32_t), 0, string_buf);
          neo430_uart_br_print(string_buf);
          break;
        case 'x': // 16-bit hexadecimal
          neo430_uart_print_hex_word(va_arg(a, unsigned int));
          break;
        case 'X': // 32-bit hexadecimal
          neo430_uart_print_hex_dword(va_arg(a, uint32_t));
          break;
        default:
          return;
      }
    }
    else {
      if (c == '\n')
        neo430_uart_putc('\r');
      neo430_uart_putc(c);
    }
  }
  va_end(a);
}


/* ------------------------------------------------------------
 * INFO Convert N hex chars into uint32
 * PARAM Pointer to buffer with hex chars
 * PARAM Number of hex chars to convert (1..8)
 * RETURN Conversion result
 * ------------------------------------------------------------ */
uint32_t neo430_hexstr_to_uint(char *buffer, uint8_t length) {

  uint32_t res = 0, d = 0;
  char c = 0;

  while (length--) {
    c = *buffer++;

    if ((c >= '0') && (c <= '9'))
      d = (uint32_t)(c - '0');
    else if ((c >= 'a') && (c <= 'f'))
      d = (uint32_t)((c - 'a') + 10);
    else if ((c >= 'A') && (c <= 'F'))
      d = (uint32_t)((c - 'A') + 10);
    else
      d = 0;

    res = res + (d << (length*4));
  }

  return res;
}


/* ------------------------------------------------------------
 * INFO Return terminal cursor n positions
 * PARAM n positions
 * ------------------------------------------------------------ */
void neo430_uart_bs(uint16_t n) {

  while (n--) {
    neo430_uart_putc(0x08);
  }
}


/* ------------------------------------------------------------
 * INFO Print signed 32-bit fixed point number (num)
 * PARAM fpf_c: Number of bin fractional bits in input (max 32)
 * PARAM num_frac_digits_c: Number of fractional digits to show (max 8)
 * ------------------------------------------------------------ */
void neo430_uart_print_fpf_32(int32_t num, uint16_t fpf_c, uint16_t num_frac_digits_c) {

  uint16_t i;

  // make positive
  if (num < 0) {
    neo430_uart_putc('-');
    num = -num;
  }
  uint32_t num_int = (uint32_t)num;


  // print integer part
  char int_buf[11];
  neo430_itoa((uint32_t)(num_int >> fpf_c), 0, int_buf);
  neo430_uart_br_print(int_buf);
  neo430_uart_putc('.');


  // compute fractional resolution
  uint32_t frac_dec_base = 1;
  for (i=0; i<num_frac_digits_c; i++) {
    frac_dec_base = frac_dec_base * 10;
  }
  frac_dec_base = frac_dec_base >> 1;

  // compute fractional part's bit-insulation shift mask
  uint32_t frac_dec_mask = 1L << (fpf_c-1);


  // compute actual fractional part
  uint32_t frac_data = num_int & ((1 << fpf_c)-1); // only keep fractional bits
  uint32_t frac_sum = 1;
  for (i=0; i<fpf_c; i++) { // test each fractional bit
    if (frac_data & frac_dec_mask) { // insulate current fractional bit
      frac_sum += frac_dec_base;
    }
    frac_dec_mask >>= 1; // go from MSB to LSB
    frac_dec_base >>= 1;
  }

  // print fractional part
  char frac_buf[11];
  neo430_itoa((uint32_t)frac_sum, num_frac_digits_c-1, frac_buf);
  frac_buf[num_frac_digits_c] = '\0'; // truncate
  neo430_uart_br_print(frac_buf);
}

