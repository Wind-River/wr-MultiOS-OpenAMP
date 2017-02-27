/* 20comp_openamp.cdf - OpenAMP Components */

/* 
* Copyright (c) 2016-2017 Wind River Systems, Inc. 
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
25jan17,ghl  update for OpenAMP-v201610 (F8373)
01jun16,mw1  Create for OpenAMP-v201604 (F5252).
*/

Folder FOLDER_OPENAMP
{
    NAME            OpenAMP Components
    SYNOPSIS        OpenAMP library components and demo components.
    _CHILDREN       FOLDER_SERVICE
    CHILDREN        INCLUDE_OPENAMP_VIRTIO        \
                    INCLUDE_OPENAMP_REMOTEPROC    \
                    INCLUDE_OPENAMP_RPMSG         \
                    INCLUDE_OPENAMP_SAMPLE_MASTER \
                    INCLUDE_OPENAMP_SAMPLE_REMOTE \
                    INCLUDE_OPENAMP_PROXY_MASTER  \
                    INCLUDE_OPENAMP_PROXY_REMOTE  \
                    INCLUDE_OPENAMP_ELFLOAD_REMOTE
}



