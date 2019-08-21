#include "sip_data_sources.h"
#include "zenmalloc.h"
#include <string.h>
#include <utility>
#include <vector>

using namespace std;

static  vector<SIP_DATA_SOURCES*> g_sip_data_sources_vector;

int addSipDataSource(unsigned char type,int len,char* code, unsigned char* data){
        SIP_DATA_SOURCES* psip = NULL;
        psip = (SIP_DATA_SOURCES*)ZENMALLOC(sizeof(SIP_DATA_SOURCES));
        if(psip==NULL)
                return -1;
        psip->data = (unsigned char*)ZENMALLOC(len);
        if(psip->data == NULL){
                ZENFREE(psip);
                psip = NULL;
                return -2;
        }
        memcpy(psip->data,data,len);
        psip->type =type;
        psip->len = len;
        memset(psip->code,0,16);
        strcpy(psip->code,code);
        g_sip_data_sources_vector.push_back(psip);
        return 0;
}

void delSipDataSource(int index){
        if(index < 0 || index > (g_sip_data_sources_vector.size()-1))
                return;
        SIP_DATA_SOURCES* psip = NULL;
        psip = g_sip_data_sources_vector[index];
        if(psip != NULL){
                if(psip->data != NULL)
                        ZENFREE(psip->data);
                ZENFREE(psip);
                g_sip_data_sources_vector.erase(g_sip_data_sources_vector.begin()+index);
        }

}

int getSipDataSourceSize(){
        return g_sip_data_sources_vector.size();
}

SIP_DATA_SOURCES* getSipDataSource(int index){
        if(index < 0 || index > (g_sip_data_sources_vector.size()-1))
                return NULL;
        SIP_DATA_SOURCES* psip = NULL;
        psip = g_sip_data_sources_vector[index];
        return psip;
}

void delAllSipDataSource(){

        int index = 0;
        SIP_DATA_SOURCES* psip = NULL;
        int size = g_sip_data_sources_vector.size();
        if(size == 0)
        	return;

        for(index = 0; index < size;  index++){
                psip = g_sip_data_sources_vector[index];
                if(psip){
                        if(psip->data)
                                ZENFREE(psip->data);
                        ZENFREE(psip);
                        psip = NULL;
                }
        }
        g_sip_data_sources_vector.clear();
}

void set_default_data()
{
	delAllSipDataSource();
	addSipDataSource(0,strlen("��ݷ���"),"88001",(unsigned char*)"��ݷ���");
	addSipDataSource(0,strlen("������ϴ����"),"88002",(unsigned char*)"������ϴ����");
	addSipDataSource(0,strlen("����Ʒ����"),"88003",(unsigned char*)"����Ʒ����");
	addSipDataSource(0,strlen("�ӵ�Ӷ��������"),"88004",(unsigned char*)"�ӵ�Ӷ��������");
	addSipDataSource(0,strlen("ˮ��ά��"),"88005",(unsigned char*)"ˮ��ά��");
	addSipDataSource(0,strlen("������˷���"),"88006",(unsigned char*)"������˷���");
	addSipDataSource(0,strlen("�ÿͽӴ�"),"88007",(unsigned char*)"�ÿͽӴ�");
	addSipDataSource(0,strlen("���Է���"),"88008",(unsigned char*)"���Է���");
	
	
	addSipDataSource(0,strlen("��ʱ�������"),"88009",(unsigned char*)"��ʱ�������");
	addSipDataSource(0,strlen("�ż��������"),"88010",(unsigned char*)"�ż��������");
	addSipDataSource(0,strlen("���Ϳ���"),"88011",(unsigned char*)"���Ϳ���");
	addSipDataSource(0,strlen("���Ϳ�ӡ"),"88012",(unsigned char*)"���Ϳ�ӡ");
	addSipDataSource(0,strlen("����ˮ���"),"88013",(unsigned char*)"����ˮ���");
	addSipDataSource(0,strlen("���ɵ��������"),"88014",(unsigned char*)"���ɵ��������");
	addSipDataSource(0,strlen("����ͣ������"),"88015",(unsigned char*)"����ͣ������");
	addSipDataSource(0,strlen("���ͳ�ӡ����"),"88016",(unsigned char*)"���ͳ�ӡ����");
	
	addSipDataSource(0,strlen("���Ͷ�������"),"88017",(unsigned char*)"���Ͷ�������");
	addSipDataSource(0,strlen("�����ʼķ���"),"88018",(unsigned char*)"�����ʼķ���");
	addSipDataSource(0,strlen("��������绰"),"88019",(unsigned char*)"��������绰");
	addSipDataSource(0,strlen("������˹�ұ�"),"88020",(unsigned char*)"������˹�ұ�");
	addSipDataSource(0,strlen("����ˮ�����"),"88021",(unsigned char*)"����ˮ�����");
	addSipDataSource(0,strlen("����Ǩ�ƻ���"),"88022",(unsigned char*)"����Ǩ�ƻ���");
	addSipDataSource(0,strlen("�ṩ��ҹ�˾����"),"88023",(unsigned char*)"�ṩ��ҹ�˾����");
	addSipDataSource(0,strlen("ס��С�������γ̷���"),"88024",(unsigned char*)"ס��С�������γ̷���");	
	
	addSipDataSource(0,strlen("�������ջҵ��"),"88025",(unsigned char*)"�������ջҵ��");
	addSipDataSource(0,strlen("�����������Ʊ����"),"88026",(unsigned char*)"�����������Ʊ����");
	addSipDataSource(0,strlen("���桢Ӱӡ����"),"88027",(unsigned char*)"���桢Ӱӡ����");
	addSipDataSource(0,strlen("�շ������ʼ�����"),"88028",(unsigned char*)"�շ������ʼ�����");	
	
	
	
	
	
}
