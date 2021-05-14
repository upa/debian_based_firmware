#!/bin/bash
#
# Copyright (c) 2013-2021 Plat'Home CO., LTD.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY PLAT'HOME CO., LTD. AND CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL PLAT'HOME CO., LTD. AND CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

NOROOT=yes

. `dirname $0`/config.sh

if [ ! -d "${LINUX_SRC}" ]; then
	echo
	echo "Linux source not exists."
	echo
	exit 1
fi

cpunum=$(grep '^processor' /proc/cpuinfo  | wc -l)

if [ "$TARGET" == "obs600" ]; then
	if [ ! -h ${LINUX_SRC}/include/asm ]; then
		cd ${LINUX_SRC}/include
		ln -s ../arch/powerpc/include/asm asm-powerpc
		ln -s asm-powerpc asm
	fi
fi


cd ${LINUX_SRC}
#[ -f "${LINUX_SRC}/localversion-rt" ] && rm -f ${LINUX_SRC}/localversion-rt
rm -f ${LINUX_SRC}/localversion*

if [ ${DEFCONFIG} ]; then
	make ${MAKE_OPTION} ${DEFCONFIG}
else
	make ${MAKE_OPTION} ${TARGET}_defconfig
fi

if [ -f "${LINUX_SRC}/../linux-${KERNEL}.dot.config" ]; then
	cp -f ${LINUX_SRC}/../linux-${KERNEL}.dot.config .config
	make ${MAKE_OPTION} oldconfig
fi

case $TARGET in
obsgem1)
	make -j$((${cpunum}+1)) ${MAKE_OPTION} ${MAKE_IMAGE} modules dtbs
	;;
*)
	make -j$((${cpunum}+1)) ${MAKE_OPTION} ${MAKE_IMAGE} modules
	[ -n "$DTBFILE" ] && make ${MAKE_OPTION} $DTBFILE
	;;
esac

case $TARGET in
obsgem1)
	[ ! -d $RELEASEDIR ] && mkdir -p $RELEASEDIR
	[ ! -d $TMPDIR ] && mkdir -p $TMPDIR
	cat ${LINUX_SRC}/arch/${KERN_ARCH}/boot/${MAKE_IMAGE} ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/${DTBFILE} > ${TMPDIR}/${MAKE_IMAGE}.dtb
	touch ${TMPDIR}/rd
	${SKALESDIR}/dtbTool -o ${TMPDIR}/dt.img -s 2048 ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/qcom/
	cp ${LINUX_SRC}/arch/${KERN_ARCH}/boot/dts/${DTBFILE} ${RELEASEDIR}/${TARGET}.dtb
	${SKALESDIR}/mkbootimg	--kernel ${TMPDIR}/${MAKE_IMAGE}.dtb \
						--ramdisk ${TMPDIR}/rd \
						--output ${RELEASEDIR}/boot-obsgem1.img \
						--dt ${TMPDIR}/dt.img \
						--pagesize 2048 \
						--base 0x80000000 \
						--cmdline "root=/dev/mmcblk0p10 rw rootwait console=ttyMSM0,115200n8 noinitrd"
	${SKALESDIR}/mkbootimg	--kernel ${TMPDIR}/${MAKE_IMAGE}.dtb \
						--ramdisk ${TMPDIR}/rd \
						--output ${RELEASEDIR}/sdboot-obsgem1.img \
						--dt ${TMPDIR}/dt.img \
						--pagesize 2048 \
						--base 0x80000000 \
						--cmdline "root=/dev/mmcblk1p9 rw rootwait console=ttyMSM0,115200n8 noinitrd"
#	rm -rf ${TMPDIR}	// move in obsiot_release_firmware.sh
	;;
esac
