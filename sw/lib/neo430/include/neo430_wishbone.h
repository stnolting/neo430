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
// # Stephan Nolting, Hannover, Germany                                                 04.10.2019 #
// #################################################################################################

#ifndef neo430_wishbone_h
#define neo430_wishbone_h

// prototypes blocking functions for 32-bit data bus width
uint32_t neo430_wishbone32_read32(uint32_t a);
void neo430_wishbone32_write32(uint32_t a, uint32_t d);

uint16_t neo430_wishbone32_read16(uint32_t a); // This function performs a data alignment based on the address!
void neo430_wishbone32_write16(uint32_t a, uint16_t d); // This function performs a data alignment based on the address!

uint8_t neo430_wishbone32_read8(uint32_t a); // This function performs a data alignment based on the address!
void neo430_wishbone32_write8(uint32_t a, uint8_t d); // This function performs a data alignment based on the address!

// prototypes non-blocking functions for 32-bit data bus width
void neo430_wishbone32_read32_start(uint32_t a);
void neo430_wishbone32_write32_start(uint32_t a, uint32_t d);

void neo430_wishbone32_read16_start(uint32_t a);
void neo430_wishbone32_write16_start(uint32_t a, uint16_t d); // This function performs a data alignment based on the address!

void neo430_wishbone32_read8_start(uint32_t a);
void neo430_wishbone32_write8_start(uint32_t a, uint8_t d); // This function performs a data alignment based on the address!

uint32_t neo430_wishbone32_get_data32(void);
uint16_t neo430_wishbone32_get_data16(uint32_t a); // This function performs a data alignment based on the address!
uint8_t neo430_wishbone32_get_data8(uint32_t a); // This function performs a data alignment based on the address!

// -------------

// prototypes blocking functions for 32-bit data bus width using NO ADDRESS ALIGNMENT
uint32_t neo430_wishbone32_read(uint32_t a);
void neo430_wishbone32_write(uint32_t a, uint32_t d);

// prototypes non-blocking functions for 32-bit data bus width using NO ADDRESS ALIGNMENT
void neo430_wishbone32_read_start(uint32_t a);
void neo430_wishbone32_write_start(uint32_t a, uint32_t d);
uint32_t neo430_wishbone32_get_data(void);

// -------------

// prototypes blocking functions for 16-bit data bus width using NO ADDRESS ALIGNMENT
uint16_t neo430_wishbone16_read(uint32_t a);
void neo430_wishbone16_write(uint32_t a, uint16_t d);

// prototypes non-blocking functions for 16-bit data bus width using NO ADDRESS ALIGNMENT
void neo430_wishbone16_read_start(uint32_t a);
void neo430_wishbone16_write_start(uint32_t a, uint16_t d);
uint16_t neo430_wishbone16_get_data(void);

// -------------

// prototypes blocking functions for 8-bit data bus width using NO ADDRESS ALIGNMENT
uint8_t neo430_wishbone8_read(uint32_t a);
void neo430_wishbone8_write(uint32_t a, uint8_t d);

// prototypes non-blocking functions for 8-bit data bus width using NO ADDRESS ALIGNMENT
void neo430_wishbone8_read_start(uint32_t a);
void neo430_wishbone8_write_start(uint32_t a, uint8_t d);
uint8_t neo430_wishbone8_get_data8(void);

// -------------

// general status funtion prototypes
uint16_t neo430_wishbone_busy(void);
void neo430_wishbone_terminate(void);

#endif // neo430_wishbone_h
