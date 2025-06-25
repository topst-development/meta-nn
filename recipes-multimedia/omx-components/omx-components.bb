SUMMARY = "These modules are OMX modules for linux"
DESCRIPTION = "OMX modules"
SECTION = "libs"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

DEPENDS = "glib-2.0 vehicle-camera-framework t-codec drm t-util"
RDEPENDS:${PN} += "vehicle-camera-framework"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
inherit cmake pkgconfig

SRC_URI = "${TELECHIPS_AUTOMOTIVE_MULTIMEDIA_GIT}/omx-components.git;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH}; \
          "
SRCREV = "${AUTOREV}"
S = "${WORKDIR}/git"
B = "${S}"

PACKAGES = "${PN}"
PACKAGE_ARCH = "${MACHINE_ARCH}"
FILES:${PN} = " \
    ${libdir}/* \
    ${bindir}/* \
    ${includedir}/* \
    "

EXTRA_OECMAKE += "-DCHIPSET=${TCC_ARCH_FAMILY} "
EXTRA_OECMAKE += "-DLINUX_KERNEL_DIR=${STAGING_KERNEL_DIR} "
EXTRA_OECMAKE += "-DKERNEL_VERSION=${LINUX_VERSION} "
EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=RELEASE "

FILES_SOLIBSDEV = ""
INSANE_SKIP:${PN} += "dev-so"
