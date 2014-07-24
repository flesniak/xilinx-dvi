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

int main() {
  unsigned char* firstVmem = (unsigned char*)DVI_VMEM_ADDRESS;
  unsigned char* secondVmem = (unsigned char*)REMAP_TO;
  unsigned int* volatile ar = (unsigned int*)DVI_BASE_ADDRESS;

  printf("Startup.\n");
  printf("Reading from AR: 0x%08x\n", *ar);

  printf("Filling the screen red at current vmem address.\n");
  fillScreen(firstVmem, 0xff, 0, 0);
  usleep(WAIT_PER_TEST*1000000);

  printf("Writing to AR (this should remap the bus to 0x%08x)\n", REMAP_TO);
  *ar = REMAP_TO;
  printf("Reading from AR: 0x%08x\n", *ar);
  usleep(WAIT_PER_TEST*1000000);

  printf("Filling the screen green at new vmem address.\n");
  fillScreen(secondVmem, 0, 0xff, 0);
  usleep(WAIT_PER_TEST*1000000);

  printf("Filling the screen blue at old vmem address. NOTHING SHOULD HAPPEN!\n");
  fillScreen(firstVmem, 0, 0, 0xff);
  usleep(WAIT_PER_TEST*1000000);

  printf("Test done, looping.\n");
  while( 1 );

  return 0;
}

