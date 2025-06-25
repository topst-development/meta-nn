DESCRIPTION = "Telechips Vision Protocol Library"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI =  "${TELECHIPS_AUTOMOTIVE_GIT}/Backup-visionprotocol;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH};"
SRCREV = "${AUTOREV}"

inherit pkgconfig cmake

DEPENDS += ""
RDEPENDS:${PN} = ""

S = "${WORKDIR}/git/C"

do_install:append() {
}

FILES:${PN} += " \
	${datadir} \
	${includedir} \
	${libdir} \
	"

INSANE_SKIP_${PN} = "ldflags "
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
