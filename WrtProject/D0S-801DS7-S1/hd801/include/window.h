
#include "SDL_LList.h"

#ifdef __cplusplus
extern "C"{
#endif

#define ROOT   0
#define CHILD  1
#define SAFEWIN 2
#define DIALOG     3
#define CALLED     4   //����
#define CALLING    5   //����
#define RECORD     6
#define LEAVE_CHILD 7   //���Բ����Ӵ���
#define MUTILMEDIA  11
#define JIAJU_CONTROL 12
#define TS_CAL 13
#define DEVICE_TEST 14
#define PLAY_RECVIDEO 15

#ifdef WRT_MORE_ROOM

#define MSG_READ    8    // ��ֻ�����׼��
#define PHOTO_READ  9    // ��ֻ�����׼��

#endif
        typedef struct _tagPos{
                int x;
                int y;
                int w;
                int h;
        }POSITION;


        typedef struct _tagWindow{
                int (*EventHandler)(int x,int y,int status);
                void (*NotifyReDraw)();
                void (*CloseWindow)();
		void (*NotifyReDraw2)(void* p);
		void (*DisplayAfterInit)();
                int   valid;
                int   type;
                unsigned long windowID;
                POSITION pos;				
        }Window;

        typedef struct _dialogwin{
                Window  baseWin;
                int (*gfunc_ok)(int param);
                int (*gfunc_cancel)(int param);
                int (*gfunc_call)(int param);
                char  text[256];	
                int   dlgtype;
        }DialogWin;

        typedef struct _tagWindowListElem
        {
                SDL_LinkedListElem   ll;
                Window       *pWindow;
        }WindowListElem;

        int WindowManagerInit();


        void WindowAddToWindowList(Window *pWindow);
        int WindowInsertToWindowList(Window *pNewW);

        Window *WindowGetTopVisibleWindow();

        int CloseAllWindows();

        int CloseTopWindow();//�رմ��ڣ�ˢ����һ������

        int CloseWindow(); //�رմ��ڣ���ˢ����һ������

        Window* New_Window();
        DialogWin* New_Dialog();

        Window* FindWindow(unsigned long type);

        int DestroyWindow(Window* p);

        void SetTopWindowID(unsigned char id);

        unsigned char GetTopWindowID();

	Window* GetRootWindow();

#ifdef __cplusplus
}
#endif

