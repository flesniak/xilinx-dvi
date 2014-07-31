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

#define REMAP_TO 0xDEADBEEF
#define WAIT_PER_TEST 2

void setPixel(unsigned char* vmem, unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b) {
  unsigned char* pixel = vmem + (DVI_VMEM_SCANLINE*y + x*DVI_VMEM_BYTES_PER_PIXEL);
  pixel[1] = r>>2;
  pixel[2] = g>>2;
  pixel[3] = b>>2;
}

void fillScreen(unsigned char* vmem, unsigned char r, unsigned char g, unsigned char b) {
  for( unsigned int y = 0; y < 480; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, r, g, b);
}

//100 MIPS = 100 kIPms = 100 IPµs
void usleep(unsigned int us) {
  while( us ) {
    for(volatile unsigned char i = 0; i < 45; i++); //this loop takes ~100 instructions
    us--;
  }
}

void waitVsync() {
  unsigned int* volatile iesr = (unsigned int*)DVI_IESR_ADDR;
  while( *iesr & DVI_IESR_STATUS_MASK );
  printf("Vsync ready\n");
}

int main() {
  unsigned char* vmem = (unsigned char*)DVI_VMEM_ADDRESS;
  volatile unsigned int* volatile iesr = (unsigned int*)DVI_IESR_ADDR;

  printf("Startup. Waiting for first vsync.\n");
  waitVsync();

  printf("Filling the screen red and waiting for vsync.\n");
  fillScreen(vmem, 0xff, 0, 0);
  waitVsync();

  printf("Filling the screen green and waiting for vsync.\n");
  fillScreen(vmem, 0, 0xff, 0);
  waitVsync();

  printf("Test done, looping.\n");
  unsigned int n = 0;
  while( 1 ) {
    unsigned int reg = *iesr;
    if( reg & DVI_IESR_STATUS_MASK ) {
      printf("vsync flag active after %d tries (0x%08x)\n", n, reg);
      n = 0;
      while( *iesr & DVI_IESR_STATUS_MASK );
    } else n++;
  }

  return 0;
}

