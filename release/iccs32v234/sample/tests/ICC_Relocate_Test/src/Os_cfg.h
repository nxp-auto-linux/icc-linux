/******************************************************************************
*
*       Freescale(TM) and the Freescale logo are trademarks of Freescale Semiconductor, Inc.
*       All other product or service names are the property of their respective owners.
*       Copyright (c) 2016 NXP Semiconductor
*
*       THIS SOURCE CODE IS CONFIDENTIAL AND PROPRIETARY AND MAY NOT
*       BE USED OR DISTRIBUTED WITHOUT THE WRITTEN PERMISSION OF
*       NXP Semiconductor
*
*       Description: Configuration Header file
*
*       Note: The implementation that was used is: AUTOSAR_SAC5xx
*       System Generator for AUTOSAR OS/SAC58R - Version: 4.0 Build 4.0.78
*
********************************************************************************/
#ifndef OSCFG_H
#define OSCFG_H
#define APP_START_SEC_CODE
#include    "Os_memmap.h"

#define OS_START_SEC_CONST_UNSPECIFIED
#include    "Os_sections.h"


/* Applications */
#define ICC_APP ((ApplicationType)OS_MKOBJID(OBJECT_APPLICATION, 0U)) /* Application ID */
void ErrorHook_ICC_APP(StatusType error);

/* Spinlock */

/* Application modes */
#define Mode01 ((AppModeType)0U)           /* AppMode ID */

/* Common stack */
#define OSHB_WORKERSTKSIZE 2064U           /* stack size of HB_WORKER */
#define OSHB_WORKERSTKBOS OSRUNNABLESTKBEG /* HB_WORKER bos */
#define OSHB_WORKERSTKTOS (OSHB_WORKERSTKBOS + OSHB_WORKERSTKSIZE/4U) /* HB_WORKER tos */
#define OSWORKER3STKSIZE 2064U             /* stack size of WORKER3 */
#define OSWORKER3STKBOS OSHB_WORKERSTKTOS  /* WORKER3 bos */
#define OSWORKER3STKTOS (OSWORKER3STKBOS + OSWORKER3STKSIZE/4U) /* WORKER3 tos */
#define OSWORKER1STKSIZE 2064U             /* stack size of WORKER1 */
#define OSWORKER1STKBOS OSWORKER3STKTOS    /* WORKER1 bos */
#define OSWORKER1STKTOS (OSWORKER1STKBOS + OSWORKER1STKSIZE/4U) /* WORKER1 tos */
#define OSWORKER2STKSIZE 2064U             /* stack size of WORKER2 */
#define OSWORKER2STKBOS OSWORKER1STKTOS    /* WORKER2 bos */
#define OSWORKER2STKTOS (OSWORKER2STKBOS + OSWORKER2STKSIZE/4U) /* WORKER2 tos */

/* Task definitions */
#define HB_WORKER ((TaskType)OS_MKOBJID(OBJECT_TASK, 0U)) /* Task ID */
extern void FuncHB_WORKER(void); /* Task entry point */
#define WORKER3 ((TaskType)OS_MKOBJID(OBJECT_TASK, 1U)) /* Task ID */
extern void FuncWORKER3(void); /* Task entry point */
#define WORKER1 ((TaskType)OS_MKOBJID(OBJECT_TASK, 2U)) /* Task ID */
extern void FuncWORKER1(void); /* Task entry point */
#define WORKER2 ((TaskType)OS_MKOBJID(OBJECT_TASK, 3U)) /* Task ID */
extern void FuncWORKER2(void); /* Task entry point */

/* ISR functions */
#define OS_isr_ICC_AsrOS_Reserved_Remote_ISR ICC_Remote_Event_Handler

/* ISRs definition */
#define OSISRICC_AsrOS_Reserved_Remote_ISR() OSISR2DISP(ICC_AsrOS_Reserved_Remote_ISR) /* IrqChannel is EXTERNAL */
extern void OS_isr_ICC_AsrOS_Reserved_Remote_ISR(void); /* irq: ICC_AsrOS_Reserved_Remote_ISR; channel: EXTERNAL; category: 2 */
#define ICC_AsrOS_Reserved_Remote_ISRLEVEL 5U /* interrupt level of ICC_AsrOS_Reserved_Remote_ISR */
#define ICC_AsrOS_Reserved_Remote_ISRPRIORITY 5U /* priority of ICC_AsrOS_Reserved_Remote_ISR */
#define ICC_AsrOS_Reserved_Remote_ISR ((ISRType)OS_MKOBJID(OBJECT_ISR, 0U)) /* ISR ID */

/* ISR1 id */

/* Resources definitions */
#define OsResource_ch0_f0 ((ResourceType)OS_MKOBJID(OBJECT_RESOURCE, 0U)) /* Resource ID */
#define OsResource_ch0_f1 ((ResourceType)OS_MKOBJID(OBJECT_RESOURCE, 1U)) /* Resource ID */
#define OsResource_ch1_f0 ((ResourceType)OS_MKOBJID(OBJECT_RESOURCE, 2U)) /* Resource ID */
#define OsResource_ch1_f1 ((ResourceType)OS_MKOBJID(OBJECT_RESOURCE, 3U)) /* Resource ID */
#define RES_SCHEDULER ((ResourceType)OS_MKOBJID(OBJECT_RESOURCE, 4U)) /* Resource ID */

/* Events definition */
#define OsEvent_ch0_f0 ((EventMaskType)4U) /* Event mask */
#define OsEvent_ch0_f1 ((EventMaskType)8U) /* Event mask */
#define OsEvent_ch1_f0 ((EventMaskType)128U) /* Event mask */
#define OsEvent_ch1_f1 ((EventMaskType)256U) /* Event mask */
#define OsEvent_HBW_ch0_f0 ((EventMaskType)16U) /* Event mask */
#define OsEvent_HBW_ch0_f1 ((EventMaskType)32U) /* Event mask */
#define OsEvent_HB_RATE ((EventMaskType)64U) /* Event mask */
#define OsEvent_W1_ch1_f0 ((EventMaskType)1U) /* Event mask */
#define OsEvent_W1_ch1_f1 ((EventMaskType)2U) /* Event mask */
#define OsEvent_W2_ch1_f0 ((EventMaskType)1U) /* Event mask */
#define OsEvent_W2_ch1_f1 ((EventMaskType)2U) /* Event mask */
#define OsEvent_W3_ch1_f0 ((EventMaskType)1U) /* Event mask */
#define OsEvent_W3_ch1_f1 ((EventMaskType)2U) /* Event mask */

/* Alarms identification */
#define OsAlarm_HB_RATE ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 0U)) /* Alarm ID */
#define OsAlarm_HBW_ch0_f0 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 1U)) /* Alarm ID */
#define OsAlarm_HBW_ch0_f1 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 2U)) /* Alarm ID */
#define OsAlarm_W1_ch1_f0 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 3U)) /* Alarm ID */
#define OsAlarm_W1_ch1_f1 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 4U)) /* Alarm ID */
#define OsAlarm_W2_ch1_f0 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 5U)) /* Alarm ID */
#define OsAlarm_W2_ch1_f1 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 6U)) /* Alarm ID */
#define OsAlarm_W3_ch1_f0 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 7U)) /* Alarm ID */
#define OsAlarm_W3_ch1_f1 ((AlarmType)OS_MKOBJID(OBJECT_ALARM, 8U)) /* Alarm ID */

/* Counters identification */
#define SYSTEMTIMER ((CounterType)OS_MKOBJID(OBJECT_COUNTER, 0U)) /* Counter ID */
#define OSMINCYCLE_SYSTEMTIMER ((TickType)0x1U) /* SYSTEMTIMER */
#define OSMAXALLOWEDVALUE_SYSTEMTIMER ((TickType)0x186a0U) /* SYSTEMTIMER */
#define OSTICKSPERBASE_SYSTEMTIMER 10UL    /* SYSTEMTIMER */
#define OS_TICKS2NS_SYSTEMTIMER(ticks) (PhysicalTimeType)(ticks*8333333U) /*  */
#define OS_TICKS2US_SYSTEMTIMER(ticks) (PhysicalTimeType)((OSQWORD)(ticks)*8333333ULL/1000UL) /*  */
#define OS_TICKS2MS_SYSTEMTIMER(ticks) (PhysicalTimeType)((OSQWORD)(ticks)*8333333ULL/1000000UL) /*  */
#define OS_TICKS2SEC_SYSTEMTIMER(ticks) (PhysicalTimeType)((OSQWORD)(ticks)*8333333ULL/1000000000UL) /*  */
#define OSMINCYCLE ((TickType)0x1U)        /* SysTimer */
#define OSMAXALLOWEDVALUE ((TickType)0x186a0U) /* SysTimer */
#define OSTICKSPERBASE 10UL                /* SysTimer */
#define OSTICKDURATION 8333333UL           /* SysTimer */

/* Messages identification */

/* Flags identification */

/* Message callback prototypes */

/* scheduletable */
#define OS_STOP_SEC_CONST_UNSPECIFIED
#include    "Os_sections.h"

#define APP_STOP_SEC_CODE
#include    "Os_memmap.h"

#endif /* OSCFG_H */

