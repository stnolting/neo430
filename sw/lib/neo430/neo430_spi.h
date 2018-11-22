// #################################################################################################
// #  < neo430_usart.h - Internal SPI module driver functions >                                    #
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
// # Stephan Nolting, Hannover, Germany                                                 19.11.2018 #
// #################################################################################################

#ifndef neo430_spi_h
#define neo430_spi_h

// prototypes
void    neo430_spi_enable(void);   // configure and activate TWI module
void    neo430_spi_disable(void);          // deactivate TWI module
void    neo430_spi_cs_en(uint8_t cs);      // activate slave
void    neo430_spi_cs_dis(uint8_t cs);     // deactivate slave
uint8_t neo430_spi_trans(uint8_t d);       // RTX transfer


/* ------------------------------------------------------------
 * INFO Enable SPI CSx (set low)
 * ------------------------------------------------------------ */
void neo430_spi_enable(void) {

  SPI_CT = (1 << SPI_CT_EN);
}


/* ------------------------------------------------------------
 * INFO Enable SPI CSx (set low)
 * ------------------------------------------------------------ */
void neo430_spi_disable(void) {

  SPI_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Enable SPI CSx (set low)
 * PARAM CS line (0..7)
 * ------------------------------------------------------------ */
void neo430_spi_cs_en(uint8_t cs) {

  cs = cs & 7;

  SPI_CT &= ~(15 << SPI_CT_CS_SEL0); // clear CS selection and CS_set
  SPI_CT |= (1 << SPI_CT_CS_SET) | (cs << SPI_CT_CS_SEL0);
}


/* ------------------------------------------------------------
 * INFO Disable SPI CSx (set high)
 * PARAM CS line (0..7)
 * ------------------------------------------------------------ */
void neo430_spi_cs_dis(uint8_t cs) {

  SPI_CT &= ~(1 << SPI_CT_CS_SET);
}


/* ------------------------------------------------------------
 * INFO SPI RTX byte transfer
 * INFO SPI SCK speed: f_main/(2*PRSC), PRSC = see below (control reg)
 * SPI clock prescaler select:
 *  0: CLK/2
 *  1: CLK/4
 *  2: CLK/8
 *  3: CLK/64
 *  4: CLK/128
 *  5: CLK/1024
 *  6: CLK/2048
 *  7: CLK/4096
 * PARAM d byte to be send
 * RETURN received byte
 * ------------------------------------------------------------ */
uint8_t neo430_spi_trans(uint8_t d) {

  SPI_RTX = (uint16_t)d; // trigger transfer
  while((SPI_CT & (1<<SPI_CT_BUSY)) != 0); // wait for current transfer to finish

  return (uint8_t)SPI_RTX;
}


#endif // neo430_spi_h
