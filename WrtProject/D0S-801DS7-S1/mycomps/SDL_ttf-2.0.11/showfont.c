/*
  showfont:  An example of using the SDL_ttf library with 2D graphics.
  Copyright (C) 2001-2012 Sam Lantinga <slouken@libsdl.org>

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

/* A simple program to test the text rendering feature of the TTF library */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#include "SDL.h"
#include "wrtfont.h"

#define DEFAULT_PTSIZE	18
#define DEFAULT_TEXT	"The quick brown fox jumped over the lazy dog按时打发"
#define NUM_COLORS      256

static char *Usage =
"Usage: %s [-solid] [-utf8|-unicode] [-b] [-i] [-u] [-s] [-outline size] [-hintlight|-hintmono|-hintnone] [-nokerning] [-fgcol r,g,b] [-bgcol r,g,b] <font>.ttf [ptsize] [text]\n";

static void cleanup(int exitcode)
{

	SDL_Quit();
	exit(exitcode);
}

int main(int argc, char *argv[])
{
	char *argv0 = argv[0];
	SDL_Surface *screen;

	SDL_Surface *text, *temp;
	int ptsize;
	int i, done;
	int rdiff, gdiff, bdiff;
	SDL_Color colors[NUM_COLORS];
	SDL_Color white = { 0xFF, 0xFF, 0xFF, 0 };
	SDL_Color black = { 0x00, 0x00, 0x00, 0 };
	SDL_Color red = { 0xff, 0x00, 0x00, 0 };
	SDL_Color *forecol;
	SDL_Color *backcol;
	SDL_Rect dstrect;
	SDL_Event event;
	int rendersolid;
	int renderstyle;
	int outline;
	int hinting;
	int kerning;
	int dump;	

	char *message, string[128];

	/* Look for special execution mode */
	dump = 0;
	/* Look for special rendering types */
	rendersolid = 0;

	/* Default is black and white */
	forecol = &black;
	backcol = &white;

	/* Check usage */
	if ( ! argv[0] ) {
		fprintf(stderr, Usage, argv0);
		return(1);
	}

	/* Initialize SDL */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return(2);
	}

	WRT_InitFont(argv[1]);

	/* Set a 640x480x8 video mode */
	screen = SDL_SetVideoMode(800, 480,32, SDL_HWSURFACE);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n",
							SDL_GetError());
		cleanup(2);
	}

	/* Set a palette that is good for the foreground colored text */
	rdiff = backcol->r - forecol->r;
	gdiff = backcol->g - forecol->g;
	bdiff = backcol->b - forecol->b;
	for ( i=0; i<NUM_COLORS; ++i ) {
		colors[i].r = forecol->r + (i*rdiff)/4;
		colors[i].g = forecol->g + (i*gdiff)/4;
		colors[i].b = forecol->b + (i*bdiff)/4;
	}
	SDL_SetColors(screen, colors, 0, NUM_COLORS);

	/* Clear the background to background color */
	SDL_FillRect(screen, NULL,
			SDL_MapRGB(screen->format, backcol->r, backcol->g, backcol->b));
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	/* Show which font file we're looking at */
	sprintf(string, "12:测试-李建文Font:%s",argv[1]);  /* possible overflow */
	printf(string);
	text = WRT_RenderTextBlended(string,red,12);
	printf("text = %x %d,%d\n",text,text->w,text->h);
	if ( text != NULL ) {
		dstrect.x = 4;
		dstrect.y = 4;
		dstrect.w = text->w;
		dstrect.h = text->h;
	}else{
		cleanup(2);
		return 0;
	}
	
	/* Blit the text surface */
	if ( SDL_BlitSurface(text, NULL, screen, &dstrect) < 0 ) {
		fprintf(stderr, "Couldn't blit text to display: %s\n", 
								SDL_GetError());
		void WRT_ExitFont();
		cleanup(2);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	
	SDL_FreeSurface(text);
	text = NULL;
	
	sprintf(string, "16:测试-李建文Font:%s",argv[1]);
	text = WRT_RenderTextBlended(string,red,16);
	printf("text = %x %d,%d\n",text,text->w,text->h);
	if ( text != NULL ) {
		dstrect.x = 4;
		dstrect.y = 4+40;
		dstrect.w = text->w;
		dstrect.h = text->h;
	}else{
		cleanup(2);
		return 0;
	}
	
	/* Blit the text surface */
	if ( SDL_BlitSurface(text, NULL, screen, &dstrect) < 0 ) {
		fprintf(stderr, "Couldn't blit text to display: %s\n", 
								SDL_GetError());
		void WRT_ExitFont();
		cleanup(2);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	
	SDL_FreeSurface(text);	
	
	sprintf(string, "24:测试-李建文Font:%s",argv[1]);
	text = WRT_RenderTextBlended(string,red,24);
	printf("text = %x %d,%d\n",text,text->w,text->h);
	if ( text != NULL ) {
		dstrect.x = 4;
		dstrect.y = 4+80;
		dstrect.w = text->w;
		dstrect.h = text->h;
	}else{
		cleanup(2);
		return 0;
	}
	
	/* Blit the text surface */
	if ( SDL_BlitSurface(text, NULL, screen, &dstrect) < 0 ) {
		fprintf(stderr, "Couldn't blit text to display: %s\n", 
								SDL_GetError());
		void WRT_ExitFont();
		cleanup(2);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	
	SDL_FreeSurface(text);	
	
	sprintf(string, "36:测试-李建文Font:%s",argv[1]);
	text = WRT_RenderTextBlended(string,red,36);
	printf("text = %x %d,%d\n",text,text->w,text->h);
	if ( text != NULL ) {
		dstrect.x = 4;
		dstrect.y = 4+120;
		dstrect.w = text->w;
		dstrect.h = text->h;
	}else{
		cleanup(2);
		return 0;
	}
	
	/* Blit the text surface */
	if ( SDL_BlitSurface(text, NULL, screen, &dstrect) < 0 ) {
		fprintf(stderr, "Couldn't blit text to display: %s\n", 
								SDL_GetError());
		void WRT_ExitFont();
		cleanup(2);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	
	//SDL_FreeSurface(text);			
	

	/* Set the text colorkey and convert to display format */
	/*
	if ( SDL_SetColorKey(text, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0) < 0 ) {
		fprintf(stderr, "Warning: Couldn't set text colorkey: %s\n",
								SDL_GetError());
	}
	
	temp = SDL_DisplayFormat(text);
	if ( temp != NULL ) {
		SDL_FreeSurface(text);
		text = temp;
	}
	*/
	

	/* Wait for a keystroke, and blit text on mouse press */
	done = 0;
	while ( ! done ) {
		if ( SDL_WaitEvent(&event) < 0 ) {
			fprintf(stderr, "SDL_PullEvent() error: %s\n",
								SDL_GetError());
			done = 1;
			continue;
		}
		switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
				dstrect.x = event.button.x - text->w/2;
				dstrect.y = event.button.y - text->h/2;
				dstrect.w = text->w;
				dstrect.h = text->h;
				if ( SDL_BlitSurface(text, NULL, screen,
							&dstrect) == 0 ) {
					SDL_UpdateRects(screen, 1, &dstrect);
				} else {
					fprintf(stderr,
					"Couldn't blit text to display: %s\n", 
								SDL_GetError());
				}
				break;
				
			case SDL_KEYDOWN:
			case SDL_QUIT:
				done = 1;
				break;
			default:
				break;
		}
	}
	SDL_FreeSurface(text);
	WRT_ExitFont();
	cleanup(0);

	/* Not reached, but fixes compiler warnings */
	return 0;
}
