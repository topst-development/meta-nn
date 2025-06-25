SUMMARY = "Minimal NN requirements"
DESCRIPTION = "The minimal set of packages required to Telechips NN System"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS:${PN} = "\
    base-files \
    base-passwd \
    busybox \
	bash \
	ldd \
	procps \
	dbus \
	kmod \
    netbase \
	util-linux-mount \
	util-linux-umount \
	util-linux-blkid \
	util-linux-fsck \
	tzdata \
	tzdata-posix \
    ${@bb.utils.contains('TCC_BSP_FEATURES', 'network', 'packagegroup-telechips-base-net', '', d)} \
    ${@bb.utils.contains('TCC_BSP_FEATURES', 'camera', 'v4l-utils', '', d)} \
	customize-rootfs \
	${NN_PKG_NAME} \
"

RRECOMMENDS:${PN} = "\
    ${MACHINE_ESSENTIAL_EXTRA_RRECOMMENDS}"
