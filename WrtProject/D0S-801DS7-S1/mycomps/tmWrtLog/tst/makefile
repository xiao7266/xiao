DIR_LOCAL :=            apps/wrt_log_test


#***********************************************************************
# Do not change the following include
#***********************************************************************
include $(_TMROOT)/sde/environment.mk


#-----------------------------------------------------------------------
# USE_UART_STDIO is an environment switch that you can use when building
# for tm_psos so that stdin, stdout and stderr go out the UART instead
# of through tmcrt to a host.
#
# You must set USE_UART_STDIO to any value in your environment and then
# do a build_exe with HelloWorld as the application to build.  (It's not
# enough to just do a gmake on HellowWorld because different libraries
# are required.)
#
# The UART chosen by psostm250/comps/tmUart is UART 1 in the board file
# (0-based) unless overridden by the "libio/tmUartPort" tmRegistry entry.
#
# Default baud rate and so on are determined in the board file too;
# typically, it's set for 38400 baud, 8 data, 1 stop, no parity, no hw
# flow ctl.
#
# USE_UART_STDIO is available only for tm_psos.
#-----------------------------------------------------------------------

ifneq           ($(_TMTGTCPUCLASS)_$(_TMTGTOSCLASS),tm_psos)
USE_UART_STDIO :=
endif  # ifneq  ($(_TMTGTCPUCLASS)_$(_TMTGTOSCLASS),tm_psos)

ifneq           ($(USE_UART_STDIO),)
LOCAL_CFLAGS += -DUSE_UART_STDIO=1
endif  # ifneq  ($(USE_UART_STDIO),)


#-----------------------------------------------------------------------
# Source environment variables
#-----------------------------------------------------------------------

CXX_SOURCES     =

C_SOURCES       = src/wrt_log_test.c                                                \

VPATH += $(_TMTGTCPUCLASS)_$(_TMTGTOSCLASS)


DIR_INCLUDE	=  ../inc    \

#-----------------------------------------------------------------------
# Required components
#-----------------------------------------------------------------------

REQUIRES        =                                                             \
                tmosal                                                        \
                tmutil                                                        \
                tmml                                                          \
                tmbslCore                                                     \
                tmbslBoards                                                   \
                tmDbg                                                         \
                tmMain                                                        \
                tmbslLan                                                      \


LIBS =                                                                        \
                tmDbg                                                         \
                tmSpOsal                                                      \
                tmutil                                                        \
                tmml                                                          \
                tmbslCore                                                     \
                tmbslBoards                                                   \
                tmMain                                                        \
                tmbslLan                                                      \
                tmWrtLog                                                       \
                

ifneq           ($(USE_UART_STDIO),)

LIBS +=                                                                       \
                tmUart                                                        \
                tmUartStdio                                                   \

endif  # ifneq  ($(USE_UART_STDIO),)


#-------------------------------------------------------------------------------
# Handle the user-defined diversities _tmethdebug_ for monitor type
#-------------------------------------------------------------------------------

include         $(_TMROOT)/inc/mk/mon.mk

#-----------------------------------------------------------------------
# Alternate suffixes specified to mix libary flavors
#-----------------------------------------------------------------------

#_tmDbg_SUFFIX =
#_tmSpOsal_SUFFIX =
#_tmutil_SUFFIX =
#_tmml_SUFFIX =
#_tmbslCore_SUFFIX =
#_tmbslBoards_SUFFIX =
#_tmMain_SUFFIX =

#-----------------------------------------------------------------------
# name of the target to build
#-----------------------------------------------------------------------

TARGET =        wrt_log_test


#-----------------------------------------------------------------------
# Do not change this
#-----------------------------------------------------------------------
all: configuration target

ifneq ($(DIR_CONFIG),_)
include $(DIR_SDE)/$(DIR_CONFIG)/maketarget$(_TMBSL).mk
endif


ifneq           ($(USE_UART_STDIO),)


#-----------------------------------------------------------------------
# Force loading of tmUart and tmUartStdio
#-----------------------------------------------------------------------

_SYS_OBJECTS +=                                                               \
                $(_TMTGTBUILDROOT)/comps/tmUart/tmp/$(DIR_CONFIG)_$(_TMTGTENDIAN)_$(_TMTGTCPUTYPE)$(_tmUart_DIVERSITY)$(_SDE_ARSUFFIX)/src/tmUart.$(_SDE_O) \
                $(_TMTGTBUILDROOT)/comps/tmUartStdio/tmp/$(DIR_CONFIG)_$(_TMTGTENDIAN)_$(_TMTGTCPUTYPE)$(_tmUartStdio_DIVERSITY)$(_SDE_ARSUFFIX)/src/tmUartStdio.$(_SDE_O) \


endif  # ifneq  ($(USE_UART_STDIO),)
