#
_XDCBUILDCOUNT = 
ifneq (,$(findstring path,$(_USEXDCENV_)))
override XDCPATH = C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/source;C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/kernel/tirtos/packages;C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/source/ti/ble5stack
override XDCROOT = C:/ti/ccs910/xdctools_3_55_02_22_core
override XDCBUILDCFG = ./config.bld
endif
ifneq (,$(findstring args,$(_USEXDCENV_)))
override XDCARGS = 
override XDCTARGETS = 
endif
#
ifeq (0,1)
PKGPATH = C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/source;C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/kernel/tirtos/packages;C:/ti/simplelink_cc2640r2_sdk_4_40_00_10/source/ti/ble5stack;C:/ti/ccs910/xdctools_3_55_02_22_core/packages;..
HOSTOS = Windows
endif
