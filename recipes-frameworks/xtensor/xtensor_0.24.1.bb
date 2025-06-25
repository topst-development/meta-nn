DESCRIPTION = "Xtensor CPP Library"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=5c67ec4d3eb9c5b7eed4c37e69571b93"

SRC_URI = "git://github.com/xtensor-stack/xtensor;protocol=https;branch=master"
SRCREV = "004370c79716a2d7b3cf9d04af947363d813a29f"

inherit cmake

S = "${WORKDIR}/git"

DEPENDS += "xtl"
