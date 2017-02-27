/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 * Copyright (c) 2016 Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include <vxWorks.h>
#include "openamp/open_amp.h"
#include "rsc_table.h"

/* Place resource table in special ELF section */
#define __section(S)            __attribute__((__section__(#S)))
#define __resource              __section(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_             7

/* Remote supports Name Service announcement */
#define VIRTIO_RPMSG_F_NS           0


#ifdef _WRS_CONFIG_FSL_IMX

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
#define RING_TX                     (0x00920000 - 0x8000)
#define RING_RX                     (0x00920000 - 0x4000)

#define VRING_SIZE                  256
#define NUM_TABLE_ENTRIES           1

#define CARVEOUT_SRC_OFFSETS 
#define CARVEOUT_SRC 
#else

#ifdef _WRS_CONFIG_TI_SITARA

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
#define RING_TX                     (0x40520000 - 0x8000)
#define RING_RX                     (0x40520000 - 0x4000)

#define VRING_SIZE                  256
#define NUM_TABLE_ENTRIES           1

#define CARVEOUT_SRC_OFFSETS 
#define CARVEOUT_SRC 

#endif /* _WRS_CONFIG_TI_SITARA */
#endif /* _WRS_CONFIG_FSL_IMX */


const struct remote_resource_table __resource resources =
{
    /* Version */
    1,

    /* NUmber of table entries */
    NUM_TABLE_ENTRIES,
    /* reserved fields */
    { 0, 0,},

    /* Offsets of rsc entries */
    {
        CARVEOUT_SRC_OFFSETS
        offsetof(struct remote_resource_table, rpmsg_vdev),
    },

    /* End of ELF file */
    CARVEOUT_SRC

    /* Virtio device entry */
    {   RSC_VDEV, VIRTIO_ID_RPMSG_, 0, RPMSG_IPU_C0_FEATURES, 0, 0, 0, NUM_VRINGS, {0, 0},
    },

    /* Vring rsc entry - part of vdev rsc entry */
    {
        RING_TX, VRING_ALIGN, VRING_SIZE, 1, 0
    },
    {
        RING_RX, VRING_ALIGN, VRING_SIZE, 2, 0
    },
};

