#ifndef WRTFONT_H_
#define WRTFONT_H_


#include "SDL.h"

#ifdef __cplusplus
extern "C"{
#endif   

//����4�����壬12,18,24,36. size=(12��18��34��36)
/*
    tmXY_t xy;
    xy.x = 100;
    xy.y = 250;
    txtbuf = NULL;
    txtbuf = WRT_RenderTextBlended("�������36������abcdABCD",&w,&h,0x0000ff,36);
    if(txtbuf){
        tmOSD_tmFillBmp(txtbuf,xy,w,h,255);
        free(txtbuf);
        txtbuf = NULL;
    }
    ����tmvideodisp�����ƣ��������tmFillBmp��ʾ���֡������һ����ɫ�ı�����
    �������Ҫȥ����ɫ�ı����������tmOSD_tmFillBmp��ʾ���֡�
*/

void    WRT_InitFont(char* filename); 

SDL_Surface* WRT_RenderTextBlended(const char *text,SDL_Color color,int size); //ʹ�������

SDL_Surface* WRT_RenderTextSolid(const char *text,SDL_Color color,int size); //��ʾЧ������

void     WRT_TextSize(const char* text,int *w,int *h,int size); //size ��ʾ��������

void WRT_ExitFont();

#ifdef __cplusplus
}
#endif

#endif