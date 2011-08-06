#
# SFS Makefile
#

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld

CFLAGS=-Wall

ifeq ($(CROSS_COMPILE),)
# Host, testing configuration
BUILD_DIR=host_build
EXTRA_CFLAGS=-DHOST_TESTING -g
HW_DRV=sfs_host.c
HW_INC=sfs_host.h
SUFFIX=-test
else
# Embedded configuration
BUILD_DIR=target_build
EXTRA_CFLAGS=-g
HW_DRV=sfs_lb4g.c
HW_INC=sfs_lb4g.h
# Copies the result to this directory
SUFFIX=-target
endif

INC_DEPS          = sfs.h $(HW_INC) crc32.h
TARGET            = $(PREFIX)sfsctl$(SUFFIX)
SFS_SRC           = crc32.c $(HW_DRV) sfs.c
SFS_OBJ           = $(addprefix $(BUILD_DIR)/, $(SFS_SRC:%.c=%.o))

all: $(BUILD_DIR) $(BUILD_DIR)/sfsctl
	sudo cp $(BUILD_DIR)/sfsctl $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/sfsctl: $(SFS_OBJ) sfsctl.c $(INC_DEPS)
	$(CC) $(EXTRA_CFLAGS) -o $(BUILD_DIR)/sfsctl sfsctl.c $(SFS_OBJ)

$(BUILD_DIR)/%.o : %.c $(INC_DEPS)
	$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)

debug:
	echo target $(TARGET)
	echo source $(SFS_SRC)
	echo obj $(SFS_OBJ)
	echo cross compile $(CROSS_COMPILE)
	echo build $(BUILD_DIR)

install: all
ifdef SFS_INSTALL_DIR
	cp ${TARGET} ${SFS_INSTALL_DIR}/sfsctl
else
	echo "ERROR: Need SFS_INSTALL_DIR defined"
endif

.phony: all clean debug install
