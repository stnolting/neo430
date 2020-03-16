// #################################################################################################
// #  < neo430_twi.c - Internal Two Wire Serial interface functions >                              #
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
#include "neo430_twi.h"


/* ------------------------------------------------------------
 * INFO Activate (and reset) TWI module
 * PARAM clock prescaler
 * ------------------------------------------------------------ */
void neo430_twi_enable(uint8_t prsc) {

  TWI_CT = 0; // reset
  TWI_CT = (1 << TWI_CT_EN) | (prsc << TWI_CT_PRSC0);
}


/* ------------------------------------------------------------
 * INFO Deactivate TWI module
 * ------------------------------------------------------------ */
void neo430_twi_disable(void) {

  TWI_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Activate sending ACK by master after transmission
 * ------------------------------------------------------------ */
void neo430_twi_mack_enable(void) {

  TWI_CT |= (1 << TWI_CT_MACK);
}


/* ------------------------------------------------------------
 * INFO Deactivate sending ACK by master after transmission (normal mode)
 * ACK is sampled from slave
 * ------------------------------------------------------------ */
void neo430_twi_mack_disable(void) {

  TWI_CT &= ~(1 << TWI_CT_MACK);
}


/* ------------------------------------------------------------
 * INFO Generate START condition and send first byte (address & R/W)
 * PARAM 8-bit including 7-bit address and read/write bit
 * RETURN 0 if ACK received, 1 if no valid ACK was received
 * ------------------------------------------------------------ */
uint8_t neo430_twi_start_trans(uint8_t a) {

  neo430_twi_generate_start(); // generate START condition

  TWI_DATA = (uint16_t)a; // send address
  while(TWI_CT & (1 << TWI_CT_BUSY)); // wait until idle again

  // check for ACK/NACK
  if (TWI_DATA & (1 << TWI_DT_ACK))
    return 0; // ACK received
  else
    return 1; // NACK received
}


/* ------------------------------------------------------------
 * INFO Send data and also sample input data
 * PARAM Data byte to be sent
 * RETURN 0 if ACK received, 1 if no valid ACK was received
 * ------------------------------------------------------------ */
uint8_t neo430_twi_trans(uint8_t d) {

  TWI_DATA = (uint16_t)d; // send data
  while(TWI_CT & (1 << TWI_CT_BUSY)); // wait until idle again

  // check for ACK/NACK
  if (TWI_DATA & (1 << TWI_DT_ACK))
    return 0; // ACK received
  else
    return 1; // NACK received
}


/* ------------------------------------------------------------
 * INFO Get rx data from previous transmission
 * RETURN Last received data byte
 * ------------------------------------------------------------ */
uint8_t neo430_twi_get_data(void) {

  return (uint8_t)TWI_DATA; // get RX data from previous transmission
}


/* ------------------------------------------------------------
 * INFO Generate STOP condition
 * ------------------------------------------------------------ */
void neo430_twi_generate_stop(void) {

  TWI_CT |= (1 << TWI_CT_STOP); // generate STOP condition
  while(TWI_CT & (1 << TWI_CT_BUSY)); // wait until idle again
}


/* ------------------------------------------------------------
 * INFO Generate START condition
 * ------------------------------------------------------------ */
void neo430_twi_generate_start(void) {

  TWI_CT |= (1 << TWI_CT_START); // generate START condition
  while(TWI_CT & (1 << TWI_CT_BUSY)); // wait until idle again
}
