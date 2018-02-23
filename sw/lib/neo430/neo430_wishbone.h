// #################################################################################################
// #  < neo430_wishbone.h - Internal Wishbone interface control functions >                        #
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
// #  Thanks to Edward Sherriff!                                                                   #
// #  Stephan Nolting, Hannover, Germany                                               13.01.2018  #
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
uint32_t wishbone_read32_nonblocking(uint32_t a, uint8_t* success);
void wishbone_write32_nonblocking(uint32_t a, uint32_t d, uint8_t* success);

uint16_t wishbone_read16_nonblocking(uint32_t a, uint8_t* success); // This function performs a data alignment based on the address!
void wishbone_write16_nonblocking(uint32_t a, uint16_t d, uint8_t* success); // This function performs a data alignment based on the address!

uint8_t wishbone_read8_nonblocking(uint32_t a, uint8_t* success); // This function performs a data alignment based on the address!
void wishbone_write8_nonblocking(uint32_t a, uint8_t d, uint8_t* success); // This function performs a data alignment based on the address!


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
 * RETURN 0 if success, 1 if timeout
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



/* ------------------------------------------------------------
 * INFO Read 32-bit from Wishbone device (non-blocking), standard mode, pipelined
 * PARAM 32-bit device address
 * PARAM pointer to byte variable to indicate success (=0)
 * RETURN read data
 * ------------------------------------------------------------ */
uint32_t wishbone_read32_nonblocking(uint32_t a, uint8_t* success) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // device address aligned to 32-bit + transfer trigger
  WB32_RA_32bit = a & (~3);

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));

  return WB32_D_32bit;
}


/* ------------------------------------------------------------
 * INFO Write 32-bit to Wishbone device (non-blocking), standard mode, pipelined
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * PARAM pointer to byte variable to indicate success (=0)
 * ------------------------------------------------------------ */
void wishbone_write32_nonblocking(uint32_t a, uint32_t d, uint8_t* success) {

  // 32-bit transfer
  WB32_CT = 0xF;

  // write data
  WB32_D_32bit = d;

  // device address aligned to 32-bit + transfer trigger
  WB32_WA_32bit = a & (~3);

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));
}


/* ------------------------------------------------------------
 * INFO Read 16-bit from Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * PARAM pointer to byte variable to indicate success (=0)
 * RETURN 16-bit read data
 * ------------------------------------------------------------ */
uint16_t wishbone_read16_nonblocking(uint32_t a, uint8_t* success) {

  // 16-bit transfer
  if (a & 2)
    WB32_CT = 0b1100; // high 16-bit word
  else
    WB32_CT = 0b0011; // low 16-bit word

  // device address aligned to 16-bit + transfer trigger
  WB32_RA_32bit = a & (~1);

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));

  if (a & 2)
    return WB32_HD; // high 16-bit word
  else
    return WB32_LD; // low 16-bit word
}


/* ------------------------------------------------------------
 * INFO Write 16-bit to Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * PARAM pointer to byte variable to indicate success (=0)
 * ------------------------------------------------------------ */
void wishbone_write16_nonblocking(uint32_t a, uint16_t d, uint8_t* success) {

  // 16-bit transfer
  if (a & 2) {
    WB32_CT = 0b1100; // high 16-bit word
    WB32_HD = d;
  }
  else {
    WB32_CT = 0b0011; // low 16-bit word
    WB32_LD = d;
  }

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));

  // wait for access to be completed - blocking!
  while((WB32_CT & (1<<WB32_CT_PENDING)) != 0);
}


/* ------------------------------------------------------------
 * INFO Read 8-bit from Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM 32-bit device address
 * PARAM pointer to byte variable to indicate success (=0)
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_read8_nonblocking(uint32_t a, uint8_t* success) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // device address aligned to 8-bit + transfer trigger
  WB32_RA_32bit = a;

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));

  // select correct byte to be written
  volatile uint8_t* in = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  return *in;
}


/* ------------------------------------------------------------
 * INFO Write 8-bit to Wishbone device (non-blocking), standard mode, pipelined
 * INFO This function performs a data alignment based on the address!
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * PARAM pointer to byte variable to indicate success (=0)
 * ------------------------------------------------------------ */
void wishbone_write8_nonblocking(uint32_t a, uint8_t d, uint8_t* success) {

  // 8-bit transfer
  WB32_CT = 1 << (a & 3); // corresponding byte enable

  // select correct byte to be written
  volatile uint8_t* out = (uint8_t*)(&WB32_D_8bit + ((uint8_t)a & 3));
  *out = d;

  // device address aligned to 8-bit + transfer trigger
  WB32_WA_32bit = a;

  // get status - transfer successful?
  *success = (uint8_t)(WB32_CT & (1<<WB32_CT_PENDING));
}


#endif // neo430_wishbone_h
