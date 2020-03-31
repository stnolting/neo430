// #################################################################################################
// #  < Conway's Game of Life >                                                                    #
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


// Libraries
#include <stdint.h>
#include <neo430.h>

// Configuration
#define NUM_CELLS_X   160 // must be a multiple of 8
#define NUM_CELLS_Y   40
#define BAUD_RATE     19200
#define GEN_DELAY     500 // delay between iterations in ms
#define TRNG_TAP_MASK 0b01010001000000 // highly experimental!

// Global variables
uint8_t universe[2][NUM_CELLS_X/8][NUM_CELLS_Y];

// Prototypes
void clear_universe(uint8_t u);
void set_cell(uint8_t u, int16_t x, int16_t y);
uint8_t get_cell(uint8_t u, int16_t x, int16_t y);
uint8_t get_neighborhood(uint8_t u, int16_t x, int16_t y);
void print_universe(uint8_t u);
uint16_t pop_count(uint8_t u);


/* ------------------------------------------------------------
 * INFO Main function
 * ------------------------------------------------------------ */
int main(void) {

  uint8_t u = 0, cell = 0, n = 0;
  int16_t x, y;
  uint16_t trng_available = 0;
  uint8_t trng_data = 0;

  // setup UART
  neo430_uart_setup(BAUD_RATE);


  // initialize universe
  uint32_t generation = 0;
  clear_universe(0);
  clear_universe(1);

  neo430_printf("\n\n<<< Conways's Game of Life >>>\n\n");
  neo430_printf("This program requires a terminal resolution of at least %ux%u characters.\n", NUM_CELLS_X+2, NUM_CELLS_Y+3);
  neo430_printf("Press any key to start a random-initialized torus-style universe of %ux%u cells.\n", NUM_CELLS_X, NUM_CELLS_Y);
  neo430_printf("You can pause/restart the simulation by pressing any key.\n");

  // check if TRNG was synthesized
  if ((SYS_FEATURES & (1<<SYS_TRNG_EN))) {
    neo430_printf("TRNG detected. Using TRNG for universe initialization.\n");
    neo430_trng_enable(TRNG_TAP_MASK);
    trng_available = 1;
  }

  if (trng_available) {
    neo430_uart_getc(); // wait for pressed key
  }
  else {
    // randomize until key pressed
    while (neo430_uart_char_received() == 0) {
      neo430_xorshift32();
    }
  }

  // initialize universe using random data
  for (x=0; x<NUM_CELLS_X/8; x++) {
    for (y=0; y<NUM_CELLS_Y; y++) {
      if (trng_available) {
        if (neo430_trng_get(&trng_data)) {
          neo430_printf("TRNG error!\n");
          return 1;
        }
        universe[0][x][y] = (uint8_t)trng_data; // use data from TRNG
      }
      else {
        universe[0][x][y] = (uint8_t)neo430_xorshift32(); // use data von PRNG
      }
    }
  }

  while(1) {

    // user abort?
    if (neo430_uart_char_received()) {
      neo430_printf("\nRestart (y/n)?");
      if (neo430_uart_getc() == 'y') {
        neo430_soft_reset();
      }
    }

    // print generation, population count and the current universe
    neo430_printf("\n\nGeneration %l: %u/%u living cells\n", generation, pop_count(u), NUM_CELLS_X*NUM_CELLS_Y);
    print_universe(u);

    // compute next generation
    clear_universe((u + 1) & 1);

    for (x=0; x<NUM_CELLS_X; x++) {
      for (y=0; y<NUM_CELLS_Y; y++) {

        cell = get_cell(u, x, y); // state of current cell
        n = get_neighborhood(u, x, y); // number of living neighbor cells

        // classic rule set
        if (((cell == 0) && (n == 3)) || ((cell != 0) && ((n == 2) || (n == 3)))) {
          set_cell((u + 1) & 1, x, y);
        }

      } // y
    } // x
    u = (u + 1) & 1; // switch universe
    generation++;

    // wait 500ms
    neo430_cpu_delay_ms(GEN_DELAY);
  }

  return 0;
}


/* ------------------------------------------------------------
 * INFO Print universe u to console
 * ------------------------------------------------------------ */
void print_universe(uint8_t u){

  int16_t x, y;

  neo430_uart_putc('+');
  for (x=0; x<NUM_CELLS_X; x++) {
    neo430_uart_putc('-');
  }
  neo430_uart_putc('+');
  neo430_uart_putc('\r');
  neo430_uart_putc('\n');

  for (y=0; y<NUM_CELLS_Y; y++) {
    neo430_uart_putc('|');
   
    for (x=0; x<NUM_CELLS_X; x++) {
      if (get_cell(u, x, y))
        neo430_uart_putc('#');
      else
        neo430_uart_putc(' ');
    }

    // end of line
    neo430_uart_putc('|');
    neo430_uart_putc('\r');
    neo430_uart_putc('\n');
  }

  neo430_uart_putc('+');
  for (x=0; x<NUM_CELLS_X; x++) {
    neo430_uart_putc('-');
  }
  neo430_uart_putc('+');
}


/* ------------------------------------------------------------
 * INFO Clear universe u
 * ------------------------------------------------------------ */
void clear_universe(uint8_t u){

  uint16_t x, y;

  for (x=0; x<NUM_CELLS_X/8; x++) {
    for (y=0; y<NUM_CELLS_Y; y++) {
      universe[u][x][y] = 0;
    }
  }
}


/* ------------------------------------------------------------
 * INFO Set single cell (make ALIVE) in uinverse u
 * ------------------------------------------------------------ */
void set_cell(uint8_t u, int16_t x, int16_t y){

  if ((x >= NUM_CELLS_X) || (y >= NUM_CELLS_Y))
    return; // out of range

  universe[u][x>>3][y] |= 1 << (7 - (x & 7));
}


/* ------------------------------------------------------------
 * INFO Get state of cell
 * RETURN Cell state (DEAD or ALIVE)
 * ------------------------------------------------------------ */
uint8_t get_cell(uint8_t u, int16_t x, int16_t y){

  // range check: wrap around -> torus-style universe
  if (x < 0)
    x = NUM_CELLS_X-1;

  if (x > NUM_CELLS_X-1)
    x = 0;

  if (y < 0)
    y = NUM_CELLS_Y-1;

  if (y > NUM_CELLS_Y-1)
    y = 0;

  // check bit according to cell
  uint8_t tmp = universe[u][x>>3][y];
  tmp &= 1 << (7 - (x & 7));

  if (tmp == 0)
    return 0; // DEAD
  else
    return 1; // ALIVE
}


/* ------------------------------------------------------------
 * INFO Get number of alive cells in direct neigborhood
 * RETURN Number of set cells in neigborhood
 * ------------------------------------------------------------ */
uint8_t get_neighborhood(uint8_t u, int16_t x, int16_t y){

// Cell index layout:
// 012
// 3#4
// 567

  uint8_t num = 0;
  num += get_cell(u, x-1, y-1); // 0
  num += get_cell(u, x,   y-1); // 1
  num += get_cell(u, x+1, y-1); // 2
  num += get_cell(u, x-1, y);   // 3
  num += get_cell(u, x+1, y);   // 4
  num += get_cell(u, x-1, y+1); // 5
  num += get_cell(u, x,   y+1); // 6
  num += get_cell(u, x+1, y+1); // 7

  return num;
}


/* ------------------------------------------------------------
 * INFO Population count
 * RETURN 16-bit number of living cells in universe u
 * ------------------------------------------------------------ */
uint16_t pop_count(uint8_t u) {

  uint16_t x, y, cnt;

  cnt = 0;
  for (x=0; x<NUM_CELLS_X; x++) {
    for (y=0; y<NUM_CELLS_Y; y++) {
      cnt += (uint16_t)get_cell(u, x, y);
    }
  }

  return cnt;
}
