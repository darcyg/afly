ARCH="MT7620"
CROSSTOOLDIR="/home/au/all/gwork/openwrt"
CROSS="mipsel-openwrt-linux-"
export  STAGING_DIR="${CROSSTOOLDIR}/staging_dir"
export  PATH="${PATH}:${STAGING_DIR}/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin"

CROSS_CFLAGS="-I${CROSSTOOLDIR}/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/include"
CROSS_CFLAGS="-I${CROSSTOOLDIR}/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include"
CROSS_LDFLAGS="-L${CROSSTOOLDIR}/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/usr/lib"
CROSS_LDFLAGS="-L${CROSSTOOLDIR}/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/"


conffile="./src/board/config.mipsel-linux.demo"

rm -rf ${conffile}
touch ${conffile}

ls ./src/board

echo "CONFIG_ENV_CFLAGS = ${CROSS_CFLAGS} -D_PLATFORM_IS_LINUX_ -Wall" >> $conffile
echo "CONFIG_ENV_LDFLAGS = ${CROSS_LDFLAGS} -lpthread -lrt" >> $conffile

echo "OVERRIDE_CC = mipsel-openwrt-linux-gcc" >> $conffile
echo "OVERRIDE_AR = mipsel-openwrt-linux-ar" >> $conffile
echo "OVERRIDE_LD = mipsel-openwrt-linux-ld" >> $conffile

echo "CONFIG_src/ref-impl/hal         :=" >> $conffile
echo "CONFIG_examples                 :=" >> $conffile
echo "CONFIG_tests                    :=" >> $conffile

echo "CONFIG_src/tools/linkkit_tsl_convert :=" >> $conffile 

rm -rf ./.config
make reconfig



rm -rf ./src/ref-impl/hal/os/mipsel-linux
cd ./src/ref-impl/hal/os/
ls
cp -rf ubuntu mipsel-linux
ls
tree -A mipsel-linux/
cd -

make
#cp ../../ilop_sdk/include/linkkit.h ./output/release/include/
ls -1 ./output/release/lib/
ls -1 output/release/include/

#vim src/board/config.mipsel-linux.demo

# CONFIG_src/ref-impl/hal         :=

#make reconfig

#make all

#make 
