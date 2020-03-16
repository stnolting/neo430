// #################################################################################################
// #  < neo430_wishbone.h - Internal Wishbone interface control functions >                        #
// # ********************************************************************************************* #
// # Use the normal Wishbone functions for BLOCKING access (until ACK is asserted).                #
// # Use non-blocking functions (*_start, wishbone_busy, wishbone_get_data*) to prevent dead locks #
// # when accessing invalid addresses and to do things in parallel when using the Wishbone bus.    #
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
