
/*
 * t23xrmBase.c
 *
 * Core of the Talitos 2/3 extensible resource manager,
 * including module registration, core detection and
 * initialization, and the global storage area for
 * each core instance.
 *
 * Current versions are 100% dependent on a flat device
 * tree entry for each core, they do not auto-detect
 * hardware capability
 *
 * Copyright (c) 2007, 2008, Freescale Semiconductor, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Freescale Semiconductor nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */


/** @file t23xrmBase.c
 * Device initialization and shutdown
 */

#include "../common/t23.h"
#include "../common/xwcRMinterface.h"
#include "t23xrmInternal.h"


extern void t23xrmCoreID(uint32_t inst, uint8_t fdt);



/** Globals for the resource manager in general */
RMinterfaceCtx ifCtx[MAX_INTERFACES];

/** Instances of cores. Only one is useful at this time
 * Should malloc this only for the number of devices alive...
 */
T2CoreInstance t2blk[MAX_T2_INSTANCES];


/**
 * t23RMdevInit() - portable part of device initialization
 *
 * @param  ins          Instance index of device in question
 *
 * @param *baseAddr     points to the base of our device's
 *                      registers, and is assumed to be a
 *                      safe logical address for our use
 *
 * @param  channels     Number of channels implemented in this device
 *
 * @param  fifo_depth   Depth of Fetch FIFOs implemented in this device
 *
 * @param  eu_present   Bitmask of EUs present in this device
 *
 * @param  desc_types   Descriptor types supported by this device
 *
 * @return
 */
int32_t t23RMdevInit(int32_t  ins,
                     void    *baseAddr,
                     uint32_t channels,
                     uint32_t fifo_depth,
                     uint32_t eu_present,
                     uint32_t desc_types)
{
    int32_t           i, stat, ctx;

    stat = 0;

    /* Init interface context area */
    for (ctx = 0; ctx < MAX_INTERFACES; ctx++)
    {
        memset(ifCtx[ctx].ifName, 0, INTFC_NAME_LEN);
        ifCtx[ctx].inUse = INTFC_AVAILABLE;
    }

    /* Init ISR and Exec queues */
    t2blk[ins].isrQhead  = 0;
    t2blk[ins].isrQtail  = 0;
    t2blk[ins].isrQlevel = 0;
    t2blk[ins].isrQpeak  = 0;
    
    /* this is a one-entry request queue for initial testing */
    /* (!sec) should keep fixed depth, or link infinitely??  */
    for (i = 0; i < EXEC_QUEUE_DEPTH; i++)
        t2blk[ins].execQ[i] = NULL;

    t2blk[ins].execQhead  = 0;
    t2blk[ins].execQtail  = 0;
    t2blk[ins].execQlevel = 0;
    t2blk[ins].execQpeak  = 0;
    spin_lock_init(&t2blk[ins].execQlock);

    /* init channel states */
    for (i = 0; i < T3_MAX_CHANNELS; i++)
    {
        t2blk[ins].channelState[i] = CHstateFree;
        t2blk[ins].chnDescCt[i]    = 0;
    }


    /* With the base address, compute pointer to controller */
    t2blk[ins].regs = baseAddr;


    /* Initialize the device geometry with the stuff passed in from */
    /* the OS-dependent part of initialization. Ports of the driver */
    /* that don't work with the device tree need to ignore this,    */
    /* and derive the configuration from registers                  */
    t2blk[ins].totalChannels = channels;
    t2blk[ins].fifoDepth     = fifo_depth;
    t2blk[ins].euPresent     = eu_present | 0x00000001; /* or-in EU_NONE */
    t2blk[ins].validTypes    = desc_types;

    /* Now go ID the core so we know what we're working with */
    t23xrmCoreID(ins, HAS_FDT);


#ifdef RM_DBG_DEVID
    printk("Extensible Crypto Driver - SEC 2/3 Resource Manager - pkg rev %s\n", T23X_PACKAGE_VERSION);
    printk("core ID = %s (0x%016llx/0x%016llx)\n",
           t2blk[ins].devName,
           t2blk[ins].regs->ctrl.ID,
           t2blk[ins].regs->ipID.id);
#endif


    /* All configured, reset the device */
    t2blk[ins].regs->ctrl.masterControl = T2_MCR_SOFTWARE_RESET;


    /* Enable all channel interrupts. May throw this out in favor */
    /* of individual channel enable/disable functions */
    t2blk[ins].regs->ctrl.intMask = (T2_IMR_ERROR_CH0 |
                                     T2_IMR_ERROR_CH1 |
                                     T2_IMR_ERROR_CH2 |
                                     T2_IMR_ERROR_CH3 |
                                     T2_IMR_DONE_CH0 |
                                     T2_IMR_DONE_CH1 |
                                     T2_IMR_DONE_CH2 |
                                     T2_IMR_DONE_CH3);



    /* ...and configure channels to taste. This is just a generic      */
    /* setting, protocol plugins may choose a different configuration. */
    /* Configuration is saved off in core instance block for resets,   */
    /* and possible mutation, if need be                               */

    for (i = 0; i < t2blk[ins].totalChannels; i++)
    {
        t2blk[ins].channelConfig[i] = T2_CCR_BURSTSIZE_128 |
                                      T2_CCR_ADDRESS_32 |
                                      T2_CCR_ICV_STATUS_WRITEBACK |
                                      T2_CCR_STATUS_WRITEBACK |
                                      T2_CCR_SELECTIVE_NOTIFY |
                                      T2_CCR_CHANNEL_DONE_INTERRUPT;

        t2blk[ins].regs->chn[i].config = t2blk[ins].channelConfig[i];
    }

    /* Any remaining miscellaneous stuff */
    t2blk[ins].processedRQs = 0;

    /* All done, return to OS-level initialization */
    return stat;
}



/**
 * Remove device
 *
 * @param ins
 *
 * @return      status
 */
RMstatus t23RMdevRemove(int32_t ins)
{
    /* Turn off all interrupts */
    t2blk[ins].regs->ctrl.intMask = 0;

    return RM_OK;
}





