// #################################################################################################
// #  < neo430_cpu.h - CPU helper functions >                                                      #
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
// # Stephan Nolting, Hannover, Germany                                                 21.11.2019 #
// #################################################################################################

#ifndef neo430_cpu_h
#define neo430_cpu_h

// prototypes
void neo430_eint(void);
void neo430_dint(void);
uint16_t neo430_get_sp(void);
uint16_t neo430_get_sreg(void);
void neo430_set_sreg(uint16_t d);
uint16_t neo430_get_parity(uint16_t d);
void neo430_sleep(void);
void neo430_clear_irq_buffer(void);
void neo430_cpu_delay(uint16_t t);
void neo430_cpu_delay_ms(uint16_t ms);
void neo430_soft_reset(void);
void neo430_jump_address(uint16_t addr);
void neo430_call_address(uint16_t addr);
uint16_t neo430_bswap(uint16_t a);
uint16_t neo430_combine_bytes(uint8_t hi, uint8_t lo);
uint16_t neo430_dadd(uint16_t a, uint16_t b);
void neo430_memset(uint8_t *dst, uint8_t data, uint16_t num);
uint8_t neo430_memcmp(uint8_t *dst, uint8_t *src, uint16_t num);
void neo430_memcpy(uint8_t *dst, uint8_t *src, uint16_t num);
uint16_t neo430_bit_rev16(uint16_t x);
uint32_t neo430_xorshift32(void);

#endif // neo430_cpu_h
