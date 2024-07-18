#编译hello.c生成内核模块文件hello.ko
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-
obj-m += ioctl.o

KERNEL_PATH=/home/tui/e2000d/kernel/local/phytium-linux-kernel

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules
	$(CROSS_COMPILE)gcc -lpthread -o ioctl_app ioctl_app.c
	cp *.ko ioctl_app /home/tui/e2000d/e2000_d/nfsroot
clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
