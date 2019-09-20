// #################################################################################################
// #  < neo430_crc.h - CRC module helper functions >                                               #
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
// # Stephan Nolting, Hannover, Germany                                                 01.03.2019 #
// #################################################################################################

#ifndef neo430_crc_h
#define neo430_crc_h

// prototypes
uint16_t neo430_crc16(uint16_t start_val, uint16_t polynomial, uint8_t *data, uint16_t length);
uint32_t neo430_crc32(uint32_t start_val, uint32_t polynomial, uint8_t *data, uint16_t length);

void neo430_crc16_set_start_value(uint16_t start_val);
void neo430_crc32_set_start_value(uint32_t start_val);
void neo430_crc16_set_polynomial(uint16_t poly);
void neo430_crc32_set_polynomial(uint32_t poly);
uint16_t neo430_crc16_iterate(uint8_t data);
uint32_t neo430_crc32_iterate(uint8_t data);

#endif // neo430_crc_h
