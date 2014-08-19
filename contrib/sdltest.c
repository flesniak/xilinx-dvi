#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <SDL.h>

void surfDump(SDL_Surface* surf, unsigned int n) {
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

int main(int argc, char** argv) {
  if( SDL_Init(SDL_INIT_VIDEO) ) {
    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Surface* surf1 = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
  if( !surf1 ) {
    printf("Couldn't initialize surface: %s\n", SDL_GetError());
    return 1;
  }

  char buf[16];
  if( !SDL_VideoDriverName(buf, 16) ) {
    printf("Couldn't get video driver name\n");
    return 1;
  }
  printf("SDL_VideoDriver: %s\n", buf);

  printf("Filling surf1 and updating. Display should turn blue now\n");
  SDL_FillRect(surf1, 0, SDL_MapRGB(surf1->format, 0, 0, 0xff));
  printf("PixelFormat of surf1:\n");
  dumpFormat(surf1->format);
  surfDump(surf1, 16);
  SDL_UpdateRect(surf1, 0, 0, 0, 0);
  usleep(300000);

  SDL_Surface* surf2 = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32, 0x003f0000, 0x00003f00, 0x0000003f, 0);
  if( !surf2 ) {
    printf("Couldn't initialize tft surface: %s\n", SDL_GetError());
    return 1;
  }
  printf("Filling surf2, blitting to surf1 and updating. Display should turn red now\n");
  SDL_FillRect(surf2, 0, SDL_MapRGB(surf2->format, 0xff, 0, 0));
  int err;
  if( (err = SDL_BlitSurface(surf2, 0, surf1, 0)) ) {
    printf("Surface blit failed (%d)", err);
    return 1;
  }
  printf("PixelFormat of surf2:\n");
  dumpFormat(surf2->format);
  surfDump(surf2, 16);
  surfDump(surf1, 16);
  SDL_UpdateRect(surf1, 0, 0, 0, 0);
  usleep(300000);

  printf("Filling surf2 manually, blitting to surf1 and updating. Display should turn green now\n");
  for( unsigned int y = 0; y < 480; y++ )
    for( unsigned int x = 0; x < 640; x++ )
      setPixel(surf2, x, y, 0, 255, 0);
  if( (err = SDL_BlitSurface(surf2, 0, surf1, 0)) ) {
    printf("Surface blit failed (%d)", err);
    return 1;
  }
  surfDump(surf2, 16);
  surfDump(surf1, 16);
  SDL_UpdateRect(surf1, 0, 0, 0, 0);
  usleep(300000);

  printf("Trying to paint a rainbow!\n");
  state_e ff_rainbowState = blueDown, f_rainbowState, rainbowState;
  unsigned char ff_r = 0, ff_g = 0xff, ff_b = 0xff, f_r, f_g, f_b, r, g, b;

  SDL_Event event;
  bool run = true;
  while( run ) {
    f_r = ff_r; f_g = ff_g; f_b = ff_b;
    f_rainbowState = ff_rainbowState;
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    advanceRainbow(&ff_rainbowState, &ff_r, &ff_g, &ff_b);
    for( unsigned short y = 0; y < 480; y++ ) {
//       advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
//       advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
//       advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
//       advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      advanceRainbow(&f_rainbowState, &f_r, &f_g, &f_b);
      r = f_r; g = f_g; b = f_b;
      rainbowState = f_rainbowState;
      for( unsigned short x = 0; x < 640; x+=1 ) {
        setPixel(surf1, x,   y, r, g, b);
//         setPixel(surf1, x+1, y, r, g, b);
//         setPixel(surf1, x+2, y, r, g, b);
//         setPixel(surf1, x+3, y, r, g, b);
//         setPixel(surf1, x+4, y, r, g, b);
        advanceRainbow(&rainbowState, &r, &g, &b);
      }
    }
    SDL_UpdateRect(surf1, 0, 0, 0, 0);
    usleep(10000);
    while( SDL_PollEvent(&event) ) {
      switch( event.type ) {
        case SDL_QUIT:
          printf("Quit.\n");
          run = false;
          break;
        default:
          printf("Unhandled SDL_Event\n");
      }
    }
  }

  SDL_FreeSurface(surf1);
  SDL_FreeSurface(surf2);
  SDL_Quit();
  return 0;
}
