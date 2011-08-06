
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)ld

CFLAGS     = -std=gnu99 -g -Wall -Werror --pedantic

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

KERN_REL_ROOT_DIRECTORY =../quanta-drivers
KERN_BASE_DIRECTORY     =$(KERNDIR)

MODPOST:
	(cd $(KERN_BASE_DIRECTORY) && ./build modules_prepare)


############################## Flash Driver ################################
FLASH_DRV_DIR         = flashDrv
FLASH_DRV_KMOD_SRC    = $(wildcard $(FLASH_DRV_DIR)/*.c)
FLASH_DRV_KMOD        = $(FLASH_DRV_DIR)/$(FLASH_DRV_DIR).ko

$(FLASH_DRV_KMOD): $(FLASH_DRV_KMOD_SRC) MODPOST
	(cd $(KERN_BASE_DIRECTORY) && ./build \
						SUBDIRS=$(KERN_REL_ROOT_DIRECTORY)/$(FLASH_DRV_DIR)/\
						modules)

############################## i2c Driver ################################
I2C_DRV_DIR         = i2cDrv
I2C_DRV_KMOD_SRC    = $(wildcard $(I2C_DRV_DIR)/*.c)
I2C_DRV_KMOD        = $(I2C_DRV_DIR)/$(I2C_DRV_DIR).ko

$(I2C_DRV_KMOD): $(I2C_DRV_KMOD_SRC) MODPOST
	(cd $(KERN_BASE_DIRECTORY) && ./build \
						SUBDIRS=$(KERN_REL_ROOT_DIRECTORY)/$(I2C_DRV_DIR)/\
						modules)

############################## cpu Driver ################################
CPU_DRV_DIR         = cpuDrv
CPU_DRV_KMOD_SRC    = $(wildcard $(CPU_DRV_DIR)/*.c)
CPU_DRV_KMOD        = $(CPU_DRV_DIR)/$(CPU_DRV_DIR).ko

$(CPU_DRV_KMOD): $(CPU_DRV_KMOD_SRC) MODPOST
	(cd $(KERN_BASE_DIRECTORY) && ./build \
						SUBDIRS=$(KERN_REL_ROOT_DIRECTORY)/$(CPU_DRV_DIR)/\
						modules)

############################## PCI Driver ################################
PCI_DRV_DIR         = pciDrv
PCI_DRV_KMOD_SRC    = $(wildcard $(PCI_DRV_DIR)/*.c)
PCI_DRV_KMOD        = $(PCI_DRV_DIR)/$(PCI_DRV_DIR).ko

$(PCI_DRV_KMOD): $(PCI_DRV_KMOD_SRC) MODPOST
	(cd $(KERN_BASE_DIRECTORY) && ./build \
						SUBDIRS=$(KERN_REL_ROOT_DIRECTORY)/$(PCI_DRV_DIR)/\
						modules)

############################## wdt Driver ################################
WDT_DRV_DIR         = wdtDrv
WDT_DRV_KMOD_SRC    = $(wildcard $(WDT_DRV_DIR)/*.c)
WDT_DRV_KMOD        = $(WDT_DRV_DIR)/$(WDT_DRV_DIR).ko

$(WDT_DRV_KMOD): $(WDT_DRV_KMOD_SRC) MODPOST
	(cd $(KERN_BASE_DIRECTORY) && ./build \
						SUBDIRS=$(KERN_REL_ROOT_DIRECTORY)/$(WDT_DRV_DIR)/\
						modules)

PLAT_TARGETS = $(FLASH_DRV_KMOD) $(I2C_DRV_KMOD) $(CPU_DRV_KMOD) $(PCI_DRV_KMOD) $(WDT_DRV_KMOD)

ifndef MODULE_INSTALL_DIR
MODULE_INSTALL_DIR=../rootfs/lib/modules
endif

# fand is not built for IODS
FAND_TARGET=


all: $(PLAT_TARGETS) ${FAND_TARGET}
	cp -f $(PLAT_TARGETS) ${MODULE_INSTALL_DIR}
#	cp -f $(PLAT_TARGETS) ../rootfs/lib/modules

clean:
	rm -f fand fand.unstripped
	find . -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.ko' -o -name '*.cmd' -o -name '.tmp_versions' \
		-o -name '*.o'  -o -name '*.a' -o -name '.depend' -o -name '*.so' \
		| xargs rm -rf
