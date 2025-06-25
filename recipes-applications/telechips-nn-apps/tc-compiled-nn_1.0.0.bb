DESCRIPTION = "Telechips Compiled NN Package"
SECTION = "applications"
LICENSE = "Telechips"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta-telechips/meta-core/licenses/Telechips;md5=e23a23ed6facb2366525db53060c05a4"

SRC_URI = "${TELECHIPS_AUTOMOTIVE_GIT}/Backup-tc-compiled-nn.git;protocol=${ALS_GIT_PROTOCOL};branch=${ALS_BRANCH} \
          "

SRCREV = "${AUTOREV}"

inherit pkgconfig cmake

S = "${WORKDIR}/git"

do_install:append() {
    install -d ${D}${datadir}

    for quantized_folder in ${S}/*_quantized; do
        install -d ${D}${datadir}/${quantized_folder##*/}/
        cp ${quantized_folder}/npu_cmd.bin ${D}${datadir}/${quantized_folder##*/}/npu_cmd.bin
        cp ${quantized_folder}/quantized_network.bin ${D}${datadir}/${quantized_folder##*/}/quantized_network.bin
        if [ -d "${quantized_folder}/sample/" ]; then
            cp -r ${quantized_folder}/sample/ ${D}${datadir}/${quantized_folder##*/}/sample/
        fi
    done
    for quantized_folder in ${B}/*_quantized; do
        install -d ${D}${datadir}/${quantized_folder##*/}/
        cp ${quantized_folder}/net.so ${D}${datadir}/${quantized_folder##*/}/net.so
    done
}

FILES:${PN} += " \
    ${datadir}/* \
    "
