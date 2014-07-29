/*
 *
 * Copyright (c) 2005-2013 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../dvi-mem.h"

//100 MIPS = 100 kIPms = 100 IPµs
void usleep(unsigned int us) {
  while( us ) {
    for(volatile unsigned char i = 0; i < 10; i++); //this loop takes ~100 instructions
    us--;
  }
}

int main() {
  unsigned int* volatile ar   = (unsigned int*)DVI_BASE_ADDRESS;
  unsigned int* volatile cr   = (unsigned int*)DVI_BASE_ADDRESS+1;
  unsigned int* volatile iesr = (unsigned int*)DVI_BASE_ADDRESS+2;
  unsigned int* volatile ccr  = (unsigned int*)DVI_BASE_ADDRESS+3;

  printf("Writing to AR\n");
  *ar = 0xDEADBEEF;
  printf("Reading from AR: 0x%08x\n", *ar);

  printf("Writing to CR\n");
  *cr = 0xBA5EBA11;
  printf("Reading from CR: 0x%08x\n", *cr);

  printf("Writing to IESR\n");
  *iesr = 0xC001D00D;
  printf("Reading from IESR: 0x%08x\n", *iesr);

  printf("Writing to CCR\n");
  *ccr = 0x1BADBABE;
  printf("Reading from AR: 0x%08x\n", *ccr);

  return 0;
}

