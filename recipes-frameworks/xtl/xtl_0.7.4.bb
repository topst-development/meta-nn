DESCRIPTION = "xtensor-stack XTL CPP Library"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=c12cbcb0f50cce3b0c58db4e3db8c2da"

SRC_URI = "git://github.com/xtensor-stack/xtl;protocol=https;branch=master"
SRCREV = "e697c91e2a3ac571d120d2b093fb3b250d060a7d"

inherit cmake

S = "${WORKDIR}/git"
