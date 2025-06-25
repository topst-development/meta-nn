DESCRIPTION = "Telechips NPU Combine Sample Application"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "${TELECHIPS_AUTOMOTIVE_GIT}/tc-nn-combine-app.git;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH} \
		"

SRCREV = "${AUTOREV}"

PATCHTOOL = "git"
inherit pkgconfig cmake systemd

DEPENDS += "json-c opencv virtual/kernel visionprotocol"
RDEPENDS:${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs mlx-kernel json-c visionprotocol"

EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/git"

# for systemd
# SYSTEMD_PACKAGES = "${PN}"
# SYSTEMD_SERVICE:${PN} = "tc-combine-app-host.service"

do_install:append() {
	# install -d ${D}${datadir}
	# install -d ${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
	${datadir} \
	${systemd_unitdir} \
	"

