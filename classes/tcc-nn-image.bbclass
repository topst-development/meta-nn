#
# Copyright (C) Telechips Inc.
#

inherit tcc-base-image

IMAGE_FSTYPES = "ext4"
IMAGE_ROOTFS_EXTRA_SPACE ??= "10240"

LINGUAS_KO_KR = "ko-kr ko-kr.euc-kr"
LINGUAS_EN_GB = "en-gb en-gb.iso-8859-1"
LINGUAS_EN_US = "en-us en-us.iso-8859-1"
IMAGE_LINGUAS = "${LINGUAS_KO_KR} ${LINGUAS_EN_GB} ${LINGUAS_EN_US}"

IMAGE_INSTALL = "${CORE_IMAGE_BASE_INSTALL}"

# telechips nn base install packages
IMAGE_INSTALL += " packagegroup-telechips-nn-base"

# language
IMAGE_INSTALL += "glibc-gconv-utf-16 glibc-gconv-utf-32"
IMAGE_INSTALL += "glibc-gconv-euc-kr glibc-gconv-libksc"
