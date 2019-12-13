// #################################################################################################
// #  < neo430_timer.h - Tim helper functions ;) >                                                 #
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
// # Stephan Nolting, Hannover, Germany                                                 10.12.2019 #
// #################################################################################################

#ifndef neo430_timer_h
#define neo430_timer_h

// prototypes
void    neo430_timer_enable(void);                    // enable timer unit
void    neo430_timer_disable(void);                   // disable timer unit
void    neo430_timer_reset(void);                     // reset timer
void    neo430_timer_start(void);                     // start timer
void    neo430_timer_stop(void);                      // stop timer
uint8_t neo430_timer_config_period(uint32_t f_timer); // configure (irq) frequency

#endif // neo430_timer_h
