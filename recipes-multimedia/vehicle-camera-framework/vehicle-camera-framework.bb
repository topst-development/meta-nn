SUMMARY = "vehicle-camera-framework is camera source management system for vehicle"
DESCRIPTION = "vehicle-camera-framework"
SECTION = "vehicle camera framework"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

inherit cmake pkgconfig systemd

DEPENDS = "virtual/kernel nng"
RDEPENDS:${PN} += "nng"

SRC_URI = "${TELECHIPS_AUTOMOTIVE_MULTIMEDIA_GIT}/Backup-vehicle-camera-framework.git;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH}; \
            file://vehicle-camera-framework.service \
            "

SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"
B = "${S}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "${PN}.service"

PATCHTOOL = "git"
PACKAGE_ARCH = "${MACHINE_ARCH}"

EXTRA_OECMAKE += "-DCHIPSET=${TCC_ARCH_FAMILY} "
EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR} "

FILES_SOLIBSDEV = ""
INSANE_SKIP:${PN} += "dev-so"

FILES:${PN} = " \
    ${systemd_unitdir} \
    ${libdir}/*.so* \
    ${bindir}/* \
"

do_install:append() {
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/${PN}.service ${D}${systemd_unitdir}/system
}
