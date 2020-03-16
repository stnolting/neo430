// #################################################################################################
// #  < neo430_crc.c - CRC module helper functions >                                               #
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
