SUMMARY = "Packages for updating NN user partition images"
DESCRIPTION = "The minimal set of packages required to update the Telechips System"
PR = "r17"

PACKAGE_ARCH = "${MACHINE_ARCH}"
inherit packagegroup

RDEPENDS:${PN} = " \
	tc-update-app \
	update-trigger-service \
	update-trigger-monitor \
"
