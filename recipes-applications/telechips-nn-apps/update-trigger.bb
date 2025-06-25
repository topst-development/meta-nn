#FILESEXTRAPATHS:prepend := "${THISDIR}/{PN}:"
DESCRIPTION = "System update trigger"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"
SECTION = "applications"

SRC_URI = " \
	file://update-trigger.service \
	file://update-trigger.path \
"

inherit systemd

SYSTEMD_PACKAGES = "${PN}-service ${PN}-monitor"
SYSTEMD_SERVICE:${PN}-service = "update-trigger.service"
SYSTEMD_SERVICE:${PN}-monitor = "update-trigger.path"
SYSTEMD_AUTO_ENABLE:${PN}-service = "disable"

do_install:append() {
	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/update-trigger.service ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/update-trigger.path ${D}${systemd_unitdir}/system
}

PACKAGES += "${PN}-service ${PN}-monitor"

FILES:${PN}-service += "${systemd_unitdir}/system/update-trigger.service"
FILES:${PN}-monitor += "${systemd_unitdir}/system/update-trigger.path"

RDEPENDS:${PN} += "tc-update-app"
