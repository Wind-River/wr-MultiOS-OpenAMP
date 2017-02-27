/* openamp_dump.c - openAMP dump library */

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
03may16,mw1 Create for openAMP (F5252).
*/

#include "openamp/open_amp.h"

void dump_rproc(void *data)
{
    struct remote_proc *rp = (struct remote_proc*)data;

    if (NULL == data)
    {
        return;
    }

    printf("proc:0x%08x\n", rp->proc);
    printf("rdev:0x%08x\n", rp->rdev);
    printf("loader:0x%08x\n", rp->loader);
    printf("channel_created:0x%08x\n", rp->channel_created);
    printf("channel_destroyed:0x%08x\n", rp->channel_destroyed);
    printf("default_cb:0x%08x\n", rp->default_cb);
    printf("role:0x%08x\n", rp->role);
}


void dump_vq(void* data)
{
    struct virtqueue *vq = (struct virtqueue*)data ;

    if (NULL == data)
    {
        return;
    }

    printf("vq_name:%s\n", vq->vq_name);

    printf("vq_dev:0x%08x\n", vq->vq_dev);

    printf("vq_queue_index:0x%08x\n", vq->vq_queue_index);
    printf("vq_nentries:0x%08x\n", vq->vq_nentries);
    printf("vq_flags:0x%08x\n", vq->vq_flags);
    printf("vq_ring_size:0x%08x\n", vq->vq_ring_size);
    printf("vq_inuse:0x%08x\n", vq->vq_inuse);
    printf("vq_ring_mem:0x%08x\n", vq->vq_ring_mem);
    printf("callback:0x%08x\n", vq->callback);
    printf("notify:0x%08x\n", vq->notify);
    printf("vq_max_indirect_size:0x%08x\n", vq->vq_max_indirect_size);
    printf("vq_indirect_mem_size:0x%08x\n", vq->vq_indirect_mem_size);
    printf("vq_free_cnt:0x%08x\n", vq->vq_free_cnt);
    printf("vq_queued_cnt:0x%08x\n", vq->vq_queued_cnt);
    printf("vq_desc_head_idx:0x%08x\n", vq->vq_desc_head_idx);
    printf("vq_used_cons_idx:0x%08x\n", vq->vq_used_cons_idx);
    printf("vq_available_idx:0x%08x\n", vq->vq_available_idx);
    printf("vq_ring.num:0x%08x\n", vq->vq_ring.num);
    printf("vq_ring.desc:0x%08x\n", vq->vq_ring.desc);
    printf("vq_ring.avail:0x%08x\n", vq->vq_ring.avail);
    printf("vq_ring.used:0x%08x\n", vq->vq_ring.used);
    printf("vq_ring.vq_descx:0x%08x\n", &vq->vq_descx[0]);

}

void dump_vdev(void* data)
{
    struct virtio_device *vd = (struct virtio_device*)data;
    
    if (NULL == data)
    {
        return;
    }
    
    printf("name:%s\n", vd->name);
    printf("device:0x%08x\n", vd->device);
    printf("features:0x%08x\n", vd->features);
    printf("func:0x%08x\n", vd->func);
    printf("data:0x%08x\n", vd->data);
}

void dump_rdev(void * data)
{
    struct remote_device *rd = (struct remote_device*)data;

    if (NULL == data)
    {
        return;
    }

    dump_vdev(data);

    printf("rvq:0x%08x\n", rd->rvq);
    printf("tvq:0x%08x\n", rd->tvq);
    printf("proc:0x%08x\n", rd->proc);
    printf("rp_channels:0x%08x\n", rd->rp_channels);
    printf("rp_endpoints:0x%08x\n", rd->rp_endpoints);
    printf("mem_pool:0x%08x\n", rd->mem_pool);
    printf("channel_created:0x%08x\n", rd->channel_created);
    printf("channel_destroyed:0x%08x\n", rd->channel_destroyed);
    printf("role:0x%08x\n", rd->role);
    printf("state:0x%08x\n", rd->state);
    printf("support_ns:0x%08x\n", rd->support_ns);
}

void dump_channel(void * data)
{
    struct rpmsg_channel *ch = (struct rpmsg_channel*)data;

    if (NULL == data)
    {
        return;
    }

    printf("name:%s\n", ch->name);
    printf("src:0x%08x\n", ch->src);
    printf("dst:0x%08x\n", ch->dst);
    printf("rdev:0x%08x\n", ch->rdev);
    printf("rp_ept:0x%08x\n", ch->rp_ept);
    printf("state:0x%08x\n", ch->state);
}

void dump_endpoint(void* data)
{
    struct rpmsg_endpoint *ep = (struct rpmsg_endpoint*)data;

    if (NULL == data)
    {
        return;
    }

    printf("rp_chnl:0x%08x\n", ep->rp_chnl);
    printf("cb:0x%08x\n", ep->cb);
    printf("addr:0x%08x\n", ep->addr);
    printf("priv:0x%08x\n", ep->priv);
}

void dump_rpmsg_hdr(void *data)
{
    int i;
    struct rpmsg_hdr *hdr = (struct rpmsg_hdr *)data;

    if (NULL == data) {
        printf("dump_rpmsg_hdr:null data\n");
        return;
    }

    printf("src:0x%08x\n", hdr->src);

    printf("dst:0x%08x\n", hdr->dst);

    printf("len:0x%04x\n", hdr->len);

    printf("flags:0x%04x\n", hdr->flags);

    printf("data:");
    for (i=0; i<hdr->len; i++){
        if (0 == (i%4))
            printf(" ");
        if (0 == (i%16))
            printf("\n");
    }
}
