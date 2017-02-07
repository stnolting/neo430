// #################################################################################################
// #  < neo430_wishbone.h - Internal Wishbone interface control functions >                        #
// # ********************************************************************************************* #
// # Always uses 32-bit address, just constrain address argument value                             #
// # when using smaller address bus sizes.                                                         #
// # Make sure to configure the control register before using this functions.                      #
// # ********************************************************************************************* #
// # This file is part of the NEO430 Processor project: http://opencores.org/project,neo430        #
// # Copyright 2015-2016, Stephan Nolting: stnolting@gmail.com                                     #
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
// # source; if not, download it from http://www.gnu.org/licenses/lgpl-3.0.en.html                 #
// # ********************************************************************************************* #
// #  Stephan Nolting, Hannover, Germany                                               13.03.2016  #
// #################################################################################################

#ifndef neo430_wishbone_h
#define neo430_wishbone_h

// prototypes
uint8_t wishbone_read32(uint32_t a, uint32_t *d);
uint8_t wishbone_write32(uint32_t a, uint32_t d);
uint8_t wishbone_read16(uint32_t a, uint16_t *d);
uint8_t wishbone_write16(uint32_t a, uint16_t d);
uint8_t wishbone_read8(uint32_t a, uint8_t *d);
uint8_t wishbone_write8(uint32_t a, uint8_t d);


/* ------------------------------------------------------------
 * INFO Read 32-bit from Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: pointer to 32-bit read data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_read32(uint32_t a, uint32_t *d) {

  register uint16_t ctrl_reg = 0;

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAR = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0) {
      *d = (((uint32_t)WB32_HDI) << 16) | (uint32_t)WB32_LDI; // read data
      return 0; // success!
    }
  }
}


/* ------------------------------------------------------------
 * INFO Write 16-bit to Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: 32-bit write data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_write32(uint32_t a, uint32_t d) {

  register uint16_t ctrl_reg = 0;

  // write data
  WB32_HDO = (uint16_t)(d >> 16);
  WB32_LDO = (uint16_t)(d >>  0);

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAW = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0)
      return 0; // success!
  }
}


/* ------------------------------------------------------------
 * INFO Read 16-bit from Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: pointer to 16-bit read data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_read16(uint32_t a, uint16_t *d) {

  register uint16_t ctrl_reg = 0;

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAR = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0) {
      *d = WB32_LDI; // read data
      return 0; // success!
    }
  }
}


/* ------------------------------------------------------------
 * INFO Write 32-bit to Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: 16-bit write data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_write16(uint32_t a, uint16_t d) {

  register uint16_t ctrl_reg = 0;

  // write data
  WB32_LDO = d;

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAW = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0)
      return 0; // success!
  }
}


/* ------------------------------------------------------------
 * INFO Read 8-bit from Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: pointer to 8-bit read data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_read8(uint32_t a, uint8_t *d) {

  register uint16_t ctrl_reg = 0;

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAR = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0) {
      *d = (uint8_t)WB32_LDI; // read data
      return 0; // success!
    }
  }
}


/* ------------------------------------------------------------
 * INFO Write 8-bit to Wishbone device
 * PARAM a: 32-bit device address
 * PARAM d: 8-bit write data
 * RETURN 0 if success, 1 if timeout
 * ------------------------------------------------------------ */
uint8_t wishbone_write8(uint32_t a, uint8_t d) {

  register uint16_t ctrl_reg = 0;

  // write data
  WB32_LDO = (uint16_t)d;

  // device address
  WB32_HA  = (uint16_t)(a >> 16);
  WB32_LAW = (uint16_t)(a >>  0); // trigger transfer

  // wait for access to be completed or terminated
  while(1) {
    ctrl_reg = WB32_CT;
    if ((ctrl_reg & (1<<WB32_CT_TIMEOUT_C)) != 0)
      return 1; // device not responding
    if ((ctrl_reg & (1<<WB32_CT_PENDING_C)) == 0)
      return 0; // success!
  }
}


#endif // neo430_wishbone_h
