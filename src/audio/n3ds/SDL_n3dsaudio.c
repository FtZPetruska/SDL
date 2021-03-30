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

/* N3DS Audio driver */

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audio_c.h"
#include "SDL_n3dsaudio.h"
#include <unistd.h>

#define N3DSAUDIO_DRIVER_NAME         "n3ds"

static dspHookCookie dsp_hook;
static SDL_AudioDevice* audio_device;

static inline void 
contextLock(_THIS)
{
	LightLock_Lock(&this->hidden->lock);
}

static inline void 
contextUnlock(_THIS)
{
	LightLock_Unlock(&this->hidden->lock);
}

static void 
N3DSAUD_LockAudio(_THIS)
{
    contextLock(this);
}

static void 
N3DSAUD_UnlockAudio(_THIS)
{
	contextUnlock(this);
}

static void
N3DSAUD_DspHook(DSP_HookType hook)
{
    if (hook == DSPHOOK_ONCANCEL)
	{
		contextLock(audio_device);
		audio_device->hidden->isCancelled = true;
		SDL_AtomicSet(&audio_device->enabled, false);
		CondVar_Broadcast(&audio_device->hidden->cv);
		contextUnlock(audio_device);
	}
}

static void 
audioFrameFinished(void * context)
{
    SDL_AudioDevice *this = (SDL_AudioDevice *) context;

	contextLock(this);

	bool shouldBroadcast = false;
	unsigned i;
	for (i = 0; i < NUM_BUFFERS; i ++) {
		if (this->hidden->waveBuf[i].status == NDSP_WBUF_DONE) {
			this->hidden->waveBuf[i].status = NDSP_WBUF_FREE;
			shouldBroadcast = true;
		}
	}

	if (shouldBroadcast)
		CondVar_Broadcast(&this->hidden->cv);

	contextUnlock(this);
}

static int
N3DSAUDIO_OpenDevice(_THIS, void *handle, const char *devname, int iscapture)
{
    this->hidden = (struct SDL_PrivateAudioData *)
        SDL_malloc(sizeof(*this->hidden));
    if (this->hidden == NULL) {
        return SDL_OutOfMemory();
    }
    //start 3ds DSP init
	Result res_code = ndspInit();
	if (R_FAILED(res_code)) {
		SDL_free(this);
		if ((R_SUMMARY(res_code) == RS_NOTFOUND) && (R_MODULE(res_code) == RM_DSP))
			SDL_SetError("DSP init failed: dspfirm.cdc missing!");
		else
			SDL_SetError("DSP init failed. Error code: 0x%lX", res_code);
		return(0);
	}

    /* Initialize internal state */
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));
	LightLock_Init(&this->hidden->lock);
	CondVar_Init(&this->hidden->cv);
    
    if (this->hidden->isCancelled) {
		SDL_SetError("DSP is in cancelled state");
		return (-1);
	}

	if(this->spec.channels > 2)
		this->spec.channels = 2;

    Uint16 test_format = SDL_FirstAudioFormat(this->spec.format);
    int valid_datatype = 0;
    while ((!valid_datatype) && (test_format)) {
        this->spec.format = test_format;
        switch (test_format) {
			case AUDIO_S8:
				/* Signed 8-bit audio supported */
				this->hidden->format=(this->spec.channels==2)?NDSP_FORMAT_STEREO_PCM8:NDSP_FORMAT_MONO_PCM8;
				this->hidden->isSigned=1;
				this->hidden->bytePerSample = (this->spec.channels);
				   valid_datatype = 1;
				break;
			case AUDIO_S16:
				/* Signed 16-bit audio supported */
				this->hidden->format=(this->spec.channels==2)?NDSP_FORMAT_STEREO_PCM16:NDSP_FORMAT_MONO_PCM16;
				this->hidden->isSigned=1;
				this->hidden->bytePerSample = (this->spec.channels) * 2;
				   valid_datatype = 1;
				break;
			default:
				test_format = SDL_NextAudioFormat();
				break;
		}
	}

    if (!valid_datatype) {  /* shouldn't happen, but just in case... */
        SDL_SetError("Unsupported audio format");
        return (-1);
    }

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(&(this->spec));

	/* Allocate mixing buffer */
	if (this->spec.size >= UINT32_MAX/2)
		return(-1);
	this->hidden->mixlen = this->spec.size;
	this->hidden->mixbuf = (Uint8 *) SDL_malloc(this->spec.size);
	if ( this->hidden->mixbuf == NULL ) {
		return(-1);
	}
	SDL_memset(this->hidden->mixbuf, this->spec.silence, this->spec.size);

	Uint8 * temp = (Uint8 *) linearAlloc(this->hidden->mixlen*NUM_BUFFERS);
	if (temp == NULL ) {
		SDL_free(this->hidden->mixbuf);
		return(-1);
	}
	memset(temp,0,this->hidden->mixlen*NUM_BUFFERS);
	DSP_FlushDataCache(temp,this->hidden->mixlen*NUM_BUFFERS);

	this->hidden->nextbuf = 0;
	this->hidden->channels = this->spec.channels;
	this->hidden->samplerate = this->spec.freq;

 	ndspChnReset(0);

	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, this->spec.freq);
	ndspChnSetFormat(0, this->hidden->format);

	float mix[12];
	memset(mix, 0, sizeof(mix));
	mix[0] = 1.0;
	mix[1] = 1.0;
	ndspChnSetMix(0, mix);

	memset(this->hidden->waveBuf,0,sizeof(ndspWaveBuf)*NUM_BUFFERS);

	unsigned i;
	for (i = 0; i < NUM_BUFFERS; i ++) {
		this->hidden->waveBuf[i].data_vaddr = temp;
		this->hidden->waveBuf[i].nsamples = this->hidden->mixlen / this->hidden->bytePerSample;
		temp += this->hidden->mixlen;
	}

	/* Setup callback */
    audio_device = this;
	ndspSetCallback(audioFrameFinished, this);
	dspHook(&dsp_hook, N3DSAUD_DspHook);

	/* We're ready to rock and roll. :-) */
    return 0;
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

static void
N3DSAUDIO_PlayDevice(_THIS) 
{
    contextLock(this);

	size_t nextbuf = this->hidden->nextbuf;
	size_t sampleLen = this->hidden->mixlen;

	if (this->hidden->isCancelled || this->hidden->waveBuf[nextbuf].status != NDSP_WBUF_FREE)
	{
		contextUnlock(this);
		return;
	}

	this->hidden->nextbuf = (nextbuf + 1) % NUM_BUFFERS;

	contextUnlock(this);

	memcpy((void*)this->hidden->waveBuf[nextbuf].data_vaddr,this->hidden->mixbuf,sampleLen);
	DSP_FlushDataCache(this->hidden->waveBuf[nextbuf].data_vaddr,sampleLen);

	ndspChnWaveBufAdd(0, &this->hidden->waveBuf[nextbuf]);
}

static void 
N3DSAUDIO_WaitDevice(_THIS)
{
    contextLock(this);
	while (!this->hidden->isCancelled && this->hidden->waveBuf[this->hidden->nextbuf].status != NDSP_WBUF_FREE) {
		CondVar_Wait(&this->hidden->cv, &this->hidden->lock);
	}
	contextUnlock(this);
}

static Uint8*
N3DSAUDIO_GetDeviceBuf(_THIS)
{
    return(this->hidden->mixbuf);
}

static void
N3DSAUDIO_CloseDevice(_THIS)
{
    contextLock(this);

	dspUnhook(&dsp_hook);
	ndspSetCallback(NULL, NULL);
	if (!this->hidden->isCancelled)
		ndspChnReset(0);

	if ( this->hidden->mixbuf != NULL ) {
		SDL_free(this->hidden->mixbuf);
		this->hidden->mixbuf = NULL;
	}
	if ( this->hidden->waveBuf[0].data_vaddr!= NULL ) {
		linearFree((void*)this->hidden->waveBuf[0].data_vaddr);
		this->hidden->waveBuf[0].data_vaddr = NULL;
	}

	if (!this->hidden->isCancelled) {
		memset(this->hidden->waveBuf,0,sizeof(ndspWaveBuf)*NUM_BUFFERS);
		CondVar_Broadcast(&this->hidden->cv);
	}

	contextUnlock(this);

	ndspExit();

	SDL_free(this->hidden);
	SDL_free(this);
}

static void
N3DSAUDIO_ThreadInit(_THIS)
{
    Thread current_thread = threadGetCurrent();
    Handle thread_handle;
    s32 current_priority;
    if(current_thread){
        thread_handle = threadGetHandle(current_thread);
    }
    else{
        thread_handle = CUR_THREAD_HANDLE;
    }
    svcGetThreadPriority(&current_priority, thread_handle);

    if(current_priority>0x19) current_priority--;
	else current_priority = 0x19; //priority 0x18 is for video thread that is activated by a signal and than must run at maximum priority to avoid flickering
	if(current_priority>0x2F) current_priority = 0x2F;

    svcSetThreadPriority(thread_handle, current_priority);
}

static int
N3DSAUDIO_Init(SDL_AudioDriverImpl * impl)
{
    /* Set the function pointers */
    impl->OpenDevice = N3DSAUDIO_OpenDevice;
    impl->PlayDevice = N3DSAUDIO_PlayDevice;
    impl->WaitDevice = N3DSAUDIO_WaitDevice;
    impl->GetDeviceBuf = N3DSAUDIO_GetDeviceBuf;
    impl->CloseDevice = N3DSAUDIO_CloseDevice;
    impl->ThreadInit = N3DSAUDIO_ThreadInit;
    impl->LockDevice = N3DSAUD_LockAudio;
    impl->UnlockDevice = N3DSAUD_UnlockAudio;

    impl->OnlyHasDefaultOutputDevice = 1;

    /* We lack a working implementation of threads */
    impl->ProvidesOwnCallbackThread = true;
    impl->SkipMixerLock = true;

    /* Should be possible with the 3DS's microphone */
    //impl->HasCaptureSupport = 1;
    //impl->CaptureFromDevice = N3DSAUDIO_CaptureFromDevice;

    return 1;   /* this audio target is available. */
}

AudioBootStrap N3DSAUDIO_bootstrap = {
    N3DSAUDIO_DRIVER_NAME, "SDL N3DS audio driver", N3DSAUDIO_Init, 0
};

/* vi: set ts=4 sw=4 expandtab: */
