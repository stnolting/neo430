// #################################################################################################
// #  < neo430_pwm.h - PWM controller helper functions >                                           #
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

#ifndef neo430_pwm_h
#define neo430_pwm_h

// prototypes
inline void neo430_pwm_enable_slow_mode(void);
inline void neo430_pwm_enable_fast_mode(void);
inline void neo430_pwm_disable(void);
inline void neo430_pwm_set_ch0(uint8_t dc);
inline void neo430_pwm_set_ch1(uint8_t dc);
inline void neo430_pwm_set_ch2(uint8_t dc);


/* ------------------------------------------------------------
 * INFO Activate PWM controller in slow mode
 * ------------------------------------------------------------ */
inline void neo430_pwm_enable_slow_mode(void) {

  PWM_CT = (1<<PWM_CT_ENABLE) | (0<<PWM_CT_FMODE);
}


/* ------------------------------------------------------------
 * INFO Activate PWM controller in fast mode
 * ------------------------------------------------------------ */
inline void neo430_pwm_enable_fast_mode(void) {

  PWM_CT = (1<<PWM_CT_ENABLE) | (1<<PWM_CT_FMODE);
}


/* ------------------------------------------------------------
 * INFO Disable PWM controller
 * ------------------------------------------------------------ */
inline void neo430_pwm_disable(void) {

  PWM_CT = 0;
}


/* ------------------------------------------------------------
 * INFO Set duty cycle of PWM channel 0
 * PARAM 8-bit duty cycle
 * ------------------------------------------------------------ */
inline void neo430_pwm_set_ch0(uint8_t dc) {

  PWM_CH0 = (uint16_t)dc;
}


/* ------------------------------------------------------------
 * INFO Set duty cycle of PWM channel 1
 * PARAM 8-bit duty cycle
 * ------------------------------------------------------------ */
inline void neo430_pwm_set_ch1(uint8_t dc) {

  PWM_CH1 = (uint16_t)dc;
}


/* ------------------------------------------------------------
 * INFO Set duty cycle of PWM channel 2
 * PARAM 8-bit duty cycle
 * ------------------------------------------------------------ */
inline void neo430_pwm_set_ch2(uint8_t dc) {

  PWM_CH2 = (uint16_t)dc;
}


#endif // neo430_pwm_h
