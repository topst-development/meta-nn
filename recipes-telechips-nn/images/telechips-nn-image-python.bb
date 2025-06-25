require recipes-telechips-nn/images/telechips-nn-image.bb

DESCRIPTION = "This image provides nn full image with python3"

# Python Packages
IMAGE_INSTALL += "python3 python3-pip python3-numpy"

# NN Sample applications
IMAGE_INSTALL += "telechips-nn-apps"
