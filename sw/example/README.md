## Example SW Project


### blink_led

A simple "hello world" program blinking some LEDs. You can use this as your first test program.


### cfu_test

This program tests if the CFU is synthesized and performs a simple write and read-back using all
CFU registers.


### coremark

CPU performance benchmark. You will see a warning when compiling this project to use a specific
flag in your command to really compile it.


### crc_test

Allows a user-defined test of the NEO430 checksum unit.


### exirq_test

Shows how to use the external interrupts controller.


### game_of_life

The classic game of life for the NEO430. Aww, so pretty :D


### gpio_interrupt

Shows how to use the pin-change interrupts of the GPIO module.


### gpio_pwm_demo

This example just uses a sw pwm for the GPIO outputs.


### hw_analysis

This program checks which optional modules are synthesized in your design and gives some basic
information regarding HW version and memory configuration.


### morse_translator

A kinda useless program that outputs a user-defined text as Morse code to an LED connected to
the GPIO output.


### muldiv_test

This test program tests the multiplier and divider unit using ALL possible test cases and compares
the results to a pure-sw reference.


### nested_irq

A simple example showing how to implement nested IRQs for the NEO430.


### prime_numbers

This program outputs prime numbers using the NEO430 version of printf.


### pwm_demo

Just an example on how to use the NEO430 PWM unit.


### timer_simple

As the name suggests, a simple how-to for the TIMER unit.


### trng_test

This sw project uses the TRNG. It can generate a histogram based on the generated random data or just
output raw random data.


### twi_test

An interactive program to check the TWI bus.


### uart_irq

Shows how to use the UART with RX and TX interrupts.


### wb_terminal

A terminal program to explore devices connected to the Wishbone bus.


### wdt_test

A simple usage example for the watchdog timer.