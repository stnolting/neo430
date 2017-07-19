// #################################################################################################
// #  < neo430_cfu_mac16.h - MAC16 CFU Access Wrapper >                                            #
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
// #  Stephan Nolting, Hannover, Germany                                               19.07.2017  #
// #################################################################################################

#ifndef neo430_cfu_mac16_h
#define neo430_cfu_mac16_h

// Interface register address definitions
#define CFU_MAC16_MPY    CFU_REG0 // -/w: operand one - multiply unsigned
#define CFU_MAC16_MPYS   CFU_REG1 // -/w: operand one - multiply signed
#define CFU_MAC16_MAC    CFU_REG2 // -/w: operand one - multiply-accumulate unsigned
#define CFU_MAC16_MACS   CFU_REG3 // -/w: operand one - multiply-accumulate signed
#define CFU_MAC16_OP2    CFU_REG4 // -/w: operand two
#define CFU_MAC16_RESLO  CFU_REG5 // r/-: result low word
#define CFU_MAC16_RESHI  CFU_REG6 // r/-: result high word
#define CFU_MAC16_RES_32bit (*(REG32 (&CFU_MAC16_RESLO))) // r/-: result word


// Function prototypes
inline uint16_t cfu_mul_u16_u16_u16(uint16_t a, uint16_t b);
inline int16_t  cfu_mul_s16_s16_s16(int16_t a, int16_t b);

inline uint32_t cfu_mul_u16_u16_u32(uint16_t a, uint16_t b);
inline int32_t  cfu_mul_s16_s16_s32(int16_t a, int16_t b);

inline void cfu_mac_clear_accu(void);

inline uint16_t cfu_mac_u16_u16_u16(uint16_t a, uint16_t b);
inline int16_t  cfu_mac_s16_s16_s16(int16_t a, int16_t b);

inline uint32_t cfu_mac_u16_u16_u32(uint16_t a, uint16_t b);
inline int32_t  cfu_mac_s16_s16_s32(int16_t a, int16_t b);



/* ------------------------------------------------------------
 * INFO Unsigned 16x16=16 multiplication
 * PARAM a unsigned 16-bit operand a
 * PARAM b unsigned 16-bit operand b
 * RETURN unsigned 16-bit result (lower result word)
 * ------------------------------------------------------------ */
inline uint16_t cfu_mul_u16_u16_u16(uint16_t a, uint16_t b) {

  CFU_MAC16_MPY = a;
  CFU_MAC16_OP2 = b;
  return CFU_MAC16_RESLO;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16=16 multiplication
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 16-bit result (lower result word)
 * ------------------------------------------------------------ */
inline int16_t cfu_mul_s16_s16_s16(int16_t a, int16_t b) {

  CFU_MAC16_MPYS = (uint16_t)a;
  CFU_MAC16_OP2  = (uint16_t)b;
  return (int16_t)CFU_MAC16_RESLO;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16x16=32 multiplication
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 16-bit result
 * ------------------------------------------------------------ */
inline uint32_t cfu_mul_u16_u16_u32(uint16_t a, uint16_t b) {

  CFU_MAC16_MPY = a;
  CFU_MAC16_OP2 = b;
  return CFU_MAC16_RES_32bit;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16=32 multiplication
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 32-bit result (lower result word)
 * ------------------------------------------------------------ */
inline int32_t cfu_mul_s16_s16_s32(int16_t a, int16_t b) {

  CFU_MAC16_MPYS = (uint16_t)a;
  CFU_MAC16_OP2  = (uint16_t)b;
  return (int32_t)CFU_MAC16_RES_32bit;
}



/* ------------------------------------------------------------
 * INFO Clear accumulator
 * ------------------------------------------------------------ */
inline void cfu_mac_clear_accu(void) {

  CFU_MAC16_MPY = 0;
  CFU_MAC16_OP2 = 0;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16x16=16 multiply-and-accumulate
 * PARAM a unsigned 16-bit operand a
 * PARAM b unsigned 16-bit operand b
 * RETURN unsigned 16-bit result (lower result word)
 * ------------------------------------------------------------ */
inline uint16_t cfu_mac_u16_u16_u16(uint16_t a, uint16_t b) {

  CFU_MAC16_MAC = a;
  CFU_MAC16_OP2 = b;
  return CFU_MAC16_RESLO;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16=16 multiply-and-accumulate
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 16-bit result (lower result word)
 * ------------------------------------------------------------ */
inline int16_t cfu_mac_s16_s16_s16(int16_t a, int16_t b) {

  CFU_MAC16_MACS = (uint16_t)a;
  CFU_MAC16_OP2  = (uint16_t)b;
  return (int16_t)CFU_MAC16_RESLO;
}


/* ------------------------------------------------------------
 * INFO Unsigned 16x16=32 multiply-and-accumulate
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 16-bit result
 * ------------------------------------------------------------ */
inline uint32_t cfu_mac_u16_u16_u32(uint16_t a, uint16_t b) {

  CFU_MAC16_MAC = a;
  CFU_MAC16_OP2 = b;
  return CFU_MAC16_RES_32bit;
}


/* ------------------------------------------------------------
 * INFO Signed 16x16=32 multiply-and-accumulate
 * PARAM a signed 16-bit operand a
 * PARAM b signed 16-bit operand b
 * RETURN signed 32-bit result (lower result word)
 * ------------------------------------------------------------ */
inline int32_t cfu_mac_s16_s16_s32(int16_t a, int16_t b) {

  CFU_MAC16_MACS = (uint16_t)a;
  CFU_MAC16_OP2  = (uint16_t)b;
  return (int32_t)CFU_MAC16_RES_32bit;
}


#endif // neo430_cfu_mac16_h