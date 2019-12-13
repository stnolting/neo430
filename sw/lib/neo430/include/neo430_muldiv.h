// #################################################################################################
// #  < neo430_muldiv.h - Multiplier/Divider function >                                            #
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
// # Stephan Nolting, Hannover, Germany                                                 04.07.2018 #
// #################################################################################################

#ifndef neo430_muldiv_h
#define neo430_muldiv_h

// prototypes
uint32_t neo430_umul32(uint16_t a, uint16_t b);
int32_t neo430_mul32(int16_t a, int16_t b);

uint16_t neo430_udiv16(uint16_t dividend, uint16_t divisor);
int16_t neo430_div16(int16_t dividend, int16_t divisor);

uint16_t neo430_umod16(uint16_t dividend, uint16_t divisor);
int16_t neo430_mod16(int16_t dividend, int16_t divisor);

uint16_t neo430_umoddiv16(uint16_t *remainder, uint16_t dividend, uint16_t divisor);
int16_t neo430_moddiv16(int16_t *remainder, int16_t dividend, int16_t divisor);

#endif // neo430_muldiv_h
