obj-$(CONFIG_XEN_NETDEV_BACKEND) := xen-netback.o

xen-netback-y := netback.o xenbus.o interface.o dnp_controller.o
#obj-m +=xen-netback.o
ccflags-y += -DDNP_XEN
#xen-netback-objs :=netback.o xenbus.o interface.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

