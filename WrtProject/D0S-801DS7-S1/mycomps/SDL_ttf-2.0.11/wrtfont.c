
#include "wrtfont.h"
#include "SDL_ttf.h"
#include "charutil.h"


typedef  unsigned short UInt16;
static TTF_Font *g_ttffont[4];

void    WRT_InitFont(char* filename){
    TTF_Init();
    g_ttffont[0] = TTF_OpenFont(filename,12);
    if(g_ttffont[0] == NULL)
    	printf("TTF_OpenFont failed  %s\n",TTF_GetError());
    	
    g_ttffont[1] = TTF_OpenFont(filename,16);
    g_ttffont[2] = TTF_OpenFont(filename,24);      
    g_ttffont[3] = TTF_OpenFont(filename,36);  
}

void WRT_ExitFont()
{
	TTF_CloseFont(g_ttffont[0]);
	TTF_CloseFont(g_ttffont[1]);
	TTF_CloseFont(g_ttffont[2]);
	TTF_CloseFont(g_ttffont[3]);
	TTF_Quit();
	g_ttffont[0] = NULL;
	g_ttffont[1] = NULL;
	g_ttffont[2] = NULL;
	g_ttffont[3] = NULL;
	
}
void     WRT_TextSize(const char* text,int *w,int *h,int size)
{
    TTF_Font* m_pfont = NULL;
    UInt16* ptext;
    int  unilen = gb2uni(text,NULL,0);
    ptext =(UInt16*) malloc((unilen+1)*sizeof(UInt16));
    if(ptext == NULL)
        return NULL;
    memset(ptext,0,(unilen+1)*sizeof(UInt16));
    gb2uni(text,ptext,unilen);   
    switch(size)
    {
        case 12:
            m_pfont = g_ttffont[0];
            break;
        case 16:
            m_pfont = g_ttffont[1];
            break;
        case 24:
            m_pfont = g_ttffont[2];
            break;
        case 36:
            m_pfont = g_ttffont[3];
            break;  
        default:
             m_pfont = g_ttffont[2];
            break; 
    }
    
    TTF_SizeUNICODE(m_pfont,ptext,w,h);  
    free(ptext);
    ptext = NULL;    	
}

SDL_Surface* WRT_RenderTextBlended(const char *text,SDL_Color color,int size){
    SDL_Surface* txtbuf;
    int i,j;
    TTF_Font* m_pfont;
    UInt16* ptext;
    int txtw,txth;
    int  unilen = gb2uni(text,NULL,0);
    ptext =(UInt16*) malloc((unilen+1)*sizeof(UInt16));
    if(ptext == NULL)
        return NULL;
    memset(ptext,0,(unilen+1)*sizeof(UInt16));
    gb2uni(text,ptext,unilen);
    switch(size)
    {
        case 12:
            m_pfont = g_ttffont[0];
            break;
        case 16:
            m_pfont = g_ttffont[1];
            break;
        case 24:
            m_pfont = g_ttffont[2];
            break;
        case 36:
            m_pfont = g_ttffont[3];
            break;  
        default:
             m_pfont = g_ttffont[2];
            break; 
    }                                 

    txtbuf= TTF_RenderUNICODE_Blended(m_pfont, ptext, color);
    free(ptext);
    ptext = NULL;

    return txtbuf;

}

SDL_Surface* WRT_RenderTextSolid(const char *text,SDL_Color color,int size){
    SDL_Surface *txtbuf;
    TTF_Font* m_pfont;
    int i,j;
     UInt16* ptext;
    int txtw,txth;
    int  unilen = gb2uni(text,NULL,0);
    ptext =(UInt16*) malloc((unilen+1)*sizeof(UInt16));
    if(ptext == NULL)
        return NULL;
    memset(ptext,0,(unilen+1)*sizeof(UInt16));
    gb2uni(text,ptext,unilen);
    switch(size)
    {
        case 12:
            m_pfont = g_ttffont[0];
            break;
        case 16:
            m_pfont = g_ttffont[1];
            break;
        case 24:
            m_pfont = g_ttffont[2];
            break;
        case 36:
            m_pfont = g_ttffont[3];
            break;  
        default:
             m_pfont = g_ttffont[2];
            break; 
    }                                 
  
    txtbuf= TTF_RenderUNICODE_Solid(m_pfont, ptext,color);
    free(ptext);
    ptext = NULL;

	return txtbuf;
	/*
    printf("w = %d,h = %d \n",txtw,txth);
    txtbuf2 = (unsigned char*)ZENMALLOC(txtw*txth*3);
    if(txtbuf2 == NULL)
        return NULL;
    for(i = 0; i<txth;i++){
        for(j = 0; j<txtw;j++){
          *(txtbuf2+((txth-i-1)*txtw+j)*3) = *(txtbuf+(i*txtw+j)*4); 
          *(txtbuf2+((txth-i-1)*txtw+j)*3+1) = *(txtbuf+(i*txtw+j)*4+1);
          *(txtbuf2+((txth-i-1)*txtw+j)*3+2) = *(txtbuf+(i*txtw+j)*4+2);
        }  
    }    
    ZENFREE(txtbuf);
    *w = txtw;
    *h = txth;
	
    return txtbuf2;    
	*/
}