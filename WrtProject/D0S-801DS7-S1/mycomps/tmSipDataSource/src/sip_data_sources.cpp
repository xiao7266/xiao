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
	addSipDataSource(0,strlen("快递服务"),"88001",(unsigned char*)"快递服务");
	addSipDataSource(0,strlen("衣物送洗服务"),"88002",(unsigned char*)"衣物送洗服务");
	addSipDataSource(0,strlen("日用品共购"),"88003",(unsigned char*)"日用品共购");
	addSipDataSource(0,strlen("钟点佣人清洁服务"),"88004",(unsigned char*)"钟点佣人清洁服务");
	addSipDataSource(0,strlen("水电维修"),"88005",(unsigned char*)"水电维修");
	addSipDataSource(0,strlen("行李搬运服务"),"88006",(unsigned char*)"行李搬运服务");
	addSipDataSource(0,strlen("访客接待"),"88007",(unsigned char*)"访客接待");
	addSipDataSource(0,strlen("留言服务"),"88008",(unsigned char*)"留言服务");
	
	
	addSipDataSource(0,strlen("临时寄物服务"),"88009",(unsigned char*)"临时寄物服务");
	addSipDataSource(0,strlen("信件管理服务"),"88010",(unsigned char*)"信件管理服务");
	addSipDataSource(0,strlen("代客开锁"),"88011",(unsigned char*)"代客开锁");
	addSipDataSource(0,strlen("代客刻印"),"88012",(unsigned char*)"代客刻印");
	addSipDataSource(0,strlen("代缴水点费"),"88013",(unsigned char*)"代缴水点费");
	addSipDataSource(0,strlen("代缴电信网络费"),"88014",(unsigned char*)"代缴电信网络费");
	addSipDataSource(0,strlen("代缴停车费用"),"88015",(unsigned char*)"代缴停车费用");
	addSipDataSource(0,strlen("代客冲印服务"),"88016",(unsigned char*)"代客冲印服务");
	
	addSipDataSource(0,strlen("代客订花服务"),"88017",(unsigned char*)"代客订花服务");
	addSipDataSource(0,strlen("代客邮寄服务"),"88018",(unsigned char*)"代客邮寄服务");
	addSipDataSource(0,strlen("代客申请电话"),"88019",(unsigned char*)"代客申请电话");
	addSipDataSource(0,strlen("代客瓦斯挂表"),"88020",(unsigned char*)"代客瓦斯挂表");
	addSipDataSource(0,strlen("代客水电过户"),"88021",(unsigned char*)"代客水电过户");
	addSipDataSource(0,strlen("代客迁移户口"),"88022",(unsigned char*)"代客迁移户口");
	addSipDataSource(0,strlen("提供搬家公司服务"),"88023",(unsigned char*)"提供搬家公司服务");
	addSipDataSource(0,strlen("住户小区导览课程服务"),"88024",(unsigned char*)"住户小区导览课程服务");	
	
	addSipDataSource(0,strlen("代订文艺活动业务"),"88025",(unsigned char*)"代订文艺活动业务");
	addSipDataSource(0,strlen("代订国内外机票服务"),"88026",(unsigned char*)"代订国内外机票服务");
	addSipDataSource(0,strlen("传真、影印服务"),"88027",(unsigned char*)"传真、影印服务");
	addSipDataSource(0,strlen("收发电子邮件服务"),"88028",(unsigned char*)"收发电子邮件服务");	
	
	
	
	
	
}
