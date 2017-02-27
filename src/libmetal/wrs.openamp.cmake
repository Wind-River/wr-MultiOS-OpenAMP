 

# Toolchain file for building applications against this VSB.
# Copyright (c) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# Must be used from a Wind River environment, i.e. WIND_HOME and
# related environment variables must be set for the build to succeed.
# This file is designed to reside in (VSB_DIR)/cmake
#
# Usage:
# WIND_HOME/wrenv.linux -p vxworks-7
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/this/file.cmake
# make
#
# Note: This file is GENERATED from a template.
# cmake caches the output of toolchain files. So if a VSB is
# reconfigured to create a new toolchain file, the cmake
# build directory must be cleaned entirely.
#
#

# Sharedlibs require at least cmake-2.8.9; cmake-3.6.2 is recommended.
cmake_minimum_required(VERSION 2.8.7)

#########################################
# WindRiver global configuration:
# Auto-detect the VSB_DIR, and convert environment variables to
# cmake macros with forward slashes as appropriate. This base 
# config is necessary for everything else, and must be done early.
#########################################
file(TO_CMAKE_PATH "$ENV{WIND_HOME}" CMAKE_WIND_HOME)
file(TO_CMAKE_PATH "$ENV{WIND_BASE}" CMAKE_WIND_BASE)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_LIST_DIR}" MY_DIR)


set(CMAKE_VERBOSE_MAKEFILE ON)

### for now, we keep the VxWorks generic / Platform definitions local with the VSB.
### once this is stabilized, those definitions may be moved to the VxWorks installation.
#list(APPEND CMAKE_MODULE_PATH "${WIND_BASE}/VxWorksCMakeModules")
list(APPEND CMAKE_MODULE_PATH "${MY_DIR}")
### Below could be moved to a global include under WIND_HOME at some point.
### Another option is running wrenv from inside cmake, to make this even
### more independent - then only WIND_HOME must be set from the outside.
set(CMAKE_WIND_HOST_TYPE $ENV{WIND_HOST_TYPE})
set(CMAKE_WIND_VX7_GNU_HOST_TYPE $ENV{WIND_VX7_GNU_HOST_TYPE})
set(CMAKE_WIND_VX7_DIAB_HOST_TYPE $ENV{WIND_VX7_DIAB_HOST_TYPE})
set(CMAKE_WIND_VX7_ICC12_HOST_TYPE $ENV{WIND_VX7_ICC12_HOST_TYPE})
set(CMAKE_WIND_VX7_ICC_HOST_TYPE $ENV{WIND_VX7_ICC_HOST_TYPE})
# If host is on windows, executables end in .EXE
if (WIN32)
	set (EXTENSION ".exe")
endif ()

# Determine the VSB_DIR, if not specified on command-line:
get_property(_IN_TC GLOBAL PROPERTY IN_TRY_COMPILE )
#if(NOT _IN_TC)
  if(NOT DEFINED ${VSB_DIR})
  ##message("inc_dirs = $ENV{VSB_DIR}")
    get_filename_component(_VSB_DIR "${MY_DIR}/.." ABSOLUTE CACHE)
	set(_VSB_DIR $ENV{VSB_DIR})
  else()
    ## TODO how do we force the VSB_DIR into the try-compile run ???
    get_filename_component(_VSB_DIR "${VSB_DIR}" ABSOLUTE CACHE)
  endif()
  set(VSB_DIR ${_VSB_DIR} CACHE PATH "The VSB_DIR")
#endif()


#########################################
# Wind River bdgen counterpart variables:
# This part is designed to be auto-generated inside the VSB during a VSB build.
# Note: When these settings change due to a VSB reconfiguration, the full application
# build tree must be cleaned since cmake caches the toolchain settings.
#
# TODO: These should perhaps be qualified with WRS_ to avoid polluting the namespace.
# But for now, we keep the namespace the same as in Workbench Managed Build,
# to stay close to the WindRiver original macros in Workbench. It should not cause
# much problems since these variables are only needed during cmake initialization phase.
# Once the Platform/VxWorks.cmake has assigned variables to CMAKE properties, the
# "Workbench style macros" should no longer be needed.
#########################################
set(PROJECT_TYPE DKM)
set(DO_STRIP 0)
set(EXPAND_DBG 0)
set(APICHECK_OPTIONS "-warning")
##message("CPU = $ENV{CPU}")
##message("_CC_VERSION = $ENV{_CC_VERSION}")
#set(VX_CPU_FAMILY arm)
set(CPU ${CPU})
set(TOOL_FAMILY ${TOOL})
set(TOOL_VERSION ${TOOL_VERSION})
set(_CC_VERSION ${_CC_VERSION})
set(TOOL ${TOOL})
#TODO preserve $(WIND_HOME)

set(TOOL_TYPE ${BUILD_PATH} CACHE STRING "")
if (${TOOL} MATCHES "diab")
set(TOOL_PATH ${WIND_DIAB_PATH}/${CMAKE_WIND_VX7_DIAB_HOST_TYPE}/bin)
elseif (${TOOL} MATCHES "gnu")
set(TOOL_PATH ${WIND_GNU_PATH}/${CMAKE_WIND_VX7_GNU_HOST_TYPE}/bin)
elseif (${TOOL} MATCHES "icc")
set(TOOL_PATH ${WIND_ICC_PATH}/${CMAKE_WIND_VX7_ICC_HOST_TYPE}/bin)
endif()


set(CC_ARCH_SPEC ${CC_ARCH_SPEC})
set(_VSB_CONFIG_FILE ${VSB_DIR}/h/config/vsbConfig.h)
set(LIBPATH "")
set(LIBS "")
set(DKM_LONGCALL "")
set(APICHECK_CMD "apicheck_listgen.sh")
set(APICHECK 0)

include_directories(${VSB_DIR}/share/h)
include_directories(SYSTEM ${VSB_DIR}/krnl/h/system)
include_directories(SYSTEM ${VSB_DIR}/krnl/h/public)

#add_definitions(-DCPU=_VX_ARMARCH7)
#add_definitions(-DCPU=${CPU})
#add_definitions(-DTOOL_FAMILY=gnu)
#add_definitions(-DTOOL_FAMILY=${TOOL})
#add_definitions(-DTOOL=gnu)
#add_definitions(-DTOOL=${TOOL})
#add_definitions(-D_WRS_KERNEL)
#add_definitions(-D_WRS_VX_SMP)
#add_definitions(-D_WRS_CONFIG_SMP)
#add_definitions(-D_VSB_CONFIG_FILE=\"${VSB_DIR}/h/config/vsbConfig.h\")
#add_definitions(-D_VSB_CONFIG_FILE)
#add_definitions(-DARMEL)
#add_definitions(-DARM_USE_VFP)

# Essential flags for try-compile to work
set(CMAKE_REQUIRED_FLAGS "-t7 -mfpu=vfp -mfloat-abi=softfp -D_VSB_CONFIG_FILE=\\\"${VSB_DIR}/h/config/vsbConfig.h\\\"")
set(CMAKE_REQUIRED_INCLUDES ${VSB_DIR}/share/h ${VSB_DIR}/krnl/h/system ${VSB_DIR}/krnl/h/public)
#set(CMAKE_REQUIRED_DEFINITIONS -DCPU=_VX_ARMARCH7 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL)

#get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#message("inc_dirs = ${inc_dirs}")

# In general, there's a potential issue where the project may want to overwrite the defaults, or append the defaults.
# Note: The -MD -MP must be stripped from original bdgen since cmake does dependencies itself internallly.
# Definitions from here will be used in CMAKE_*_FLAGS, in the Platform/VxWorks.cmake module.
# Compilers need to be defined here, since they need to be known early.
# TODO replace the _DEBUG _RELEASE with respective cmake concept, <CONFIG>.
if(NOT VXWORKS_BUILD_TYPE)
	SET(VXWORKS_BUILD_TYPE DEBUG)
endif()
set(C_FLAGS_DKM_DEBUG "-gdwarf-3")
set(C_FLAGS_DKM_RELEASE "-O2")
#set(C_FLAGS_DKM "-t7 -mfpu=vfp -mfloat-abi=softfp -ansi -fno-zero-initialized-in-bss  -O2 -fno-builtin -Wall -Wsystem-headers    -DCPU=_VX_ARMARCH7 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL -DARMEL -DINET -DARM_USE_VFP  -D_WRS_LIB_BUILD    -D_VSB_CONFIG_FILE=\\\"${VSB_DIR}/h/config/vsbConfig.h\\\"  -I${VSB_DIR}/krnl/h/common -isystem${VSB_DIR}/krnl/h/public -isystem${VSB_DIR}/krnl/h/system -I${VSB_DIR}/share/h  ")

#if (${CFLAGS})
#message("cflags=${CFLAGS}")
#string(REGEX REPLACE "^(-D_VSB_CONFIG_FILE)" "" CFLAGS ${CFLAGS})
#message("cflags=${CFLAGS}")
#endif()

set(C_FLAGS_DKM "-tARMV7MT2MF:vxworks7 -Wa,-Xgnu-thumb -ei5388 -Xclib-optim-off -Xansi -Xlocal-data-area-static-only -Xsystem-headers-warn -W:c++:.CPP  -Xc-new -Xdialect-c89   -XO -Xsize-opt  -ei4177,4301,4550 -ei4177,4550,2273,5387,5388,1546,5849,1824 -ei4111,4171,4188,4191,4513,5457 -Xforce-declarations    -DCPU=${CPU} -DTOOL_FAMILY=${TOOL} -DTOOL=${TOOL} -D_WRS_KERNEL -DARMEL -DINET -DARM_USE_VFP  -D_WRS_LIB_BUILD    -D_VSB_CONFIG_FILE=\\\"${VSB_DIR}/h/config/vsbConfig.h\\\"  -I${VSB_DIR}/krnl/h/common -I${VSB_DIR}/krnl/h/public -I${VSB_DIR}/krnl/h/system -I${VSB_DIR}/share/h  ")
#set(C_FLAGS_DKM ${CFLAGS})
set(CMAKE_C_FLAGS_DEBUG_INIT ${C_FLAGS_DKM_DEBUG})
set(CMAKE_C_FLAGS_RELEASE_INIT ${C_FLAGS_DKM_RELEASE})
#set(CMAKE_C_FLAGS_INIT ${C_FLAGS_DKM} CACHE STRING "")
set(CMAKE_C_FLAGS_INIT ${C_FLAGS_DKM})

set(CMAKE_C_COMPILER ${CC})
set(CMAKE_C_COMPILER_FORCED TRUE)
## GNU and Intel are auto-discovered, but Diab needs manual specification
set(CMAKE_C_COMPILER_ID ${TOOL})
set(CMAKE_C_PLATFORM_ID VxWorks)

set(CXX_FLAGS_DKM_DEBUG "-gdwarf-3")
set(CXX_FLAGS_DKM_RELEASE "-O2")
set(CXX_FLAGS_DKM "-t7 -mfpu=vfp -mfloat-abi=softfp -ansi  -fno-zero-initialized-in-bss  -O2 -fno-builtin -Wall -Wsystem-headers     -DCPU=_VX_ARMARCH7 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL -DARMEL -DINET -DARM_USE_VFP  -D_WRS_LIB_BUILD    -D_VSB_CONFIG_FILE=\\\"${VSB_DIR}/h/config/vsbConfig.h\\\"  -I${VSB_DIR}/krnl/h/common -isystem${VSB_DIR}/krnl/h/public -isystem${VSB_DIR}/krnl/h/system -I${VSB_DIR}/share/h  ")
set(CMAKE_CXX_FLAGS_DEBUG_INIT )
set(CMAKE_CXX_FLAGS_RELEASE_INIT )
set(CMAKE_CXX_FLAGS_INIT )
#set(CMAKE_CXX_COMPILER ${TOOL_PATH}/g++arm)
set(CMAKE_CXX_COMPILER ${CXX})
set(CMAKE_CXX_COMPILER_FORCED TRUE)
## GNU and Intel are auto-discovered, but Diab needs manual specification
set(CMAKE_CXX_COMPILER_ID ${TOOL})
set(CMAKE_CXX_PLATFORM_ID VxWorks)

# VxWorks always preprocesses assembly, even with the .s extension.
# To support that, more variables need to be overridden than for other languages.
set(ASM_DIALECT)
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S;asm)
set(CMAKE_ASM_FLAGS_DEBUG_INIT "-gdwarf-3")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "-O2")
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT "-XO -Xsize-opt -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT "-XO -Xsize-opt -g3 -Xdebug-dwarf3 -Xdebug-inline-on -DNDEBUG")
set(CMAKE_ASM_FLAGS_INIT "-t7 -mfpu=vfp -mfloat-abi=softfp -ansi  -fno-zero-initialized-in-bss  -O2 -fno-builtin     -DCPU=_VX_ARMARCH7 -DTOOL_FAMILY=gnu -DTOOL=gnu -D_WRS_KERNEL -DARMEL -DINET -DARM_USE_VFP  -D_WRS_LIB_BUILD  -xassembler-with-cpp    -D_VSB_CONFIG_FILE=\\\"${VSB_DIR}/h/config/vsbConfig.h\\\"  -I${VSB_DIR}/krnl/h/common -isystem${VSB_DIR}/krnl/h/public -isystem${VSB_DIR}/krnl/h/system -I${VSB_DIR}/share/h  ")
#set(CMAKE_ASM_COMPILER ${CMAKE_WIND_HOME}/compilers/gnu-4.8.1.6/x86-linux2/bin/ccarm)
set(CMAKE_ASM_COMPILER ${AS})
set(CMAKE_ASM_COMPILER_FORCED TRUE)
# Fake compiler ID allows us to avoid GNU overriding our settings again.
set(CMAKE_ASM_COMPILER_ID ${TOOL})
set(CMAKE_ASM_PLATFORM_ID VxWorks)

#BuildSpec.qsp_arm_smp_vsb_ARMARCH7diab_SMP.BuildTool.Partial Image Linker.Command=echo "building $@"; %linkerprefix% #TOOL_PATH#/dld -tARMCORTEXA9MV:vxworks7 -X -r5 %ToolFlags% -o %OutFile% %Objects% #ADDED_OBJECTS# %Libraries% #LIBPATH# #LIBS# #ADDED_LIBPATH# #ADDED_LIBS# && if [ "#EXPAND_DBG#" = "1" ]; then plink "$@";fi
set(PARTIAL_LINK_FLAGS_DKM_TOOL "")
set(PARTIAL_LINK_FLAGS_DKM "-t7 -mfpu=vfp -mfloat-abi=softfp -X -r5 ")
set(PARTIAL_LINK_TOOL "/dld")
#set(HAVE_NO_STDATOMIC_H )
## TODO is there a cmake standard for the NM tool ? These come from Linker.Command
set(WRS_NM nmarm)
set(WRS_NM_FLAGS "")
set(LINK_FLAGS_DKM_DEBUG "-gdwarf-3")
set(LINK_FLAGS_DKM_RELEASE "-O2")
set(LINK_FLAGS_DKM_TOOL "-T /vxnet/hguo2/vx7-repo/vx7-fixes/vxworks-7/build/tool/gnu_4_8_1/krnl/ldscripts/link.OUT")
#BuildSpec.qsp_arm_smp_vsb_ARMARCH7diab_SMP.BuildTool.Linker.Command=echo "building $@";rm -f %OutFile%;#TOOL_PATH#/ddump -Ng %Objects% | tclsh #WIND_BASE#/host/resource/hutils/tcl/munch.tcl -c arm -tags #VSB_DIR#/krnl/tags/dkm.tags > #OBJ_DIR#/ctdt.c;%ccompilerprefix% #TOOL_PATH#/dcc %DebugModeFlags% #CC_ARCH_SPEC# -Xdollar-in-ident -ei4177,4301,4550 -ei4177,4550,2273,5387,5388,1546,5849,1824 -ei4111,4171,4188,4191,4513,5457 -Xforce-declarations  #ADDED_CFLAGS# %Includes% #ADDED_INCLUDES#  %Defines% #DEFINES# -o #OBJ_DIR#/ctdt.o -c #OBJ_DIR#/ctdt.c; %linkerprefix% #TOOL_PATH#/dld -tARMCORTEXA9MV:vxworks7 -X -r5 %ToolFlags% -o %OutFile% #OBJ_DIR#/ctdt.o %Objects% %Libraries% #LIBPATH# #LIBS# #ADDED_LIBPATH# #ADDED_LIBS# && if [ "#EXPAND_DBG#" = "1" ]; then plink "$@";fi
# Need to define this here due to ctdt munching. Removed CMAKE_C_LINK_FLAGS to avoid duplication.
# On Windows with Ninja, we cannot use the "one big sh -c" since file paths use backslashes and we cannot use ; separators.
# On Linux, on the other hand, we cannot have "&&" in the command since it may not be executed by sh.
# Therefore, make the configuration host system dependent:
if("" MATCHES "Windows" AND "" MATCHES "ninja")
  #TODO unclear how this behaves when the number of objects to link gets too large.
  set(CMAKE_C_LINK_EXECUTABLE
	"  -o <TARGET>.partial.o <OBJECTS> && sh -c \"   <TARGET>.partial.o | tclsh $(CMAKE_WIND_BASE)/host/resource/hutils/tcl/munch.tcl -c arm -tags $(VSB_DIR)/krnl/tags/dkm.tags > <TARGET>.ctdt.c \" &&   -t7 -mfpu=vfp -mfloat-abi=softfp -Xdollar-in-ident -ei4177,4301,4550 -ei4177,4550,2273,5387,5388,1546,5849,1824 -ei4111,4171,4188,4191,4513,5457 -Xforce-declarations    -o <TARGET>.ctdt.o -c <TARGET>.ctdt.c && /dld -t7 -mfpu=vfp -mfloat-abi=softfp -X -r5  -o <TARGET> <TARGET>.partial.o <TARGET>.ctdt.o <LINK_LIBRARIES>    ")
else()
  set(CMAKE_C_LINK_EXECUTABLE
	"sh -c '  -o <TARGET>.partial.o <OBJECTS> &&   <TARGET>.partial.o | tclsh $(WIND_BASE)/host/resource/hutils/tcl/munch.tcl -c arm -tags $(VSB_DIR)/krnl/tags/dkm.tags > <TARGET>.ctdt.c &&   -t7 -mfpu=vfp -mfloat-abi=softfp -Xdollar-in-ident -ei4177,4301,4550 -ei4177,4550,2273,5387,5388,1546,5849,1824 -ei4111,4171,4188,4191,4513,5457 -Xforce-declarations    -o <TARGET>.ctdt.o -c <TARGET>.ctdt.c && /dld -t7 -mfpu=vfp -mfloat-abi=softfp -X -r5  -o <TARGET> <TARGET>.partial.o <TARGET>.ctdt.o <LINK_LIBRARIES>    '")
endif()
set(CMAKE_CXX_LINK_EXECUTABLE )

#set(CMAKE_C_LINK_EXECUTABLE
#	"<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")

## CMAKE_AR gets overridden by CMakeFindBinUtils.cmake, therefore set WRS_AR here and replace in VxWorks.cmake
set(CMAKE_AR ${AR} CACHE STRING "")
## CMAKE_AR and strip get set in CMakeFindBinUtils.cmake, help it here by setting prefix and postfix
set(_CMAKE_TOOLCHAIN_PREFIX "")
set(_CMAKE_TOOLCHAIN_SUFFIX "")
set(CMAKE_RANLIB "")
## TODO on Windows, response files would be preferred due to commandline length limit
## TODO extract the crus flags into a FLAGS macro so users can change it
#set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> crus <TARGET> <OBJECTS>")
message("###nothing####")
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> crus <TARGET> <OBJECTS>")
#set(CMAKE_CXX_CREATE_STATIC_LIBRARY " crus <TARGET> <OBJECTS>")
#set(CMAKE_ASM_CREATE_STATIC_LIBRARY " crus <TARGET> <OBJECTS>")

#########################################
# CMake Cross-Compile Configuration (specific)
#########################################
set (CMAKE_SYSTEM_NAME VxWorks)
set (CMAKE_SYSTEM_VERSION 7)
set (CMAKE_CROSSCOMPILING ON)

# TODO this should be computed from the ARMARCH7
set (CMAKE_SYSTEM_PROCESSOR ${VX_CPU_FAMILY})
set (VXWORKS TRUE)
# where is the target environment located
set(CMAKE_FIND_ROOT_PATH ${VSB_DIR}/krnl/h)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
## TODO these are currently in VxWorks.cmake and should perhaps move back
#set(CMAKE_SYSTEM_INCLUDE_PATH "/h/public")
#set(CMAKE_SYSTEM_LIBRARY_PATH "/ARMARCH7/common")
#set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")


#########################################
# Message early if the project type or the VSB appears not to be sane
# to ease troubleshooting, since find_package would fail without VSB
#########################################
if(NOT _IN_TC)
  if(NOT _WRS_TC_LOADED)
    message(STATUS "WIND_HOME:    ${CMAKE_WIND_HOME}")
    message(STATUS "WIND_BASE:    ${CMAKE_WIND_BASE}")
    message(STATUS "VSB_DIR:      ${VSB_DIR}")
    message(STATUS "TOOL_PATH:    ${TOOL_PATH}")
    message(STATUS "_VSB_CONFIG_FILE: ${_VSB_CONFIG_FILE}")
    set(_WRS_TC_LOADED 1)
  endif()
endif()

