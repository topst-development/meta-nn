DESCRIPTION = "Telechips Camera Sample Application"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "file://${@bb.utils.contains('TCC_BSP_FEATURES', 'ir-camera', 'git-ir.tar', 'git.tar',d)} \
          "

inherit pkgconfig cmake

DEPENDS += "virtual/kernel"

EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/git"

do_install:append() {

}

FILES:${PN} += " \
	${datadir} \
	"

