
/*
 * t23xrmModule.c
 *
 * Linux specific driver module initialization, 
 *
 * Current version is 100% dependent on a flat device
 * tree entry for each core, it does not auto-detect
 * hardware capability
 *
 * Copyright (c) 2007, 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <asm/io.h>
//#include <asm/ppcboot.h>

#include <asm/of_platform.h>

#include "../common/t23.h"
#include "../common/xwcRMinterface.h"
#include "t23xrmInternal.h"



#define PROC_CHAN_NAME_LEN (32)


/* FIXME: This should be picked up from a header... */
phys_addr_t get_immrbase(void);
irqreturn_t t23RMintDoneHandler(int32_t, void *);
irqreturn_t t23RMintOvflHandler(int32_t, void *);

extern T2CoreInstance t2blk[MAX_T2_INSTANCES];

/* /proc entries for diagnostics */
/* these aren't instance safed yet */
static struct proc_dir_entry *procDrvRoot;
static struct proc_dir_entry *procInst[MAX_T2_INSTANCES];
static struct proc_dir_entry *procCtrl[MAX_T2_INSTANCES];
static struct proc_dir_entry *procState[MAX_T2_INSTANCES];
static struct proc_dir_entry *procData[MAX_T2_INSTANCES];
static struct proc_dir_entry *procChnEnt[MAX_T2_INSTANCES];
static struct proc_dir_entry *procChn[MAX_T2_INSTANCES][T3_MAX_CHANNELS];
static struct proc_dir_entry *procChnRegs[MAX_T2_INSTANCES][T3_MAX_CHANNELS];
static struct proc_dir_entry *procChnStat[MAX_T2_INSTANCES][T3_MAX_CHANNELS];

static const uint8_t procChnIndex[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };



/* callback for controller register view */
/* /proc/t23x/0/controller */
int t23xCtrlView(char    *page,
                 char   **start,
                 off_t    off,
                 int      count,
                 int     *eof,
                 void    *data)
{

    if (off > 0)
    {
        *eof = 1;
        return 0;
    }


    return sprintf(page,
                   "Interrupt Mask:  0x%016llx\nInterrupt State: 0x%016llx\n",
                   t2blk[0].regs->ctrl.intMask,
                   t2blk[0].regs->ctrl.intStatus);

}

/* callback for instance state view */
/* /proc/t23x/0/state */
int t23xStateView(char    *page,
                  char   **start,
                  off_t    off,
                  int      count,
                  int     *eof,
                  void    *data)
{

    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    return sprintf(page,
                   "free channels:               %d\nchannel states:              %d %d %d %d\ninterrupt event queue depth: %d\nglobal request queue depth:  %d\n",
                   t2blk[0].freeChannels,
                   t2blk[0].channelState[0],
                   t2blk[0].channelState[1],
                   t2blk[0].channelState[2],
                   t2blk[0].channelState[3],
                   t2blk[0].isrQlevel,
                   t2blk[0].execQlevel);

}

/* callback for instance statistics view */
/* /proc/t23x/0/statistics */
int t23xCoreStatisticsView(char    *page,
                           char   **start,
                           off_t    off,
                           int      count,
                           int     *eof,
                           void    *data)
{
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    return sprintf(page,
                   "submitted requests:               %lld\nglobal request queue peak depth:  %d\ninterrupt event queue peak depth: %d\n",
                   t2blk[0].processedRQs,
                   t2blk[0].execQpeak,
                   t2blk[0].isrQpeak);
}

/* callback for channel statistics view */
/* /proc/t23x/<inst>/channel/<chn>/statistics */
int t23xChannelStatisticsView(char    *page,
                              char   **start,
                              off_t    off,
                              int      count,
                              int     *eof,
                              void    *data)
{
    uint8_t *chn;
    
    chn = (uint8_t *)data;
    
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    return sprintf(page,
                   "descriptors processed:      %lld\n",
                   t2blk[0].chnDescCt[*chn]);
}

/* callback for channel register view */
/* /proc/t23x/<inst>/channel/<chn>/registers */
int t23xChannelRegisterView(char    *page,
                            char   **start,
                            off_t    off,
                            int      count,
                            int     *eof,
                            void    *data)
{
    uint8_t *ch;
    
    ch = (uint8_t *)data;
    
    if (off > 0)
    {
        *eof = 1;
        return 0;
    }

    return sprintf(page,
                   "Configuration: 0x%016llx\nStatus:        0x%016llx\nDescriptor:    0x%016llx\n",
                   t2blk[0].regs->chn[*ch].config,
                   t2blk[0].regs->chn[*ch].pointerStatus,
                   t2blk[0].regs->chn[*ch].currentDesc);
}



static struct of_device_id t23x_match[] =
{
    {
        .type       = "crypto",
        .compatible = "talitos",
    },
    {},
};

MODULE_DEVICE_TABLE(of, t23x_match);

static struct of_platform_driver t23x_driver =
{
    .name        = "t23x",
    .match_table = t23x_match,
/*    .probe       = t23x_probe, */
/*    .remove      = t23x_remove, */
};






/**
 * Basic initializer, called from RM module installation
 *
 * Note that this only maps in 1 instance of a Talitos core,
 * all that is possible at the present time. The OS dependent
 * parts of initialization happen here; the portable core
 * initialization code happens in t23RMdevInit()
 *
 * @return
 */
static int32_t __init t23xwc_componentInit(void)
{
    int         stat;

    int32_t     chn;
    uint8_t     ins = 0; /* if ever goes multi instance, pass this in */
    uint8_t     chname[PROC_CHAN_NAME_LEN];
    
    void       *devBase;
    phys_addr_t immrbase;

    struct device_node  *dn = NULL;
    uint32_t    devirq = 0;
    uint32_t    oflirq = 0;

    uint32_t   *channels = NULL;
    uint32_t   *fifod    = NULL;
    uint32_t   *eu_mask  = NULL;
    uint32_t   *typ_mask = NULL;



    stat = of_register_platform_driver(&t23x_driver);

    if (stat == -1)
    {
        printk("t23xrm: can't register driver\n");
        return -1;
    }


    dn = of_find_node_by_type(dn, "crypto");
    if (dn == NULL)
    {
        printk("t23xrm: no compatible entry in device tree, exiting\n");
        return -1;
    }

    /* can do this with OF platform function? */
    immrbase = get_immrbase();

    /* Now get an equivalent logical address to use                   */
    /* as long as we're alive                                         */
    devBase = ioremap((phys_addr_t)(immrbase + T2_BASEADDR_OFFSET),
                      T2_REGISTER_BANK_SIZE);


    /* Now map interrupts */
    /* 0 interrupt is main (done/error) */
    /* 1 interrupt is done-overflow, T3 only */

    devirq = irq_of_parse_and_map(dn, 0);
    oflirq = irq_of_parse_and_map(dn, 1);

    /* Need to fetch capability bits from the dev node here */
    channels = (uint32_t *)of_get_property(dn, "num-channels", NULL);
    fifod    = (uint32_t *)of_get_property(dn, "channel-fifo-len", NULL);
    eu_mask  = (uint32_t *)of_get_property(dn, "exec-units-mask", NULL);
    typ_mask = (uint32_t *)of_get_property(dn, "descriptor-types-mask", NULL);

    if ((channels == NULL) ||
        (fifod    == NULL) ||
        (eu_mask  == NULL) ||
        (typ_mask == NULL))
    {
        printk("t23xrm: can't get a required property from device tree\n");
        return -1;
    }


    /*
     * Now go call the standard driver initialization
     * It's primary argument is the base address of the security block
     * in the address space of the chip
     */

    stat = t23RMdevInit(ins,           /* instance */
                        devBase,     /* ioremapped base address */
                       *channels,    /* number of channels present */
                       *fifod,       /* Fetch FIFO depth */
                       *eu_mask,     /* EUs-present mask */
                       *typ_mask);   /* descriptor type-supported mask */

    if (stat == -1)
    {
        iounmap(devBase);
        return stat;
    }

    /* Now call the OS to connect interrupts */
    /* First, save the detected interrupt numbers in the state block */
    t2blk[ins].doneIRQid = devirq;
    t2blk[ins].ovflIRQid = oflirq;

    /* connect the primary "done" handler. All Talitos devs use this */
    stat = request_irq(devirq,
                       t23RMintDoneHandler,
                       0,
                       "t23x-done",
                       &t2blk[ins]);

    if (stat)
    {
        printk("t23xrm: can't connect 'done' interrupt %d\n", devirq);
        return -1;
    }

    /* connect the overflow handler, if we have a line for it. */
    /* Only T3 does this */
    if (oflirq)
    {
        stat = request_irq(oflirq,
                           t23RMintOvflHandler,
                           0,
                           "t23x-overflow",
                           &t2blk[ins]);

        if (stat)
        {
            printk("t23xrm: can't connect 'overflow' interrupt %d\n", oflirq);
            free_irq(t2blk[ins].doneIRQid, &t2blk[ins]);
            return -1;
        }
    }



    /*
     * If all initialized OK, then go create the proc entries for diagnostics
     * This needs cleaned up for multi-instance
     */
     
    procDrvRoot               = proc_mkdir("t23x", NULL);
    procDrvRoot->owner        = THIS_MODULE;

    if (procDrvRoot == NULL)
    {
        iounmap(devBase);
        printk("t23xrm: can't create proc entry\n");
        return ENOMEM;
    }


    procInst[ins]             = proc_mkdir("0", procDrvRoot);
    procInst[ins]->owner      = THIS_MODULE;

    procCtrl[ins]             = create_proc_entry("controller", 0644, procInst[ins]);
    procCtrl[ins]->owner      = THIS_MODULE;
    procCtrl[ins]->read_proc  = t23xCtrlView;

    procState[ins]            = create_proc_entry("state", 0644, procInst[ins]);
    procState[ins]->owner     = THIS_MODULE;
    procState[ins]->read_proc = t23xStateView;

    procData[ins]             = create_proc_entry("statistics", 0644, procInst[ins]);
    procData[ins]->owner      = THIS_MODULE;
    procData[ins]->read_proc  = t23xCoreStatisticsView;

    procChnEnt[ins]           = proc_mkdir("channel", procInst[ins]);
    procChnEnt[ins]->owner    = THIS_MODULE;
    
    for (chn = 0; chn < t2blk[ins].totalChannels; chn++)
    {
        sprintf(chname, "%d", chn);
        procChn[ins][chn]        = proc_mkdir(chname, procChnEnt[ins]);
        procChn[ins][chn]->owner = THIS_MODULE;
        
        procChnRegs[ins][chn]    = create_proc_entry("registers",
                                                     0644, 
                                                     procChn[ins][chn]);
        procChnRegs[ins][chn]->owner     = THIS_MODULE;
        procChnRegs[ins][chn]->data      = (void *)&procChnIndex[chn];
        procChnRegs[ins][chn]->read_proc = t23xChannelRegisterView;
                
        procChnStat[ins][chn]            = create_proc_entry("statistics",
                                                           0644,
                                                           procChn[ins][chn]);
        procChnStat[ins][chn]->owner     = THIS_MODULE;
        procChnStat[ins][chn]->data      = (void *)&procChnIndex[chn];
        procChnStat[ins][chn]->read_proc = t23xChannelStatisticsView;
    }
    
    return stat;
}


/**
 * Device shutdown and removed, from RM module removal
 */
static void __exit t23xwc_componentExit(void)
{
    int8_t  chn;
    int8_t  ins = 0;
    uint8_t chname[PROC_CHAN_NAME_LEN];    
    
    
    of_unregister_platform_driver(&t23x_driver);

    /*
     * Go call the "portable" remove function before releasing
     * OS resources. It will shut off the core-level interrupt,
     * we need to shut off the handlers
     */
    t23RMdevRemove(ins);

    /* Now disconnect interrupts */
    free_irq(t2blk[ins].doneIRQid, &t2blk[ins]);

    if (t2blk[ins].ovflIRQid)
        free_irq(t2blk[ins].ovflIRQid, &t2blk[ins]);

    /* Delete proc entries in reverse order of creation */
    for (chn = (t2blk[ins].totalChannels - 1); chn >= 0; chn--)
    {
        remove_proc_entry("statistics", procChn[ins][chn]);
        remove_proc_entry("registers",  procChn[ins][chn]);
        sprintf(chname, "%d", chn);
        remove_proc_entry(chname,       procChnEnt[ins]);
    }
    remove_proc_entry("channel",    procInst[ins]);
    remove_proc_entry("statistics", procInst[ins]);
    remove_proc_entry("state",      procInst[ins]);
    remove_proc_entry("controller", procInst[ins]);
    remove_proc_entry("0",          procDrvRoot);
//    remove_proc_entry("t23x",      &proc_root);

    /* Unmap the register region, and unregister the driver */
    iounmap(t2blk[ins].regs);
}


EXPORT_SYMBOL (xwcRMregisterInterface);
EXPORT_SYMBOL (xwcRMderegisterInterface);
EXPORT_SYMBOL (xwcRMqueueRequest);
EXPORT_SYMBOL (xwcRMcancelRequest);

EXPORT_SYMBOL (xwcMemTranslateLogical);
EXPORT_SYMBOL (xwcMemTranslateUserVirtual);
EXPORT_SYMBOL (xwcMemTranslateKernelVirtual);
EXPORT_SYMBOL (xwcMemReleaseLogical);
EXPORT_SYMBOL (xwcMemReleaseUserVirtual);
EXPORT_SYMBOL (xwcMemReleaseKernelVirtual);


module_init(t23xwc_componentInit);
module_exit(t23xwc_componentExit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Extensible Crypto Driver " \
                    "- Resource Manager (SEC 2/3)");
MODULE_AUTHOR("Freescale Semiconductor - NMG/STC");





