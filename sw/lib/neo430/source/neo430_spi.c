// #################################################################################################
// #  < neo430_spi.c - Internal SPI module driver functions >                                      #
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
#include "neo430_spi.h"


/* ------------------------------------------------------------
 * INFO Reset, configure speed and enable SPI module
 * INFO SPI SCK speed: f_main/(2*PRSC), prsc = see below (control reg)
 * SPI clock prescaler select:
 *  0: CLK/2
 *  1: CLK/4
 *  2: CLK/8
 *  3: CLK/64
 *  4: CLK/128
 *  5: CLK/1024
 *  6: CLK/2048
 *  7: CLK/4096
 * ------------------------------------------------------------ */
void neo430_spi_enable(uint8_t prsc) {

  SPI_CT = 0; // reset
  SPI_CT = (1 << SPI_CT_EN) | (prsc<<SPI_CT_PRSC0);
}


/* ------------------------------------------------------------
 * INFO Enable SPI module
 * ------------------------------------------------------------ */
void neo430_spi_disable(void) {

  SPI_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Enable SPI CSx (set low)
 * PARAM CS line (0..5)
 * ------------------------------------------------------------ */
void neo430_spi_cs_en(uint8_t cs) {

  SPI_CT |= 1 << (cs+SPI_CT_CS_SEL0);
}


/* ------------------------------------------------------------
 * INFO Disable ALL SPI CS lines (set high)
 * ------------------------------------------------------------ */
void neo430_spi_cs_dis(void) {

  SPI_CT &= ~(0b111111 << SPI_CT_CS_SEL0); // clear all 6 CS lines
}


/* ------------------------------------------------------------
 * INFO SPI RTX byte transfer
 * PARAM d byte to be send
 * RETURN received byte
 * ------------------------------------------------------------ */
uint16_t neo430_spi_trans(uint16_t d) {

  SPI_RTX = d; // trigger transfer
  while((SPI_CT & (1<<SPI_CT_BUSY)) != 0); // wait for current transfer to finish

  return SPI_RTX;
}
