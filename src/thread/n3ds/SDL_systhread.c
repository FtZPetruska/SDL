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

#ifdef SDL_THREAD_N3DS

/* Thread management routines for SDL */

#include "../SDL_systhread.h"
#include "SDL_thread.h"

static const size_t DEFAULT_STACK_SIZE = (128 * 1024);

static void ThreadEntry(void *arg) { SDL_RunThread((SDL_Thread *)arg); }

#ifdef SDL_PASSED_BEGINTHREAD_ENDTHREAD
int SDL_SYS_CreateThread(SDL_Thread *thread,
                         pfnSDL_CurrentBeginThread pfnBeginThread,
                         pfnSDL_CurrentEndThread pfnEndThread)
#else
int SDL_SYS_CreateThread(SDL_Thread *thread)
#endif /* SDL_PASSED_BEGINTHREAD_ENDTHREAD */
{
  /* Set default priority if svcGetThreadPriority fails */
  s32 priority = 0x30;

  /* Set priority of new thread higher than the current thread */
  svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);

  /* priority 0x18 is for video thread that is activated by a signal and than
   * must run at maximum priority to avoid flickering. On the other hand,
   * priority 0x30 is the default priority for the main thread and the
   * minimum priority, so 0x2F is the minimum priority given.
   **/
  if (priority > 0x19) {
    priority--;
  } else {
    priority = 0x19;
  }
  if (priority > 0x2F) {
    priority = 0x2F;
  }

  size_t stack_size =
      thread->stacksize ? thread->stacksize : DEFAULT_STACK_SIZE;

  thread->handle =
      threadCreate(ThreadEntry, thread, stack_size, priority, -2, false);

  if (!thread->handle) {
    return SDL_SetError("Couldn't create thread");
  }

  return 0;
}

void SDL_SYS_SetupThread(const char *name) { return; }

SDL_threadID SDL_ThreadID(void) {
  u32 thread_ID = 0;
  svcGetThreadId(&thread_ID, CUR_THREAD_HANDLE);
  return (SDL_threadID)thread_ID;
}

int SDL_SYS_SetThreadPriority(SDL_ThreadPriority priority) {
  return (int)svcSetThreadPriority(CUR_THREAD_HANDLE, priority);
}

void SDL_SYS_WaitThread(SDL_Thread *thread) {
  threadJoin(thread->handle, U64_MAX);
}

void SDL_SYS_DetachThread(SDL_Thread *thread) { threadDetach(thread->handle); }

#endif /* SDL_THREAD_N3DS */

/* clang-format -style=Google */
