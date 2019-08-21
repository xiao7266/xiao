#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "mi_sys.h"
#include "sstardisp.h"


#include "mi_panel_datatype.h"
#include "mi_panel.h"
#include "mi_disp_datatype.h"
#include "mi_disp.h"
//#include "RM68200_720x1280.h"
//#include "SAT070A_800x480.h"

#include "SAT070CP50_1024x600.h"

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)

#if defined(__cplusplus)||defined(c_plusplus)
extern "C"{
#endif
typedef void (*SUBROOM_CALLBACK)(unsigned long t_msg[4]);

MI_S32 sstar_disp_init(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_PANEL_LinkType_e eLinkType;

    MI_SYS_Init();

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_VGA)
    {
        eLinkType = E_MI_PNL_LINK_DAC_P;
    }
    else if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        pstDispPubAttr->stSyncInfo.u16Vact = stPanelParam.u16Height;
        pstDispPubAttr->stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth +
                                                                      stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Hact = stPanelParam.u16Width;
        pstDispPubAttr->stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        pstDispPubAttr->stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth +
                                                                      stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        pstDispPubAttr->stSyncInfo.u16Bvact = 0;
        pstDispPubAttr->stSyncInfo.u16Bvbb = 0;
        pstDispPubAttr->stSyncInfo.u16Bvfb = 0;
        pstDispPubAttr->stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        pstDispPubAttr->stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        pstDispPubAttr->stSyncInfo.u32FrameRate = stPanelParam.u16DCLK * 1000000 / (stPanelParam.u16HTotal * stPanelParam.u16VTotal);
        pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_USER;
        pstDispPubAttr->eIntfType = E_MI_DISP_INTF_LCD;
        pstDispPubAttr->u32BgColor = YUYV_BLACK;
        eLinkType = E_MI_PNL_LINK_TTL;
    }

    MI_DISP_SetPubAttr(0, pstDispPubAttr);
    MI_DISP_Enable(0);
    MI_DISP_BindVideoLayer(0, 0);
    MI_DISP_EnableVideoLayer(0);

    if (pstDispPubAttr->eIntfType == E_MI_DISP_INTF_LCD)
    {
        MI_PANEL_Init(eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            //MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
    }
    return 0;
}

MI_S32 WRT_EnableDispChn(MI_S32 s32Layer, MI_S32 s32Chn, WRT_DispChnParam_t *pDispChnParam)
{
    MI_S32 s32Ret = -1;
    MI_DISP_InputPortAttr_t stInputPortAttr;

    memset(&stInputPortAttr, 0, sizeof(stInputPortAttr));
    stInputPortAttr.u16SrcWidth = pDispChnParam->u16SrcVideoW;
    stInputPortAttr.u16SrcHeight = pDispChnParam->u16SrcVideoH;
    stInputPortAttr.stDispWin.u16X = pDispChnParam->u16X;
    stInputPortAttr.stDispWin.u16Y = pDispChnParam->u16Y;
    stInputPortAttr.stDispWin.u16Width = pDispChnParam->u16Width;
    stInputPortAttr.stDispWin.u16Height = pDispChnParam->u16Height;
    s32Ret = MI_DISP_SetInputPortAttr(s32Layer, s32Chn, &stInputPortAttr);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s:%d: MI_DISP_SetInputPortAttr fail\n");
        return -1;
    }
    s32Ret = MI_DISP_EnableInputPort(s32Layer, s32Chn);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s:%d: MI_DISP_EnableInputPort fail\n");
        return -1;
    }
    s32Ret = MI_DISP_SetInputPortSyncMode(s32Layer, s32Chn, E_MI_DISP_SYNC_MODE_FREE_RUN);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s:%d: MI_DISP_SetInputPortSyncMode fail\n");
        return -1;
    }

    return s32Ret;
}

MI_S32 WRT_DiableDispChn(MI_S32 s32Layer, MI_S32 s32Chn)
{
    MI_S32 s32Ret = -1;
    s32Ret = MI_DISP_DisableInputPort(s32Layer, s32Chn);
    if (MI_SUCCESS != s32Ret)
    {
        printf("%s:%d: MI_DISP_DisableInputPort fail\n");
        return -1;
    }

    return s32Ret;
}

void init_subroom()
{}
void reinit_subroom_ip()
{}


int send_msg_to_subroom(unsigned long t_msg[4])
{return 0;}

void set_subroom_callback(SUBROOM_CALLBACK  _callback)
{}

int have_idle_subroom()
{return 0;}

int have_connect_subroom()
{return 0;}

unsigned long get_subroom_connect_ip(int index)
{return 0;}

unsigned long get_subroom_ip_by_index(int index)
{return 0;}

void init_wrt_audiostream(int debug)
{}

void uninit_wrt_audiostream()
{}

void set_audio_param(int rate,int nchannel,int pttime,int ectime,int echomode)
{}

int start_wrt_audiostream(unsigned long destip,short destport,short localport,int ec)
{return 0;}

int stop_wrt_audiostream()
{return 0;}

int  start_wrt_record_audiostream(const char* file)
{return 0;}

int  stop_wrt_record_audiostream()
{return 0;}

int start_wrt_local_leave(const char* file)
{return 0;}

void stop_wrt_local_leave()
{}


void set_wrt_player_db_gain(float* db)
{}

void set_wrt_recorder_db_gain(float* db)
{}

int start_wrt_remote_leave(const char* file,int localport)
{return 0;}

int stop_wrt_remote_leave()
{return 0;}

int start_wrt_subroomaudio(unsigned long srcip,short srcport,unsigned long destip,short destport,short localport1,short localport2)
{return 0;}

int stop_wrt_subroomaudio()
{return 0;}

int wrt_test_echo_delay()
{return 0;}

#if defined(__cplusplus)||defined(c_plusplus)
}
#endif

