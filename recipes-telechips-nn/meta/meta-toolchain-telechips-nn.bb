SUMMARY = "Meta package for building a installable toolchain for Telechips Automotive Linux NN SDK"
LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

inherit populate_sdk

TOOLCHAIN_OUTPUTNAME = "${NN_VERSION}-${TCC_ARCH_FAMILY}-toolchain-${PACKAGE_ARCH}-${SDK_ARCH}-gcc-${SDKGCCVERSION}"
TOOLCHAIN_TARGET_TASK:append = " packagegroup-nn-toolchain-target"
