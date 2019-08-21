#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sstarcommon.h"
#include "mi_sys.h"
#include "mi_sys_datatype.h"

MI_S32 WRT_ModuleBind(MI_S32 s32SrcMod, MI_S32 s32SrcDev, MI_S32 s32SrcChn,MI_S32 s32SrcPort,
    MI_S32 s32DstMod,MI_S32 s32DstDev,MI_S32 s32DstChn,MI_S32 s32DstPort)
{
    MI_S32 s32Ret = -1;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    
    memset(&stSrcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = (MI_ModuleId_e)s32SrcMod;
    stSrcChnPort.u32DevId = s32SrcDev;
    stSrcChnPort.u32ChnId = s32SrcChn;
    stSrcChnPort.u32PortId = s32SrcPort;

    memset(&stDstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stDstChnPort.eModId = (MI_ModuleId_e)s32DstMod;
    stDstChnPort.u32DevId = s32DstDev;
    stDstChnPort.u32ChnId = s32DstChn;
    stDstChnPort.u32PortId = s32DstPort;

    printf("xxxxxxxModule bind src(%d-%d-%d-%d) dst(%d-%d-%d-%d)...\n", s32SrcMod, s32SrcDev, s32SrcChn, s32SrcPort,
        s32DstMod, s32DstDev, s32DstChn, s32DstPort);
    s32Ret = MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, 0, 0);
    if (MI_SUCCESS != s32Ret)
    {
        printf("Err: bind fail\n");
        return -1;
    }

    return 0;
}

MI_S32 WRT_ModuleUnBind(MI_S32 s32SrcMod, MI_S32 s32SrcDev, MI_S32 s32SrcChn,MI_S32 s32SrcPort,
    MI_S32 s32DstMod,MI_S32 s32DstDev,MI_S32 s32DstChn,MI_S32 s32DstPort)
{
    MI_S32 s32Ret = -1;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    
    memset(&stSrcChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = (MI_ModuleId_e)s32SrcMod;
    stSrcChnPort.u32DevId = s32SrcDev;
    stSrcChnPort.u32ChnId = s32SrcChn;
    stSrcChnPort.u32PortId = s32SrcPort;

    memset(&stDstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stDstChnPort.eModId = (MI_ModuleId_e)s32DstMod;
    stDstChnPort.u32DevId = s32DstDev;
    stDstChnPort.u32ChnId = s32DstChn;
    stDstChnPort.u32PortId = s32DstPort;
    printf("xxxxxxxModule Unbind src(%d-%d-%d-%d) dst(%d-%d-%d-%d)...\n", s32SrcMod, s32SrcDev, s32SrcChn, s32SrcPort,
        s32DstMod, s32DstDev, s32DstChn, s32DstPort);
    s32Ret = MI_SYS_UnBindChnPort(&stSrcChnPort, &stDstChnPort);
    if (MI_SUCCESS != s32Ret)
    {
        printf("Err: bind fail\n");
        return -1;
    }

    return 0;
}

