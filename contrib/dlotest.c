#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
  //Initialize libdlo, claim device and get current mode
  dlo_init_t initFlags;
  dlo_retcode_t err = dlo_init(initFlags);
  if( err != dlo_ok ) {
    printf("Failed to initialize DisplayLink library: %s\n", dlo_strerror(err));
    return 1;
  }

  dlo_claim_t claimFlags;
  dlo_dev_t dev = dlo_claim_first_device(claimFlags, 5000); //dont use dlo_claim_default_device because semihost wont get any argc/argv
  if( !dev ) {
    printf("Failed to claim DisplayLink device\n");
    goto end;
  }

  dlo_mode_t* displayMode = dlo_get_mode(dev);
  if( 0 ) { //activate to set display to 640x480
    dlo_mode_t mode = { .view = {.width = 640, .height = 480, .bpp = (*displayMode).view.bpp, .base = (*displayMode).view.base}, .refresh = 0 };
    err = dlo_set_mode(dev, &mode);
    if( err != dlo_ok ) {
      printf("Failed to set mode: %s\n", dlo_strerror(err));
      goto end;
    }
  }

  //fill the screen using native libdlo methods
  printf("filling full screen red (without view/rect)\n");
  err = dlo_fill_rect(dev, 0, 0, DLO_RGB(0xff, 0, 0));
  if( err != dlo_ok ) {
    printf("Failed to fill rect: %s\n", dlo_strerror(err));
    goto end;
  }
  usleep(500000);

  printf("filling screen green (native view width %d height %d bpp %d base 0x%08x, no rect)\n", (*displayMode).view.width, (*displayMode).view.height, (*displayMode).view.bpp, (*displayMode).view.base);
  err = dlo_fill_rect(dev, &(*displayMode).view, 0 , DLO_RGB(0, 0xff, 0));
  if( err != dlo_ok ) {
    printf("Failed to fill rect: %s\n", dlo_strerror(err));
    goto end;
  }
  usleep(500000);

  dlo_rect_t rect = { .origin = { .x = 0, .y = 0 }, .width = 640, .height = 480 };
  printf("filling custom mode blue (native view, rect x %d y %d width %d height %d)\n", rect.origin.x, rect.origin.y, rect.width, rect.height);
  err = dlo_fill_rect(dev, &(*displayMode).view, &rect, DLO_RGB(0, 0, 0xff));
  if( err != dlo_ok ) {
    printf("Failed to fill rect: %s\n", dlo_strerror(err));
    goto end;
  }
  usleep(500000);

  //internal framebuffer and information for dlo_copy_host_bmp
  uint32_t* vmem = calloc(4, 640*480); //initialize vmem for 640x480 32bpp abgr
  dlo_bmpflags_t bmpFlags = {0};
  dlo_fbuf_t fbuf = { .width = 640, .height = 480, .fmt = dlo_pixfmt_abgr8888, .base = vmem, .stride = 640 };

  //fill the screen using dlo_copy_host_bmp and local vmem
  printf("filling the screen red using local vmem\n");
  for( unsigned short y = 0; y < 480; y++ ) {
    for( unsigned short x = 0; x < 640; x+=1 )
      *(vmem+(640*y+x)) = DLO_RGB(0xff, 0, 0);
  }
  dlo_dot_t dot = {640,0};
  err = dlo_copy_host_bmp(dev, bmpFlags, &fbuf, &(*displayMode).view, &dot);
  if( err != dlo_ok ) {
    printf("Failed to copy host bmp: %s\n", dlo_strerror(err));
    goto end;
  }
  sleep(1);

  //draw 45° line
  for( unsigned int i = 0; i < 480; i++ )
      *(vmem+(640*i+i)) = DLO_RGB(0xff, 0xff, 0xff);
  //fill rightmost part black
  for( unsigned int y = 0; y < 480; y++ )
    for( unsigned int x = 480; x < 640; x++ )
      *(vmem+(640*y+x)) = 0;

  err = dlo_copy_host_bmp(dev, bmpFlags, &fbuf, &(*displayMode).view, &dot);

  state_e ff_rainbowState = blueDown, f_rainbowState, rainbowState;
  unsigned char ff_r = 0, ff_g = 0xff, ff_b = 0xff, f_r, f_g, f_b, r, g, b;

  bool run = true;
  time_t startTime = time(0);
  time_t currentTime = startTime;
  time_t newTime = startTime;
  unsigned int frames = 0;
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
        *(vmem+(640*y+x)) = DLO_RGB(r, g, b);
        //*(vmem+(640*y+x)) = (uint32_t)(b << 16 | g << 8 | r);
        advanceRainbow(&rainbowState, &r, &g, &b);
      }
    }
    err = dlo_copy_host_bmp(dev, bmpFlags, &fbuf, &(*displayMode).view, 0);
    if( err != dlo_ok ) {
      printf("Failed to copy host bmp: %s\n", dlo_strerror(err));
      run = false;
      //goto end;
    }
    frames++;
    newTime = time(0);
    if( currentTime != newTime ) {
      float fps = (float)frames/(newTime-startTime);
      printf("\rfps: %#6.2f", fps);
      fflush(stdout);
      currentTime = newTime;
    }
    //usleep(24000);
  }
  putchar('\n');  

end:
  err = dlo_release_device(dev);
  if( err != dlo_ok ) {
    printf("Failed to release DisplayLink device: %s\n", dlo_strerror(err));
    goto end;
  }

  dlo_final_t finalFlags;
  err = dlo_final(finalFlags);
  if( err != dlo_ok ) {
    printf("Failed to free DisplayLink library: %s\n", dlo_strerror(err));
    goto end;
  }

  return 0;
}
