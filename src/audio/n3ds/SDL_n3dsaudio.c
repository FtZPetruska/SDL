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

/* Stub of the dummy audio driver */

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_n3dsaudio.h"

static int
N3DSAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    return 0;                   /* always succeeds. */
}

static int
N3DSAUDIO_CaptureFromDevice(_THIS, void *buffer, int buflen)
{
    /* Delay to make this sort of simulate real audio input. */
    SDL_Delay((this->spec.samples * 1000) / this->spec.freq);

    /* always return a full buffer of silence. */
    SDL_memset(buffer, this->spec.silence, buflen);
    return buflen;
}

static int
N3DSAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Set the function pointers */
    impl->OpenDevice = N3DSAUDIO_OpenDevice;
    impl->CaptureFromDevice = N3DSAUDIO_CaptureFromDevice;

    impl->OnlyHasDefaultOutputDevice = 1;
    impl->OnlyHasDefaultCaptureDevice = 1;
    impl->HasCaptureSupport = SDL_TRUE;

    return 1;   /* this audio target is available. */
}

AudioBootStrap N3DSAUDIO_bootstrap = {
    "N3DS", "SDL N3DS audio driver", N3DSAUDIO_Init, 1
};

/* vi: set ts=4 sw=4 expandtab: */
