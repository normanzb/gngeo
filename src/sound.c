/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */  
    
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
//#include "streams.h"
#include "emu.h"
#include "memory.h"
#include "profiler.h"
#ifdef GP2X
#include "ym2610-940/940shared.h"
#endif

SDL_AudioSpec * desired, *obtain;

#define MIXER_MAX_CHANNELS 16
//#define CPU_FPS 60
#define BUFFER_LEN 16384
extern int throttle;
static int audio_sample_rate;
Uint16 play_buffer[BUFFER_LEN];

#ifndef GP2X
#define NB_SAMPLES 512 /* better resolution */
#else
//#define NB_SAMPLES 128
#define NB_SAMPLES 64
//#define NB_SAMPLES 512
#endif

void update_sdl_stream(void *userdata, Uint8 * stream, int len)
{
	static Uint32 play_buffer_pos;
	//printf("sdl %d\n", len);
	PROFILER_START(PROF_SOUND);
	//streamupdate(len);
	
#ifdef ENABLE_940T
	if (shared_ctl->buf_pos >= play_buffer_pos && 
	    shared_ctl->buf_pos <= play_buffer_pos + len) {
		//printf("SOUND WARN 1 %d %d\n",shared_ctl->buf_pos,play_buffer_pos);
		
		return;
	}
	if (shared_ctl->buf_pos + shared_ctl->sample_len >= play_buffer_pos && 
	    shared_ctl->buf_pos + shared_ctl->sample_len <= play_buffer_pos + len) {
		//printf("SOUND WARN 2 %d %d\n",shared_ctl->buf_pos,play_buffer_pos);
		
		return;
	}
	if ( play_buffer_pos+len>SAMPLE_BUFLEN) {
		unsigned int last=(SAMPLE_BUFLEN-play_buffer_pos);
		memcpy(stream, (Uint8 *) shared_ctl->play_buffer+ play_buffer_pos, last);
		memcpy(stream+last, (Uint8 *) shared_ctl->play_buffer, len-last);
		//printf("Case 1\n");
		play_buffer_pos=len-last;
	} else {
		memcpy(stream, (Uint8 *) shared_ctl->play_buffer+play_buffer_pos, len);
		play_buffer_pos+=len;
		//printf("Case 2\n");
	}


#else
	YM2610Update_stream(len/4);
	memcpy(stream, (Uint8 *) play_buffer, len);
#endif
	PROFILER_STOP(PROF_SOUND);

}

int init_sdl_audio(void)
{

    SDL_InitSubSystem(SDL_INIT_AUDIO);

    desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    obtain = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    audio_sample_rate = conf.sample_rate;
    desired->freq = conf.sample_rate;
    desired->samples = NB_SAMPLES;
    
#ifdef WORDS_BIGENDIAN
    desired->format = AUDIO_S16MSB;
#else	/* */
    desired->format = AUDIO_S16;
#endif	/* */
    desired->channels = 2;
    desired->callback = update_sdl_stream;
    desired->userdata = NULL;
    SDL_OpenAudio(desired, NULL);
    //SDL_OpenAudio(desired, obtain);
    //printf("Obtained sample rate: %d\n",obtain->freq);
    //conf.sample_rate=obtain->freq;
    return 1;
}

void close_sdl_audio(void) {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}



