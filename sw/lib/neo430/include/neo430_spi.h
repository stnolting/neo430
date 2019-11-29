// #################################################################################################
// #  < neo430_spi.h - Internal SPI module driver functions >                                      #
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
// # Stephan Nolting, Hannover, Germany                                                 04.10.2019 #
// #################################################################################################

#ifndef neo430_spi_h
#define neo430_spi_h

// prototypes
void    neo430_spi_enable(uint8_t prsc);   // configure and activate SPI module
void    neo430_spi_disable(void);          // deactivate SPI module
void    neo430_spi_cs_en(uint8_t cs);      // activate slave
void    neo430_spi_cs_dis(void);           // deactivate all slaves
uint8_t neo430_spi_trans(uint8_t d);       // RTX transfer

#endif // neo430_spi_h
