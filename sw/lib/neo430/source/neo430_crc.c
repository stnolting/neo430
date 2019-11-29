// #################################################################################################
// #  < neo430_crc.c - CRC module helper functions >                                               #
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
// # Stephan Nolting, Hannover, Germany                                                 13.03.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_crc.h"


/* ------------------------------------------------------------
 * INFO Compute CRC16 from buffer
 * PARAM start_val: Start value for CRC shift register
 * PARAM polynomial: 16-bit polynomial XOR mask
 * PARAM data: Pointer to BYTE input data array
 * PARAM length: Number of elements in input data array
 * RETURN CRC16 result
 * ------------------------------------------------------------ */
uint16_t neo430_crc16(uint16_t start_val, uint16_t polynomial, uint8_t *data, uint16_t length) {

  CRC_POLY_LO = polynomial;
  CRC_RESX = start_val;

  while(length) {
    CRC_CRC16IN = (uint16_t)(*data++); // no wait required here
    length--;
  }

  return CRC_RESX;
}


/* ------------------------------------------------------------
 * INFO Compute CRC32 from buffer
 * PARAM start_val: Start value for CRC shift register
 * PARAM polynomial: 32-bit polynomial XOR mask
 * PARAM data: Pointer to BYTE input data array
 * PARAM length: Number of elements in input data array
 * RETURN CRC32 result
 * ------------------------------------------------------------ */
uint32_t neo430_crc32(uint32_t start_val, uint32_t polynomial, uint8_t *data, uint16_t length) {

  CRC_POLY32bit = polynomial;
  CRC_R32bit = start_val;

  while(length) {
    CRC_CRC32IN = (uint16_t)(*data++); // no wait required here
    length--;
  }

  return CRC_R32bit;
}


/* ------------------------------------------------------------
 * INFO Initialize start value for CRC16
 * PARAM 16-bit CRC shift reg start value
 * ------------------------------------------------------------ */
void neo430_crc16_set_start_value(uint16_t start_val) {

  CRC_RESX = start_val;
}


/* ------------------------------------------------------------
 * INFO Initialize start value for CRC32
 * PARAM 32-bit CRC shift reg start value
 * ------------------------------------------------------------ */
void neo430_crc32_set_start_value(uint32_t start_val) {

  CRC_R32bit = start_val;
}


/* ------------------------------------------------------------
 * INFO Set polynomial mask for CRC16
 * PARAM 16-bit CRC16 polynomial XOR mask
 * ------------------------------------------------------------ */
void neo430_crc16_set_polynomial(uint16_t poly) {

  CRC_POLY_LO = poly;
}


/* ------------------------------------------------------------
 * INFO Set polynomial mask for CRC32
 * PARAM 32-bit CRC16 polynomial XOR mask
 * ------------------------------------------------------------ */
void neo430_crc32_set_polynomial(uint32_t poly) {

  CRC_POLY32bit = poly;
}


/* ------------------------------------------------------------
 * INFO Compute CRC16 for one new data byte
 * PARAM 8-bit data input
 * RETURN Current result of CRC16 shift reg
 * ------------------------------------------------------------ */
uint16_t neo430_crc16_iterate(uint8_t data) {

  CRC_CRC16IN = (uint16_t)data;
  asm volatile ("nop");
  return CRC_RESX;
}


/* ------------------------------------------------------------
 * INFO Compute CRC32 for one new data byte
 * PARAM 8-bit data input
 * RETURN Current result of CRC32 shift reg
 * ------------------------------------------------------------ */
uint32_t neo430_crc32_iterate(uint8_t data) {

  CRC_CRC32IN = (uint16_t)data;
  asm volatile ("nop");
  return CRC_R32bit;
}
