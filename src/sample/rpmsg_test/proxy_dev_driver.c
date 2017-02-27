/* proxy_dev_driver.c - rpmsg Proxy Device Driver */
 
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
03may16,mw1  Create for OpenAMP (F5252).
*/

#include <vxWorks.h>
#include <vsbConfig.h>
#include <errnoLib.h>
#include <errno.h>
#include <iosLib.h>
#include <stdio.h>
#include <string.h>
#include <semLib.h>
#include <rngLib.h>
#include <hwif/vxBus.h>
#include <hwif/vxbus/vxbLib.h>
#include <hwif/buslib/vxbFdtLib.h>
#include "openamp/rpmsg.h"
#include "openamp/open_amp_vx.h"

#ifdef  LOCAL
#undef  LOCAL
#define LOCAL static
#endif

#define VXB_PROXY_DEVICE_NAME   "/rpmsg_proxy/0"
#define VXB_PROXY_NAME_LEN      32
#define RPMSG_PACKET_SIZE       512
#define RPMSG_RNGBUF_SIZE       (RPMSG_PACKET_SIZE * 4)

#define IOCTL_GET_BUFFER_SIZE       1
#define IOCTL_GET_AVAIL_DATA_SIZE   2
#define IOCTL_GET_FREE_BUFF_SIZE    3

/* Shutdown message ID */
#define SHUTDOWN_MSG            0xEF56A55A
#define PROXY_ENDPOINT          127

#define RPMG_INIT_MSG "init_msg"

typedef struct _proxy_dev_params {
    DEV_HDR         devHdr;
    VXB_DEV_ID      devId;              /* vxBus device ID */
    SEM_ID          accessSem;          /* access semaphore */
    SEM_ID          syncSem;
    RING_ID         rpmsg_ring;
    struct rpmsg_channel  *rpmsg_chnl;
    struct rpmsg_endpoint *ept;
    char      tx_buff[RPMSG_PACKET_SIZE]; /* buffer to keep the message to send */
    UINT32    rpmsg_dst;
}RPMSG_PROXY_CTL;


/* forward declarations */
LOCAL STATUS vxbProxyDevAttach (VXB_DEV_ID);

/* locals */

LOCAL int vxbRpmsgProxyIosDrvNum = ERROR;

LOCAL VXB_DRV_METHOD vxbRpmsgProxyMethodList[] =
{
    { VXB_DEVMETHOD_CALL(vxbDevProbe),  vxbDevMatch  },
    { VXB_DEVMETHOD_CALL(vxbDevAttach), vxbProxyDevAttach },
    { 0, NULL }
};

/* globals */

VXB_DRV vxbFdtRpmsgProxyDrv =
{
    { NULL } ,
    "rpmsg-proxy",                             /* Name */
    "OpenAMP RPMSG Proxy Device driver",       /* Description */
    VXB_BUSID_FDT,                             /* Class */
    0,                                         /* Flags */
    0,                                         /* Reference count */
    &vxbRpmsgProxyMethodList[0]                /* Method table */
};

/*VXB_DRV_DEF(vxbFdtRpmsgProxyDrv)*/

/* functions */

LOCAL int proxy_dev_open
    (
    DEV_HDR * hdr,  /* pointer to the DEV_HDR struct */
    char * name,    /* device name */
    int flag,       /* open flag */
    int mode        /* open mode */
    )
{
    RPMSG_PROXY_CTL* device = (RPMSG_PROXY_CTL*)hdr;

    OPEN_AMP_DBG(2,"accessSem=%p\n",device->accessSem);
    /* access control */
    if (ERROR == semTake(device->accessSem, NO_WAIT))
    {
        OPEN_AMP_MSG("rpmsg proxy access denied\n");
        return (int)ERROR;
    }

    return (int)hdr;
}

LOCAL int proxy_dev_read
    (
    RPMSG_PROXY_CTL *   device, /* pointer to the device specific data */
    char *              buffer, /* read buffer address */
    int                 len     /* read buffer length */
    )
{
    int retval;
    int data_available, data_used;

    semTake(device->syncSem, WAIT_FOREVER);
    
    data_available = rngNBytes(device->rpmsg_ring);

    if (data_available == 0) {
        semGive(device->syncSem);
        return 0;
    }

    data_available = rngNBytes(device->rpmsg_ring);
    data_used = (data_available > len) ? len : data_available;
    retval = rngBufGet(device->rpmsg_ring, buffer, data_used);

    semGive(device->syncSem);
    OPEN_AMP_DBG(2,"Read %d bytes\n",retval);

    return retval;
}

LOCAL int proxy_dev_write
    (
    RPMSG_PROXY_CTL *   device, /* pointer to the device specific data */
    char *              buffer, /* write buffer address */
    int                 len     /* write buffer length */
    )
{

    int err;
    int size;

    if (len < RPMSG_PACKET_SIZE)
    {
        size = len;
    }
    else
    {
        size = RPMSG_PACKET_SIZE;
    }

    memcpy(device->tx_buff, buffer, size);
    err = rpmsg_sendto(device->rpmsg_chnl,
    				device->tx_buff,
    				size,
    				device->rpmsg_dst);

    if (err) {
        OPEN_AMP_MSG("rpmsg_sendto (size = %d) error: %d\n", size, err);
        size = 0;
    }
    OPEN_AMP_DBG(2,"Write %d bytes\n",size);

    return size;
}

LOCAL int proxy_dev_close
    (
    RPMSG_PROXY_CTL* device   /* device specific data */
    )
{
    /* give up access right */
    OPEN_AMP_DBG(2, "accessSem=%p\n", device->accessSem);
    (void)semGive (device->accessSem);
    return OK;
}

LOCAL int proxy_dev_ioctl
    (
    RPMSG_PROXY_CTL *   pDev,
    int         cmd,    /* ioctl function */
    int         arg         /* function arg */
    )
{
    unsigned int tmp;

    switch (cmd) {
        case IOCTL_GET_BUFFER_SIZE:
            tmp = RPMSG_RNGBUF_SIZE;
            memcpy((unsigned int *)arg, &tmp, sizeof(int));
            break;

        case IOCTL_GET_AVAIL_DATA_SIZE:
            tmp = rngNBytes(pDev->rpmsg_ring);
            memcpy((unsigned int *)arg, &tmp, sizeof(int));
            break;

        case IOCTL_GET_FREE_BUFF_SIZE:
            tmp = rngFreeBytes(pDev->rpmsg_ring);
            memcpy((unsigned int *)arg, &tmp, sizeof(int));
            break;

        default:
            return ERROR;
    }

    return OK;
}

LOCAL void proxy_dev_ept_cb(struct rpmsg_channel *rpdev, void *data,
					int len, void *priv, UINT32 src)
{

    RPMSG_PROXY_CTL *pCtrl = (RPMSG_PROXY_CTL*)priv;

    if ((*(int *) data) == SHUTDOWN_MSG)
    {
        OPEN_AMP_MSG("shutdown message is received. Shutting down...\n");
    }
    else
    {
        semTake(pCtrl->syncSem, WAIT_FOREVER);
        if (rngFreeBytes(pCtrl->rpmsg_ring) < len) 
        {
    	    semGive(pCtrl->syncSem);
            return;
        }

        rngBufPut(pCtrl->rpmsg_ring, data, (unsigned int)len);
        semGive(pCtrl->syncSem);
        OPEN_AMP_DBG(2,"Received %d bytes@%x\n",len,pCtrl);
    }
}

extern struct rpmsg_channel *app_rp_chnl;
LOCAL int vxbProxyDevInit
    (
    RPMSG_PROXY_CTL*    pCtrl
    )
{
    int status;

    pCtrl->syncSem = semMCreate(SEM_Q_PRIORITY);
    pCtrl->rpmsg_ring = rngCreate(RPMSG_RNGBUF_SIZE);
    if (pCtrl->rpmsg_ring == NULL)
    {
        OPEN_AMP_MSG("Failed to allocate rpmsg_ring.");
        return ERROR;
    }
    rngFlush(pCtrl->rpmsg_ring);

    pCtrl->rpmsg_chnl = app_rp_chnl;

    sprintf(pCtrl->tx_buff, RPMG_INIT_MSG);
    pCtrl->ept = rpmsg_create_ept(pCtrl->rpmsg_chnl,
                                  (rpmsg_rx_cb_t)proxy_dev_ept_cb,
                                  pCtrl,
                                  PROXY_ENDPOINT);
    if (!pCtrl->ept) {
        OPEN_AMP_MSG("Failed to create proxy service endpoint.");
        rngDelete(pCtrl->rpmsg_ring);
        semDelete(pCtrl->syncSem);
        return ERROR;
    }

    pCtrl->rpmsg_dst = PROXY_ENDPOINT;
    if (rpmsg_send(pCtrl->rpmsg_chnl, pCtrl->tx_buff, sizeof(RPMG_INIT_MSG)))
    {
        OPEN_AMP_MSG("Failed to send init_msg to target 0x%x.", pCtrl->rpmsg_dst);
        rpmsg_destroy_ept(pCtrl->ept);
        rngDelete(pCtrl->rpmsg_ring);
        semDelete(pCtrl->syncSem);
        return ERROR;
    }

    return OK;
}

LOCAL STATUS vxbProxyDevAttach
    (
    VXB_DEV_ID          pDev
    )
{
    char            iosDevName[VXB_PROXY_NAME_LEN];
    RPMSG_PROXY_CTL *pCtrl;

    OPEN_AMP_DBG(2, "pDev=%p\n", pDev);
    VXB_ASSERT_NONNULL (pDev, ERROR);
    vxbDevSoftcSet (pDev, NULL);  /* just in case an error occurs */

    /* allocate the memory for device specific data */
    pCtrl = (RPMSG_PROXY_CTL *) vxbMemAlloc (sizeof (RPMSG_PROXY_CTL));
    if (pCtrl == NULL)
    {
        return ERROR;
    }

    OPEN_AMP_DBG(2, "pCtrl=%p\n", pCtrl);
    memset(pCtrl, 0, sizeof (RPMSG_PROXY_CTL));

    /* create access semaphore */
    pCtrl->accessSem= semBCreate(SEM_Q_FIFO, SEM_FULL);
    if (SEM_ID_NULL == pCtrl->accessSem)
    {
        vxbMemFree (pCtrl);
        return ERROR;
    }

    vxbDevSoftcSet (pDev, (void *)pCtrl);
           
    /* initialize device */
    vxbProxyDevInit (pCtrl);

    /* if IO driver is not yet installed */

    if (ERROR == vxbRpmsgProxyIosDrvNum)
    {
        vxbRpmsgProxyIosDrvNum = iosDrvInstall (
            (DRV_CREATE_PTR)  NULL,
            (DRV_REMOVE_PTR)  NULL,
            (DRV_OPEN_PTR)    proxy_dev_open,
            (DRV_CLOSE_PTR)   proxy_dev_close,
            (DRV_READ_PTR)    proxy_dev_read,
            (DRV_WRITE_PTR)   proxy_dev_write,
            (DRV_IOCTL_PTR)   proxy_dev_ioctl
            );
        if (ERROR == vxbRpmsgProxyIosDrvNum)
        {
            OPEN_AMP_MSG ("Install Proxy device driver failed.\n");
            (void) semDelete(pCtrl->accessSem);
            vxbMemFree (pCtrl);
            return ERROR;
        }
    }

    /* format device name */

    (void) snprintf (iosDevName, VXB_PROXY_NAME_LEN, "%s", VXB_PROXY_DEVICE_NAME);

    /* add device */
    if (ERROR == iosDevAdd ((DEV_HDR *)&(pCtrl->devHdr), iosDevName,
                            vxbRpmsgProxyIosDrvNum))
    {
        (void) semDelete(pCtrl->accessSem);
        vxbMemFree (pCtrl);
        return ERROR;
    }

    pCtrl->devId = pDev;

    return OK;
}

IMPORT struct vxbDev vxbRoot;
LOCAL VXB_DEV_ID pProxyDev = NULL;
int vxbRpmsgProxyInit(void)
{
    if (vxbDrvAdd(&vxbFdtRpmsgProxyDrv) != OK)
    {
        OPEN_AMP_MSG("Adding device driver failed!\n");
        return -1;
    }
    OPEN_AMP_DBG(2, "pProxyDev=%x\n", pProxyDev);
    if (vxbDevCreate(&pProxyDev) != OK)
    {
        OPEN_AMP_MSG("Creating device failed for dev!\n");
        return -1;
    }
       
    OPEN_AMP_DBG(2,"pProxyDev=%x\n",pProxyDev);
    vxbDevNameSet(pProxyDev, "rpmsg-proxy", FALSE);
    vxbDevClassSet(pProxyDev, VXB_BUSID_FDT);
    
    /* add bus ctlr */
    if (vxbDevAdd((VXB_DEV_ID)&vxbRoot, pProxyDev) != OK)
    {
        OPEN_AMP_MSG("Adding dev failed on bus!\n");
        return -1;
    }
    
    return 0;
}

