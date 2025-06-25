DESCRIPTION = "SPI test app"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"
PN = 'spi_example_app'
PV = '0.0.1'

SRC_URI = "file://src/"

inherit pkgconfig cmake

DEPENDS += "virtual/kernel"

EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/src"
B = "${S}/build"

do_install:append() {

}

FILES:${PN} += " \
        ${datadir} \
        "

