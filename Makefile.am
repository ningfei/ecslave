bin_PROGRAMS = ec_slave
ec_slave_SOURCES = \
src/ec_cmd_apwr.c \
src/ec_cmd_fpwr.c \
src/ec_cmd_brd.c \
src/ec_cmd_brw.c \
src/ec_cmd_fprd.c \
src/ec_cmd_lrw.c \
src/ec_cmd_lrd.c \
src/ec_sii.c \
src/ec_regs.c \
src/ecs_slave.c \
src/ec_net.c \
src/ec_coe.c \
src/ec_mbox.c \
src/ec_debug.c

ec_slave_CFLAGS  = -m32 -Wall -Werror
if HAVE_DEBUG
ec_slave_CFLAGS += -D__MAKE_DEBUG__
endif
ec_slave_LDADD = -lrt
