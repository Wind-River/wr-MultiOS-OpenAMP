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
03may16,mw1  Create for OpenAMP (F5252)
*/

#define SHUTDOWN_MSG	0xEF56A55A


/* Internal functions */
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl);
static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl);
static void rpmsg_read_cb(struct rpmsg_channel *, void *, int, void *, unsigned long);
void rpmsg_read_matrix_cb(struct rpmsg_channel *, void *, int ,void * , unsigned long);
void rpmsg_read_result_cb(struct rpmsg_channel *, void *, int ,void * , unsigned long);

/* Globals */
struct rpmsg_channel *app_rp_chnl = NULL;
struct rpmsg_endpoint *rp_ept = NULL;
struct remote_proc *proc = NULL;
int proxy_test = 0;

void rpmsg_echo_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) 
{
    int stat;
    
	OPEN_AMP_MSG("Msg:%s,len:%d,src:%d\r\n", data, len, src);
	if (len > 2)
    {
        *(((char *)data)+len-2) = '\0'; /* Append EOS */
		stat = rpmsg_send_offchannel(rp_chnl, 0x0F, 0x0F, data, len-1);/* bounce between and diminish */
		if (stat != 0) {OPEN_AMP_MSG("rpmsg_send error:%d\n", stat);}
	}
}

static void rpmsg_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) 
{
	OPEN_AMP_MSG("Msg:%s,len:%d,src:%d\r\n", data, len, src);
}

void send_rpmsg(char* pattern)
{
	int stat;
    
	if (0 == pattern){
		OPEN_AMP_MSG("Invalid parameters\r\n");
		return;		
	}

	stat = rpmsg_sendto(app_rp_chnl, pattern, strlen(pattern), 0x12);
	if (stat != 0) 
    {
        OPEN_AMP_MSG("rpmsg_send error:%d\r\n", stat);
    }
}

void rpmsg_channel_created(struct rpmsg_channel *rp_chnl) 
{
	OPEN_AMP_MSG("Channel %s @0x%08x is created\r\n", rp_chnl->name, rp_chnl);
    app_rp_chnl = rp_chnl;
    rpmsg_create_ept(rp_chnl, rpmsg_echo_cb, RPMSG_NULL, 0x0F);
	rpmsg_create_ept(rp_chnl, rpmsg_read_matrix_cb, RPMSG_NULL, 0x10);
	rpmsg_create_ept(rp_chnl, rpmsg_read_result_cb, RPMSG_NULL, 0x11);
    rpmsg_create_ept(rp_chnl, rpmsg_cb, RPMSG_NULL, 0x12);
}

void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl) 
{
	OPEN_AMP_MSG("Channel %s is deleted, delete endpoint as well\r\n", rp_chnl->name);
    rpmsg_destroy_ept(rp_ept);
	rp_ept = NULL;
}


#define SAMPLE_PKT_MAX_LEN (RPMSG_BUFFER_SIZE - sizeof(struct rpmsg_hdr))
unsigned char sample_pkt[SAMPLE_PKT_MAX_LEN];
void send_sample_packet(char* pattern, int pattern_len, int repeat_count)
{
	int i, sum = 0,stat;
    
	if ((0 == pattern) || (0>= pattern_len) || (0>= repeat_count)){
		OPEN_AMP_MSG("Invalid parameters\r\n");
		return;		
	}
		
	for (i=0; i<repeat_count; i++)
	{
		if ((sum+pattern_len) >= SAMPLE_PKT_MAX_LEN-1 )
			pattern_len = SAMPLE_PKT_MAX_LEN-1 - sum;
		
		memcpy(&sample_pkt[sum], pattern, pattern_len);
		sum += pattern_len;
		if (sum >= SAMPLE_PKT_MAX_LEN-1)
			break;
	}

    sample_pkt[sum]='\0';
	stat = rpmsg_send_offchannel(app_rp_chnl, 0x0F, 0x0F, sample_pkt, sum+1);
	if (stat != 0)
    {
        OPEN_AMP_MSG("rpmsg_send error:%d\r\n", stat);
    }
}


void dump_packets(char* data, int len)
{
	int i;

	if ((0 == data) || (0 > len))
		return;
	
	for (i=0; i<len; i++){
		kprintf("%c", data[i]);
	}
	kprintf("\r\n");
}

#define MAX_SIZE        15
#define NUM_MATRIX      2

typedef struct _matrix
{
    unsigned int size;
    unsigned short elements[MAX_SIZE][MAX_SIZE];
} matrix;

static matrix matrix_array[NUM_MATRIX];
static matrix matrix_result;

static void matrix_print(matrix *m)
{
    int i, j;

    for (i = 0; i < m->size; ++i) {
        for (j = 0; j < m->size; ++j)
        {
            kprintf(" %5d ", (unsigned short)m->elements[i][j]);
        }
        kprintf("\r\n");
    }
}

static void generate_matrix(unsigned int matrix_size, matrix *p_matrix)
{
    int	j, k;

    p_matrix->size = matrix_size;
    for (j = 0; j < matrix_size; j++) 
    {
        for (k = 0; k < matrix_size; k++) 
        {
            p_matrix->elements[j][k] = (tickGet() + 10*j+k) & 0x3F;
            kprintf(" %5d ",p_matrix->elements[j][k]);
        }
        kprintf("\r\n");
    }
}

static void matrix_multiply(const matrix *m, const matrix *n, matrix *r)
{
    int i, j, k;

    OPEN_AMP_MSG("%d x %d matrix multiply\r\n", m->size, n->size);
    r->size = m->size;

    for (i = 0; i < m->size; ++i) {
        for (j = 0; j < n->size; ++j) {
            r->elements[i][j] = 0;
        }
    }

    for (i = 0; i < m->size; ++i) {
        for (j = 0; j < n->size; ++j) {
            for (k = 0; k < r->size; ++k) {
                r->elements[i][j] += m->elements[i][k] * n->elements[k][j];
            }
        }
    }
}

void rpmsg_read_result_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) 
{
    OPEN_AMP_MSG("Read %d bytes from endpoint %d\r\n", len, src);
    memcpy((void *)&matrix_result, data, len);
    OPEN_AMP_MSG("Printing results:\r\n");
    matrix_print(&matrix_result);
}

void rpmsg_read_matrix_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
                void * priv, unsigned long src) 
{
    static int idx = 0;
    OPEN_AMP_MSG("Read %d bytes from endpoint %d\r\n", len, src);

    /* Process received data and multiple matrices. */
    memcpy((void *)&matrix_array[idx], data, len);
    OPEN_AMP_MSG("Printing matrix #%d:\r\n", idx);
    matrix_print(&matrix_array[idx]);
	
    idx++;
    if (idx >= NUM_MATRIX)
    {
        matrix_multiply(&matrix_array[0], &matrix_array[1], &matrix_result);
        OPEN_AMP_MSG("Printing results:\r\n");
        matrix_print(&matrix_result);

        /* Send the result of matrix multiplication back */
        OPEN_AMP_MSG("Send results %d bytes from endpoint %d to %d\r\n", sizeof(matrix),0x10,0x11);
        rpmsg_send_offchannel(app_rp_chnl, 0x10, 0x11, &matrix_result, sizeof(matrix));
        idx = 0;
    }
}

int matrix_multi_test(int num) 
{
    if (num > MAX_SIZE) num = MAX_SIZE;

    OPEN_AMP_MSG("Generate matrix #0: \r\n");
    generate_matrix(num, &matrix_array[0]);
    OPEN_AMP_MSG("Send matrix #0 from endpoint %d to %d\r\n", 0x11, 0x10);
    rpmsg_send_offchannel(app_rp_chnl, 0x11, 0x10, &matrix_array[0], sizeof(matrix));

    taskDelay(10);
    
    OPEN_AMP_MSG("Generate matrix #1: \r\n");
    generate_matrix(num, &matrix_array[1]);
    OPEN_AMP_MSG("Send matrix #1 from endpoint %d to %d\r\n", 0x11, 0x10);
    rpmsg_send_offchannel(app_rp_chnl, 0x11, 0x10, &matrix_array[1], sizeof(matrix));
    
    return 0;
}
void ssp(int cnt)
{
    send_sample_packet("1234567890", strlen("1234567890"), cnt);
}
void mmt(int cnt)
{
    matrix_multi_test(cnt);
}
