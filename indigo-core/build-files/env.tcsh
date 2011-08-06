#
# Defines for Broadcom based switch builds, tcsh file
#
# Normally, this file is sourced from from indigo-core

#echo "ERROR: Edit build-files/env(.tcsh) and comment out this line" && exit 1
# Set TOOLS_DIR to where ELDK (or alternative) is found
setenv TOOLS_DIR $HOME/tools
setenv NETGEAR_TOP ${TOOLS_DIR}/iods-ntgr-bld


# TOP_LEVEL_DIR is where core, platform, ui etc are found
setenv TOP_LEVEL_DIR `pwd`/..
# CORE_DIR is normally indigo-core
setenv CORE_DIR `pwd`

if (! -d ${CORE_DIR}/config-files ) then
   echo "TOP_LEVEL_DIR: $TOP_LEVEL_DIR"
   echo "CORE_DIR: $CORE_DIR"
   echo "TOOLS_DIR: TOOLS_DIR"
   echo "CORE_DIR does not appear to be properly defined"
   echo "CORE should be the root directory containing openflow"
   exit 2
endif

