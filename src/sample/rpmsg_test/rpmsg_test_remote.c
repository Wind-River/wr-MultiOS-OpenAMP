/* rpmsg_test_remote.c - rpmsg test code for remote */

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
03may16,mw1  Create for OpenAMP (F5252)
*/

#include <vxWorks.h>
#include <string.h>
#include <tickLib.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"
#include "openamp/open_amp_vx.h"

static struct rsc_table_info rsc_info;
extern const struct remote_resource_table resources;

/* External functions */
extern int init_system();
extern void cleanup_system();
extern struct hil_proc *platform_create_proc(int proc_index);

#include "rpmsg_test_helper.c"

volatile int shutdown_called = 0;

void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src)
{
    if ((*(int *) data) == SHUTDOWN_MSG)
    {
        shutdown_called  = 1;
        remoteproc_resource_deinit(proc);
    }
}

UINT32 rpmsg_ready = 0;
void rpmsg_test_remote(void)
{
    int status = 0;
    struct hil_proc *hproc;

    if (rpmsg_ready == 1)
        return;
    
    if (init_system() != 0)
    {
        OPEN_AMP_MSG("init system fails!\r\n");
        return;
    }

    rsc_info.rsc_tab = (struct resource_table *)&resources;
    rsc_info.size = sizeof(resources);

    hproc = platform_create_proc(0);
    if (hproc == NULL)
    {
        OPEN_AMP_MSG("platform_create_proc fails!\r\n");
        return;
    }
    
    /* Initialize RPMSG framework */
    status = remoteproc_resource_init(&rsc_info, 
                                    hproc, 
                                    rpmsg_channel_created, 
                                    rpmsg_channel_deleted, 
                                    rpmsg_read_cb,
                                    &proc, 
                                    0);

    if (status < 0) {
       	OPEN_AMP_MSG("remoteproc_resource_init fails, return %d\r\n", status);
        return;
    }
    else {
       	OPEN_AMP_MSG("remoteproc_resource_init OK, return %d\r\n", status);
    }
    
    while (!app_rp_chnl) {
        hil_poll(proc->proc, 0);
    }
    
    rpmsg_ready = 1;
    
    do {
        if (proxy_test == 0)
        {
            hil_poll(proc->proc, 1);
            taskDelay(0);
        }
        else
        {
            taskDelay(10);
        }
    } while (!shutdown_called);

    /* disable interrupts and free resources */
    remoteproc_resource_deinit(proc);

    cleanup_system();
    
	return;
}

#ifdef _WRS_CONFIG_TI_SITARA
extern UINT32 cmdRx(void);
extern int matrix_multi_test(int num);
extern void send_sample_packet(char* pattern, int pattern_len, int repeat_count);
extern void proxy_test_read(char *filename, int maxLen);
extern int mailboxDbgLvl;

void tM4Main(void)
{
    UINT32 rxCmd, cmd, arg;
    
    while(1)
    {
        rxCmd = cmdRx();
        if (rxCmd == 0)
        {
            taskDelay(60);
            continue;
        }
        
        cmd = rxCmd/100;
        arg = rxCmd%100;
       	OPEN_AMP_DBG(1, "cmd = %d, arg = %d\n", cmd, arg);
        switch (cmd)
        {
            case 1:
                rpmsg_test_remote();
                break;
            case 2:
                send_sample_packet("1234567890", strlen("1234567890"), arg);
                break;
            case 3:
                matrix_multi_test(arg);
                break;
            case 4:
                proxy_test_read("/romfs/read.txt", 4096);
                break;
            case 10:
                taskShow(0, 2);
                break;
            case 11:
                openampDbgLvl = arg;
                mailboxDbgLvl = arg;
                break;
            default:
                break;
        }
            
    }
}
/*void myAppStart(void)
{
    taskSpawn ((char *) "tM4", 180, 0, (size_t) 3000,
               (FUNCPTR) tM4Main,0L,0L, 0L,0L, 0L, 0L, 0L, 0L, 0L, 0L);
}*/
#endif
