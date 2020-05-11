// #################################################################################################
// #  < NEO430 Boot image generator >                                                              #
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

  if (argc != 4) {
  	printf("<<< NEO430 executable image generator >>>\n"
	       "Three arguments are required.\n"
	       "1st: Option\n"
	       " -app_bin : Generate application executable binary (with header!) \n"
	       " -app_img : Generate application raw executable memory image (text file, no header!)\n"
	       " -bld_img : Generate bootloader raw executable memory image (text file, no header!)\n"
		     "2nd: Input file (raw binary image)\n"
		     "3rd: Output file\n");
  	return 0;
  }

  FILE *input, *output;
  unsigned char buffer[2];
  char tmp_string[512];
  uint16_t tmp = 0, size = 0, checksum = 0;
  int i = 0;
  int option = 0;

  if (strcmp(argv[1], "-app_bin") == 0)
    option = 1;
  else if (strcmp(argv[1], "-app_img") == 0)
    option = 2;
  else if (strcmp(argv[1], "-bld_img") == 0)
    option = 3;
  else {
  	printf("Invalid option!");
  	return 2;
  }

  // open input file
  input = fopen(argv[2], "rb");
  if(input == NULL){
    printf("Input file error!");
    return 3;
  }

  // open output file
  output = fopen(argv[3], "wb");
  if(output == NULL){
    printf("Output file error!");
    return 4;
  }


// ------------------------------------------------------------
// Generate BINARY executable (with header!!!) for bootloader upload
// ------------------------------------------------------------
  if (option == 1) {
    // reserve space for header
    fputc(char(0), output);
    fputc(char(0), output);
    fputc(char(0), output);
    fputc(char(0), output);
    fputc(char(0), output);
    fputc(char(0), output);

    buffer[0] = 0;
    buffer[1] = 0;

    checksum = 0;
    size = 0;
    rewind(input);
    while(fread(&buffer, sizeof(unsigned char), 2, input) != 0) {
      tmp = (((uint16_t)buffer[1] << 8) | buffer[0]);
      checksum = checksum ^ tmp;
      fputc(buffer[1], output);
      fputc(buffer[0], output);
      size += 2;
    }

    // build header
    rewind(output);
    fputc((unsigned char)(0xCA), output);
    fputc((unsigned char)(0xFE), output);
    fputc((unsigned char)((size & 0xFF00) >> 8), output);
    fputc((unsigned char)((size & 0x00FF) >> 0), output);
    fputc((unsigned char)((checksum & 0xFF00) >> 8), output);
    fputc((unsigned char)((checksum & 0x00FF) >> 0), output);
  }


// ------------------------------------------------------------
// Generate APPLICATION's executable memory init file (no header!!!)
// ------------------------------------------------------------
  if (option == 2) {

	// header
    sprintf(tmp_string, "-- The NEO430 Processor Project, by Stephan Nolting\r\n"
	 					"-- Auto-generated memory init file (for APPLICATION)\r\n"
						"\r\n"
						"library ieee;\r\n"
						"use ieee.std_logic_1164.all;\r\n"
						"\r\n"
						"package neo430_application_image is\r\n"
						"\r\n"
						"  type application_init_image_t is array (0 to 65535) of std_ulogic_vector(15 downto 0);\r\n"
						"  constant application_init_image : application_init_image_t := (\r\n");
    fputs(tmp_string, output);

	// data
    buffer[0] = 0;
    buffer[1] = 0;
    i = 0;
    while(fread(&buffer, sizeof(unsigned char), 2, input) != 0) {
      tmp = (((uint16_t)buffer[1] << 8) | buffer[0]);
      sprintf(tmp_string, "    %06d => x\"%04x\",\r\n", i, tmp);
      fputs(tmp_string, output);
      buffer[0] = 0;
      buffer[1] = 0;
      i++;
    }

    sprintf(tmp_string, "    others => x\"0000\"\r\n");
    fputs(tmp_string, output);

	// end
    sprintf(tmp_string, "  );\r\n"
						"\r\n"
						"end neo430_application_image;\r\n");
    fputs(tmp_string, output);
  }


// ------------------------------------------------------------
// Generate BOOTLOADER's executable memory init file (no header!!!)
// ------------------------------------------------------------
  if (option == 3) {

	// header
    sprintf(tmp_string, "-- The NEO430 Processor Project, by Stephan Nolting\r\n"
	 					"-- Auto-generated memory init file (for BOOTLOADER)\r\n"
						"\r\n"
						"library ieee;\r\n"
						"use ieee.std_logic_1164.all;\r\n"
						"\r\n"
						"package neo430_bootloader_image is\r\n"
						"\r\n"
						"  type bootloader_init_image_t is array (0 to 65535) of std_ulogic_vector(15 downto 0);\r\n"
						"  constant bootloader_init_image : bootloader_init_image_t := (\r\n");
    fputs(tmp_string, output);

	// data
    buffer[0] = 0;
    buffer[1] = 0;
    i = 0;
    while(fread(&buffer, sizeof(unsigned char), 2, input) != 0) {
      tmp = (((uint16_t)buffer[1] << 8) | buffer[0]);
      sprintf(tmp_string, "    %06d => x\"%04x\",\r\n", i, tmp);
      fputs(tmp_string, output);
      buffer[0] = 0;
      buffer[1] = 0;
      i++;
    }

    sprintf(tmp_string, "    others => x\"0000\"\r\n");
    fputs(tmp_string, output);

	// end
    sprintf(tmp_string, "  );\r\n"
						"\r\n"
						"end neo430_bootloader_image;\r\n");
    fputs(tmp_string, output);
  }


  fclose(input);
  fclose(output);

  return 0;
}

