// #################################################################################################
// #  < neo430_wishbone.h - Internal Wishbone interface control functions >                        #
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
// # Stephan Nolting, Hannover, Germany                                                 01.06.2018 #
// #################################################################################################

#ifndef neo430_wishbone_h
#define neo430_wishbone_h

// prototypes blocking functions
uint32_t wishbone_read32(uint32_t a);
void wishbone_write32(uint32_t a, uint32_t d);

uint16_t wishbone_read16(uint32_t a); // This function performs a data alignment based on the address!
void wishbone_write16(uint32_t a, uint16_t d); // This function performs a data alignment based on the address!

uint8_t wishbone_read8(uint32_t a); // This function performs a data alignment based on the address!
void wishbone_write8(uint32_t a, uint8_t d); // This function performs a data alignment based on the address!

// prototypes non-blocking functions
void wishbone_read32_start(uint32_t a);
void wishbone_write32_start(uint32_t a, uint32_t d);

void wishbone_read16_start(uint32_t a);
void wishbone_write16_start(uint32_t a, uint16_t d); // This function performs a data alignment based on the address!

void wishbone_read8_start(uint32_t a);
void wishbone_write8_start(uint32_t a, uint8_t d); // This function performs a data alignment based on the address!


uint16_t wishbone_busy(void);
uint32_t wishbone_get_data32(uint32_t a);
uint16_t wishbone_get_data16(uint32_t a); // This function performs a data alignment based on the address!
uint8_t wishbone_get_data8(uint32_t a); // This function performs a data alignment based on the address!


/* ------------------------------------------------------------
 * INFO Read 32-bit from Wishbone device (blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint32_t wishbone_read32(uint32_t a) {

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
void wishbone_write32(uint32_t a, uint32_t d) {

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
uint16_t wishbone_read16(uint32_t a) {

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
void wishbone_write16(uint32_t a, uint16_t d) {

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
uint8_t wishbone_read8(uint32_t a) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // device address aligned to 8-bit + transfer trigger
  WB32_RA_32bit = a;

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);

  // select correct byte to be written
  volatile uint8_t* in = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  return *in;
}


/* ------------------------------------------------------------
 * INFO Write 8-bit to Wishbone device (blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * ------------------------------------------------------------ */
void wishbone_write8(uint32_t a, uint8_t d) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // select correct byte to be written
  volatile uint8_t* out = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  *out = d;

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
void wishbone_read32_start(uint32_t a) {

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
void wishbone_write32_start(uint32_t a, uint32_t d) {

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
void wishbone_read16_start(uint32_t a) {

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
void wishbone_write16_start(uint32_t a, uint16_t d) {

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
void wishbone_read8_start(uint32_t a) {

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
void wishbone_write8_start(uint32_t a, uint8_t d) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // select correct byte to be written
  volatile uint8_t* out = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  *out = d;

  // device address aligned to 8-bit + transfer trigger
  WB32_WA_32bit = a;
}


/* ------------------------------------------------------------
 * INFO Check if Wishbone transaction is (still) in progress
 * RETURN 1 if transfer in progress, 0 if idel
 * ------------------------------------------------------------ */
uint16_t wishbone_busy(void) {

  return (WB32_CT & (1<<WB32_CT_PENDING));
}


/* ------------------------------------------------------------
 * INFO Read 32-bit data after nonblocking transaction has been started
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint32_t wishbone_get_data32(uint32_t a) {

  return WB32_D_32bit;
}


/* ------------------------------------------------------------
 * INFO Read 16-bit data after nonblocking transaction has been started
 * PARAM 32-bit device address
 * RETURN read data
 * ------------------------------------------------------------ */
uint16_t wishbone_get_data16(uint32_t a) {

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
uint8_t wishbone_get_data8(uint32_t a) {

  // select correct byte to be written
  volatile uint8_t* in = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  return *in;
}


#endif // neo430_wishbone_h
