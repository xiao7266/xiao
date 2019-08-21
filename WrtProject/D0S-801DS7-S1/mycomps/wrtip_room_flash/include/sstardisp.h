#ifndef __SSTARDISP__H__
#define __SSTARDISP__H__

#include <mi_disp_datatype.h>
#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

typedef struct WRT_DispChnParam_s
{
    MI_U16 u16SrcVideoW;
    MI_U16 u16SrcVideoH;
    MI_U16 u16X;
    MI_U16 u16Y;
    MI_U16 u16Width;
    MI_U16 u16Height;
} WRT_DispChnParam_t;

MI_S32 sstar_disp_init(MI_DISP_PubAttr_t* pstDispPubAttr);
MI_S32 WRT_EnableDispChn(MI_S32 s32Layer, MI_S32 s32Chn, WRT_DispChnParam_t *pDispChnParam);
MI_S32 WRT_DiableDispChn(MI_S32 s32Layer, MI_S32 s32Chn);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif
