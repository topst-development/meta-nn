DESCRIPTION = "Telechips ISP Firmware"
SECTION = "BSP"
LICENSE = "CLOSED"

S = "${WORKDIR}"

do_install() {
	install -d ${D}${base_libdir}/firmware
}

FILES:${PN} += " \
	${base_libdir} \
	"