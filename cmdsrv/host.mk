# Copyright 2011,  Big Switch Networks
#
# Host version of makefile for command server and test application

CMDSRV_DIR=$(shell pwd)

# Target is Unix host testing
CMDSRV=cmdsrv
CLI=cli
CFLAGS += -DCS_TEST_ENV
OF_SRC_DIR=/home/dtalayco/work/ui-ntgr/indigo-core/openflow

CMDSRV_CLIENT=cmdsrv_client.a
CLI_OBJ=cs_port.o rest.o jsonflow.o client.o
CMDSRV_OBJ=cs_port.o rest.o jsonflow.o
CMDSRV_CLIENT_OBJ=rest.o jsonflow.o
OBJ=${CLI_OBJ} ${CMDSRV_OBJ}

OF_INC=${OF_SRC_DIR}/include
LIB_INC=${OF_SRC_DIR}/lib
INCS += -I${OF_SRC_DIR} -I${OF_INC} -I${LIB_INC}
RM= rm -f

CFLAGS+=-g -O0 -Wall ${INCS}
LIBS=-ljson

CC=gcc
RM=rm -f

all: show ${CMDSRV} ${CLI}

# cmdsrv_client.a

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

cmdsrv: clean ${CMDSRV_OBJ} cmdsrv.c ${JSON_INCLUDE_DIR}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${CMDSRV_OBJ} cmdsrv.c ${LIBS}

cli: ${CLI_OBJ} cli.c
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${CLI_OBJ} cli.c ${LIBS}

# This is the (currently incomplete) client C library
#cmdsrv_client.a: client.o ${CMDSRV_CLIENT_OBJ} ${JSON_INCLUDE_DIR}
#	${AR} $@ client.o ${CMDSRV_CLIENT_OBJ}
#	${RANLIB} $@

clean:
	${RM} ${OBJ} ${LIB} cmdsrv.a cmdsrv.o cmdsrv_client.a client.o

show:
	@echo "PATH:          ${PATH}"
	@echo "CFLAGS:        ${CFLAGS}"
	@echo "LDFLAGS:       ${LDFLAGS}"
	@echo "OBJ:           ${OBJ}"

# Dependencies
# TODO: Auto gen these
cli.c: cs_int.h rest.h

cs_port.o: cs_port.c cmdsrv.h cs_int.h

rest.o: rest.c rest.h cs_int.h

cmdsrv.c: cs_int.h rest.h cmdsrv.h

jsonflow.c: cmdsrv.h

client.o: client.c cmdsrv_client.h cs_int.h rest.h
