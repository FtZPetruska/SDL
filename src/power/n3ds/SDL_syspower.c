/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2015 Sam Lantinga <slouken@libsdl.org>

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

#ifndef SDL_POWER_DISABLED
#if SDL_POWER_N3DS

#include "SDL_power.h"
#include <3ds.h>


SDL_bool
SDL_GetPowerInfo_N3DS(SDL_PowerState * state, int *seconds,
                            int *percent)
{
    int battery = 1;
    bool plugged;
    u8 charging;

    ptmuInit();
    PTMU_GetAdapterState(&plugged);
    PTMU_GetBatteryChargeState(&charging);
    ptmuExit();

    *state = SDL_POWERSTATE_UNKNOWN;
    *seconds = -1; // libctru doesn't provide a way to estimate battery life
    *percent = -1;
    u8 battery_level;
    mcuHwcInit();
    if (!battery) {
        *state = SDL_POWERSTATE_NO_BATTERY;
        *percent = -1;
    } else if (charging) {
        *state = SDL_POWERSTATE_CHARGING;
        MCUHWC_GetBatteryLevel(&battery_level);
        *percent = (int)battery_level;
    } else if (plugged) {
        *state = SDL_POWERSTATE_CHARGED;
        MCUHWC_GetBatteryLevel(&battery_level);
        *percent = (int)battery_level;
    } else {
        *state = SDL_POWERSTATE_ON_BATTERY;
        MCUHWC_GetBatteryLevel(&battery_level);
        *percent = (int)battery_level;
    }
    mcuHwcExit();
    
    return SDL_TRUE;            /* always the definitive answer on N3DS. */
}

#endif /* SDL_POWER_N3DS */
#endif /* SDL_POWER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
