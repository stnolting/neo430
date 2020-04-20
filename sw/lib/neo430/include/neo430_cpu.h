// #################################################################################################
// #  < neo430_cpu.h - CPU helper functions >                                                      #
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

#ifndef neo430_cpu_h
#define neo430_cpu_h

// prototypes
void     neo430_critical_start(void);                             // start critical section
void     neo430_critical_end(void);                               // end critical section
void     neo430_eint(void);                                       // enable global interrupts
void     neo430_dint(void);                                       // disable global interrupts
uint16_t neo430_get_sp(void);                                     // get stack pointer
uint16_t neo430_get_sreg(void);                                   // get status register
void     neo430_set_sreg(uint16_t d);                             // set status register
uint16_t neo430_get_parity(uint16_t d);                           // get parity (EXTENDED ALU OPERATION!)
void     neo430_sleep(void);                                      // set CPU to sleep mode
void     neo430_clear_irq_buffer(void);                           // clear pending IRQs
void     neo430_cpu_delay(uint16_t t);                            // wait cycles
void     neo430_cpu_delay_ms(uint16_t ms);                        // wait ms
void     neo430_soft_reset(void);                                 // perform soft reset
void     neo430_jump_address(uint16_t addr);                      // jump to certain address
void     neo430_call_address(uint16_t addr);                      // call certain address
uint16_t neo430_bswap(uint16_t a);                                // swap bytes in word
uint16_t neo430_combine_bytes(uint8_t hi, uint8_t lo);            // combine two bytes into a single word
void     neo430_memset(uint8_t *dst, uint8_t data, uint16_t num); // set num bytes in memory
uint8_t  neo430_memcmp(uint8_t *dst, uint8_t *src, uint16_t num); // compare num bytes in memory
void     neo430_memcpy(uint8_t *dst, uint8_t *src, uint16_t num); // copy num bytes from memory to memory
uint16_t neo430_bit_rev16(uint16_t x);                            // reverse bit order of word
uint16_t neo430_rotate_right_w(uint16_t x);                       // rotate right word by one bit
uint16_t neo430_rotate_left_w(uint16_t x);                        // rotate left word by one bit
uint8_t  neo430_rotate_right_b(uint8_t x);                        // rotate right byte by one bit
uint8_t  neo430_rotate_left_b(uint8_t x);                         // rotate left byte by one bit
uint32_t neo430_xorshift32(void);                                 // simple PRNG

#endif // neo430_cpu_h
