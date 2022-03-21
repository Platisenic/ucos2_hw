/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        5       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
char          printBuffercopy[2048];

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task(void *data);                       /* Function prototypes of tasks                  */
        void  printmsgTask(void *data);
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
static  void  TaskStartCreateTasks(void);
/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 10);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */

    TaskStartCreateTasks();                                /* Create all the application tasks         */

    while (1) {
        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
#if OS_TASK_ONE_EN == 1
    OSTaskCreate(Task, (void *)0, &TaskStk[0][TASK_STK_SIZE - 1], 1);
#endif  
#if OS_TASK_TWO_EN == 1
    OSTaskCreate(Task, (void *)0, &TaskStk[1][TASK_STK_SIZE - 1], 2);
#endif 
#if OS_TASK_THREE_EN == 1
    OSTaskCreate(Task, (void *)0, &TaskStk[2][TASK_STK_SIZE - 1], 3);
#endif

    OSTaskCreate(printmsgTask, (void *)0, &TaskStk[4][TASK_STK_SIZE - 1], 5);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void Task (void *pdata)
{
    INT8U start;
    INT8U end;
    INT8U toDelay;

    pdata = pdata;
    start = OSTimeGet();
    while (1) {
        while (OSTCBCur->compTime > 0);

        end = OSTimeGet();

        toDelay = OSTCBCur->taskPeriod - (end-start);
        start += OSTCBCur->taskPeriod;

        OS_ENTER_CRITICAL();
        OSTCBCur->compTime = OSTCBCur->taskcomp;
        OS_EXIT_CRITICAL();

        OSTimeDly ((INT16U)toDelay);
    }
}

void printmsgTask (void *pdata) 
{
    while (1) {
        OS_ENTER_CRITICAL();
        strncpy(printBuffercopy, printBuffer, 2048);
        memset(printBuffer, 0, 2048);
        OS_EXIT_CRITICAL();
        printf("%s", printBuffercopy);
        OSTimeDly ((INT16U)10);
    }
    

}