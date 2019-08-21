$(error run there)
IMAGE_INSTALL_DIR:=$(OUTPUTDIR)
-include $(PROJ_ROOT)/../sdk/verify/application/app.mk
.PHONY: rootfs root app
rootfs:root app
root:
	cd rootfs; tar xf rootfs.tar.gz -C $(OUTPUTDIR)
	tar xf busybox/$(BUSYBOX).tar.gz -C $(OUTPUTDIR)/rootfs
	tar xf $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/package/$(LIBC).tar.gz -C $(OUTPUTDIR)/rootfs/lib
	cp $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/dynamic/* $(OUTPUTDIR)/rootfs/lib/

	if [ "$(BENCH)" = "yes" ]; then \
		cp $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/dynamic/libgcc_s.so.1 $(OUTPUTDIR)/rootfs/lib/ ; \
	fi;

	mkdir -p $(OUTPUTDIR)/tvconfig/config
	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/ini/* $(OUTPUTDIR)/tvconfig/config ;\
	fi;
	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/* $(OUTPUTDIR)/tvconfig/config ; \
	else \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/$(BOARD_NAME)/config/fbdev.ini $(OUTPUTDIR)/tvconfig/config ; \
	fi;
	cp -vf $(PROJ_ROOT)/board/$(CHIP)/mmap/$(MMAP) $(OUTPUTDIR)/tvconfig/config/mmap.ini
	cp -rvf $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/config_tool/* $(OUTPUTDIR)/tvconfig/config
	cd $(OUTPUTDIR)/tvconfig/config; chmod +x config_tool; ln -sf config_tool dump_config; ln -sf config_tool dump_mmap
	if [ "$(BOARD)" = "010A" ]; then \
		cp -rf $(PROJ_ROOT)/board/$(CHIP)/pq $(OUTPUTDIR)/tvconfig/config/ ; \
		find  $(OUTPUTDIR)/tvconfig/config/pq/ -type f ! -name "Main.bin" -type f ! -name "Main_Ex.bin" -type f ! -name "Bandwidth_RegTable.bin"| xargs rm -rf ; \
	fi;

	cp -rf $(PROJ_ROOT)/board/$(CHIP)/vdec_fw $(OUTPUTDIR)/tvconfig/config/

	mkdir -p $(OUTPUTDIR)/rootfs/config
	cp -rf etc/* $(OUTPUTDIR)/rootfs/etc

	if [ $(BENCH) = "yes" ]; then \
		cp -rf /home/edie.chen/bench $(OUTPUTDIR)/tvconfig/config ; \
		cp $(OUTPUTDIR)/tvconfig/config/bench/demo.bash $(OUTPUTDIR)/tvconfig/config/bench.sh ; \
		chmod 755 $(OUTPUTDIR)/tvconfig/config/bench.sh ; \
	fi;

	if [ "$(PHY_TEST)" = "yes" ]; then \
		mkdir $(OUTPUTDIR)/tvconfig/config/sata_phy ; \
		cp $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/sata_phy/* $(OUTPUTDIR)/tvconfig/config/sata_phy ; \
	fi;

	mkdir -p $(OUTPUTDIR)/rootfs/lib/modules/
	mkdir -p $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION)

	touch ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mice 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo mouse.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf
	echo event.* 0:0 0660 =input/ >> ${OUTPUTDIR}/rootfs/etc/mdev.conf

	echo export PATH=\$$PATH:/config >> ${OUTPUTDIR}/rootfs/etc/profile
	echo export TERMINFO=/config/terminfo >> ${OUTPUTDIR}/rootfs/etc/profile
	sed -i '/^mount.*/d' $(OUTPUTDIR)/rootfs/etc/profile
	echo mkdir -p /dev/pts >> ${OUTPUTDIR}/rootfs/etc/profile
	echo mount -t sysfs none /sys >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mount -t tmpfs mdev /dev >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mount -t debugfs none /sys/kernel/debug/ >>  $(OUTPUTDIR)/rootfs/etc/profile
	echo mdev -s >> $(OUTPUTDIR)/rootfs/etc/profile

	if [ "$(FPGA)" = "1" ]; then \
		echo mount -t jffs2 /dev/mtdblock1 /config >> $(OUTPUTDIR)/rootfs/etc/profile ; \
	else \
		if [ "$(FLASH_TYPE)"x = "spinand"x  ]; then \
			cp $(PROJ_ROOT)/tools/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/iperf $(OUTPUTDIR)/tvconfig/config ; \
		fi;\
	fi;
	echo -e $(foreach block, $(USR_MOUNT_BLOCKS), "mount -t $($(block)$(FSTYPE)) $($(block)$(MOUNTPT)) $($(block)$(MOUNTTG))\n") >> $(OUTPUTDIR)/rootfs/etc/profile

	chmod 755 $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/debug/*
	cp -rf $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/debug/* $(OUTPUTDIR)/tvconfig/config
	rm -rf $(OUTPUTDIR)/customer/demo.sh
	mkdir -p $(OUTPUTDIR)/customer
	touch $(OUTPUTDIR)/customer/demo.sh
	chmod 755 $(OUTPUTDIR)/customer/demo.sh


	# creat insmod ko scrpit
	if [ -f "$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list" ]; then \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' > $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/kernel_mod_list | sed 's#\(.*\).ko\(.*\)#$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/$(CHIP)/configs/$(PRODUCT)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(FLASH_TYPE)/modules/\1.ko#' | xargs -i cp -rvf {} $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION); \
		echo "#kernel_mod_list" >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	if [ -f "$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list" ]; then \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list | sed 's#\(.*\).ko\(.*\)#$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION); \
		echo "#misc_mod_list" >> $(OUTPUTDIR)/customer/demo.sh; \
		fi;

	if [ -f "$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/.mods_depend" ]; then \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/.mods_depend | sed '2,20s#\(.*\)#insmod /config/modules/$(KERNEL_VERSION)/\1.ko\nif [ $$? -eq 0 ]; then\n	busybox mknod /dev/\1 c $$major $$minor\n	let minor++\nfi\n\n#' >> $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/.mods_depend | sed 's#\(.*\)#$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION); \
		echo "#mi module" >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	if [ -f "$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list_late" ]; then \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list_late | sed 's#\(.*\).ko#insmod /config/modules/$(KERNEL_VERSION)/\1.ko#' >> $(OUTPUTDIR)/customer/demo.sh; \
		cat $(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/misc_mod_list_late | sed 's#\(.*\).ko\(.*\)#$(PROJ_ROOT)/release/$(PRODUCT)/$(CHIP)/$(BOARD)/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/lib/modules/$(KERNEL_VERSION)/\1.ko#' | xargs -i cp -rvf {} $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION); \
		echo "#misc_mod_list_late" >> $(OUTPUTDIR)/customer/demo.sh; \
	fi;

	sed -i 's/mi_common/insmod \/config\/modules\/$(KERNEL_VERSION)\/mi_common.ko\nmajor=\`cat \/proc\/devices \| busybox awk "\\\\$$2==\\""mi"\\" {print \\\\$$1}"\\n`\nminor=0/g' $(OUTPUTDIR)/customer/demo.sh; \
	sed -i '/#mi module/a	major=`cat /proc/devices | busybox awk "\\\\$$2==\\""mi_poll"\\" {print \\\\$$1}"`\nbusybox mknod \/dev\/mi_poll c $$major 0' $(OUTPUTDIR)/customer/demo.sh; \
	if [ $(PHY_TEST) = "yes" ]; then \
		echo -e "\033[41;33;5m !!! Replace "mdrv-sata-host.ko" with "sata_bench_test.ko" !!!\033[0m" ; \
		sed '/mdrv-sata-host/d' $(OUTPUTDIR)/customer/demo.sh > $(OUTPUTDIR)/tvconfig/config/demotemp.sh ; \
		echo insmod /config/sata_phy/sata_bench_test.ko >> $(OUTPUTDIR)/tvconfig/config/demotemp.sh ; \
		cp $(OUTPUTDIR)/tvconfig/config/demotemp.sh $(OUTPUTDIR)/customer/demo.sh ; \
		rm $(OUTPUTDIR)/tvconfig/config/demotemp.sh ; \
	fi;

	# Enable MIU protect on CMDQ buffer as default (While List: CPU)
	# [I5] The 1st 1MB of MIU is not for CMDQ buffer
#	echo 'echo set_miu_block3_status 0 0x70 0 0x100000 1 > /proc/mi_modules/mi_sys_mma/miu_protect' >> $(OUTPUTDIR)/tvconfig/config/demo.sh

#	echo mount -t jffs2 /dev/mtdblock3 /config >> $(OUTPUTDIR)/rootfs/etc/profile
	ln -fs /config/modules/$(KERNEL_VERSION) $(OUTPUTDIR)/rootfs/lib/modules/
	find $(OUTPUTDIR)/tvconfig/config/modules/$(KERNEL_VERSION) -name "*.ko" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;
	find $(OUTPUTDIR)/rootfs/lib/ -name "*.so*" | xargs $(TOOLCHAIN_REL)strip  --strip-unneeded;
	echo mkdir -p /dev/pts >> $(OUTPUTDIR)/rootfs/etc/profile
	echo mount -t devpts devpts /dev/pts >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "busybox telnetd&" >> $(OUTPUTDIR)/rootfs/etc/profile
	
	echo "export TSLIB_TSDEVICE=/dev/input/event0" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "export TSLIB_CALIBFILE=/etc/pointercal" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "export TSLIB_CONFFILE=/etc/ts.conf" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "export TSLIB_PLUGINDIR=/lib/ts" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "export TSLIB_CONSOLEDEVICE=none" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "export TSLIB_FBDEVICE=/dev/fb0" >> $(OUTPUTDIR)/rootfs/etc/profile
	
	echo "if [ -e /customer/demo.sh ]; then" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "    /customer/demo.sh" >> $(OUTPUTDIR)/rootfs/etc/profile
	echo "fi;" >> $(OUTPUTDIR)/rootfs/etc/profile
	
	echo mdev -s >> $(OUTPUTDIR)/customer/demo.sh
	if [ $(BENCH) = "yes" ]; then \
		echo ./config/bench.sh >> $(OUTPUTDIR)/customer/demo.sh ; \
	fi;
	if [ "$(BOARD)" = "011A" ]; then \
		sed -i 's/mi_sys.ko/mi_sys.ko logBufSize=0/g' $(OUTPUTDIR)/customer/demo.sh ;\
	fi;
	cp $(ROOT_PATH)/release/D0S-801DS7-S1/wrt $(OUTPUTDIR)/rootfs/home/ -rf
	cp $(ROOT_PATH)/release/D0S-801DS7-S1/3party/3PARTY_INSTALL/lib/* $(OUTPUTDIR)/rootfs/lib/
	cp $(ROOT_PATH)/release/D0S-801DS7-S1/ts.conf $(OUTPUTDIR)/rootfs/etc/
	cp $(ROOT_PATH)/release/D0S-801DS7-S1/wrt_room $(OUTPUTDIR)/customer/
	echo "/customer/wrt_room" >> $(OUTPUTDIR)/customer/demo.sh
	
	mkdir -p $(OUTPUTDIR)/vendor
	mkdir -p $(OUTPUTDIR)/customer
	mkdir -p $(OUTPUTDIR)/rootfs/vendor
	mkdir -p $(OUTPUTDIR)/rootfs/customer
