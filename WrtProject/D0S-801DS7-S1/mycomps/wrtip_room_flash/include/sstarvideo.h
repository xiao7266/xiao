#ifndef _SSTAR_VIDEO_H_
#define _SSTAR_VIDEO_H_

#include "mi_sys_datatype.h"
#include "mi_vdec_datatype.h"

#include "mi_common_datatype.h"
#define DISP_WIDTH_ALIGN 2
#define DISP_HEIGHT_ALIGN 2
#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_BACK(x, a)        (((x) / (a)) * (a))

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

MI_S32 WRT_CreateVdecChannel(MI_S32 s32VdecChn, MI_S32 s32CodecType,
    MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32OutWidth, MI_U32 u32OutHeight);
MI_S32 WRT_DestroyVdecChannel(MI_S32 s32VdecChn);
MI_S32 WRT_SendVdecFrame(MI_S32 s32VdecChn, MI_U8 *pu8Buffer, MI_S32 s32Len);

MI_S32 WRT_CreateDivpChannel(MI_S32 s32DivpChn, MI_U32 u32Width,
    MI_U32 u32Height, MI_S32 s32ColorFmt, MI_SYS_WindowRect_t stCropRect);
MI_S32 WRT_DestroyDivpChannel(MI_S32 s32DivpChn);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_SSTAR_VIDEO_H_

