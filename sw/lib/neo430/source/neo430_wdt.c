// #################################################################################################
// #  < neo430_wdt.c - Watchdog helper functions >                                                 #
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
#include "neo430_wdt.h"


/* ------------------------------------------------------------
 * INFO Enable watchdog
 * PARAM Prescaler selection (0..7)
 * ------------------------------------------------------------ */
void neo430_wdt_enable(uint8_t prsc) {

  WDT_CT = (WDT_CT_PASSWORD<<8) | (1<<WDT_CT_EN) | (prsc & 0x07);
}


/* ------------------------------------------------------------
 * INFO Disable watchdog
 * ------------------------------------------------------------ */
void neo430_wdt_disable(void) {

  WDT_CT = (WDT_CT_PASSWORD<<8) | (0<<WDT_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Reset watchdog
 * ------------------------------------------------------------ */
void neo430_wdt_reset(void) {

  WDT_CT = WDT_CT | (WDT_CT_PASSWORD<<8);
}


/* ------------------------------------------------------------
 * INFO Perform a hardware reset by activating the WDT and
 * performing an invalid access (wrong password)
 * ------------------------------------------------------------ */
void neo430_wdt_force_hw_reset(void) {

  WDT_CT = (WDT_CT_PASSWORD<<8) | (1<<WDT_CT_EN);
  WDT_CT = 0; // invalid access -> triggers reset
}
