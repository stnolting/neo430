// #################################################################################################
// #  < neo430_wishbone.c - Internal Wishbone interface control functions >                        #
// # ********************************************************************************************* #
// # Use the normal Wishbone functions for BLOCKING access (until ACK is asserted).                #
// # Use non-blocking functions (*_start, wishbone_busy, wishbone_get_data*) to prevent dead locks #
// # when accessing invalid addresses and to do things in parallel when using the Wishbone bus.    #
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
// # Thanks to Edward Sherriff!                                                                    #
// # Stephan Nolting, Hannover, Germany                                                 04.10.2019 #
// #################################################################################################

#include "neo430.h"
#include "neo430_wishbone.h"


// ************************************************************************************************
// Byte-wise access functions, with address alignment, blocking
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Read 32-bit from Wishbone device (blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint32_t neo430_wishbone32_read32(uint32_t a) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // device address aligned to 32-bit + transfer trigger
  WB32_RA_32bit = a & (~3);

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  return WB32_D_32bit;
}


/* ------------------------------------------------------------
 * INFO Write 32-bit to Wishbone device (blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write32(uint32_t a, uint32_t d) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // write data
  WB32_D_32bit = d;

  // device address aligned to 32-bit + transfer trigger
  WB32_WA_32bit = a & (~3);

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


/* ------------------------------------------------------------
 * INFO Read 16-bit from Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * RETURN 16-bit read data
 * ------------------------------------------------------------ */
uint16_t neo430_wishbone32_read16(uint32_t a) {

  // 16-bit transfer
  if (a & 2)
    WB32_CT = 0b1100; // high 16-bit word
  else
    WB32_CT = 0b0011; // low 16-bit word

  // device address aligned to 16-bit + transfer trigger
  WB32_RA_32bit = a & (~1);

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  if (a & 2)
    return WB32_HD; // high 16-bit word
  else
    return WB32_LD; // low 16-bit word
}


/* ------------------------------------------------------------
 * INFO Write 16-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write16(uint32_t a, uint16_t d) {

  // 16-bit transfer
  if (a & 2) {
    WB32_CT = 0b1100; // high 16-bit word
    WB32_HD = d;
  }
  else {
    WB32_CT = 0b0011; // low 16-bit word
    WB32_LD = d;
  }

  // device address aligned to 16-bit + transfer trigger
  WB32_WA_32bit = a & (~1);

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


/* ------------------------------------------------------------
 * INFO Read 8-bit from Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * RETURN 0 if fail, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t neo430_wishbone32_read8(uint32_t a) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // device address aligned to 8-bit + transfer trigger
  WB32_RA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  // select correct byte to be read
  uint16_t data;
  if (a & 2)
    data = WB32_HD;
  else
    data = WB32_LD;

  if (a & 1)
    data = neo430_bswap(data);

  return (uint8_t)data;
}


/* ------------------------------------------------------------
 * INFO Write 8-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write8(uint32_t a, uint8_t d) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // select correct byte to be written
  uint16_t data = (uint16_t)d;
  data = (data << 8) | data;
  WB32_LD = data;
  WB32_HD = data;

  // device address aligned to 8-bit + transfer trigger
  WB32_WA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}

// ************************************************************************************************
// NONBLOCKING FUNCTIONS
// ************************************************************************************************
// Use wishbone_busy() to check status
// Use Wishbone_get_data(address) to get data from read accesses
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Initiate read 32-bit from Wishbone device (non-blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone32_read32_start(uint32_t a) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // device address aligned to 32-bit + transfer trigger
  WB32_RA_32bit = a & (~3);
}


/* ------------------------------------------------------------
 * INFO Initiate write 32-bit to Wishbone device (non-blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write32_start(uint32_t a, uint32_t d) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // write data
  WB32_D_32bit = d;

  // device address aligned to 32-bit + transfer trigger
  WB32_WA_32bit = a & (~3);
}


/* ------------------------------------------------------------
 * INFO Initiate read 16-bit from Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone32_read16_start(uint32_t a) {

  // 16-bit transfer
  if (a & 2)
    WB32_CT = 0b1100; // high 16-bit word
  else
    WB32_CT = 0b0011; // low 16-bit word

  // device address aligned to 16-bit + transfer trigger
  WB32_RA_32bit = a & (~1);
}


/* ------------------------------------------------------------
 * INFO Initiate write 16-bit to Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write16_start(uint32_t a, uint16_t d) {

  // 16-bit transfer
  if (a & 2) {
    WB32_CT = 0b1100; // high 16-bit word
    WB32_HD = d;
  }
  else {
    WB32_CT = 0b0011; // low 16-bit word
    WB32_LD = d;
  }
}


/* ------------------------------------------------------------
 * INFO Initiate read 8-bit from Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone32_read8_start(uint32_t a) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // device address aligned to 8-bit + transfer trigger
  WB32_RA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Initiate write 8-bit to Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write8_start(uint32_t a, uint8_t d) {

  // select correct byte to be written
  uint16_t data = (uint16_t)d;
  data = (data << 8) | data;
  WB32_LD = data;
  WB32_HD = data;

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // device address aligned to 8-bit + transfer trigger
  WB32_WA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Read 32-bit data after nonblocking transaction has been started
 * RETURN read data
 * ------------------------------------------------------------ */
uint32_t neo430_wishbone32_get_data32(void) {

  return WB32_D_32bit;
}


/* ------------------------------------------------------------
 * INFO Read 16-bit data after nonblocking transaction has been started
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint16_t neo430_wishbone32_get_data16(uint32_t a) {

  if (a & 2)
    return WB32_HD; // high 16-bit word
  else
    return WB32_LD; // low 16-bit word
}


/* ------------------------------------------------------------
 * INFO Read 8-bit data after nonblocking transaction has been started
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint8_t neo430_wishbone32_get_data8(uint32_t a) {

  // select correct byte to be read
  uint16_t data;
  if (a & 2)
    data = WB32_HD;
  else
    data = WB32_LD;

  if (a & 1)
    data = neo430_bswap(data);

  return (uint8_t)data;
}


// ************************************************************************************************
// Blocking access functions for data bus width = 32-bit, NO ADDRESS ALIGNMENT
// ************************************************************************************************


/* ------------------------------------------------------------
 * INFO Read 32-bit from Wishbone device (blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * RETURN 32-bit read data
 * ------------------------------------------------------------ */
uint32_t neo430_wishbone32_read(uint32_t a) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // device address + transfer trigger
  WB32_RA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  return WB32_D_32bit;
}


/* ------------------------------------------------------------
 * INFO Write 32-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write(uint32_t a, uint32_t d) {

  // 32-bit transfer
  WB32_CT = 0xf;
  WB32_D_32bit = d;

  // device address + transfer trigger
  WB32_WA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


// ************************************************************************************************
// NONBLOCKING FUNCTIONS
// ************************************************************************************************
// Use wishbone_busy() to check status
// Use Wishbone_get_data(address) to get data from read accesses
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Initiate read 32-bit from Wishbone device (non-blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone32_read_start(uint32_t a) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // device address + transfer trigger
  WB32_RA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Initiate write 32-bit to Wishbone device (non-blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone32_write_start(uint32_t a, uint32_t d) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // write data
  WB32_D_32bit = d;

  // device address + transfer trigger
  WB32_WA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Read 32-bit data after nonblocking transaction has been started
 * PARAM 32-bit device address
 * RETURN 32-bit read data
 * ------------------------------------------------------------ */
uint32_t neo430_wishbone32_get_data(void) {

  return WB32_D_32bit;
}


// ************************************************************************************************
// Blocking access functions for data bus width = 16-bit, NO ADDRESS ALIGNMENT
// ************************************************************************************************


/* ------------------------------------------------------------
 * INFO Read 16-bit from Wishbone device (blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * RETURN 16-bit read data
 * ------------------------------------------------------------ */
uint16_t neo430_wishbone16_read(uint32_t a) {

  // 16-bit transfer
  WB32_CT = 0x3; // low 16-bit word

  // device address + transfer trigger
  WB32_RA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  return WB32_LD; // low 16-bit word
}


/* ------------------------------------------------------------
 * INFO Write 16-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone16_write(uint32_t a, uint16_t d) {

  // 16-bit transfer
  WB32_CT = 0x3; // low 16-bit word
  WB32_LD = d;

  // device address + transfer trigger
  WB32_WA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


// ************************************************************************************************
// NONBLOCKING FUNCTIONS
// ************************************************************************************************
// Use wishbone_busy() to check status
// Use Wishbone_get_data(address) to get data from read accesses
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Initiate read 16-bit from Wishbone device (non-blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone16_read_start(uint32_t a) {

  // 16-bit transfer
  WB32_CT = 0x3;

  // device address + transfer trigger
  WB32_RA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Initiate write 16-bit to Wishbone device (non-blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone16_write_start(uint32_t a, uint16_t d) {

  // 16-bit transfer
  WB32_CT = 0x3;

  // write data
  WB32_LD = d;

  // device address + transfer trigger
  WB32_WA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Read 16-bit data after nonblocking transaction has been started
 * RETURN 16-bit read data
 * ------------------------------------------------------------ */
uint16_t neo430_wishbone16_get_data(void) {

  return WB32_LD;
}


// ************************************************************************************************
// Blocking access functions for data bus width = 8-bit, NO ADDRESS ALIGNMENT
// ************************************************************************************************


/* ------------------------------------------------------------
 * INFO Read 8-bit from Wishbone device (blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * RETURN 8-bit read data
 * ------------------------------------------------------------ */
uint8_t neo430_wishbone8_read(uint32_t a) {

  // 8-bit transfer
  WB32_CT = 0x1;

  // device address + transfer trigger
  WB32_RA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  return (uint8_t)WB32_LD; // low 16-bit word
}


/* ------------------------------------------------------------
 * INFO Write 8-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone8_write(uint32_t a, uint8_t d) {

  // 8-bit transfer
  WB32_CT = 0x1; // low 8-bit word
  WB32_LD = (uint16_t)d;

  // device address + transfer trigger
  WB32_WA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


// ************************************************************************************************
// NONBLOCKING FUNCTIONS
// ************************************************************************************************
// Use wishbone_busy() to check status
// Use Wishbone_get_data(address) to get data from read accesses
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Initiate read 16-bit from Wishbone device (non-blocking), standard mode, pipelined
 * PARAM 8-bit device address
 * ------------------------------------------------------------ */
void neo430_wishbone8_read_start(uint32_t a) {

  // 8-bit transfer
  WB32_CT = 0x1;

  // device address + transfer trigger
  WB32_RA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Initiate write 8-bit to Wishbone device (non-blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * ------------------------------------------------------------ */
void neo430_wishbone8_write_start(uint32_t a, uint8_t d) {

  // 8-bit transfer
  WB32_CT = 0x1;

  // write data
  WB32_LD = (uint16_t)d;

  // device address + transfer trigger
  WB32_WA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Read 8-bit data after nonblocking transaction has been started
 * RETURN 8-bit read data
 * ------------------------------------------------------------ */
uint8_t neo430_wishbone8_get_data(void) {

  return (uint8_t)WB32_LD;
}


// ************************************************************************************************
// NONBLOCKING ARBITRATION FUNCTIONS
// ************************************************************************************************

/* ------------------------------------------------------------
 * INFO Check if Wishbone transaction is (still) in progress
 * RETURN 1 if transfer in progress, 0 if idel
 * ------------------------------------------------------------ */
uint16_t neo430_wishbone_busy(void) {

  return (WB32_CT & (1<<WB32_CT_PENDING));
}


/* ------------------------------------------------------------
 * INFO Terminate current Wishbone transfer
 * ------------------------------------------------------------ */
void neo430_wishbone_terminate(void) {

  WB32_CT = 0;
}
