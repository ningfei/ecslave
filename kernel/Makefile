TARGET= ecslave
OBJS  = \
../src/ec_cmd_apwr.o \
../src/ec_cmd_aprd.o \
../src/ec_cmd_aprw.o \
../src/ec_cmd_armw.o \
../src/ec_cmd_brd.o \
../src/ec_cmd_brw.o \
../src/ec_cmd_bwr.o \
../src/ec_cmd_fprd.o \
../src/ec_cmd_fprw.o \
../src/ec_cmd_fpwr.o \
../src/ec_cmd_frmw.o \
../src/ec_cmd_lrw.o \
../src/ec_cmd_lrd.o \
../src/ec_cmd_lwr.o \
../src/ec_sii.o \
../src/ec_regs.o \
../src/ec_com.o \
../src/ec_net.o \
../src/ec_coe.o \
../src/ec_mbox.o \
../src/ec_process_data.o \
src/ec_debug.o

CURRENT=`uname -r`
KDIR = /lib/modules/$(CURRENT)/build/
PWD = $(shell pwd) 

obj-m      := $(TARGET).o
$(TARGET)-objs   := $(OBJS)

default:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)
clean:
	-rm -f *.o .*.o.cmd .*.ko.cmd *.[ch]~ Modules.symvers *.order *.*.ko *.ko *.mod.c .*.c  Module.symvers
indent:
	indent -kr -i8 *.[ch]
