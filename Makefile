obj-m += hid-rakk-dasig-x.o
hid-rakk-dasig-x-y := src/hid-rakk-dasig-x.o

KDIR := /lib/modules/$(shell uname -r)/build
MODULE_INSTALL_DIR := /lib/modules/$(shell uname -r)/kernel/drivers/hid

all:
	make -C $(KDIR) M=$(PWD) modules
	python3 $(KDIR)/scripts/clang-tools/gen_compile_commands.py -d $(PWD) -o $(PWD)/compile_commands.json
	zstd -f hid-rakk-dasig-x.ko

test: tests/test_hid_rakk_dasig_x
	./tests/test_hid_rakk_dasig_x

tests/test_hid_rakk_dasig_x: tests/test_hid_rakk_dasig_x.c
	$(CC) -Wall -Wextra -Werror -o $@ $<

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f tests/test_hid_rakk_dasig_x

install:
	cp hid-rakk-dasig-x.ko.zst $(MODULE_INSTALL_DIR)/
	depmod -a

uninstall:
	rm -f $(MODULE_INSTALL_DIR)/hid-rakk-dasig-x.ko.zst
	depmod -a

DKMS_NAME := hid-rakk-dasig-x
DKMS_VERSION := 1.0.0
DKMS_SRC := /usr/src/$(DKMS_NAME)-$(DKMS_VERSION)

dkms-install:
	mkdir -p $(DKMS_SRC)
	cp -r src/ $(DKMS_SRC)/
	cp Makefile $(DKMS_SRC)/
	cp dkms.conf $(DKMS_SRC)/
	dkms add $(DKMS_NAME)/$(DKMS_VERSION)
	dkms build $(DKMS_NAME)/$(DKMS_VERSION)
	dkms install $(DKMS_NAME)/$(DKMS_VERSION)

dkms-uninstall:
	dkms remove $(DKMS_NAME)/$(DKMS_VERSION) --all
	rm -rf $(DKMS_SRC)
