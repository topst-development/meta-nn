FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#SRCREV = "12a109fce2bc7907f566116ecd8e5260c34df83f"
SRC_URI += "${@bb.utils.contains('INVITE_PLATFORM', 'USE_PCIE_EP', 'file://pcie-endpoint.cfg', '',d)} \
            ${@bb.utils.contains('TCC_BSP_FEATURES', 'camera', 'file://camera.cfg', '',d)} \
            ${@bb.utils.contains('TOPST_CAM_MODULE', 'ov5645_t', 'file://ov5645_t.cfg file://ov5645_t.o', '',d)} \
            ${@bb.utils.contains('TOPST_CAM_MODULE', 'arducam', 'file://arducam.cfg file://arducam.o', '',d)} \
            ${@bb.utils.contains('TCC_BSP_FEATURES', 'ir-camera', 'file://0001-fea-Add-IR-Camera-Code-and-IR-Data-Enable.patch file://0001-ref-Modify-camera-format-to-yuv422m.patch', '',d)} \
"
#            file://0001-ref-expand-npu-buffer-size.patch \
#            file://expand-cma-512mb.cfg \
#            "

do_compile:prepend() {
    if ${@bb.utils.contains('TOPST_CAM_MODULE', 'ov5645_t', 'true', 'false', d)}; then
        sed -i 's/\/\/#include \"tcc750x-videoinput-ov5645.dtsi\"/#include \"tcc750x-videoinput-ov5645.dtsi\"/g' ${WORKDIR}/git/arch/arm64/boot/dts/telechips/tcc750x/override/tcc750x-videoinput-camera-module.dtsi
        cp ${WORKDIR}/ov5645_t.o ${WORKDIR}/git/drivers/media/i2c/
    fi

    if ${@bb.utils.contains('TOPST_CAM_MODULE', 'arducam', 'true', 'false', d)}; then
        sed -i 's/\/\/#include \"tcc750x-videoinput-arducam.dtsi\"/#include \"tcc750x-videoinput-arducam.dtsi\"/g' ${WORKDIR}/git/arch/arm64/boot/dts/telechips/tcc750x/override/tcc750x-videoinput-camera-module.dtsi
        cp ${WORKDIR}/arducam.o ${WORKDIR}/git/drivers/media/i2c/
    fi
}

KERNEL_VERSION_SANITY_SKIP="1"

do_validate_branches[noexec] = "1"
