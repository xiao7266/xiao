/***********************************************************************/
/*                                                                     */
/*   MODULE:  sys_conf.h                                               */
/*   DATE:    Updated 2003 for TriMedia                                                      */
/*   PURPOSE: pSOSystem configuration definitions                      */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*           Copyright 1991 - 1995, Integrated Systems, Inc.           */
/*                      ALL RIGHTS RESERVED                            */
/*                                                                     */
/*   Permission is hereby granted to licensees of Integrated Systems,  */
/*   Inc. products to use or abstract this computer program for the    */
/*   sole purpose of implementing a product based on Integrated        */
/*   Systems, Inc. products.   No other rights to reproduce, use,      */
/*   or disseminate this computer program, whether in part or in       */
/*   whole, are granted.                                               */
/*                                                                     */
/*   Integrated Systems, Inc. makes no representation or warranties    */
/*   with respect to the performance of this computer program, and     */
/*   specifically disclaims any responsibility for any damages,        */
/*   special or consequential, connected with the use of this program. */
/*                                                                     */
/*---------------------------------------------------------------------*/
/*                                                                     */
/*   This is a header file which defines all of the system-level       */
/*   parameters that are likely to need changing.                      */
/*                                                                     */
/*   It has been set up to be as close as possible to the settings     */
/*   which are appropriate for this application.  However, you may     */
/*   need to make some changes to parameters to reflect your           */
/*   environment, such as IP addresses, etc.                           */
/*                                                                     */
/*   Chapter 7 of the pSOSystem Getting Started manual contains a      */
/*   complete description of the symbols that are defined in this      */
/*   file.                                                             */
/*                                                                     */
/***********************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#ifndef _SYS_CONF_H
#define _SYS_CONF_H

/***********************************************************************/
/*                                                                     */
/*  These symbols are used to define other symbols in this file.       */
/*  They should never be changed.                                      */
/*                                                                     */
/***********************************************************************/
#define YES     1
#define NO      0

/***********************************************************************/
/*  The defines close to the top of this file are more likely to be    */
/*  used in TriMedia programs.                                         */
/***********************************************************************/

/*---------------------------------------------------------------------*/
/* pSOS+ configuration parameters                                      */
/*  These parameters determine the operation of the pSOS kernel.       */
/*  The number of objects are usually tuned to an application          */
/*---------------------------------------------------------------------*/
#define KC_NTASK       103      /* max number of tasks */
#define KC_NQUEUE      100      /* max number of message queues */
#define KC_NSEMA4      64       /* max number of semaphores */
#define KC_NMUTEX      64       /* max number of mutexes */
#define KC_NTIMER      10       /* max number of timers */
/*---------------------------------------------------------------------*/
/* The number above are "Local Objects."  The total number used		   */
/* at any time must be less than the following define                  */
/*---------------------------------------------------------------------*/
#define KC_NLOCOBJ     256       /* max number of local objects */
/*---------------------------------------------------------------------*/
#define KC_NMSGBUF     (128*32) /* number of message buffers used for queues */
/*---------------------------------------------------------------------*/
#define KC_TICKS2SEC   1000      /* clock tick interrupt frequency */
#define KC_TICKS2SLICE 10       /* time slice quantum, in ticks */
/*---------------------------------------------------------------------*/
/* The system stack can be used for interrupts */ 
#define KC_SYSSTK      0x8000   /* pSOS+ system stack size (bytes) */
/*---------------------------------------------------------------------*/
#define KC_ROOTSSTK    0x8000   /* ROOT supervisor stack size */
#define KC_ROOTUSTK    0x8000   /* ROOT user stack size */
#define KC_ROOTMODE    0x2000   /* ROOT initial mode */
#define KC_ROOTPRI     220       /* ROOT task priority */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/* User- installable callback functions:                               */
/*                                                                     */
/* The following macros can be used for installing functions to be     */
/* called at the following events:                                     */
/*                                                                     */
/*      KC_FATAL       at fatal error                                  */
/*      KC_STARTCO     at task start                                   */
/*      KC_DELETECO    at task deletion                                */
/*      KC_SWITCHCO    at task switch                                  */
/*                                                                     */
/* The required prototypes of the callback functions are as shown      */
/* below. The names of the callback functions may be altered.          */
/* When enabled, the implementations of the callback function must be  */
/* provided elsewhere. For instance, a fatal error handler might be    */
/* enabled by adding the following code to e.g. root.c, and changing   */
/* the corresponding '#if 0' below to an '#if 1":                      */
/*                                                                     */
/*       extern void MyFatalHandler(                                   */
/*                 long   error                                        */
/*                 long   node                                         */
/*                     )                                               */
/*       {                                                             */
/*           printf("pSOS execution terminated due to"                 */
/*                  " fatal error 0x%08x on node %d\n",                */
/*                  error, node );                                     */
/*       }                                                             */
/*                                                                     */
/* NOTE1: The functions KC_STARTCO, KC_DELETECO, and KC_SWITCHCO       */
/*        identify the started-, deleted, and switched tasks by means  */
/*        of both their task id (tid, as returned by t_create), and    */
/*        pointers to their task control blocks (tcb). The useful      */
/*        contents of the task control blocks can be obtained by       */
/*        the pseudo psos function t_taskinfo, and therefore the       */
/*        tcb pointers should not be used.                             */
/*                                                                     */
/* NOTE2: The functions KC_STARTCO, KC_DELETECO, and KC_SWITCHCO are   */
/*        called in kernel context, which is to be treated similar to  */
/*        interrupt service routines. For this reason, no calls to     */
/*        functions that might block are allowed. More specifically,   */
/*        calls to printf etc are not allowed. DP is allowed, and so   */
/*        is t_taskinfo.                                               */
/*                                                                     */
/*---------------------------------------------------------------------*/
#if 0
         extern void MyFatalHandler(
                 long   error,
                 long   node
                       );

#define KC_FATAL        MyFatalHandler
#else
#define KC_FATAL        0
#endif

/*---------------------------------------------------------------------*/
/* Prototype of task switch function is:							   */
/*        extern void MySwitchHandler (								   */
/*                long   entering_tid,								   */
/*                void*  entering_tcb,								   */
/*                long   leaving_tid,								   */
/*                void*  leaving_tcb								   */
/*                      );											   */
/*---------------------------------------------------------------------*/

#if defined (USE_TMPROFILER)
extern void tmosProfileTaskSwitchFunc (
			unsigned long  entering_tid, 
			void* entering_tcb,
            unsigned long  leaving_tid,  
            void* leaving_tcb);
#define KC_SWITCHCO    ((void (*)()) tmosProfileTaskSwitchFunc)        /* callout at task switch */
#else
#define KC_SWITCHCO        0
#endif

/*---------------------------------------------------------------------*/
#if 0
         extern void MyStartHandler (
                 long   starting_tid,
                 void*  starting_tcb
                       );

#define KC_STARTCO      MyStartHandler
#else
#define KC_STARTCO      0
#endif
/*---------------------------------------------------------------------*/
#if 0
         extern void MyDeleteHandler (
                 long   starting_tid,
                 void*  starting_tcb
                       );

#define KC_DELETECO     MyDeleteHandler
#else
#define KC_DELETECO     0
#endif
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*  TM Specific pSOS extension:                                        */
/* Stack overflow check. The following macro defines the number of     */
/* bytes at the end of the stack (of each task) which will be filled   */
/* with a known pattern, and which will be checked for being           */
/* overwritten at each system call. In case overwrite is detected,     */
/* then pSOS execution will abort with fatal error FAT_STKOVF (0xF30)  */
/* SC_STACK_OVF_CHECK will be truncated to an integral number of words.*/
/* It has a default of 8 bytes (but set to a slightly higher value     */
/* below).                                                             */
/*---------------------------------------------------------------------*/
#define SC_STACK_OVF_CHECK   24


/*---------------------------------------------------------------------*/
/*    TCS_MALLOC_USE:   When YES, do *not* map malloc/free on          */
/*                      rn_getseg/rn_free from region 0, as is standard*/
/*                      in pSOS. Instead, use the TCS memory manager   */
/*                      instead.                                       */
/*                      The pSOS region manager might be more          */
/*                      predictable in its real-time behavior, but this*/
/*                      at the cost of larger unit sizes (see          */
/*                      KC_RN0USIZE). Also, the pSOS region manager    */
/*                      cannot hold more than 32K units, which is 8M   */
/*                      with the current KC_RN0USIZE, but              */
/*                      proportionally less when the unit size is      */
/*                      decreased.                                     */
/*                      If this option is enabled, then define         */
/*                      TCS_REGION0_SIZE such that region 0 does not   */
/*                      occupy all free memory.                        */
/*---------------------------------------------------------------------*/
#define TCS_MALLOC_USE            YES
/*---------------------------------------------------------------------*/
/*    TCS_REGION0_SIZE: When *not* defined, then all free memory       */
/*                      (limited to 32K units) is given to region 0.   */
/*                      otherwise, region 0 is created with the        */
/*                      specifiedsize, but limited to 32K units; all   */
/*                      other memory is available via the TCS memory   */
/*                      manager.                                       */
/*                      This option might be used in combination with  */
/*                      TCS_MALLOC_USE when the desired KC_RN0USIZE    */
/*                      results in a region 0 that is not able to      */
/*                      contain all available SDRAM.                   */
/*                                                                     */
/*  Note that some region zero memory is required as v_queue           */
/*  buffers and some other pSOS things are allocated from there.       */
/*---------------------------------------------------------------------*/
#define TCS_REGION0_SIZE    32768

/*---------------------------------------------------------------------*/
/*    KC_RN0USIZE     : Allocations from the pSOS region zero have     */
/*                      this granularity.                              */
/*---------------------------------------------------------------------*/
#define KC_RN0USIZE    0x400    /* region 0 unit size */

/***********************************************************************/
/* SC_RAM_SIZE                                                         */
/*                                                                     */
/* Normally, pSOSystem uses ALL of the memory remaining on a board for */
/* dynamic allocation ("region 0").  You may override this by setting  */
/* SC_RAM_SIZE to a non-zero value.  If you do, pSOSystem will not     */
/* touch any memory after address (BSP_RAM_BASE + SC_RAM_SIZE - 1).    */
/*                                                                     */
/* NOTE: This is useful when making a BOOT ROM and you want to ensure  */
/* that the ROM's RAM area does not overflow the space alloted for it. */
/* (applies only to rn_getseg calls, not TCS malloc)                   */
/***********************************************************************/
#define SC_RAM_SIZE           (4*1024*1024)



/***********************************************************************/
/*                                                                     */
/*        O P E R A T I N G   S Y S T E M   C O M P O N E N T S        */
/*                                                                     */
/*  Setting the definition line to YES will cause that component to    */
/*  be built into the system.  NO means the component will NOT be      */
/*  put into the system.  It is an error to specify both SC_PSOS       */
/*  and SC_PSOSM as present!                                           */
/*                                                                     */
/*   The choice between pSOS+ and pSOS+m and the choice for using PNA, */
/*   ( SC_PSOS, SC_PSOSM and SC_PNA ) are now defined in TCS standard  */
/*   psos makefile. (in other words, the following are not used)       */
/*                                                                     */
/*   See for instance $(TCS)/examples/psos/psos_demo1                  */
/***********************************************************************/

#define SC_PSOS             YES  /* pSOS+ real-time kernel */
#define SC_PSOSM            NO   /* pSOS+ real-time multiprocessing kernel */
#define SC_PROBE            NO   /* pROBE+ (processor svcs) */
#define SC_PROBE_DISASM     NO   /* pROBE+ (disassembler) */
#define SC_PROBE_CIE        NO   /* pROBE+ (console executive) */
#define SC_PROBE_QUERY      NO   /* pROBE+ (query services) */
#define SC_PROBE_DEBUG      NO   /* pROBE+ (debug interface executive) */
#define SC_PHILE            NO   /* pHILE+ file system manager */
#define SC_PREPC            NO   /* pREPC+ C run-time library */
#define SC_PNA              YES   /* pNA+ TCP/IP networking manager */
#define SC_PRPC             NO   /* pRPC+ RPC component */
#define SC_PSE              NO   /* pSE+ streams component */
#define SC_PTLI             NO   /* pTLI+ TLI library component */
#define SC_PSKT             NO   /* pSKT+ SKT library component */
#define SC_PMONT            NO   /* pMONT+ */

/***********************************************************************/
/*                                                                     */
/*                     I / O   D E V I C E S                           */
/*                                                                     */
/*  Each device may be included in the system by specifying a major    */
/*  number for it, which determines its slot in the pSOS+ I/O switch   */
/*  table.  To leave a device driver out of the system, use NO for     */
/*  the major number.                                                  */
/*                                                                     */
/*  Note the following:                                                */
/*    * Major device 0 is reserved and cannot be used for any of these */
/*      devices.  Setting a device number to 0 here is the same as NO. */
/*    * No device number may be higher than SC_DEVMAX (SC_DEVMAX may   */
/*      be increased, if desired)                                      */
/*    * The lines defining the symbols DEV_SERIAL, DEV_TIMER, etc,     */
/*      should not be changed.  These are for use by application       */
/*      programs as the "device number" parameter on de_* calls.       */
/*                                                                     */
/***********************************************************************/
#define SC_DEV_SERIAL      NO           /* Serial driver */
#define SC_DEV_TIMER       1            /* Periodic tick timer */
#define SC_DEV_RAMDISK     NO           /* 3 RAM disk */
#define SC_DEV_SCSI        NO           /* 4 SCSI bus */
#define SC_DEV_SCSI_TAPE   NO           /* 5 SCSI bus, tape device */
#define SC_DEV_TFTP        NO           /* 6 TFTP pseudo driver */
#define SC_DEV_DLPI        NO           /* DLPI pseudo driver */
#define SC_DEV_OTCP        NO           /* 8 TCP/IP for OpEN */
#define SC_IP              SC_DEV_OTCP  /* 8 IP */
#define SC_ARP             NO           /* 9 ARP */
#define SC_TCP             NO           /* 10 TCP */
#define SC_UDP             NO           /* 11 UDP */
#define SC_RAW             NO           /* 12 RAW */
#define SC_LOOP            NO           /* 13 LOOP = (SC_DEV_OTCP + 5) */
#define SC_DEV_SOSI        NO           /* 14 OSI for OpEN */
#define SC_DEVMAX          20           /* Maximum device number */

#define DEV_SERIAL    (SC_DEV_SERIAL << 16)
#define DEV_TIMER     (SC_DEV_TIMER << 16)
#define DEV_RAMDISK   (SC_DEV_RAMDISK << 16)
#define DEV_SCSI      (SC_DEV_SCSI << 16)
#define DEV_SCSI_TAPE (SC_DEV_SCSI_TAPE << 16)
#define DEV_TFTP      (SC_DEV_TFTP << 16)


/*---------------------------------------------------------------------*/
/* pSOS+m configuration parameters                                     */
/*---------------------------------------------------------------------*/
#define MC_NGLBOBJ     8       /* size of gbl obj table */
#define MC_NAGENT      10       /* number of RPC agents in this node */
#define MC_FLAGS        0       /* operating mode flags */

/*---------------------------------------------------------------------*/
/* The following are examples for modifying the following definition   */
/*                                                                     */
/* Using a pSOSystem routine as a roster change handler                */
/* #define MC_ROSTER   ((void (*)()) SysInitFail)                      */
/*                                                                     */
/* Using a user written routine as a roster change handler             */
/* extern void MyHandler (void);                                       */
/* #define MC_ROSTER   ((void (*)()) MyHandler)                        */
/*                                                                     */
/*---------------------------------------------------------------------*/
#define MC_ROSTER       0       /* addr of user roster change callout */

/*---------------------------------------------------------------------*/
/* The max length of buffers are defined here. This is a field required*/
/* by the the configuration table for pSOS+m. The Kernel Interface     */
/* driver ki_smem.c also defines the size of buffers for ki MAXPKTSZ.  */
/* These two definitions must specify the same size of buffers for     */
/* pSOS+m to work correctly.                                           */
/*---------------------------------------------------------------------*/
#define MC_KIMAXBUF   100       /* maximum KI packet buffer length */
#define MC_ASYNCERR     0       /* asynchronous calls error callout */

/*---------------------------------------------------------------------*/
/* pNA+ configuration parameters                                       */
/*                                                                     */
/* NOTE: If you get pNA+ errno 5F01 FAT_INSUFFMEM you may need to      */
/*       decrease the number of buffers allocated here for pNA+.       */
/*       In general this error means pNA+ could not allocate enough    */
/*       memory from Region 0 for the number of buffers requested.     */
/*       NOTE: ethernet sockets require 4k buffers, and 0 length ones  */
/*             are required for proper pna functioning.                */
/*---------------------------------------------------------------------*/
#define NC_NNI         5        /* size of pNA NI Table */
#define NC_NROUTE      10       /* size of pNA Routing Table */
#define NC_NARP        20       /* size of pNA ARP table */
#define NC_DEFUID      0        /* Default User ID of a task */
#define NC_DEFGID      0        /* Default Group ID of a task */
#define NC_HOSTNAME    "scg"    /* Hostname of the node */
#define NC_NHENTRY     8        /* Number of Host table entries */
#define NC_NSOCKETS   20        /* Number of sockets in the system */
#define NC_NDESCS     20        /* # of socket descriptors/task */
#define NC_MBLKS     2000        /* # of message blocks in the system */

#define NC_BUFS_0     128       /* number of 0 length buffers */
#define NC_BUFS_128   128        /* number of 128 byte buffers */
#define NC_BUFS_1024  128        /* number of 1k byte buffers */
#define NC_BUFS_2048  128        /* number of 2k byte buffers */
#define NC_BUFS_4096  128        /* number of 4k byte buffers */
#define NC_BUFS_8192  32        /* number of 4k byte buffers */
#define SE_MAX_PNA_NC_BUFS  6   /* max number of NC_BUFS types */

#define NC_DAEMSSTK    KC_ROOTSSTK  /* pNA daemon supervisor stack size */
#define NC_DAEMUSTK    KC_ROOTUSTK  /* pNA daemon user stack size */
#define NC_DAEMMODE    KC_ROOTMODE  /* pNA daemon initial mode */

/***********************************************************************/
/*                                                                     */
/*              L A N   C O N F I G U R A T I O N                      */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
#define SD_LAN1               YES
#define SD_LAN1_IP            0xc0a80123
#define SD_LAN1_SUBNET_MASK   0xffffff00

/*=====================================================================*/
/*                                                                     */
/*                    L A N   I N T E R F A C E S                      */
/*                                                                     */
/*   * BSP_LAN1            - YES if LAN is supported                   */
/*   * BSP_LAN1_MTU        - Maximum transmission unit (bytes)         */
/*   * BSP_LAN1_HWALEN     - Length of hardware address in bytes       */
/*   * BSP_LAN1_FLAGS      - Interface flags (per pNA manual)          */
/*        IFF_POLL             0x8000   interface is a polling type    */
/*        IFF_BROADCAST        0x0001   NI supports broadcasts         */
/*        IFF_RAWMEM           0x2000   driver uses "mblk" interface   */
/*   * BSP_LAN1_PKB        - Total number of packet buffers            */
/*                                                                     */
/*=====================================================================*/

#define BSP_LAN1               YES
#define BSP_LAN1_MTU           1500
#define BSP_LAN1_HWALEN        6
#define BSP_LAN1_FLAGS         0x2001
#define BSP_LAN1_PKB           128

/***********************************************************************/
/*                                                                     */
/*         M I S C E L L A N E O U S   N E T W O R K I N G             */
/*                       P A R A M E T E R S                           */
/*                                                                     */
/***********************************************************************/
#define SD_DEF_GTWY_IP        0xc0a801FE  /* Default gateway ip address */



/***********************************************************************/
/*                                                                     */
/*  These symbols are used to define other symbols in this file.       */
/*  They should never be changed.                                      */
/*                                                                     */
/***********************************************************************/
#define YES     1
#define NO      0

#define USE_RARP 0

#define DBG_SA    1  /* Standalone pROBE debugging */
#define DBG_XS    2  /* talk to host debugger via RS-232 */
#define DBG_XN    3  /* talk to host debugger via network */
#define DBG_AP    4  /* Run application (w/ pROBE in standalone mode) */

#define STORAGE   5  /* Get SD_'s from storage area */
#define SYS_CONF  6  /* Get SD_'s from #defines set in this file */

/***********************************************************************/
/*                                                                     */
/*                 B A S I C   P A R A M E T E R S                     */
/*                                                                     */
/*  The parameters in this section determine the fashion in which      */
/*  many of the other parameters in this file will be used.            */
/*                                                                     */
/*  Many of the parameters in this file have names beginning with      */
/*  either "SC_" or "SD_".  Those beginning with "SC_" are ALWAYS      */
/*  controlled by the values you set in this file.  The values of      */
/*  the "SD_" parameters can be determined either by the definitions   */
/*  given in this file, or by the data found in the target board's     */
/*  parameter storage area.  SC_SD_PARAMETERS determines this.  If     */
/*  SC_SD_PARAMETERS is "SYS_CONF", then the values given in this      */
/*  file will be used for the SD_ parameters.  If SC_SD_PARAMETERS     */
/*  is "STORAGE", then pSOSystem will attempt to use the values found  */
/*  in the board's parameter storage area for the SD_ variables.  If   */
/*  the parameter storage area has not been initialized or has been    */
/*  corrupted, then the values given in this file will be used.  They  */
/*  will also be written to the storage area.                          */
/*                                                                     */
/*  If SC_SD_PARAMETERS is STORAGE, you may enable a "startup dialog"  */
/*  which will allow you to view, and optionally change, the           */
/*  parameter values found in the storage area.  The dialog is enabled */
/*  by setting SC_STARTUP_DIALOG to YES.  In this case,                */
/*  SD_STARTUP_DELAY specifies the number of seconds that the dialog   */
/*  will wait for input before proceeding to boot the system.          */
/*                                                                     */
/*  SE_DEBUG_MODE determines how the system will operate, as follows:  */
/*                                                                     */
/*     DBG_SA: Boot pROBE+ in standalone mode.                         */
/*                                                                     */
/*     DBG_XS: Boot into pROBE+ and wait for the host debugger         */
/*             via a serial connection                                 */
/*                                                                     */
/*     DBG_XN: Boot into pROBE+ and wait for the host debugger         */
/*             via a network connection                                */
/*                                                                     */
/*     DBG_AP: Same as DBG_SA, but also does a pROBE+ "silent startup",*/
/*             meaning that pROBE+ initializes itself without          */
/*             printing a startup banner.  pROBE+ then initializes     */
/*             pSOS+ and starts the application running.   This mode   */
/*             was created to simplify implementation of the pSOSystem */
/*             Boot ROMs.  Note that when this mode is used, the value */
/*             you specify for RC_SMODE (see "pROBE+ Configuration     */
/*             Parameters", below) is overridden.                      */
/*                                                                     */
/*    STORAGE: Use the mode (DBG_SA, DBG_XS, or DBG_XN) found in the   */
/*             parameter storage area.  If a valid mode is not found,  */
/*             then use DBG_SA.                                        */
/*                                                                     */
/*  An important consequence of using SE_DEBUG_MODE = STORAGE should   */
/*  be noted.  If you use the TFTP bootloader in the pSOSystem ROMs    */
/*  to download and start your system, you will be setting             */
/*  SE_DEBUG_MODE = DBG_AP in your target board's parameter storage    */
/*  area.  Thus, if you have left SE_DEBUG_MODE set to STORAGE in      */
/*  this file, your downloaded system will also use DBG_AP, meaning    */
/*  that the application will begin running immediately after the      */
/*  TFTP bootloader has loaded and started your system.                */
/*                                                                     */
/***********************************************************************/
#define SC_SD_PARAMETERS    STORAGE
#define SC_STARTUP_DIALOG   NO
#define SC_BOOT_ROM         NO
#define SD_STARTUP_DELAY    60
#define SE_DEBUG_MODE       STORAGE


/***********************************************************************/
/*                                                                     */
/*      S E R I A L   C H A N N E L   C O N F I G U R A T I O N        */
/*                                                                     */
/*  * SD_DEF_BAUD specifies the default baud rate to be used for the   */
/*    serial channels                                                  */
/*  * SC_APP_CONSOLE specifies the serial channel number (starting     */
/*    from 1) which should be used for the application's "console".    */
/*  * SC_PROBE_CONSOLE specifies the serial channel number (starting   */
/*    from 1) which pROBE+ should use to display output and receive    */
/*    commands, or to communicate with the host debugger.              */
/*  * SC_RBUG_PORT specifies the serial channel number (starting       */
/*    from 1) which should be used for the pROBE+ REMOTE DEBUG Channel.*/
/*    SC_APP_CONSOLE and SC_RBUG_PORT cannot be same port numbers.     */
/*                                                                     */
/***********************************************************************/
#define SD_DEF_BAUD           9600
#define SC_APP_CONSOLE        1
#define SC_PROBE_CONSOLE      1
#define SC_RBUG_PORT          0

/***********************************************************************/
/* SC_NumNon_pSOSChan is the number of non pSOS users of serial        */
/* channels. These are users that will be initiated before pSOS        */
/* such as pROBE.                                                      */
/* NOTE: These channels will not be closed on a soft reset.            */
/***********************************************************************/
#define SC_NumNon_pSOSChan 1

/***********************************************************************/
/*                                                                     */
/*      S H A R E D   M E M O R Y   C O N F I G U R A T I O N          */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
/*---------------------------------------------------------------------*/
/* If you want to build a Shared Memory Interface for the network or   */
/* multi-processing pSOS+m, set SD_SM_NODE (a non-zero value) to the   */
/* node number of THIS node.                                           */
/*---------------------------------------------------------------------*/
#define SD_SM_NODE            0

/*---------------------------------------------------------------------*/
/* Shared memory network interface - SC_NISM_LEVEL should be 1 if this */
/* will be the FIRST system to use the SMNI on this board.  It should  */
/* be 2 if this will not be the first system to use the SMNI on this   */
/* board.  Usually this means that SC_NISM_LEVEL should be 1 if you    */
/* are building Boot ROMs, and 2 for a system that will be downloaded  */
/* and started using the Boot ROMs.  See the comments at the beginning */
/* of drivers/ni_smem.c for more explanation.                          */
/*---------------------------------------------------------------------*/
#define SD_NISM               NO
#define SD_NISM_IP            0x00000000
#define SD_NISM_DIRADDR       SD_VME_BASE_ADDR+0x400
#define SC_NISM_BUFFS         30
#define SC_NISM_LEVEL         2
#define SD_NISM_SUBNET_MASK   0

/*---------------------------------------------------------------------*/
/* If you want to build a multi-node pSOS+m system, set SD_KISM        */
/* to the number of nodes in the system.                               */
/*---------------------------------------------------------------------*/
#define SD_KISM               0
#define SD_KISM_DIRADDR       SD_VME_BASE_ADDR+0x580

/***********************************************************************/
/*                                                                     */
/*         M I S C E L L A N E O U S   N E T W O R K I N G             */
/*                       P A R A M E T E R S                           */
/*                                                                     */
/***********************************************************************/
//#define SD_DEF_GTWY_IP        0

/***********************************************************************/
/*                                                                     */
/*                B U S   C O N F I G U R A T I O N                    */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
#define SD_VME_BASE_ADDR      0x01000000


#endif /* _SYS_CONF_H */

#if defined(__cplusplus)
}
#endif
