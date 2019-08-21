#ifndef __SSTARCOMMON__H__
#define __SSTARCOMMON__H__

#include <mi_common_datatype.h>
#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

MI_S32 WRT_ModuleBind(MI_S32 s32SrcMod, MI_S32 s32SrcDev, MI_S32 s32SrcChn,MI_S32 s32SrcPort,
    MI_S32 s32DstMod, MI_S32 s32DstDev, MI_S32 s32DstChn, MI_S32 s32DstPort);

MI_S32 WRT_ModuleUnBind(MI_S32 s32SrcMod, MI_S32 s32SrcDev, MI_S32 s32SrcChn,MI_S32 s32SrcPort,
    MI_S32 s32DstMod, MI_S32 s32DstDev, MI_S32 s32DstChn, MI_S32 s32DstPort);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__SSTARCOMMON__H__

