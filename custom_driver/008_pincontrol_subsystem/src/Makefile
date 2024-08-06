obj-m:= 007_practice_gpiosysfs.o
CROSS=/home/phong/work/BeagleBone_Green_wireless/bb-kernel/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KERNEL=/home/phong/work/BeagleBone_Green_wireless/bb-kernel/KERNEL/ 
HOST_KERNEL = /lib/modules/$(shell uname -r)/build/
CROSS_APP:=/home/phong/work/BSP_learn/toolchain/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

target:
	make ARCH=arm CROSS_COMPILE=${CROSS} -C ${KERNEL} M=$(PWD) modules
clean:
	make -C $(KERNEL) M=$(PWD) clean
help:
	make ARCH=arm CROSS_COMPILE=${CROSS} -C ${KERNEL} M=$(PWD) help
host:
	make -C $(HOST_KERNEL) M=$(PWD) modules
clean-host:
	make -C $(HOST_KERNEL) M=$(PWD) clean
build_app: gpio.o lcd.o lcd_app.o 
	${CROSS_APP} $^ -o app 
%.o:%.c 
	$(CROSS_APP) -c $< -o $@ -I.
clean_app:
	rm app gpio.o lcd.o lcd_app.o
