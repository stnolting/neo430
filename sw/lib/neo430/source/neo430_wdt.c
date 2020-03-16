// #################################################################################################
// #  < neo430_wdt.c - Watchdog helper functions >                                                 #
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
