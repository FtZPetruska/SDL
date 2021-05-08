/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#ifdef SDL_VIDEO_DRIVER_N3DS

#include <citro3d.h>

#include "../../events/SDL_events_c.h"
#include "../SDL_pixels_c.h"
#include "../SDL_sysvideo.h"
#include "SDL_mouse.h"
#include "SDL_n3dsevents_c.h"
#include "SDL_n3dsframebuffer_c.h"
#include "SDL_n3dsvideo.h"
#include "SDL_video.h"

#define N3DSVID_DRIVER_NAME "n3ds"

typedef struct {
  float dpi;
} SDL_DisplayData;

static int N3DS_VideoInit(_THIS);
static int N3DS_SetDisplayMode(_THIS, SDL_VideoDisplay *display,
                               SDL_DisplayMode *mode);
static void N3DS_VideoQuit(_THIS);
static int N3DS_GetDisplayDPI(_THIS, SDL_VideoDisplay *sdl_display, float *ddpi,
                              float *hdpi, float *vdpi);

/* N3DS driver bootstrap functions */

static int N3DS_Available(void) {
  return (1);  // Always available
}

static void N3DS_DeleteDevice(SDL_VideoDevice *device) {
  SDL_free(device->displays);
  SDL_free(device->driverdata);
  SDL_free(device);
}

static SDL_VideoDevice *N3DS_CreateDevice(int devindex) {
  SDL_VideoDevice *device;

  if (!N3DS_Available()) {
    return (0);
  }

  /* Initialize all variables that we clean on shutdown */
  device = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
  if (!device) {
    SDL_OutOfMemory();
    return (0);
  }

  /* Set the function pointers */
  device->VideoInit = N3DS_VideoInit;
  device->VideoQuit = N3DS_VideoQuit;
  device->SetDisplayMode = N3DS_SetDisplayMode;
  device->PumpEvents = N3DS_PumpEvents;
  device->CreateWindowFramebuffer = SDL_N3DS_CreateWindowFramebuffer;
  device->UpdateWindowFramebuffer = SDL_N3DS_UpdateWindowFramebuffer;
  device->DestroyWindowFramebuffer = SDL_N3DS_DestroyWindowFramebuffer;

  device->GetDisplayDPI = N3DS_GetDisplayDPI;

  device->free = N3DS_DeleteDevice;

  return device;
}

VideoBootStrap N3DS_bootstrap = {N3DSVID_DRIVER_NAME, "N3DS Video Driver",
                                 N3DS_CreateDevice};

int N3DS_VideoInit(_THIS) {
  gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false);
  gfxSet3D(false);
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

  SDL_VideoDisplay top_display;
  SDL_VideoDisplay bottom_display;

  SDL_DisplayMode top_mode;
  SDL_DisplayMode bottom_mode;

  SDL_zero(top_display);
  SDL_zero(bottom_display);
  SDL_zero(top_mode);
  SDL_zero(bottom_mode);

  SDL_DisplayData *top_data =
      (SDL_DisplayData *)SDL_calloc(1, sizeof(SDL_DisplayData));
  if (!top_data) {
    return SDL_OutOfMemory();
  }

  SDL_DisplayData *bottom_data =
      (SDL_DisplayData *)SDL_calloc(1, sizeof(SDL_DisplayData));
  if (!bottom_data) {
    SDL_free(top_data);
    return SDL_OutOfMemory();
  }

  cfguInit();
  u8 n3ds_model;
  CFGU_GetSystemModel(&n3ds_model);
  switch (n3ds_model) {
    case (CFG_MODEL_3DS):
    case (CFG_MODEL_2DS):
      top_data->dpi = 132.1f;
      bottom_data->dpi = 132.4f;
      break;
    case (CFG_MODEL_N3DS):
      top_data->dpi = 120.2f;
      bottom_data->dpi = 120.1f;
      break;
    case (CFG_MODEL_3DSXL):
    case (CFG_MODEL_N3DSXL):
    case (CFG_MODEL_N2DSXL):
      top_data->dpi = 95.5f;
      bottom_data->dpi = 95.6f;
      break;
  }
  cfguExit();

  top_mode.w = GSP_SCREEN_HEIGHT_TOP;
  top_mode.h = GSP_SCREEN_WIDTH;
  top_mode.refresh_rate = 60;
  top_mode.format = SDL_PIXELFORMAT_RGBA8888;
  top_mode.driverdata = NULL;

  top_display.desktop_mode = top_mode;
  top_display.current_mode = top_mode;
  top_display.driverdata = (void *)top_data;

  bottom_mode.w = GSP_SCREEN_HEIGHT_BOTTOM;
  bottom_mode.h = GSP_SCREEN_WIDTH;
  bottom_mode.refresh_rate = 60;
  bottom_mode.format = SDL_PIXELFORMAT_RGBA8888;
  bottom_mode.driverdata = NULL;

  bottom_display.desktop_mode = bottom_mode;
  bottom_display.current_mode = bottom_mode;
  bottom_display.driverdata = (void *)bottom_data;

  SDL_AddVideoDisplay(&top_display, SDL_FALSE);
  SDL_AddVideoDisplay(&bottom_display, SDL_FALSE);

  /* We're done! */
  return 0;
}

static int N3DS_SetDisplayMode(_THIS, SDL_VideoDisplay *display,
                               SDL_DisplayMode *mode) {
  return 0;
}

void N3DS_VideoQuit(_THIS) {
  C3D_Fini();
  gfxExit();
}

static int N3DS_GetDisplayDPI(_THIS, SDL_VideoDisplay *sdl_display, float *ddpi,
                              float *hdpi, float *vdpi) {
  SDL_DisplayData *data = (SDL_DisplayData *)sdl_display->driverdata;

  if (ddpi) {
    *ddpi = data->dpi;
  }
  if (hdpi) {
    *hdpi = data->dpi;
  }
  if (vdpi) {
    *vdpi = data->dpi;
  }

  return data->dpi != 0.0f ? 0 : SDL_SetError("Couldn't get DPI");
}

#endif /* SDL_VIDEO_DRIVER_N3DS */

/* clang-format -style=Google */
