/* proxy_app.c - rpmsg proxy application */

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
#include <stdio.h>
#include <stdlib.h>
#include <ioLib.h>
#include <string.h>
#include "openamp/rpmsg_retarget.h"
#include "openamp/open_amp_vx.h"

/* System call definitions */
#define OPEN_SYSCALL_ID		1
#define CLOSE_SYSCALL_ID	2
#define WRITE_SYSCALL_ID	3
#define READ_SYSCALL_ID		4
#define ACK_STATUS_ID		5
#define TERM_SYSCALL_ID		6


#define RPC_BUFF_SIZE 512
#define RPC_CHANNEL_READY_TO_CLOSE "rpc_channel_ready_to_close"

struct _proxy_data {
    int active;
    int proxy_fd;
    struct _sys_rpc *rpc;
    struct _sys_rpc *response;
};

static struct _proxy_data *proxy;
extern int proxy_test;

static int handle_open(struct _sys_rpc *rpc)
{
    int fd, bytes_written;

    /* Open remote fd */
    fd = open(rpc->sys_call_args.data, rpc->sys_call_args.int_field1,
              rpc->sys_call_args.int_field2);

    OPEN_AMP_DBG(1, "fd=%d,%s,flags=0x%x,mode=0x%x\n", fd, 
                    rpc->sys_call_args.data,
                    rpc->sys_call_args.int_field1,
                    rpc->sys_call_args.int_field2);
    
    /* Construct rpc response */
    proxy->response->id = OPEN_SYSCALL_ID;
    proxy->response->sys_call_args.int_field1 = fd;
    proxy->response->sys_call_args.int_field2 = 0; /*not used*/
    proxy->response->sys_call_args.data_len = 0; /*not used*/

    /* Transmit rpc response */
    bytes_written = write(proxy->proxy_fd, (char *)proxy->response,
                          sizeof(struct _sys_rpc));

    return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

static int handle_read(struct _sys_rpc *rpc)
{
    int bytes_read, bytes_written, payload_size;
    char *buff;
    int bufLen;

    if (rpc->sys_call_args.int_field2 == 0)
    {
        bufLen = 512;
    }
    else
    {
        bufLen = rpc->sys_call_args.int_field2;
    }

    OPEN_AMP_DBG(2, "bufLen=%d\n", bufLen);
    /* Allocate buffer for requested data size */
    buff = malloc(bufLen);
    if (buff == NULL)
    {
        OPEN_AMP_DBG(0, "Failed to allocate %d bytes\n", bufLen);
        return -1;
    }
    bytes_read = read(rpc->sys_call_args.int_field1, buff, bufLen);

    /* Construct rpc response */
    proxy->response->id = READ_SYSCALL_ID;
    proxy->response->sys_call_args.int_field1 = bytes_read;
    proxy->response->sys_call_args.int_field2 = 0; /* not used */
    proxy->response->sys_call_args.data_len = bytes_read;
    if (bytes_read > 0)
        memcpy(proxy->response->sys_call_args.data, buff, bytes_read);

    payload_size = sizeof(struct _sys_rpc) +
                    ((bytes_read > 0) ? bytes_read : 0);

    /* Transmit rpc response */
    bytes_written = write(proxy->proxy_fd, (char *)proxy->response,
                          payload_size);

    free(buff);
    return (bytes_written != payload_size) ? -1 : 0;
}

static int handle_write(struct _sys_rpc *rpc)
{
    int bytes_written;

    OPEN_AMP_DBG(1, "fd=%d\n", rpc->sys_call_args.int_field1);
    /* Write to remote fd */
    bytes_written = write(rpc->sys_call_args.int_field1, 
                          rpc->sys_call_args.data,
                          rpc->sys_call_args.int_field2);

    /* Construct rpc response */
    proxy->response->id = WRITE_SYSCALL_ID;
    proxy->response->sys_call_args.int_field1 = bytes_written;
    proxy->response->sys_call_args.int_field2 = 0; /*not used*/
    proxy->response->sys_call_args.data_len = 0; /*not used*/

    /* Transmit rpc response */
    bytes_written = write(proxy->proxy_fd, (char *)proxy->response,
                          sizeof(struct _sys_rpc));

    return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

static int handle_close(struct _sys_rpc *rpc)
{
    int retval, bytes_written;

    OPEN_AMP_DBG(1, "fd=%d\n", rpc->sys_call_args.int_field1);
    /* Close remote fd */
    retval = close(rpc->sys_call_args.int_field1);

    /* Construct rpc response */
    proxy->response->id = CLOSE_SYSCALL_ID;
    proxy->response->sys_call_args.int_field1 = retval;
    proxy->response->sys_call_args.int_field2 = 0; /*not used*/
    proxy->response->sys_call_args.data_len = 0; /*not used*/

    /* Transmit rpc response */
    bytes_written = write(proxy->proxy_fd, (char *)proxy->response,
                          sizeof(struct _sys_rpc));

    return (bytes_written != sizeof(struct _sys_rpc)) ? -1 : 0;
}

static int handle_rpc(struct _sys_rpc *rpc)
{
    int retval;
    char *data = (char *)rpc;
    
    if (!strcmp(data, RPC_CHANNEL_READY_TO_CLOSE)) {
        proxy->active = 0;
        return 0;
    }

    /* Handle RPC */
    switch ((int)(rpc->id)) {
        case OPEN_SYSCALL_ID:
        {
            retval = handle_open(rpc);
            break;
        }
        case CLOSE_SYSCALL_ID:
        {
            retval = handle_close(rpc);
            break;
        }
        case READ_SYSCALL_ID:
        {
            retval = handle_read(rpc);
            break;
        }
        case WRITE_SYSCALL_ID:
        {
            retval = handle_write(rpc);
            break;
        }
        default:
        {
            printf("\r\nMaster>Err:Invalid RPC sys call ID: %d:%d! \r\n", 
                    rpc->id, WRITE_SYSCALL_ID);
            retval = -1;
            break;
        }
    }

    return retval;
}

static int terminate_rpc()
{
    int bytes_written;
    int msg = TERM_SYSCALL_ID;
    
    printf ("Master> sending shutdown signal.\n");
    bytes_written = write(proxy->proxy_fd, (char *)&msg, sizeof(int));
    return bytes_written;
}

int proxy_app(void)
{
    unsigned int bytes_rcvd;
    int i = 0;
    int ret = 0;

    /* Allocate memory for proxy data structure */
    proxy = malloc(sizeof(struct _proxy_data));
    if (proxy == 0) {
        printf("\r\nMaster>Failed to allocate memory.\r\n");
        return -1;
    }
    proxy->active = 1;

    /* Open proxy rpmsg device */
    printf("\r\nMaster>Opening rpmsg proxy device\r\n");
    i = 0;
    do {
        taskDelay(60);
        proxy->proxy_fd = open("/rpmsg_proxy/0", O_RDWR, 0644);
    } while (proxy->proxy_fd < 0 && (i++ < 2));

    if (proxy->proxy_fd < 0) {
        printf("\r\nMaster>Failed to open rpmsg proxy driver device file.\r\n");
        free(proxy);
        return ERROR;
    }

    /* Allocate memory for rpc payloads */
    proxy->rpc = malloc(RPC_BUFF_SIZE);
    proxy->response = malloc(RPC_BUFF_SIZE);

    /* RPC service starts */
    printf("\r\nMaster>RPC service started (%d)!!\r\n", proxy->proxy_fd);
    while (proxy->active)
    {
        do {
            bytes_rcvd = read(proxy->proxy_fd, (char *)proxy->rpc, RPC_BUFF_SIZE);
            if (bytes_rcvd)
                OPEN_AMP_DBG(1, "Received %d bytes\r\n", bytes_rcvd);
            else
                taskDelay(120);
            if (!proxy->active)
            	break;
        } while(bytes_rcvd <= 0);

        if (!proxy->active)
            break;

        /* Handle rpc */
        if (handle_rpc(proxy->rpc))
        {
            printf("\r\nMaster>Handling remote procedure call!\r\n");
            printf("\r\nrpc id = %d\r\n", proxy->rpc->id);
            printf("\r\nrpc field1 = %d\r\n", proxy->rpc->sys_call_args.int_field1);
            printf("\r\nrpc field2 = %d\r\n", proxy->rpc->sys_call_args.int_field2);
            break;
        }
    }

    printf("\r\nMaster>RPC service exiting !!\r\n");

    /* Send shutdown signal to remote application */
    terminate_rpc();

    /* Need to wait here for sometime to allow remote application to
    complete its unintialization */
    taskDelay(60);
    
    proxy_test = 0;

    /* Close proxy rpmsg device */
    close(proxy->proxy_fd);

    /* Free up resources */
    free(proxy->rpc);
    free(proxy->response);
    free(proxy);

    return OK;
}

extern int vxbRpmsgProxyInit(void);

void proxy_test_demod(void)
{
    if (vxbRpmsgProxyInit() != 0)
    {
        OPEN_AMP_MSG("Failed to Init proxy driver!\n");
        return ;
    }
    
    proxy_test = 1;
    if (taskSpawn("tProxy", 200, 0, 4000, proxy_app, 0, 0, 0, 0 , 0, 0, 0, 0, 0, 0) == TASK_ID_NULL)
    {
        OPEN_AMP_MSG("Failed to create proxy demod task!\n");;
    }
}

extern struct rpmsg_channel *app_rp_chnl;

static void proxy_shutdown_cb(struct rpmsg_channel *rp_chnl)
{
	(void)rp_chnl;
}

void proxy_test_read(char *filename, int maxLen)
{
#define READ_LEN 400
    char *buffer;
    int fd, len, offset, rdLen;

    if (app_rp_chnl == NULL)
    {
        OPEN_AMP_MSG("Failed to create rpmsg channel\n");
        return ;
    }
    
    proxy_test = 1;

    if (rpmsg_retarget_init(app_rp_chnl, proxy_shutdown_cb) != 0)
    {
        OPEN_AMP_MSG("Failed to create rpmsg channel endpoint\n");
        return;
    }

    if (maxLen <= 0)
        maxLen = 4096;

	buffer = (char *)malloc(maxLen);
    if (buffer == NULL)
    {
        OPEN_AMP_DBG(0, "Failed to allocate %d bytes\n", maxLen);
        return;
    }
        
    fd = _open(filename, 0, 0);
    if (fd == ERROR){
        OPEN_AMP_DBG(0, "Failed to open %s,fd=%d\n", filename, fd);
        return;
    }
    OPEN_AMP_DBG(1, "Succeed to open %s,fd=%d\n", filename, fd);
    
    offset = 0;
    do{
        if (maxLen - offset > READ_LEN)
        {
            rdLen = READ_LEN;
        }
        else
        {
            rdLen = maxLen - offset;
        }
        
        len = _read(fd, buffer+offset, rdLen);
        OPEN_AMP_DBG(0, "offset = %d,len = %d\n", offset, len);
        offset += len;
    }while ((len == rdLen) && (offset < maxLen));
    
    OPEN_AMP_DBG(0, "Read %d bytes from %s\n", offset, filename);
    dump_packets(buffer, offset);
    (void) _close(fd);
	free(buffer);

    rpmsg_retarget_send(RPC_CHANNEL_READY_TO_CLOSE, sizeof (RPC_CHANNEL_READY_TO_CLOSE) + 1);
    rpmsg_retarget_deinit(app_rp_chnl);
    proxy_test = 0;
    
    return;
}

