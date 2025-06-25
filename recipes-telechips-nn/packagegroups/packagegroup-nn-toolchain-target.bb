DESCRIPTION = "Target packages for Telechips Automotive Linux SDK"

inherit packagegroup

PACKAGEGROUP_DISABLE_COMPLEMENTARY = "1"

RDEPENDS:${PN} += "\
	packagegroup-core-standalone-sdk-target \
	expat-dev \
	base-files \
	glib-2.0-dev \
	dbus-dev \
	dbus-glib-dev \
	boost-dev \
	libusb1-dev \
	taglib-dev \
	python3 \
	libxml2-dev \
	opencv-dev \
	vehicle-camera-framework-dev \
	omx-components \
"
