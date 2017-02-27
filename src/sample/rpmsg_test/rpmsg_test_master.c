/* rpmsg_test_master.c - rpmsg test code for master */

/* 
* Copyright (c) 2016 Wind River Systems, Inc. 
* 
* Redistribution and use in source and binary forms, with or without modification, are 
* permitted provided that the following conditions are met: 
* 
* 1) Redistributions of source code must retain the above copyright notice, 
* this list of conditions and the following disclaimer. 
* 
* 2) Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation and/or 
* other materials provided with the distribution. 
* 
* 3) Neither the name of Wind River Systems nor the names of its contributors may be 
* used to endorse or promote products derived from this software without specific 
* prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/
 
/*
modification history
--------------------
12dec16,ghl  update for OpenAMP-v201610 (F8373)
03may16,mw1  Create for OpenAMP (F5252).
*/

#include <vxWorks.h>
#include <string.h>
#include <tickLib.h>
#include "openamp/open_amp.h"
#include "openamp/open_amp_vx.h"
#include "rpmsg_test_helper.c"

/* External functions */
extern int init_system();
extern void cleanup_system();
extern struct hil_proc *platform_create_proc(int proc_index);

volatile int shutdown_called = 0;

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) 
{
    if ((*(int *) data) == SHUTDOWN_MSG)
    {
        shutdown_called  = 1;
    }
}

void rpmsg_test_master(const char * fw_name)
{
    int status;
    struct hil_proc *hproc = NULL;

    if (init_system() != 0)
    {
        OPEN_AMP_MSG("init system fail!\r\n");
        return;
    }

    hproc = platform_create_proc(0);
    if (hproc == NULL)
    {
        OPEN_AMP_MSG("platform_create_proc() fail!\r\n");
        return;
    }
    status = remoteproc_init((void *) fw_name, hproc, rpmsg_channel_created, 
                         rpmsg_channel_deleted, rpmsg_read_cb, &proc);

    if(!status)
    {
        OPEN_AMP_MSG("remoteproc_init OK\r\n");

        status = remoteproc_boot(proc);
        if (!status){
            OPEN_AMP_MSG("remoteproc_boot OK\r\n");
        }
        else{
            OPEN_AMP_MSG("remoteproc_boot fails, return %d\r\n", status);
            return;
        }
    }
    else {
        OPEN_AMP_MSG("remoteproc_init fails, return %d\r\n", status);
        return;
    }

    OPEN_AMP_MSG("remote is ready ...\n");
    while(shutdown_called != 1){
        if (proxy_test == 0)
        {
            hil_poll(proc->proc, 1);
            taskDelay(0);
        }
        else
        {
            taskDelay(5);
            hil_poll(proc->proc, 1);
        }
    }
    OPEN_AMP_MSG("shutdown remote ...\n");

    remoteproc_shutdown(proc);

    remoteproc_deinit(proc);

    cleanup_system();
}

