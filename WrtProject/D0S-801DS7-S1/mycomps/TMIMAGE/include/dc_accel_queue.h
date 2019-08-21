/******************************************************************************
**                                                                           **
**                 Proprietary and Confidential Information                  **
**                                                                           **
**                 Copyright (c) Conexant, Inc., 2012						 **
**                                                                           **
** This source code and the algorithms implemented therein constitute        **
** confidential information and may compromise trade secrets of Conexant,    **
** Inc. or its associates, and any unauthorized use thereof is prohibited.   **
**                                                                           **
******************************************************************************/

#ifndef _dc_accel_queue_h_
#define _dc_accel_queue_h_

/******************************************************************************
*  FILE FORMAT:
*      1. INCLUDES 
*      2. ENUMERATIONS 
*      3. DEFINES
*      4. STRUCTURES
*      5. GLOBALS
*      6. PRIVATE FUNCTION PROTOTYPES
*      7. FUNCTIONS
******************************************************************************/

/******************************************************************************
*  1. INCLUDES
******************************************************************************/

/******************************************************************************
*  2. ENUMERATIONS
******************************************************************************/

/******************************************************************************
*  3. DEFINES
******************************************************************************/

typedef struct DA_QUEUE_LINK
{
    struct DA_QUEUE_LINK *prev;
    struct DA_QUEUE_LINK *next;
} DA_QUEUE_LINK;

typedef struct DA_QUEUE_DESC
{
    DA_QUEUE_LINK *first;
    DA_QUEUE_LINK *last;
    int count;
} DA_QUEUE_DESC;

/******************************************************************************
*  4. STRUCTURES
******************************************************************************/

/******************************************************************************
*  5. GLOBALS
******************************************************************************/

/******************************************************************************
*  6. FUNCTION PROTOTYPES
******************************************************************************/

void DAQueueInsertAfter(DA_QUEUE_DESC *listPtr,void *curQLPtr,void *newQLPtr);
void DAQueueInsertBefore(DA_QUEUE_DESC *listPtr,void *curQLPtr,void *newQLPtr);
void DAQueueInsertBeginning(DA_QUEUE_DESC *listPtr,void *newQLPtr);
void DAQueueInsertEnd(DA_QUEUE_DESC *listPtr,void *newQLPtr);
void DAQueueRemove(DA_QUEUE_DESC *listPtr,void *curQLPtr);

#endif
