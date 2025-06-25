require recipes-telechips-nn/images/telechips-nn-image-minimal.bb
inherit ${@bb.utils.contains('INVITE_PLATFORM', 'fw-update', 'make-main-updatedir', '',d)}

DESCRIPTION = "This image provides nn full image"

####### QT
IMAGE_INSTALL += "${@bb.utils.contains("DISTRO_FEATURES", "wayland", "wayland weston-init weston-examples", "", d)}"

QT_EXAMPLES = " \
	qtbase-examples \
	${@bb.utils.contains("INVITE_PLATFORM", "qt5/wayland", "qtwayland-examples", "", d)} \
"

IMAGE_INSTALL += " \
	qtbase \
	qtbase-plugins \
	${@bb.utils.contains("INVITE_PLATFORM", "qt5/wayland", "qtwayland qtwayland-plugins", "", d)} \
"
IMAGE_INSTALL += "${@bb.utils.contains("INVITE_PLATFORM", "qt-examples", " ${QT_EXAMPLES}", "", d)}"
####### 

IMAGE_INSTALL += "${@bb.utils.contains('INVITE_PLATFORM', 'fw-update', 'packagegroup-telechips-nn-update', '', d)}"

IMAGE_INSTALL += "telechips-nn-apps"
