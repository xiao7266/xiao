#include <stdio.h>
#include <stdlib.h>
//#include <SDL.h>
#include "zenmalloc.h"
#include "wrt_log.h"
#include "wrt_audio.h"

int processAudioRecord(void* pvoid)
{
	return 0;
}

int processAudioPlay(void* pvoid)
{
	
}

int  main()
{
//	OpenAudio(TYPE_CAPTURE,2,16000,16,768);
	OpenAudio(TYPE_PLAY,2,16000,16,0);
	
//	SDL_Delay(5000);
	usleep(5000);
	//StopAudio(TYPE_CAPTURE);
	StopAudio(TYPE_PLAY);
	return 1;
}
