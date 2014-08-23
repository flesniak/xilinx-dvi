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

#define DELAY 2000000

void setPixel(unsigned char* vmem, unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b) {
  unsigned char* pixel = vmem + (DVI_VMEM_SCANLINE_BYTES*y + x*DVI_VMEM_BYTES_PER_PIXEL);
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
    for(volatile unsigned char i = 0; i < 80; i++); //this loop takes ~100 instructions
    us--;
  }
}

int main() {
  unsigned char* vmem = (unsigned char*)DVI_VMEM_ADDRESS;
  unsigned int* volatile cr = (unsigned int*)(DVI_CR_ADDR);

  printf("Startup.\n");
  printf("CR address: 0x%08x\n", (unsigned int)cr);
  printf("Reading from CR: 0x%08x\n", *cr);

  printf("Filling upper screen white, lower screen black\n");
  for( unsigned int y = 0; y < 240; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, 0xff, 0xff, 0xff);
  for( unsigned int y = 240; y < 480; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, 0, 0, 0);
  usleep(DELAY);

  printf("Clearing DVI_CR_EN_MASK in CR\n");
  *cr &= ~DVI_CR_EN_MASK;
  printf("Reading from CR: 0x%08x\n", *cr);
  usleep(DELAY);

  printf("Filling screen red, you should NOT SEE ANY CHANGES (display is disabled)\n");
  fillScreen(vmem, 0xff, 0, 0);
  usleep(DELAY);

  printf("Setting DVI_CR_EN_MASK in CR, you should then see the red screen\n");
  *cr |= DVI_CR_EN_MASK;
  printf("Reading from CR: 0x%08x\n", *cr);
  usleep(DELAY);

  printf("Writing DVI_CR_DSC_MASK to CR (inversing scan direction)\n");
  *cr |= DVI_CR_DSC_MASK;
  printf("Reading from CR: 0x%08x\n", *cr);
  usleep(DELAY);

  printf("Filling upper screen white, lower screen black (you should see the opposite)\n");
  for( unsigned int y = 0; y < 240; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, 0xff, 0xff, 0xff);
  for( unsigned int y = 240; y < 480; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, 0, 0, 0);
  usleep(DELAY);

  printf("Clearing DVI_CR_DSC_MASK in CR (restoring scan direction), you should see upper white, lower black again\n");
  *cr &= ~DVI_CR_DSC_MASK;
  printf("Reading from CR: 0x%08x\n", *cr);
  usleep(DELAY);

  return 0;
}
