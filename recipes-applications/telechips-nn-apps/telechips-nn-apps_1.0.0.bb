SUMMARY = "Telechips NN Application Package Groups"
LICENSE = "CLOSED"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS:${PN} += "tc-nn-app \
                   camera-app \
                   tc-chiptest-app \
                   i2c_example_app \
                   spi_example_app \
                   i2c-tools \
                   ${@bb.utils.contains('INVITE_PLATFORM', 'USE_PCIE_EP', 'tc-combine-app-client', '',d)} \
                   "
