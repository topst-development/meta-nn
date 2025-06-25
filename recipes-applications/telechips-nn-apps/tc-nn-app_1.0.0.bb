DESCRIPTION = "Telechips NPU Sample Application"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "${TELECHIPS_AUTOMOTIVE_GIT}/tc-nn-app.git;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH} \
           file://tc-nn-app.service \
"

SRCREV = "${AUTOREV}"

PATCHTOOL = "git"
inherit pkgconfig cmake systemd

DEPENDS += "opencv virtual/kernel json-c visionprotocol omx-components"
RDEPENDS:${PN} = "libopencv-core libopencv-imgproc libopencv-highgui libopencv-videoio libopencv-imgcodecs mlx-kernel json-c visionprotocol omx-components tc-compiled-nn"

EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR}"

S = "${WORKDIR}/git"

# for systemd
# SYSTEMD_PACKAGES = "${PN}"
# SYSTEMD_SERVICE:${PN} = "tc-nn-app.service"

do_install:append() {
    install -d ${D}${systemd_unitdir}/system
    install -m 644 ${WORKDIR}/tc-nn-app.service	${D}${systemd_unitdir}/system
}

FILES:${PN} += " \
    ${systemd_unitdir} \
    "
