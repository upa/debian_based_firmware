#!/bin/bash

. `dirname $0`/config.sh

BUILDDIR=/tmp/obstools.$$

FLASHCFG="flashcfg.c"
if [ "$TARGET" == "obsax3" ] ; then
	MODEL="-DCONFIG_OBSAX3"
elif [ "$TARGET" == "obsa7" ] ; then
	MODEL="-DCONFIG_OBSA6 -DCONFIG_OBSA7"
elif [ "$TARGET" == "obsa6" ] ; then
	MODEL="-DCONFIG_OBSA6"
else
	FLASHCFG="flashcfg_obs600.c"
	if [ ! -h ${LINUX_SRC}/include/asm ]; then
		cd ${LINUX_SRC}/include
		ln -s ../arch/powerpc/include/asm asm-powerpc
	fi
fi

LINUX_INC=$(dirname $0)/../source/${TARGET}/linux-${KERNEL}/include

CFLAGS="-Wall -I$LINUX_INC -DDEBIAN ${MODEL}"

if [ "$TARGET" == "obs600" ]; then
	CFLAGS+=" -DHAVE_PUSHSW_OBS600_H"
else
	CFLAGS+=" -DHAVE_PUSHSW_OBSAXX_H"
fi

if [ "$TARGET" == "obsax3" -a "$DIST" == "wheezy" ] ; then
	CFLAGS+=" -DCONFIG_LINUX_3_2_X"
fi

#if [ "$KERNEL" == "3.11.0" ] ; then
	CFLAGS+=" -DCONFIG_LINUX_3_11_X"
#fi

mkdir -p ${BUILDDIR}

echo "FLASHCFG"
$CC -lz -o ${BUILDDIR}/flashcfg-debian ${FILESDIR}/${FLASHCFG} -DFLASHCFG_S -DEXTRACT_LZMA $CFLAGS

echo "RUNLED"
$CC -o ${BUILDDIR}/runled ${FILESDIR}/runled.c $CFLAGS

echo "PSHD"
$CC -o ${BUILDDIR}/pshd ${FILESDIR}/pshd.c $CFLAGS

echo;echo;echo
(cd ${BUILDDIR}; ls -l flashcfg-debian runled pshd)

cp ${FILESDIR}/flashcfg.sh ${DISTDIR}/usr/sbin/flashcfg
chmod 555 ${DISTDIR}/usr/sbin/flashcfg

if [ "$TARGET" == "obsa6" -o "$TARGET" == "obsax3" ]; then
	cp ${FILESDIR}/usbreset.sh ${DISTDIR}/usr/sbin/usbreset
	chmod 555 ${DISTDIR}/usr/sbin/usbreset
fi

for cmd in flashcfg-debian runled pshd; do
	(cd ${BUILDDIR}; install -c -o root -g root -m 555 $cmd ${DISTDIR}/usr/sbin/$cmd)
	$STRIP ${DISTDIR}/usr/sbin/$cmd
done

rm -rf ${BUILDDIR}
