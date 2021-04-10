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

#ifdef SDL_FILESYSTEM_N3DS

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* System dependent filesystem routines                                */

#include "SDL_error.h"
#include "SDL_filesystem.h"
#include <3ds.h>
#include <sys/stat.h>

char *
SDL_GetBasePath(void)
{
    const char *basepath = "romfs:/";
    char *retval = SDL_strdup(basepath);
    return retval;
}

char *
SDL_GetPrefPath(const char *org, const char *app)
{
    if (!app)
    {
        SDL_InvalidParamError("app");
        return NULL;
    }

    fsInit();
    FS_Archive sdmc_archive;
    FSUSER_OpenArchive(&sdmc_archive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));

    struct stat file_stat;
    int stat_code;
    const char *base_folder = "/3ds/";
    char *intermediate_result = NULL;
    char *return_value = NULL;
    size_t length = SDL_strlen(base_folder) + 1;

    stat_code = stat(base_folder, &file_stat);
    if (stat_code == -1 || S_ISDIR(file_stat.st_mode))
    {
        FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_ASCII, base_folder), 0);
    }

    char *additional_sep = "/";
    if (!org)
    {
        org = "";
        additional_sep = "";
    }
    length += SDL_strlen(org) + 1;
    intermediate_result = (char *)SDL_malloc(length);
    if (!intermediate_result)
    {
        SDL_Log("Couldn't malloc an intermediate string of size %u\n", length);
        SDL_OutOfMemory();
        return NULL;
    }
    SDL_snprintf(intermediate_result, length, "%s%s%s", base_folder, org, additional_sep);
    stat_code = stat(intermediate_result, &file_stat);
    if (stat_code == -1 || S_ISDIR(file_stat.st_mode))
    {
        FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_ASCII, intermediate_result), 0);
    }

    length += SDL_strlen(app) + 1;
    return_value = (char *)SDL_malloc(length);
    if (!return_value)
    {
        SDL_Log("Couldn't malloc a result string of size %u\n", length);
        SDL_OutOfMemory();
        return NULL;
    }
    SDL_snprintf(return_value, length, "%s%s/", intermediate_result, app);
    stat_code = stat(return_value, &file_stat);
    if (stat_code == -1 || S_ISDIR(file_stat.st_mode))
    {
        FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_ASCII, return_value), 0);
    }
    
    SDL_free(intermediate_result);

    FSUSER_CloseArchive(sdmc_archive);
    fsExit();

    return return_value;
}

#endif /* SDL_FILESYSTEM_N3DS */

/* vi: set ts=4 sw=4 expandtab: */
