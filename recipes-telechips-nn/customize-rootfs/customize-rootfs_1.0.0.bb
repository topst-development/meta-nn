SUMMARY = "Update rootfs for customizing root filesystem"
DESCRIPTION = "Update rootfs for customizing root filesystem"
SECTION = "base"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips-bsp/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
	install -d ${D}${sysconfdir}
	echo "Telechips Automotive Linux NN SDK ${NN_VERSION} (${NN_RELEASE_DATE})" > ${D}${sysconfdir}/alcs-release
}

FILES:${PN} += "${sysconfdir}"
