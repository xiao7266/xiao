
#ifndef _WARNING_TASK_H_
#define _WARNING_TASK_H_

#ifdef __cplusplus
extern "C"
{
#endif

int WarningInit(int reflag,unsigned long centerip,unsigned long managerip,unsigned long warningip,unsigned long centerminorip,unsigned long warningminorip);
void WarningUninit();
void WarningRecv();
int WarningSend(void* pvoid);

int SendWarning(char *RoomNO,int WarningID);



#ifdef __cplusplus
}
#endif

#endif

