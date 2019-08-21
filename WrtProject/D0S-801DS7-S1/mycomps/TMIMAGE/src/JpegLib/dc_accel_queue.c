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

/**************************************************************************//**
*  @file dc_accel_queue.c
******************************************************************************/

/*
 * Acknowledgement:
 *
 * These routines are based on the explanation of doubly-linked lists
 * that were found on Wikipedia in the Linked List article on 11/6/2006.
 * (http://en.wikipedia.org/wiki/Linked_list)
 *
 */

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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dc_accel_queue.h"

/******************************************************************************
*  2. ENUMERATIONS
******************************************************************************/

/******************************************************************************
*  3. DEFINES
******************************************************************************/

/******************************************************************************
*  4. STRUCTURES
******************************************************************************/

/******************************************************************************
*  5. GLOBALS
******************************************************************************/

/******************************************************************************
*  6. PRIVATE FUNCTION PROTOTYPES
******************************************************************************/

/******************************************************************************
*  7. FUNCTIONS
******************************************************************************/

void DAQueueInsertAfter(DA_QUEUE_DESC *listPtr,void *curQLPtr,void *newQLPtr)
{
    DA_QUEUE_LINK *curPtr = (DA_QUEUE_LINK *) curQLPtr;
    DA_QUEUE_LINK *newPtr = (DA_QUEUE_LINK *) newQLPtr;
   
    newPtr->prev = curPtr;
    newPtr->next = curPtr->next;
    
    if (curPtr->next == NULL)
    {
        listPtr->last = newPtr;
    }
    else
    {
        curPtr->next->prev = newPtr;
    }
    
    curPtr->next = newPtr;

    listPtr->count++;
}

void DAQueueInsertBefore(DA_QUEUE_DESC *listPtr,void *curQLPtr,void *newQLPtr)
{
    DA_QUEUE_LINK *curPtr = (DA_QUEUE_LINK *) curQLPtr;
    DA_QUEUE_LINK *newPtr = (DA_QUEUE_LINK *) newQLPtr;
    
    newPtr->prev = curPtr->prev;
    newPtr->next = curPtr;
    
    if (curPtr->prev == NULL)
    {
        listPtr->first = newPtr;
    }
    else
    {
        curPtr->prev->next = newPtr;
    }
    
    curPtr->prev = newPtr;

    listPtr->count++;
}

void DAQueueInsertBeginning(DA_QUEUE_DESC *listPtr,void *newQLPtr)
{
    DA_QUEUE_LINK *newPtr = (DA_QUEUE_LINK *) newQLPtr;
    
    if (listPtr->first == NULL)
    {
        listPtr->first = newPtr;
        listPtr->last = newPtr;
        newPtr->next = NULL;
        newPtr->prev = NULL;
        listPtr->count++;
    }
    else
    {
        DAQueueInsertBefore(listPtr,listPtr->first,newPtr);
    }
}

void DAQueueInsertEnd(DA_QUEUE_DESC *listPtr,void *newQLPtr)
{
    DA_QUEUE_LINK *newPtr = (DA_QUEUE_LINK *) newQLPtr;
    
    if (listPtr->last == NULL)
    {
        DAQueueInsertBeginning(listPtr,newPtr);
    }
    else
    {
        DAQueueInsertAfter(listPtr,listPtr->last,newPtr);
    }
}

void DAQueueRemove(DA_QUEUE_DESC *listPtr,void *curQLPtr)
{
    DA_QUEUE_LINK *curPtr = (DA_QUEUE_LINK *) curQLPtr;
    
    if (curPtr->prev == NULL)
    {
        listPtr->first = curPtr->next;
    }
    else
    {
        curPtr->prev->next = curPtr->next;
    }

    if (curPtr->next == NULL)
    {
        listPtr->last = curPtr->prev;
    }
    else
    {
        curPtr->next->prev = curPtr->prev;
    }

    listPtr->count--;
}

