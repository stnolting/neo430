// #################################################################################################
// #  < neo430_twi.h - Internal Two Wire Serial interface functions >                              #
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
// # Stephan Nolting, Hannover, Germany                                                 10.10.2019 #
// #################################################################################################

#ifndef neo430_twi_h
#define neo430_twi_h

// prototypes
void    neo430_twi_enable(uint8_t prsc);   // configure and activate TWI module
void    neo430_twi_disable(void);          // deactivate TWI module
uint8_t neo430_twi_start_trans(uint8_t a); // start transmission (START) and send address and r/w bit
uint8_t neo430_twi_trans(uint8_t d);       // send byte to active slave
uint8_t neo430_twi_get_data(void);         // get last received data byte
void    neo430_twi_generate_stop(void);    // stop transmission (STOP)
void    neo430_twi_generate_start(void);   // generate start condition

#endif // neo430_twi_h
