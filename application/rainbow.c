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

inline void setPixel(unsigned char* vmem, unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b) {
  unsigned int* pixel = (unsigned int*)(vmem + (DVI_VMEM_SCANLINE_BYTES*y + x*DVI_VMEM_BYTES_PER_PIXEL));
  *pixel = (r << 16) | (g << 8) | b;
}

typedef enum state_e { redUp, redDown, greenUp, greenDown, blueUp, blueDown } state_e;

void advanceRainbow(state_e* state, unsigned char* r, unsigned char* g, unsigned char* b) {
  switch( *state ) {
    case redUp :
      if( *r != 255  )
        (*r)++;
      else
        *state = greenDown;
      break;
    case greenUp :
      if( *g != 255 )
        (*g)++;
      else
        *state = blueDown;
      break;
    case blueUp :
      if( *b != 255 )
        (*b)++;
      else
        *state = redDown;
      break;
    case redDown :
      if( *r )
        (*r)--;
      else
        *state = greenUp;
      break;
    case greenDown :
      if( *g )
        (*g)--;
      else
        *state = blueUp;
      break;
    case blueDown :
      if( *b )
        (*b)--;
      else
        *state = redUp;
      break;
  }
}

//100 MIPS = 100 kIPms = 100 IPµs
void usleep(unsigned int us) {
  while( us ) {
    for(volatile unsigned char i = 0; i < 10; i++); //this loop takes ~100 instructions
    us--;
  }
}

int main() {
  unsigned char* vmem = (unsigned char*)DVI_VMEM_ADDRESS;
  unsigned int vmemSize = DVI_VMEM_SIZE;

  state_e rainbowState1 = blueDown, rainbowState2, rainbowState3;
  unsigned char r1 = 0, g1 = 0xff, b1 = 0xff, r2, g2, b2, r3, g3, b3;

  /*for( unsigned int y = 0; y < 480; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(vmem, x, y, 0, 0, 255);
  while( 1 );*/

  int frames = 0;
  while( frames < 1000 ) {
    r2 = r1; g2 = g1; b2 = b1;
    rainbowState2 = rainbowState1;
    advanceRainbow(&rainbowState1, &r1, &g1, &b1);
    advanceRainbow(&rainbowState1, &r1, &g1, &b1);
    advanceRainbow(&rainbowState1, &r1, &g1, &b1);
    for( unsigned short y = 0; y < 480; y++ ) {
      advanceRainbow(&rainbowState2, &r2, &g2, &b2);
      advanceRainbow(&rainbowState2, &r2, &g2, &b2);
      advanceRainbow(&rainbowState2, &r2, &g2, &b2);
      r3 = r2; g3 = g2; b3 = b2;
      rainbowState3 = rainbowState2;
      for( unsigned short x = 0; x < 640; x+=1 ) {
        //setPixel(vmem, x, y, r3, g3, b3);
        *((unsigned int*)(vmem + (DVI_VMEM_SCANLINE_BYTES*y + x*DVI_VMEM_BYTES_PER_PIXEL))) = (b3 | g3 << 8 | r3 << 16) & 0x00FDFDFD;
        advanceRainbow(&rainbowState3, &r3, &g3, &b3);
      }
    }
    //printf("TFT_APP one full frame painted\n");
    //usleep(10000);
    frames++;
  }
  printf("1000 frames drawn, exit\n");

  return 0;
}
