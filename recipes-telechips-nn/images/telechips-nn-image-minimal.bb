inherit tcc-nn-image
inherit make_fai

DESCRIPTION = "This image provides nn minimal image"

# install kernel modules
IMAGE_INSTALL:append = " kernel-modules"

# install optee modules
IMAGE_INSTALL:append = "${@bb.utils.contains_any('INVITE_PLATFORM', 'TEE optee', ' packagegroup-telechips-security', '', d)}"

# install imx424 isp firmware
IMAGE_INSTALL:append = "${@bb.utils.contains_any('INVITE_PLATFORM', 'USE_QHD_CAMERA_IMX424 USE_QHD_CAMERA', ' isp-firmware', '', d)}"

# install omx component for video encoding
IMAGE_INSTALL:append = "${@bb.utils.contains_any('TCC_BSP_FEATURES', 'multimedia', ' omx-components', '', d)}"

# disable rngd : Problem that occupies 100% of CPU usage for 20 seconds after booting
BAD_RECOMMENDATIONS = "rng-tools rng-tools-dev"
