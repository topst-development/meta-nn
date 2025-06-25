DESCRIPTION = "MLX Kernel for NN Application"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "file://mlx_kernel.bin"

do_install:append() {
	install -d ${D}${base_libdir}/firmware
	install -m 0755 ${WORKDIR}/mlx_kernel.bin ${D}${base_libdir}/firmware/mlx_kernel.bin
}

FILES:${PN} += " \
	${base_libdir} \
	"
