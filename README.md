meta-nn, the Yocto layer for TCC750x Linux NN SDK
===================================================================

This layer's purpose is to add NN support when used with Poky.
The goal is to make the TCC750x Linux NN SDK.

Supported Machines
------------------

We do smoke test the builds of the three machines that we currently support:

* TCC750x EVB Board        - emulated machine: tcc7500-main

Build a TCC750x Linux NN SDK image
----------------------------------------------

You can build a TCC750x Linux NN SDK image using the following steps:
If you want more information for build, you can refer to 'TCC750x Linux NN SDK-Getting Started' document

1. Set-up build envrionment and Choose MACHINE
   > $ source poky/meta-telechips/meta-nn/nn-build.sh
   Choose MACHINE
     1. tcc7500-main
   select number(1-1) => 1
   machine(tcc7500-main) selected.
   ...

2. Modify local.conf(default value)
   > $ vi conf/local.conf
   a. set numbers of thread : BB_NUMBER_THREADS(8)
   b. set parallel make : PARALLEL_MAKE(16)
   c. additional install packages : CORE_IMAGE_EXTRA_INSTALL
      > you can install extra packages to telechips-nn-image using CORE_IMAGE_EXTRA_INSTALL

3. Build telechips-nn-image including TCC750x Linux NN SDK components
   > $ bitbake telechips-nn-image

4. Deploy images: build/telechips/tmp/deploy/images/machine
   > bootloader : ca53_bl3.rom
   > kernel : tc-boot-machine.img
   > dtb : chip-linux-lpd4x32x_svx.x-machine.dtb
   > rootfs : telechips-nn-image-machine.ext4(read/write or read-only, default is read-only)
              telechips-nn-image-machine.squashfs(only read-only)

6. To login use these credentials:
   > User - root
   > Password - root
