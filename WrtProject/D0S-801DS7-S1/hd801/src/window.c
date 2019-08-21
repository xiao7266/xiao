#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "wrt_log.h"
#include "window.h"
#include "zenmalloc.h"




static SDL_LinkedList WindowList;

static Window *WindowGetTopWindow();
static int WindowAvailable();

extern  void UpdateWindow(int x,int y,int w,int h,int style);

extern int g_isUpdated ;
static unsigned char  g_isclosewin = 0;

extern int g_iseffect;
int WindowManagerInit()
{
        SDL_LListInit(&WindowList);
        srand(time(NULL));
        //if(mu_create("wloc",MU_LOCAL|MU_FIFO,0,&wlock)!= 0)
        //	  printf("create window lock error \n");
        return 1;
}    
int WindowAvailable()
{
        return (WindowList.last != NULL);
}

int CloseWindow(){
        WindowListElem *l;
        Window *pwindow = WindowGetTopVisibleWindow();
        if(pwindow == NULL)
                pwindow = WindowGetTopWindow();
        if(pwindow->type == ROOT){//根窗口不能关闭
                pwindow->NotifyReDraw2(pwindow);
                return 0;
        }

        l = (WindowListElem*) WindowList.last;

        while(l)
        {
                if(l->pWindow == pwindow)
                {
                        break;
                }
                l= (WindowListElem*)l->ll.prev;
        }
        //if(l->pWindow->type == CALLED || l->pWindow->type == CALLING){
        if(l->pWindow->CloseWindow)
                l->pWindow->CloseWindow();
        //  }

        SDL_LListRemove(&WindowList,&l->ll);
        ZENFREE(l->pWindow);
        ZENFREE(l);   
        return 1;   
}

int CloseAllWindows()
{
        Window* pwindow = WindowGetTopWindow();
        if(pwindow && pwindow->type == SAFEWIN)
                return 1;

        while(WindowAvailable()){
                if(!CloseWindow())
                        break;
        }
        WRT_DEBUG("CloseAllWindows");
        return 1;
}


void WindowAddToWindowList(Window *pWindow)
{
        WindowListElem *WindowNode;

        WindowNode=(WindowListElem*)ZENMALLOC(sizeof(WindowListElem));
        memset(WindowNode,0,sizeof(WindowListElem));

        WindowNode->pWindow=pWindow;

        SDL_LListAddToEnd(&WindowList,&WindowNode->ll);
}
Window* GetRootWindow(){
        WindowListElem *WindowListItem = (WindowListElem*)WindowList.last;

        if(WindowListItem == NULL)
                return NULL;
        do
        {
                if(WindowListItem->pWindow && WindowListItem->pWindow->type == ROOT)
                        return WindowListItem->pWindow;

                WindowListItem =(WindowListElem*) WindowListItem->ll.prev;

        }
        while(WindowListItem);

        return NULL;

}

int WindowInsertToWindowList(Window *pNewW){
        WindowListElem *WindowNode;
        Window * pTempWindow;

        WindowNode=(WindowListElem*)ZENMALLOC(sizeof(WindowListElem));
        memset(WindowNode,0,sizeof(WindowListElem));

        WindowNode->pWindow = pNewW;
        pTempWindow = WindowGetTopVisibleWindow();
        if(pTempWindow->type == CALLED || pTempWindow->type == CALLING){
                SDL_LListInsert(&WindowList,&WindowNode->ll, (WindowListElem*) WindowList.last);
                return 0;
        }
        SDL_LListAddToEnd(&WindowList,&WindowNode->ll);
        return 1;
}


int CloseTopWindow()
{
        WindowListElem *l;

        Window *pwindow = WindowGetTopVisibleWindow();

        if(pwindow == NULL)
                pwindow = WindowGetTopWindow();
        if(pwindow->type == ROOT)//根窗口不能关闭
                return 0;
        l = (WindowListElem*) WindowList.last;

        while(l)
        {
                if(l->pWindow == pwindow)
                {
                        break;
                }
                l= (WindowListElem*)l->ll.prev;
        }
        if(l->pWindow->CloseWindow)
                l->pWindow->CloseWindow();   
        SDL_LListRemove(&WindowList,&l->ll);

        ZENFREE(l->pWindow);
        ZENFREE(l);   
        l = (WindowListElem*) WindowList.last;
        if(l != NULL && l->pWindow != NULL){
                g_isclosewin = 1;
                l->pWindow->NotifyReDraw2(l->pWindow);
        }
        return 1;
}

Window *WindowGetTopVisibleWindow()
{
        WindowListElem *WindowListItem = (WindowListElem*)WindowList.last;

        if(WindowListItem == NULL)
                return NULL;
        do
        {
                if(WindowListItem->pWindow && WindowListItem->pWindow->valid)
                        return WindowListItem->pWindow;

                WindowListItem =(WindowListElem*) WindowListItem->ll.prev;
        }
        while(WindowListItem);

        return NULL;
}

Window *WindowGetTopWindow()
{
        WindowListElem *WindowListItem = (WindowListElem*)WindowList.last;

        if(WindowListItem)
        {
                return WindowListItem->pWindow;
        }
        return NULL;
}

Window* FindWindow(unsigned long type){
        WindowListElem *WindowListItem = (WindowListElem*)WindowList.last;

        if(WindowListItem == NULL)
                return NULL;
        do
        {
                if(WindowListItem->pWindow && WindowListItem->pWindow->type == type)
                        return WindowListItem->pWindow;

                WindowListItem =(WindowListElem*) WindowListItem->ll.prev;

        }
        while(WindowListItem);

        return NULL;	
}

Window* FindWindow2(void* p)
{
        WindowListElem *WindowListItem = (WindowListElem*)WindowList.last;

        if(WindowListItem == NULL)
                return NULL;
        do
        {
                if(WindowListItem->pWindow && WindowListItem->pWindow == (Window*)p)
                        return WindowListItem->pWindow;

                WindowListItem =(WindowListElem*) WindowListItem->ll.prev;

        }
        while(WindowListItem);

        return NULL;		
}
static void DrawWindows(void* p)
{
        POSITION pos;
        int effect = 5;
        Window* pWindow = (Window*)p;
        g_isUpdated = 0;
        pos = pWindow->pos;
        
        pWindow->NotifyReDraw();
        g_isUpdated = 1;
        if(g_iseffect == 1)
                effect = 0;
        
        if(FindWindow2(pWindow) != NULL){ //判断窗口是否被销毁如果窗口在NotifyReDraw()中销毁了自己，则不在继续后续动作
                if(g_isclosewin == 1){
        	        UpdateWindow(pos.x,pos.y,pos.w,pos.h,effect);
        	        g_isclosewin = 0;
        	}else
        	        UpdateWindow(pos.x,pos.y,pos.w,pos.h,effect);
        	if(pWindow->DisplayAfterInit){
        		pWindow->DisplayAfterInit();
        	}
	}

}

Window* New_Window(){
        Window* p = (Window*)ZENMALLOC(sizeof(Window));
        if(p == NULL)
                return NULL;
        memset(p,0,sizeof(Window));
        p->pos.x = p->pos.y = p->pos.w = p->pos.h  = 0;
        p->NotifyReDraw2 = DrawWindows;
        return p;
}

DialogWin* New_Dialog(){
        DialogWin* p = (DialogWin*)ZENMALLOC(sizeof(DialogWin));
        if(p == NULL)
                return NULL;
        memset(p,0,sizeof(DialogWin));
        p->baseWin.pos.x = p->baseWin.pos.y = p->baseWin.pos.w = p->baseWin.pos.h  = 0;
        p->baseWin.NotifyReDraw2 = DrawWindows;
        return p;	
}

int DestroyWindow(Window* p){
        if(p && p->type != ROOT){
                WindowListElem* l;
                l = (WindowListElem*) WindowList.last;

                while(l)
                {
                        if(l->pWindow == p)
                        {
                                break;
                        }
                        l= (WindowListElem*)l->ll.prev;
                }
                if(l->pWindow->CloseWindow)
                        l->pWindow->CloseWindow(); 

                SDL_LListRemove(&WindowList,&l->ll);
                ZENFREE(l->pWindow);
                ZENFREE(l);   
                return 0;
        }
        return -1;
}

void SetTopWindowID(unsigned char id)
{
        Window * pWindow = WindowGetTopVisibleWindow();
        pWindow->windowID = id;
}

unsigned char GetTopWindowID()
{

        Window * pWindow = WindowGetTopVisibleWindow();
        return pWindow->windowID;
}