#ifndef WRTFONT_H_
#define WRTFONT_H_


#include "SDL.h"

#ifdef __cplusplus
extern "C"{
#endif   

//内置4种字体，12,18,24,36. size=(12，18，34，36)
/*
    tmXY_t xy;
    xy.x = 100;
    xy.y = 250;
    txtbuf = NULL;
    txtbuf = WRT_RenderTextBlended("粗体测试36号字体abcdABCD",&w,&h,0x0000ff,36);
    if(txtbuf){
        tmOSD_tmFillBmp(txtbuf,xy,w,h,255);
        free(txtbuf);
        txtbuf = NULL;
    }
    由于tmvideodisp的限制，如果调用tmFillBmp显示文字。则会有一个黑色的背景。
    所以如果要去掉黑色的背景，则调用tmOSD_tmFillBmp显示文字。
*/

void    WRT_InitFont(char* filename); 

SDL_Surface* WRT_RenderTextBlended(const char *text,SDL_Color color,int size); //使用这个。

SDL_Surface* WRT_RenderTextSolid(const char *text,SDL_Color color,int size); //显示效果不好

void     WRT_TextSize(const char* text,int *w,int *h,int size); //size 表示几号字体

void WRT_ExitFont();

#ifdef __cplusplus
}
#endif

#endif