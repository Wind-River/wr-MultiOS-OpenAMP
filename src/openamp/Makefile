# Makefile for open-amp library
#
# Copyright (c) 2017 Wind River Systems, Inc.
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
# 26jan17,ghl  created for open-amp
#

include $(WIND_KRNL_MK)/defs.ver.mk

PKG_URL = https://github.com/OpenAMP/open-amp/archive/v$(PKG_VER).tar.gz
PKG_DOWN_NAME = v$(PKG_VER)
PKG_NAME = open-amp-$(PKG_VER).tar.gz
PKG_FILE_TYPE = unpack
PKG_DIR_NAME = openamp
PKG_BUILD_DIR = open-amp-$(PKG_VER)/lib

				
include $(WIND_KRNL_MK)/defs.unix.ports.mk

include $(WIND_KRNL_MK)/defs.library.mk

ifeq "$(BUILDSTAGE)" "POSTBUILD"
PKG_BUILD_TARGETS := openamp
else
PKG_BUILD_TARGETS := libopenamp
endif 

#
#include $(WIND_KRNL_MK)/rules.unix.ports.mk
# below is copy from $(WIND_KRNL_MK)/rules.unix.ports.mk

ifndef PKG_BUILD_TARGETS
PKG_BUILD_TARGETS := $(notdir $(realpath .))
$(warning warning: Setting PKG_BUILD_TARGETS based on directory name: '$(PKG_BUILD_TARGETS)')
endif

PATCHES := $(wildcard *.patch)

PATCH_STAMPS := $(addprefix $(PKG_STAMP_DIR)/,  $(addsuffix .applied, $(basename $(PATCHES)))) 

PATCH   ?= patch -p2 -N  
SVN     ?= svn co
GIT     ?= git clone
CVS     ?= cvs checkout

UNZIP    := unzip
UNTAR    := tar xf

OTHER_UNPACK ?= echo "ERROR: Unknown package type, can't extract" ; exit 1 
OTHER_CHECKOUT ?= echo "ERROR: Unknown archive type, can't checkout" ; exit 1 

# Use cURL on windows, and on Linux if it's installed
# otherwise use wget 

ifneq "$(WIND_HOST_TYPE)" "x86-win32"
WHICH_CURL := $(shell which curl 1> /dev/null 2> /dev/null; echo $$?)
CHMOD := chmod -R +w 
else
WHICH_CURL := 0
CHMOD := ATTRIB -R /s /d 
endif

.NOTPARALLEL: 

# -O -J --trust-server-names to let server pick filename (instead of end of odd URL)
# -L follow redirects
ifeq ($(WHICH_CURL),0) 
WGET ?= curl -o $(PKG_NAME) -JL 
else
#WGET ?= wget --trust-server-names 
WGET ?= wget -O $(PKG_NAME)
endif


.PHONY : do_patches $(PKG_BUILD_TARGETS) clean default prebuild _mk


ifeq "$(BUILDSTAGE)" "PREBUILD" 

default : prebuild

else

default : $(PKG_BUILD_TARGETS)

endif

ifeq "$(BUILDSTAGE)" "BUILD"
VXWORKS_MAK:=vxworks.krnl.mak
endif
ifeq "$(BUILDSTAGE)" "BUILD_USR_SHARED"
VXWORKS_MAK:=vxworks.lib.mak
endif

ifeq "$(BUILDSTAGE)" "BUILD_USR_STATIC"
VXWORKS_MAK:=vxworks.lib.mak
endif

ifeq "$(BUILDSTAGE)" "POST_USR_BUILD_STATIC"
VXWORKS_MAK:=vxworks.lib.mak
endif

ifeq "$(BUILDSTAGE)" "POST_USR_BUILD_SHARED"
VXWORKS_MAK:=vxworks.lib.mak
endif




VXWORKS_MAK?=vxworks.mak


# descend into the build directory after all other packaging
# steps are complete and build with vxworks.mak  
#
$(PKG_BUILD_TARGETS):: prebuild 
	@echo building $(VSBL_NAME): $@
	cd $(PKG_SRC_BUILD_DIR) && $(MAKE) -f Makefile

#cd $(VSBL_SRC_DIR) && $(MAKE) -f $(VXWORKS_MAK) $(MAKE_ARGS) VXE=$@ $(MAKECMDGOALS)
#
# Create directories to build in and hold stamps in
# the VSB
#

$(PKG_SRC_BUILD_DIR) : $(VSBL_SRC_DIR)

$(VSBL_SRC_DIR) : $(VSB_SRC_DIR)

$(PKG_STAMP_DIR) : $(VSBL_STAMP_DIR)

$(VSBL_STAMP_DIR) : $(VSBL_SRC_DIR)

$(VSB_USR_H)/public : $(VSB_USR_H)

$(VSB_SRC_DIR) $(VSBL_SRC_DIR) $(VSBL_STAMP_DIR) $(PKG_STAMP_DIR) $(PKG_SRC_BUILD_DIR) $(DOWNLOADS_DIR) $(VSB_USR_H)/public $(VSB_USR_H):
		@test -d $@ || mkdir -p $@


# obtain lock before downloading file
define __flock

if [ -f $1.lock ] ; then echo " Waiting on download $1.lock "; fi ;
while [ -f $1.lock ] ; do sleep 1 ; done ; 
touch $1.lock ;
echo "locked $1.lock"
date +%s.%N  

endef

define __funlock

if [ -f $1.lock ] ; then rm $1.lock ; echo "unlocked $1.lock"; date +%s.%N; fi ;

endef


# rule to download the package sources if they are not present in download directory
# ( both curl and wget leave small files around on failure, so detect and delete those)
#
$(DOWNLOADS_DIR)/$(PKG_NAME) : $(DOWNLOADS_DIR)
	@$(call __flock,$@)
	@if [ ! -f $@ ] ; then \
		echo building $(VSBL_NAME): downloading $(PKG_NAME) ; \
		cd $(DOWNLOADS_DIR) && $(WGET) $(PKG_URL) 2>&1;       \
		sleep 1 ;                                             \
		if [  -f $@ ] ; then                                  \
			if [ `du -k $@ | cut -f 1` -le 2 ] ; then         \
				rm $@ ;                                       \
				echo  "Download failed, file too small" >&2 ; \
				exit 1 ;                                      \
			fi ;                                              \
		fi ;                                                  \
	fi
	@$(call __funlock,$@)



# Rule to check if source dir is present 
# when package type
# 'unpackaged' is used
#
$(PKG_STAMP_DIR)/direxits.stamp : $(PKG_BUILD_DIR)  $(PKG_STAMP_DIR)
	@touch $@


# suggest SCM download location to Windows users

ifeq  "$(WIND_HOST_TYPE)" "x86-win32"
TOOLCHECK_STAMP := $(PKG_STAMP_DIR)/toolcheck.$(PKG_FILE_TYPE).stamp

TOOLCHECK_FAIL_MESSAGE1 = Error: $(PKG_FILE_TYPE) command not found, cannot obtain sources for layer $(VSBL_NAME). 
TOOLCHECK_FAIL_MESSAGE2 = Please install the $(PKG_FILE_TYPE) client before building. 
ifeq "git" "$(PKG_FILE_TYPE)"
TOOLCHECK_FAIL_MESSAGE3 += A client is avalible from https://git-scm.com/downloads 
endif
ifeq "svn" "$(PKG_FILE_TYPE)"
TOOLCHECK_FAIL_MESSAGE3 += For example, an SVN command line client is avalible with https://tortoisesvn.net/downloads.html 
endif

#
# the tool for checking out source is avalible ?
#
$(TOOLCHECK_STAMP) : $(VSBL_SRC_DIR) $(PKG_STAMP_DIR) 
	@if [ ! -f $@ ] ; then                          \
		which $(PKG_FILE_TYPE) ;                    \
		if [ $$? -ne 0 ] ; then                     \
			echo $(TOOLCHECK_FAIL_MESSAGE1) 1>&2 ;  \
			echo $(TOOLCHECK_FAIL_MESSAGE2) 1>&2 ;  \
			echo $(TOOLCHECK_FAIL_MESSAGE3) 1>&2 ;  \
			exit 1        ;                         \
		fi ;                                        \
	fi
	@touch $@	

endif
#
# check the sources out of repository the first time the package is built
# when package type
# 'checkout' is used
# make it writable so we can apply patches
#
$(PKG_STAMP_DIR)/checkout.stamp : $(VSBL_SRC_DIR) $(PKG_STAMP_DIR) $(TOOLCHECK_STAMP)
	@if [ ! -f $@ ] ; then              \
		echo building $(VSBL_NAME): checking out $(PKG_NAME) ; \
		case '$(PKG_FILE_TYPE)' in      \
			svn)  cd $(VSBL_SRC_DIR) && $(SVN) $(PKG_URL) ;;  \
			git)  cd $(VSBL_SRC_DIR) && $(GIT) $(PKG_URL) ;;  \
			cvs)  cd $(VSBL_SRC_DIR) && $(CVS) $(PKG_URL) ;;  \
			*)    cd $(VSBL_SRC_DIR) && $(OTHER_CHECKOUT)  ;; \
		esac ;                                                \
		$(CHMOD) $(PKG_SRC_BUILD_DIR)  ;                      \
	fi
	@touch $@


# if the package tar ball or zip file is in the download directory
# unpack/explode it into the build directory
# first time though the build
# when package type
# 'unpack' is used

$(PKG_STAMP_DIR)/unpack.stamp:  $(DOWNLOADS_DIR)/$(PKG_NAME) $(PKG_STAMP_DIR) $(VSBL_SRC_DIR)
	@$(call __flock,$(DOWNLOADS_DIR)/$(PKG_NAME))
	@if [ ! -f $@ ] ; then                 \
		echo building $(VSBL_NAME): unpacking $(PKG_NAME) ;       \
		case '$(PKG_NAME)' in              \
			*.zip)     cd $(VSBL_SRC_DIR) && $(UNZIP) $< $(UNZIP_FILES) ;;	  \
			*.tar)     cd $(VSBL_SRC_DIR) && $(UNTAR)  $< $(UNTAR_FILES);;	  \
			*.tar.gz)  cd $(VSBL_SRC_DIR) && gzip -d -c $< | tar -x $(UNTAR_FILES);;  \
			*.tgz)     cd $(VSBL_SRC_DIR) && gzip -d -c $< | tar -x $(UNTAR_FILES);;  \
			*.tar.Z)   cd $(VSBL_SRC_DIR) && zcat -d -c $< | tar -x $(UNTAR_FILES);;  \
			*.tar.bz2) cd $(VSBL_SRC_DIR) && bzip2 -d -c $< | tar -x $(UNTAR_FILES);;  \
			*)  cd $(VSBL_SRC_DIR) && $(OTHER_UNPACK)  ;; \
		esac  ;							   \
	$(CHMOD) $(VSBL_SRC_DIR) ;             \
	fi	
	@$(call __funlock,$(DOWNLOADS_DIR)/$(PKG_NAME))
	@if [ -d "$(PKG_SRC_BUILD_DIR)" ] ; then  \
		touch $@ ;                          \
	fi



#
# an alternative unpack rule for sources that are are packaged
# without filepath information 
# when package type
# 'cd_unpack' is used
# 

$(PKG_STAMP_DIR)/cd_unpack.stamp:  $(DOWNLOADS_DIR)/$(PKG_NAME) $(PKG_SRC_BUILD_DIR) $(PKG_STAMP_DIR)
	@$(call __flock,$(DOWNLOADS_DIR)/$(PKG_NAME))
	@if [ ! -f $@ ] ; then \
		echo building $(VSBL_NAME): unpacking $(PKG_NAME) ;       \
		case '$(PKG_NAME)' in              \
			*.zip)     cd $(PKG_SRC_BUILD_DIR) && $(UNZIP) -u  $< ;;	    \
			*.tar)     cd $(PKG_SRC_BUILD_DIR) && $(UNTAR)  $< ;;		\
			*.tar.gz)  cd $(PKG_SRC_BUILD_DIR) && gzip -d -c $< | tar -x  ;; \
			*.tgz)     cd $(PKG_SRC_BUILD_DIR) && gzip -d -c $< | tar -x  ;; \
			*.tar.bz2) cd $(PKG_SRC_BUILD_DIR) && bzip2 -d -c $< | tar -x  ;; \
			*.tar.Z)   cd $(PKG_SRC_BUILD_DIR) && zcat -d -c $< | tar -x  ;; \
			*)         cd $(PKG_SRC_BUILD_DIR) && $(OTHER_UNPACK)  ;; \
		esac   ;                            \
	fi	
	@$(call __funlock,$(DOWNLOADS_DIR)/$(PKG_NAME))
	@touch $@


# rule to apply patches
# ( don't re-apply whatever you do )
ifneq  "$(WIND_HOST_TYPE)" "x86-win32"

$(PKG_STAMP_DIR)/%.applied : %.patch $(PKG_PATCH_DEPENDS)
	$(warning warning: PKG_STAMP_DIR : apply patch '$(PKG_SRC_PATCH_DIR)' '$(PKG_PATCH_DEPENDS)')
	@if [ ! -f "$@" ]; then         \
		echo "building $(VSBL_NAME): apply " $(<)         ;\
		pwd ;\
		echo $(PKG_SRC_PATCH_DIR)   ;\
		cd $(PKG_SRC_PATCH_DIR)  &&  \
		$(PATCH) < $(THIS_DIR)/$<  ;\
	fi 
	@touch $@ 

else
# bug: windows patch often gets confused and makes bad assert 
# workaround: convert .patch file to DOS/CRLF before applying
# 

$(PKG_STAMP_DIR)/%.applied : %.patch $(PKG_PATCH_DEPENDS)
	@if [ ! -f "$@" ]; then         \
		echo "building $(VSBL_NAME): apply " $(<)         ;\
		cd $(PKG_SRC_PATCH_DIR)  && \
		cp $(THIS_DIR)/$< $(PKG_STAMP_DIR)/$<.dos && \
		unix2dos $(PKG_STAMP_DIR)/$<.dos  && \
		$(PATCH) < $(PKG_STAMP_DIR)/$<.dos  ;\
	fi 
	@touch $@ 

endif



#
# apply patches after sources are placed in build directory
# but before we do the actual build 
#

do_patches: $(PATCH_STAMPS) $(PKG_PATCH_DEPENDS)
	@if [ -f vxworks.mak ]; then  cp vxworks.mak $(PKG_SRC_BUILD_DIR)/vxworks.mak; fi 
	@if [ -f vxworks.rtp.mak ]; then cp vxworks.rtp.mak $(PKG_SRC_BUILD_DIR)/vxworks.rtp.mak; fi
	@if [ -f vxworks.lib.mak ]; then cp vxworks.lib.mak $(PKG_SRC_BUILD_DIR)/vxworks.lib.mak; fi
	@if [ -f vxworks.krnl.mak ]; then cp vxworks.krnl.mak $(VSBL_SRC_DIR)/vxworks.krnl.mak; fi
	@if [ -f config.vx.app ]; then cp config.vx.app $(PKG_SRC_BUILD_DIR)/config.vx.app; fi
	cp -rf $(PKG_SRC_BUILD_DIR)/include/openamp $(VSB_KERNEL_PUBLIC_INCLUDE_DIR) ;


# TODO why not defined?
ifeq ($(VSB_KERNEL_PUBLIC_INCLUDE_DIR),)
VSB_KERNEL_PUBLIC_INCLUDE_DIR=$(VSB_DIR)/krnl/h/public
endif


ifeq "$(BUILDSTAGE)" "PREBUILD" 

ifneq ($(USER_PUBLIC_H_FILES),)

INSTALL_H_DEPENENCE+= $(PKG_STAMP_DIR)/user_files.installed

endif	

ifneq ($(USER_PUBLIC_H_DIRS),)

INSTALL_H_DEPENENCE+= $(PKG_STAMP_DIR)/user_dirs.installed

endif

ifneq ($(KERNEL_PUBLIC_H_FILES),)

INSTALL_H_DEPENENCE+= $(PKG_STAMP_DIR)/kernel_files.installed

endif	

ifneq ($(KERNEL_PUBLIC_H_DIRS),)

INSTALL_H_DEPENENCE+= $(PKG_STAMP_DIR)/kernel_dirs.installed

endif

ifneq ($(PKG_EXTRA_SRC),)

PKG_FILEPATH_EXTRA_SRC := $(addprefix $(PKG_SRC_BUILD_DIR)/, $(PKG_EXTRA_SRC))

$(PKG_FILEPATH_EXTRA_SRC): do_patches 
	@if [ ! -e $@ ]; then  \
		echo  building $(VSBL_NAME): adding source $(notdir $@); \
		cp -r $(notdir $@) $@ ; \
	fi

endif

endif #PREBUILD


define __install_file 

if [ ! -e $2/$1 ]; then  \
	cp -r $(PKG_SRC_BUILD_DIR)/$1 $2/$1 ; \
	echo  building $(VSBL_NAME): installing $1 for $3 ; \
fi

endef

define __install_dir

if [ ! -d $2/$1 ]; then  \
	cp -r $(PKG_SRC_BUILD_DIR)/$1 $2/$1  ; \
	echo  building $(VSBL_NAME): installing $1 for $3  ; \
fi

endef


$(PKG_STAMP_DIR)/kernel_files.installed : do_patches
	@$(foreach file,$(KERNEL_PUBLIC_H_FILES),$(call __install_file,$(file),$(VSB_KERNEL_PUBLIC_INCLUDE_DIR),kernel))
	@touch $@

$(PKG_STAMP_DIR)/user_files.installed : do_patches
	@$(foreach file,$(USER_PUBLIC_H_FILES),$(call __install_file,$(file),$(VSB_USER_PUBLIC_INCLUDE_DIR),user))
	@touch $@


$(PKG_STAMP_DIR)/kernel_dirs.installed : do_patches
	@$(foreach file,$(KERNEL_PUBLIC_H_DIRS),$(call __install_dir,$(file),$(VSB_KERNEL_PUBLIC_INCLUDE_DIR),kernel))
	@touch $@

$(PKG_STAMP_DIR)/user_dirs.installed : do_patches
	@$(foreach file,$(USER_PUBLIC_H_DIRS),$(call __install_dir,$(file),$(VSB_USER_PUBLIC_INCLUDE_DIR),user))
	@touch $@



prebuild: $(INSTALL_H_DEPENENCE) $(PKG_PATCH_DEPENDS) do_patches $(PKG_FILEPATH_EXTRA_SRC) $(PKG_CMAKE_GENERATE)
	
_mk: $(PKG_BUILD_TARGETS) 


