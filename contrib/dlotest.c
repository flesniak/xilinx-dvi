#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <libdlo.h>

/*void surfDump(SDL_Surface* surf, unsigned int n) {
  while( n%4 )
    n++;
  printf("Surface dump first %d bytes:", n);
  for(unsigned int i=0; i<n; i+=4)
    printf(" 0x%02x%02x%02x%02x",
        ((unsigned char*)surf->pixels)[i],
        ((unsigned char*)surf->pixels)[i+1],
        ((unsigned char*)surf->pixels)[i+2],
        ((unsigned char*)surf->pixels)[i+3]);
  printf("\n");
}

void dumpFormat(SDL_PixelFormat* format) {
  printf("BitsPerPixel:  %d\n", format->BitsPerPixel);
  printf("BytesPerPixel: %d\n", format->BytesPerPixel);
  printf("[RGBA]loss:    %d %d %d %d\n", format->Rloss, format->Gloss, format->Bloss, format->Aloss);
  printf("[RGBA]shift:   0x%02x 0x%02x 0x%02x 0x%02x\n", format->Rshift, format->Gshift, format->Bshift, format->Ashift);
  printf("[RGBA]mask:    0x%08x 0x%08x 0x%08x 0x%08x\n", format->Rmask, format->Gmask, format->Bmask, format->Amask);
  printf("colorkey:      %d\n", format->colorkey);
  printf("alpha:         %d\n", format->alpha);
}

void setPixel(SDL_Surface* surf, unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b) {
  unsigned int* pixel = surf->pixels+(surf->pitch*y + x*surf->format->BytesPerPixel);
  *pixel = (r<<(surf->format->Rshift) & surf->format->Rmask)
         | (g<<(surf->format->Gshift) & surf->format->Gmask)
         | (b<<(surf->format->Bshift) & surf->format->Bmask);
}*/

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

int main(int argc, char** argv) {
  unsigned char* vmem = calloc(4, 640*480); //initialize vmem for 640x480 32bpp abgr

  dlo_init_t initFlags;
  dlo_retcode_t err = dlo_init(initFlags);
  if( err != dlo_ok ) {
    printf("Failed to initialize DisplayLink library: %s\n", dlo_strerror(err));
    return 1;
  }

  dlo_claim_t claimFlags;
  dlo_dev_t dev = dlo_claim_first_device(claimFlags, 5000);
  if( !dev ) {
    printf("Failed to claim DisplayLink device\n");
    return 1;
  }

  dlo_mode_t* displayMode = dlo_get_mode(dev);
  dlo_mode_t mode = { .view = {.width = 640, .height = 480, .bpp = 8, .base = (*displayMode).view.base}, .refresh = 50 };
  //err = dlo_set_mode(dev, &mode);
  if( err != dlo_ok ) {
    printf("Failed to set mode: %s\n", dlo_strerror(err));
    return 1;
  }

  dlo_dot_t pos = { .x = 0, .y = 0 };
  dlo_rect_t rect = { .origin = pos, .width = 640, .height = 480 };
  printf("filling screen red x %d y %d width %d height %d\n", rect.origin.x, rect.origin.y, rect.width, rect.height);
  err = dlo_fill_rect(dev, &mode.view, &rect, 0xff0000ff);
  sleep(5);

  state_e ff_rainbowState = blueDown, f_rainbowState, rainbowState;
  unsigned char ff_r = 0, ff_g = 0xff, ff_b = 0xff, f_r, f_g, f_b, r, g, b;

  dlo_bmpflags_t bmpFlags;
  dlo_fbuf_t fbuf = { .width = 640, .height = 480, .fmt = dlo_pixfmt_abgr8888, vmem, 480 };

  /*bool run = true;
  while( run ) {
    f_r = ff_r; f_g = ff_g; f_b = ff_b;
    f_rainbowState = ff_rainbowState;
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    for( unsigned short y = 0; y < 480; y++ ) {
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      r = f_r; g = f_g; b = f_b;
      rainbowState = f_rainbowState;
      for( unsigned short x = 0; x < 640; x+=1 ) {
        *(uint32_t*)(vmem+4*(480*y+x)) = (uint32_t)(b << 16 | g << 8 | r);
        advanceRainbow(&rainbowState, &r, &g, &b);
      }
    }
    err = dlo_copy_host_bmp(dev, bmpFlags, &fbuf, &mode.view, &pos);
    printf("screen dumped\n");
  }*/

  err = dlo_release_device(dev);
  if( err != dlo_ok ) {
    printf("Failed to release DisplayLink device: %s\n", dlo_strerror(err));
    return 1;
  }

  dlo_final_t finalFlags;
  err = dlo_final(finalFlags);
  if( err != dlo_ok ) {
    printf("Failed to free DisplayLink library: %s\n", dlo_strerror(err));
    return 1;
  }

  return 0;
}
