obj-m:= main_n.o
CROSS=/home/phong/work/BeagleBone_Green_wireless/bb-kernel/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KERNEL=/home/phong/work/BeagleBone_Green_wireless/bb-kernel/KERNEL/ 
HOST_KERNEL = /lib/modules/$(shell uname -r)/build/

target:
	make ARCH=arm CROSS_COMPILE=${CROSS} -C ${KERNEL} M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD) clean
help:
	make ARCH=arm CROSS_COMPILE=${CROSS} -C ${KERNEL} M=$(PWD) help
host:
	make -C $(HOST_KERNEL) M=$(PWD) modules
