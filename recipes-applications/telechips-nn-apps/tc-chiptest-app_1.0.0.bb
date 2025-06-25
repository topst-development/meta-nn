DESCRIPTION = "Telechips ND chip test Sample Application"
SECTION = "applications"
LICENSE = "CLOSED"

SRC_URI += "file://src/"

inherit pkgconfig cmake

DEPENDS += "virtual/kernel"
RDEPENDS_${PN} = "mlx-kernel tc-compiled-nn"
EXTRA_OECMAKE += " -DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/src"
B = "${S}/build"

do_install:append() {
}

FILES:${PN} += " \
    ${datadir} \
    "