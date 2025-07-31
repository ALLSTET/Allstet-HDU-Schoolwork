obj-m := test.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# 添加头文件搜索路径
EXTRA_CFLAGS := -I/usr/src/asm

default:
    make -C $(KDIR) M=$(PWD) modules

clean:
    make -C $(KDIR) M=$(PWD) clean