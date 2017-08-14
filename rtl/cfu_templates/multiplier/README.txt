***************************************************************************************************
* Multiply-and-accumulate unit for the custom function unit (CFU) slot of the NEO430 processor    *
* By Stephan Nolting                                                                              *
***************************************************************************************************

This unit implements a 16-bit times 16-bit multiplier computing a 32-bit wide result.
The operations can be either signed or unsigned.
Furthermore, the unit provides an internal 32-bit accumulator to perform multiply-and-accumulate operations.
It is partly compatible to the original MSP430 multiplier.


This unit features seven interface registers, that can only be accessed in word (16-bit) mode.
Write the first operand to one of the following register to perform the according operations:

 - CFU_MAC16_MPY  : Operand A for unsigned multiplication (write-only)
 - CFU_MAC16_MPYS : Operand A for signed multiplication (write-only)
 - CFU_MAC16_MAC  : Operand A for unsigned multiply-and-accumulate (write-only)
 - CFU_MAC16_MACS : Operand A for signed multiply-and-accumulate (write-only)

The second operand is always written to the OP2 register, regardless of the actual operation:

 - CFU_MAC16_OP2 : Operand B for all operations (write-only)

The final 32-bit wide result is split-up into two 16-bit registers:

 - CFU_MAC16_RESLO : Low word of the result (read-only)
 - CFU_MAC16_RESHI : high word of the result (read-only)

The result can also be "read" as 32-bit value:

 - CFU_MAC16_RES_32bit : 32-bit wide result (read-only)


To install the hardware of this unit, replace the original neo430_cfu.vhd in the rtl\core folder with the
neo430_cfu.vhd from this folder.

Include the header file from this folder to your application code to install the required software driver functions.

The header file (cfu_mac16.h) provides the required definitions of the unit's interface registers
as well as some rudimentary multiplication/multiply-and-accumulate operations implemented as inlined
C functions.

Include the cfu_mac16.h AFTER you have included the general NEO430 include file:
#include ".../lib/neo430/neo430.h"
#include ".../cfu_mac16.h"
