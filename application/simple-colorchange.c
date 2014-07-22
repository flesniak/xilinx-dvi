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

void setPixel(unsigned char* vmem, unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b) {
  unsigned char* pixel = vmem + (DVI_VMEM_SCANLINE*y + x*DVI_VMEM_BYTES_PER_PIXEL);
  pixel[1] = r>>2;
  pixel[2] = g>>2;
  pixel[3] = b>>2;
}

//100 MIPS = 100 kIPms = 100 IPµs
void usleep(unsigned int us) {
  while( us ) {
    for(volatile unsigned char i = 0; i < 40; i++); //this loop takes ~100 instructions
    us--;
  }
}

int main() {
  unsigned char* vmem = (unsigned char*)DVI_VMEM_ADDRESS;

  printf("Startup.\n");
  while( 1 ) {
    printf("Filling the screen red.\n");
    for( unsigned int y = 0; y < 480; y++ )
      for( unsigned int x = 0; x < 640; x++ )
        setPixel(vmem, x, y, 255, 0, 0);
    usleep(1000000); //should be ~1 sec

    printf("Filling the screen green.\n");
    for( unsigned int y = 0; y < 480; y++ )
      for( unsigned int x = 0; x < 640; x++ )
        setPixel(vmem, x, y, 0, 255, 0);
    usleep(1000000); //should be ~1 sec

    printf("Filling the screen blue.\n");
    for( unsigned int y = 0; y < 480; y++ )
      for( unsigned int x = 0; x < 640; x++ )
        setPixel(vmem, x, y, 0, 0, 255);
    usleep(1000000); //should be ~1 sec
  }

  return 0;
}

