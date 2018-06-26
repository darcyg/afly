#!/bin/sh 

OPENWRTDIR=/home/dingyuxiang/projects/openwrt
TOOLCHAIN=${OPENWRTDIR}/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2
TARGET=target-mipsel_24kec+dsp_uClibc-0.9.33.2
KERNELDIR=${OPENWRTDIR}/build_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/linux-ramips_mt7620/linux-3.18.20
STAGING_DIR=${OPENWRTDIR}/staging_dir
CROSS_COMPILE=${OPENWRTDIR}/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-
CC=${OPENWRTDIR}/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc
ARCH=mips
INC=-I${OPENWRTDIR}/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include
LIB=-L${OPENWRTDIR}/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib

export ARCH
export TOOLCHAIN
export TARGET
export STAGING_DIR
export CROSS_COMPILE
export KERNELDIR
export CC
export INC
export LIB
