

//#include "client.h"
#include <psos.h>
#include <list>
#include <algorithm>

using namespace std;


typedef struct __CLIENT{
        int sd;
        unsigned long windowid;
        unsigned long ip;
        char id[16];
}CLIENT;


void printfclient(CLIENT* p){
        if(p){
                printf("\n");
                printf("p->sd = %d \n",p->sd);
                printf("p->windowid = %d \n",p->windowid);
                printf("p->ip = %x \n",p->ip);
                printf("p->id = %s \n",p->id);
                printf("\n");
        }
}

list<CLIENT*>     m_clientlist;
unsigned long     m_clientlock;  	

void InitClient(){
        if( mu_create("clis",MU_LOCAL|MU_FIFO,0,&m_clientlock) != 0)
                printf("client list create lock faield \n");
}

void AddClient(int sd,int windowid,unsigned long ip,char* id){
        mu_lock(m_clientlock,MU_WAIT,0);
        CLIENT* p = (CLIENT*)malloc(sizeof(CLIENT));
        if(p != NULL){
                p->sd = sd;
                p->windowid = windowid;
                p->ip = ip;
                memset(p->id,0,16);
                if(id)
                        memcpy(p->id,id,15);
                m_clientlist.insert(m_clientlist.end(),p);
        }
        mu_unlock(m_clientlock);
}
void DelClientBysd(int sd){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->sd == sd){
                        m_clientlist.erase(it);
                        free(pt);
                        break;
                }
        }
        mu_unlock(m_clientlock);
}
void DelClientByWindowID(unsigned long windowid){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->windowid == windowid){
                        m_clientlist.erase(it);
                        free(pt);
                        break;
                }
        }
        mu_unlock(m_clientlock);
}

void ModifyWindowIDBySD(int sd,unsigned long windowid){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->sd == sd){
                        pt->windowid = windowid;
                        break;
                }
        }
        mu_unlock(m_clientlock);
}

void ModifySDByWindowID(unsigned long windowid,int sd){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->windowid == windowid){
                        pt->sd = sd;
                        break;
                }
        }
        mu_unlock(m_clientlock);
}	
void ModifyIDBySD(int sd,char id[16]){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->sd == sd){
                        memset(pt->id,0,16);
                        memcpy(pt->id,id,15);
                        break;
                }
        }
        mu_unlock(m_clientlock);  	

}

void ModifyIPByWindowID(unsigned long windowid,unsigned long ip){
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->windowid == windowid){
                        pt->ip = ip;
                        break;
                }
        }
        mu_unlock(m_clientlock);  
}
int  GetSDByWindowID(unsigned long windowid){
        int sd = -1;
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->windowid == windowid){
                        sd = pt->sd;
                        break;
                }
        }
        mu_unlock(m_clientlock);  	
        return sd;
}

unsigned long  GetWindowIDBySD(int sd){
        unsigned long windowid = -1;
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->sd == sd){
                        windowid = pt->windowid;
                        break;
                }
        }
        mu_unlock(m_clientlock);  	
        return windowid;
}  
int GetIDAndIPBySD(int sd,char* id,unsigned long * ip){
        unsigned long ret = -1;
        list<CLIENT*>::iterator it;
        mu_lock(m_clientlock,MU_WAIT,0);
        for(it = m_clientlist.begin(); it != m_clientlist.end();it++){
                CLIENT* pt = *it;
                if(pt && pt->sd == sd){
                        if(id){
                                memcpy(id,pt->id,16);
                        }
                        if(ip)
                                *ip = pt->ip;
                        ret =0;
                        break;
                }
        }
        mu_unlock(m_clientlock);  	
        return ret;  	

}

#ifdef _DEBUG_
void printall(){
        printf("\n");
        for_each(m_clientlist.begin(),m_clientlist.end(),printfclient);
        printf("\n");
}



void testclient(){
        int i =0;
        for(i =0;i<10;i++)
                AddClient(i,-1,0x0102a8c0,"000010010100101");

        for(i =0;i<10;i++)
                ModifyWindowIDbysd(i,rand());

        printall();
}

#endif

