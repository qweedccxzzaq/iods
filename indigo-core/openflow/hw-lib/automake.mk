#
# Hardware library dependency file definitions.
#

if LB4G
#
# Stanford-LB4G library
#
noinst_LIBRARIES += hw-lib/libstanford-lb4g.a

hw_lib_libstanford_lb4g_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/lb4g.c		\
	hw-lib/bcm-platforms/lb4g.h		\
	hw-lib/bcm-platforms/lb4g_led.c

hw_lib_libstanford_lb4g_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)
hw_lib_libstanford_lb4g_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libstanford_lb4g_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libstanford_lb4g_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libstanford_lb4g_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"BMW_MPC8245/PPC603e\"
hw_lib_libstanford_lb4g_a_CPPFLAGS += -DQUANTA_PLAT
hw_lib_libstanford_lb4g_a_CPPFLAGS += -DNO_BCM_56634_A0
hw_lib_libstanford_lb4g_a_CPPFLAGS += -DNO_BCM_56634_B0
hw_lib_libstanford_lb4g_a_CPPFLAGS += -DNO_BCM_56820_A0

endif

if LB6B
#
# Stanford-LB6B library
#
noinst_LIBRARIES += hw-lib/libstanford-lb6b.a

hw_lib_libstanford_lb6b_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/triumph2_ref.c	\
	hw-lib/bcm-platforms/triumph2_ref.h	\
	hw-lib/bcm-platforms/lb6b.c		\
	hw-lib/bcm-platforms/lb6b.h

hw_lib_libstanford_lb6b_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)
hw_lib_libstanford_lb6b_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libstanford_lb6b_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libstanford_lb6b_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"BMW_MPC8245/PPC603e\"
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DQUANTA_PLAT
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DNO_BCM_56634_A0
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DNO_BCM_56634_B0
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libstanford_lb6b_a_CPPFLAGS += -DNO_BCM_56514_A0

endif

if LB9A
#
# Stanford-LB9A library
#
noinst_LIBRARIES += hw-lib/libstanford-lb9a.a

hw_lib_libstanford_lb9a_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/lb9a.c		\
	hw-lib/bcm-platforms/lb9a.h


hw_lib_libstanford_lb9a_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)
hw_lib_libstanford_lb9a_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libstanford_lb9a_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libstanford_lb9a_a_CPPFLAGS += -I $(BCM_SDK)/diagCode/i2cLib
hw_lib_libstanford_lb9a_a_CPPFLAGS += -I $(BCM_SDK)/diagCode/sys_include
hw_lib_libstanford_lb9a_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libstanford_lb9a_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"BMW_MPC8245/PPC603e\"
hw_lib_libstanford_lb9a_a_CPPFLAGS += -DQUANTA_PLAT
hw_lib_libstanford_lb9a_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libstanford_lb9a_a_CPPFLAGS += -DNO_BCM_56514_A0

endif

if LB8
#
# Stanford-LB8 library
#
noinst_LIBRARIES += hw-lib/libstanford-lb8.a

hw_lib_libstanford_lb8_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/lb8.c		\
	hw-lib/bcm-platforms/lb8.h


hw_lib_libstanford_lb8_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)
hw_lib_libstanford_lb8_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libstanford_lb8_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libstanford_lb8_a_CPPFLAGS += -I $(BCM_SDK)/diagCode/i2cLib
hw_lib_libstanford_lb8_a_CPPFLAGS += -I $(BCM_SDK)/diagCode/sys_include
hw_lib_libstanford_lb8_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libstanford_lb8_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"BMW_MPC8245/PPC603e\"
hw_lib_libstanford_lb8_a_CPPFLAGS += -DQUANTA_PLAT
hw_lib_libstanford_lb8_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libstanford_lb8_a_CPPFLAGS += -DNO_BCM_56514_A0

endif


if GSM73XX
#
# GSM73XX library
#
noinst_LIBRARIES += hw-lib/libgsm73xx.a

hw_lib_libgsm73xx_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/gsm73xx.c		\
	hw-lib/bcm-platforms/gsm73xx.h


hw_lib_libgsm73xx_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)
hw_lib_libgsm73xx_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libgsm73xx_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libgsm73xx_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libgsm73xx_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"Unknown/PPC500e\"
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56514_A0
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56634_A0
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56634_B0
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libgsm73xx_a_CPPFLAGS += -DNO_BCM_56514_A0

endif

if T2REF
#
# BCM56634 (Triumph2) Reference design library
#
noinst_LIBRARIES += hw-lib/libbcm56634-ref.a

hw_lib_libbcm56634_ref_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/triumph2_ref.c	\
	hw-lib/bcm-platforms/triumph2_ref.h

hw_lib_libbcm56634_ref_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)

hw_lib_libbcm56634_ref_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libbcm56634_ref_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libbcm56634_ref_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libbcm56634_ref_a_CPPFLAGS += -DNO_BCM_56820_A0
hw_lib_libbcm56634_ref_a_CPPFLAGS += -DNO_BCM_56514_A0
hw_lib_libbcm56634_ref_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"GTO_MPC8548\"

endif

if SCORREF
#
# BCM56820 (Scorpion) Reference design library
#
noinst_LIBRARIES += hw-lib/libbcm56820-ref.a

hw_lib_libbcm56820_ref_a_SOURCES =		\
	hw-lib/bcm-platforms/bcm_platform.h	\
	hw-lib/bcm-platforms/txrx.c		\
	hw-lib/bcm-platforms/txrx.h		\
	hw-lib/bcm-platforms/l2_cache.c		\
	hw-lib/bcm-platforms/hw_drv.c		\
	hw-lib/bcm-platforms/hw_drv.h		\
	hw-lib/bcm-platforms/hw_flow.c		\
	hw-lib/bcm-platforms/hw_flow.h		\
	hw-lib/bcm-platforms/port.c		\
	hw-lib/bcm-platforms/port.h		\
	hw-lib/bcm-platforms/debug.h		\
	hw-lib/bcm-platforms/triumph2_ref.c	\
	hw-lib/bcm-platforms/triumph2_ref.h	\
	hw-lib/bcm-platforms/scorpion_ref.c	\
	hw-lib/bcm-platforms/scorpion_ref.h

hw_lib_libbcm56820_ref_a_CPPFLAGS = $(AM_CPPFLAGS) $(OF_BCM_CPP_FLAGS)

hw_lib_libbcm56820_ref_a_CPPFLAGS += -I hw-lib/bcm-platforms
hw_lib_libbcm56820_ref_a_CPPFLAGS += -I $(BCM_SDK)/include
hw_lib_libbcm56820_ref_a_CPPFLAGS += -I $(BCM_SDK)/systems/bde/linux/include
hw_lib_libbcm56820_ref_a_CPPFLAGS += -DBCM_PLATFORM_STRING=\"GTO_MPC8548\"
hw_lib_libbcm56820_ref_a_CPPFLAGS += -DNO_BCM_56514_A0
hw_lib_libbcm56820_ref_a_CPPFLAGS += -DNO_BCM_56634_A0
hw_lib_libbcm56820_ref_a_CPPFLAGS += -DNO_BCM_56634_B0

endif

