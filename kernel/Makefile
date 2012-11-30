TARGET= ecslave
OBJS  = ec_slave.o \
ec_net.o \
ecat_protocol.o \
ec_debug.o \
ec_device.o \
../stack/ec_cmd_apwr.o \
../stack/ec_cmd_aprd.o \
../stack/ec_cmd_aprw.o \
../stack/ec_cmd_armw.o \
../stack/ec_cmd_brd.o \
../stack/ec_cmd_brw.o \
../stack/ec_cmd_bwr.o \
../stack/ec_cmd_fprd.o \
../stack/ec_cmd_fprw.o \
../stack/ec_cmd_fpwr.o \
../stack/ec_cmd_frmw.o \
../stack/ec_cmd_lrw.o \
../stack/ec_cmd_lrd.o \
../stack/ec_cmd_lwr.o \
../stack/ec_sii.o \
../stack/ec_regs.o \
../stack/ec_coe.o \
../stack/ec_cmd.o \
../stack/ec_mbox.o \
../stack/ec_process_data.o

CURRENT=`uname -r`
KDIR = /lib/modules/$(CURRENT)/build/

obj-m  := $(TARGET).o
ccflags-y += -I$(PWD)/../include/ -I$(PWD)/../stack/ -I$(PWD)/../devices/
$(TARGET)-objs := $(OBJS)

default:
	make -C $(KDIR) M=$(PWD) SUBDIRS=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)
clean:
	-rm -rf *.o .*.o.cmd .*.ko.cmd *.[ch]~ Modules.symvers *.order *.*.ko *.ko *.mod.c .*.c  Module.symvers .tmp_versions/
indent:
	indent -kr -i8 *.[ch]
