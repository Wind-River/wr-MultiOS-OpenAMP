VxWorks? 7 Recipe Layer for openAMP
===
---

#Overview

OpenAMP is a framework providing software components that enable 
development of software applications for Asymmetric Multiprocessing 
(AMP) systems. It uses libmetal to provide abstractions that allow for 
porting of the OpenAMP Framework to various software environments 
(operating systems and bare metal environments) and machines 
(processors/platforms). It provides Life Cycle Management, and Inter 
Processor Communication capabilities for management of remote compute 
resources and their associated software contexts. Provides a stand alone 
library usable with RTOS and Baremetal software environments. 

This layer is an adapter to make open-source open-amp and libmetal build 
and run as one lib on VxWorks. This layer does not contain the open-amp or 
libmetal source, it only contains all functions required to allow open-amp 
to build and execute on top of VxWorks. Use this layer to add the open-amp 
and libmetal library to your kernel space, and to build the open-amp and 
libmetal as a library.

NOTE: open-amp and libmetal is not part of any VxWorks? product. If you 
need help, use the resources available or contact your Wind River sales 
representative to arrange for consulting services.

#Project License

The source code for this project is provided under the BSD-3-Clause license. 
Text for the open-amp and libmetal applicable license notices can be found in 
the LICENSE_NOTICES.txt file in the project top level directory. Different 
files may be under different licenses. Each source file should include a 
license notice that designates the licensing terms for the respective file.

#Prerequisite(s)

* Install the Wind River? VxWorks? 7 operating system. And ensure the ***Unix 
  Compatibility layer*** was supported in your release. The layer support for 
  porting third party code to VxWorks.

* Ensure the open-amp and libmetal for C source code is available from the 
  following location:

	https://github.com/OpenAMP/open-amp/archive/v *version* .tar.gz
	https://github.com/OpenAMP/libmetal/archive/v *version* .tar.gz

* You must have **patch** installed in addition to the regular build support 
  for VxWorks. 

* Ensure the CMake software have been installed to pre-build libmetal.

#Building and Using

###Setup

Checkout feature of GitHub to place the contents of this repository 
in your VxWorks? install tree at an appropriate location, for example:

***installDir***/vxworks-7/pkgs/ipc/

###VSB

The name of this layer is *OPENAMP*. The layer is not included by
default in VSBs, it must be manually added. One example of how to do
this using the *vxprj* command line tool is:

    $ cd VSB_DIR
    $ vxprj vsb add OPENAMP
    $ make
    
During the build, the open-amp and libmetal source is downloaded and placed 
in the VSB directory:

***VSB_DIR***/3pp/OPENAMP/open-amp-*version*
***VSB_DIR***/3pp/OPENAMP/libmetal-*version*

The source is patched for VxWorks and libraries are built and placed 
in the standard locations in the VSB directory tree.

###VIP

Adding *OPENAMP* to the VSB will build the *openamp* library. The library 
is not automatically added to VIP. There are eight VIP components for OpenAMP.

####OpenAMP library

The OpenAMP library is consisted of *INCLUDE_OPENAMP_RPMSG*, *INCLUDE_OPENAMP_REMOTEPROC*,
*INCLUDE_OPENAMP_VIRTIO*.   included into VxWorks image via the *INCLUDE_OPENAMP_RPMSG* 
VIP component. It can be added like this

    $ cd VIP_DIR
    $ vxprj component add INCLUDE_OPENAMP_RPMSG
    $ make

####OpenAMP load remote vxworks image with ROM Resident 

When the remote target's image will be produced with ROM Resident, it should 
be added the "INCLUDE_OPENAMP_ELFLOAD_REMOTE* VIP component in the remote 
target's VIP project.

####OpenAMP sample

The OpenAMP sample is included into VxWorks image via the *INCLUDE_OPENAMP_SAMPLE_MASTER*
or *INCLUDE_OPENAMP_SAMPLE_REMOTE* VIP component on the master/remote target 
respectively. 

It might be convinient to run the OpenAMP demo through shell, it can be done 
with the VxWorks *sp* command (spawn). Spawning an OpenAMP demo on the master 
target is done like this:

    -> sp rpmsg_test_master,"/romfs/remote_target_image"

this cmd will load remote target image and start master OpenAMP demo, it should 
be to spawn an OpenAMP demo on the remote target like bemow:

    -> sp rpmsg_test_remote

then it can run echo sample between master and remote targets via below commond:

    -> send_sample_packet("1234567890", strlen("1234567890"), 3)

#Legal Notices

All product names, logos, and brands are property of their respective owners. All company, 
product and service names used in this software are for identification purposes only. 
Wind River and VxWorks are registered trademarks of Wind River Systems, Inc. UNIX is a 
registered trademark of The Open Group.

Disclaimer of Warranty / No Support: Wind River does not provide support 
and maintenance services for this software, under Wind River¡¯s standard 
Software Support and Maintenance Agreement or otherwise. Unless required 
by applicable law, Wind River provides the software (and each contributor 
provides its contribution) on an ¡°AS IS¡± BASIS, WITHOUT WARRANTIES OF ANY 
KIND, either express or implied, including, without limitation, any warranties 
of TITLE, NONINFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR 
PURPOSE. You are solely responsible for determining the appropriateness of 
using or redistributing the software and assume any risks associated with 
your exercise of permissions under the license.

