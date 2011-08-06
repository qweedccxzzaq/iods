bin_PROGRAMS += secchan/ofprotocol
man_MANS += secchan/ofprotocol.8

secchan_ofprotocol_SOURCES = \
	secchan/discovery.c \
	secchan/discovery.h \
	secchan/emerg-flow.c \
	secchan/emerg-flow.h \
	secchan/fail-open.c \
	secchan/fail-open.h \
	secchan/failover.c \
	secchan/failover.h \
	secchan/in-band.c \
	secchan/in-band.h \
	secchan/port-watcher.c \
	secchan/port-watcher.h \
	secchan/protocol-stat.c \
	secchan/protocol-stat.h \
	secchan/ratelimit.c \
	secchan/ratelimit.h \
	secchan/secchan.c \
	secchan/secchan.h \
	secchan/status.c \
	secchan/status.h \
	secchan/stp-secchan.c \
	secchan/stp-secchan.h
secchan_ofprotocol_LDADD = lib/libopenflow.a $(FAULT_LIBS) $(SSL_LIBS)
secchan_ofprotocol_LDADD += ${TOP_LEVEL_DIR}/cmdsrv/cmdsrv_client.a
secchan_ofprotocol_LDADD += -L${JSON_DIR}/lib -ljson

EXTRA_DIST += secchan/ofprotocol.8.in
DISTCLEANFILES += secchan/ofprotocol.8

secchan_ofprotocol_CPPFLAGS = $(AM_CPPFLAGS)
secchan_ofprotocol_CPPFLAGS += -I${TOP_LEVEL_DIR}/cmdsrv -I${JSON_DIR}/include

include secchan/commands/automake.mk
