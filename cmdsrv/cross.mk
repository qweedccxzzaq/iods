# Copyright 2011, Big Switch Networks
#
# Cross compile make file for command server

ifndef CROSS_COMPILE
$(error "Need CROSS_COMPILE defined")
endif

ifndef ELDK_DIR
$(error "Need ELDK_DIR defined")
endif

PATH:=${PATH}:${ELDK_DIR}/bin:${ELDK_DIR}/usr/bin

ifdef SDK
SDK_INCLUDE=-I${SDK}/include
endif

ifndef CFG_CFLAGS
$(error "Need CFG_CFLAGS for target SDK")
endif

ifndef OF_SRC_DIR
$(error "Need OF_SRC_DIR defined")
endif

ifndef CMDSRV_DIR
CMDSRV_DIR=$(shell pwd)
endif

# Target is embedded libraries
JSON_TOP_DIR=${CMDSRV_DIR}/json-c-0.9
export JSON_INSTALL_DIR=${JSON_TOP_DIR}/ppc
ifndef JSON_PREBUILT_DIR
JSON_PREBUILT_DIR=${JSON_TOP_DIR}/prebuilt-ppc
endif
JSON_INCLUDE_DIR=${JSON_INSTALL_DIR}/include
INCS += -I${JSON_INCLUDE_DIR} ${SDK_INCLUDE}
CMDSRV_CLIENT=cmdsrv_client.a
CFLAGS += ${CFG_CFLAGS}

CMDSRV_OBJ=cs_port.o rest.o jsonflow.o
CMDSRV_CLIENT_OBJ=rest.o jsonflow.o
OBJ=${CMDSRV_OBJ}

OF_INC=${OF_SRC_DIR}/include
LIB_INC=${OF_SRC_DIR}/lib
INCS += -I${OF_SRC_DIR} -I${OF_INC} -I${LIB_INC}

CC=${CROSS_COMPILE}gcc
AR= ${CROSS_COMPILE}ar rcu
RANLIB= ${CROSS_COMPILE}ranlib
RM= rm -f

CFLAGS += -g -O0 -Wall ${INCS} 
#CFLAGS += -DOF_HW_PLAT -DBCM_HW_PLAT -DWATCHDOG_SUPPORT -DGSM73XX
ifdef BCM_BINARY_RELEASE
CFLAGS += -DBCM_BINARY_RELEASE
endif
LDFLAGS =
LIBS=-ljson

BUILD_TARGETS=cmdsrv_client.a
ifndef IODS_BUILD
BUILD_TARGETS+=cmdsrv.a
endif

all: show ${BUILD_TARGETS}

# Rule to cross compile JSON library
ifdef BUILD_JSON_LIB
${JSON_INCLUDE_DIR}:
	@echo "Compling JSON, log in mkjson.log"
	$(shell (cd ${JSON_TOP_DIR} && ./bootstrap.sh) > mkjson.log 2>&1)
else
${JSON_INCLUDE_DIR}:
	@echo "Using prebuilt JSON code"
	cp -a ${JSON_PREBUILT_DIR}  ${JSON_INSTALL_DIR}
endif

%.o: %.c  ${JSON_INCLUDE_DIR}
	${CC} ${CFLAGS} -c -o $@ $<

cmdsrv.a: cmdsrv.o ${CMDSRV_OBJ} ${JSON_INCLUDE_DIR}
	${AR} $@ cmdsrv.o ${CMDSRV_OBJ}
	${RANLIB} $@

# This is the (currently incomplete) client C library
cmdsrv_client.a: client.o ${CMDSRV_CLIENT_OBJ} ${JSON_INCLUDE_DIR}
	${AR} $@ client.o ${CMDSRV_CLIENT_OBJ}
	${RANLIB} $@

clean:
	${RM} ${OBJ} ${LIB} cmdsrv.a cmdsrv.o cmdsrv_client.a client.o
	${RM} -r ${JSON_INSTALL_DIR}

show:
	@echo "PATH:          ${PATH}"
	@echo "CROSS_COMPILE: ${CROSS_COMPILE}"
	@echo "CFLAGS:        ${CFLAGS}"
	@echo "LDFLAGS:       ${LDFLAGS}"
	@echo "OBJ:           ${OBJ}"

# Dependencies
# TODO: Auto gen these

cs_port.o: cs_port.c cmdsrv.h cs_int.h

rest.o: rest.c rest.h cs_int.h

cmdsrv.o: cmdsrv.c cs_int.h rest.h cmdsrv.h

jsonflow.o: jsonflow.c cmdsrv.h

client.o: client.c cmdsrv_client.h cs_int.h rest.h
