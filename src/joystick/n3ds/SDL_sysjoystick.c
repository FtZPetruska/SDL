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

#if defined(SDL_JOYSTICK_N3DS)

/* This is the dummy implementation of the SDL joystick API */

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"


static int
N3DS_JoystickInit(void)
{
    return 0;
}

static int
N3DS_JoystickGetCount(void)
{
    return 0;
}

static void
N3DS_JoystickDetect(void)
{
}

static const char *
N3DS_JoystickGetDeviceName(int device_index)
{
    return NULL;
}

static int
N3DS_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
N3DS_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID
N3DS_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    SDL_zero(guid);
    return guid;
}

static SDL_JoystickID
N3DS_JoystickGetDeviceInstanceID(int device_index)
{
    return -1;
}

static int
N3DS_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    return SDL_SetError("Logic error: No joysticks available");
}

static int
N3DS_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static int
N3DS_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static SDL_bool
N3DS_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

static int
N3DS_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static int
N3DS_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

static void
N3DS_JoystickUpdate(SDL_Joystick *joystick)
{
}

static void
N3DS_JoystickClose(SDL_Joystick *joystick)
{
}

static void
N3DS_JoystickQuit(void)
{
}

static SDL_bool
N3DS_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    return SDL_FALSE;
}

SDL_JoystickDriver SDL_N3DS_JoystickDriver =
{
    N3DS_JoystickInit,
    N3DS_JoystickGetCount,
    N3DS_JoystickDetect,
    N3DS_JoystickGetDeviceName,
    N3DS_JoystickGetDevicePlayerIndex,
    N3DS_JoystickSetDevicePlayerIndex,
    N3DS_JoystickGetDeviceGUID,
    N3DS_JoystickGetDeviceInstanceID,
    N3DS_JoystickOpen,
    N3DS_JoystickRumble,
    N3DS_JoystickRumbleTriggers,
    N3DS_JoystickHasLED,
    N3DS_JoystickSetLED,
    N3DS_JoystickSetSensorsEnabled,
    N3DS_JoystickUpdate,
    N3DS_JoystickClose,
    N3DS_JoystickQuit,
    N3DS_JoystickGetGamepadMapping
};

#endif /* SDL_JOYSTICK_N3DS */

/* vi: set ts=4 sw=4 expandtab: */
