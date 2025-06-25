DESCRIPTION = "I2C Application"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "file://i2c-tools-4.3.tar.xz \
		  "

inherit pkgconfig 

#PROVIDES += "i2c-tools"

DEPENDS += "virtual/kernel"

EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/i2c-tools-4.3"

do_install:append() {
#install -m 0755 ${S}/tools/i2c*		${D}${bindir}
	install -d ${D}${bindir}
	install -d ${D}${libdir}

	install -m 0755 ${S}/lib/libi2c.so.0.1.1	${D}${libdir}
	ln -s -r ${D}${libdir}/libi2c.so.0.1.1		${D}${libdir}/libi2c.so.0
	ln -s -r ${D}${libdir}/libi2c.so.0.1.1		${D}${libdir}/libi2c.so

	install -m 0755 ${S}/tools/i2cdetect		${D}/usr/bin
	install -m 0755 ${S}/tools/i2cdump			${D}/usr/bin
	install -m 0755 ${S}/tools/i2cget			${D}/usr/bin
	install -m 0755 ${S}/tools/i2cset			${D}/usr/bin
	install -m 0755 ${S}/tools/i2ctransfer		${D}/usr/bin

}

FILES_${PN} += " \
		${bindir} \
		${libdir} \
		"
