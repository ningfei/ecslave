bin_PROGRAMS = ec_slave
ec_slave_SOURCES = \
src/ec_cmd_apwr.c \
src/ec_cmd_aprd.c \
src/ec_cmd_aprw.c \
src/ec_cmd_armw.c \
src/ec_cmd_brd.c \
src/ec_cmd_brw.c \
src/ec_cmd_bwr.c \
src/ec_cmd_fprd.c \
src/ec_cmd_fprw.c \
src/ec_cmd_fpwr.c \
src/ec_cmd_frmw.c \
src/ec_cmd_lrw.c \
src/ec_cmd_lrd.c \
src/ec_cmd_lwr.c \
src/ec_sii.c \
src/ec_regs.c \
src/ec_slave.c \
src/ec_com.c \
src/ec_net.c \
src/ec_coe.c \
src/ec_mbox.c \
src/ec_process_data.c \
src/ec_debug.c

ec_slave_CFLAGS  = -Wall -Werror -O0
if HAVE_DEBUG
ec_slave_CFLAGS += -D__MAKE_DEBUG__
endif
ec_slave_LDADD = -lrt -lpcap
