DESCRIPTION = "Xsimd CPP Library for NEON"
LICENSE = "BSD-3-Clause"

LIC_FILES_CHKSUM = "file://LICENSE;md5=88b0e6c5e0cfdc34a62377d454240611"

SRCREV = "${PV}"

SRC_URI = "git://github.com/xtensor-stack/xsimd;protocol=https;branch=master"

inherit cmake

S = "${WORKDIR}/git"

DEPENDS += "xtl"
