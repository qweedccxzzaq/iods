#!/bin/bash
# Bootstrap ppc build for JSON

if [ -z $ELDK_DIR ]; then
    echo "Need ELDK_DIR defined"
    exit 2
fi

export PATH=${PATH}:${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin

if [ -z $CROSS_COMPILE ]; then
    echo "Need CROSS_COMPILE defined"
    exit 3
fi

if [ -z $JSON_INSTALL_DIR ]; then
    export JSON_INSTALL_DIR=`pwd`/ppc
    echo "Using JSON_INSTALL_DIR $JSON_INSTALL_DIR"
fi

# These are for building JSON for ppc
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export MAINCC=${CROSS_COMPILE}gcc
export LINKCC=${CROSS_COMPILE}gcc
export AR=${CROSS_COMPILE}ar
export RANLIB=${CROSS_COMPILE}ranlib
export LDFLAGS='-static'

mkdir -p ${JSON_INSTALL_DIR}
cd ${JSON_INSTALL_DIR}
../configure --host=powerpc-linux --prefix=${JSON_INSTALL_DIR}
cp ../config.h.no-alloc-redef config.h
make
make install
