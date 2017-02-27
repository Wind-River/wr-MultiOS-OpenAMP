# Makefile - makefile for OpenAMP sample code
#
# Copyright (c) 2015 Wind River Systems, Inc. 
# 
# Redistribution and use in source and binary forms, with or without modification, are 
# permitted provided that the following conditions are met: 
# 
# 1) Redistributions of source code must retain the above copyright notice, 
# this list of conditions and the following disclaimer. 
# 
# 2) Redistributions in binary form must reproduce the above copyright notice, 
# this list of conditions and the following disclaimer in the documentation and/or 
# other materials provided with the distribution. 
# 
# 3) Neither the name of Wind River Systems nor the names of its contributors may be 
# used to endorse or promote products derived from this software without specific 
# prior written permission. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
#
# modification history
# --------------------
# 10nov15,m_w  Created for OpenAMP sample code (F5252)
#
# DESCRIPTION
# This file contains the makefile rules for building OpenAMP sample code.
#

LIB_BASE_NAME = OPENAMP

include $(WIND_KRNL_MK)/defs.library.mk

OBJS_ARMARCH7M = rpmsg_test_remote.o rsc_table.o proxy_dev_driver.o proxy_app.o
OBJS_ARMARCH7  = rpmsg_test_master.o proxy_dev_driver.o proxy_app.o
DOC_FILES      = 

include $(wildcard $(sort *.mk))

OBJS=$(OBJS_$(CPU))

include $(WIND_KRNL_MK)/rules.library.mk
