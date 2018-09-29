MODULE_NAME := src/protocol/mqtt
ALL_SUB_DIRS := examples src/infra/log src/infra/system src/infra/utils src/protocol/alcs src/protocol/coap src/protocol/http src/protocol/http2 src/protocol/mqtt src/ref-impl/hal src/ref-impl/tls src/sdk-impl src/services/awss src/services/file_upload src/services/linkkit/cm src/services/linkkit/dm src/services/shadow src/services/subdev src/services/uOTA src/tools/linkkit_tsl_convert tests
AR := mipsel-openwrt-linux-ar
CC := mipsel-openwrt-linux-gcc
CFLAGS := -Iexamples -DALCS_ENABLED -DCOAP_OBSERVE_CLIENT_SUPPORTED -DCOAP_OBSERVE_SERVER_SUPPORTED -DCOAP_SERV_MULTITHREAD -DCOAP_USE_PLATFORM_LOG -DCOAP_USE_PLATFORM_MEMORY -DCONFIG_CM_SUPPORT_LOCAL_CONN -DCONFIG_CM_VIA_CLOUD_CONN -DCONFIG_DM_DEVTYPE_SINGLE -DCONFIG_DM_SUPPORT_LOCAL_CONN -DDEPRECATED_LINKKIT -DFORCE_SSL_VERIFY -DMQTT_COMM_ENABLED -DMQTT_DIRECT -DOTA_ENABLED -DOTA_HAL_MODULE -DOTA_SIGNAL_CHANNEL=1 -DOTA_WITH_LINKKIT -DSDK_ENHANCE -DSUPPORT_TLS -DWIFI_AWSS_ENABLED -D_PLATFORM_IS_LINUX_ -I/home/au/all/gwork/openwrt/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include -Wall
COMP_LIB := libiot_sdk.a
COMP_LIB_COMPONENTS := src/infra/log src/infra/system src/infra/utils src/protocol/alcs src/protocol/mqtt src/sdk-impl src/services/awss src/services/linkkit/cm src/services/linkkit/dm src/services/uOTA
CONFIG_VENDOR := mipsel-linux
CROSS_CANDIDATES := AR CC CXX LD OBJCOPY STRIP
CROSS_PREFIX := 
CXX := g++
EXPORT_DIR := include
IMPORT_DIR := prebuilt
INSTALL_DIR := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c/.O/usr
INSTALL_LIB_DIR := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c/.O/usr/lib
KERNEL_DIR := 
LD := mipsel-openwrt-linux-ld
LDFLAGS := -L/home/au/all/gwork/openwrt/staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/lib/ -lpthread -lrt
MAKE_ENV_VARS := 
NOEXEC_CMAKE_DIRS := src/services/linkkit/dm tests
OBJCOPY := objcopy
PACKAGE_DIR := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c/packages
RULE_DIR := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c/build-rules
STRIP := true
SYSROOT_INC := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c/.O/usr/include
TOP_DIR := /home/au/all/gwork/tmp/ali_feiyan/sdk_packs/iotx-sdk-c
WIN32_CMAKE_SKIP := coap-example linkkit_tsl_convert
COMP_LIB_FILES :=  libiot_utils.a  libiot_log.a  libiot_system.a  libsdk_impl.a  libiot_mqtt.a  libiot_ota.a  libawss.a  libiot_alcs.a  libiot_cm.a  libiot_dm.a

include $(RULE_DIR)/settings.mk
include $(CONFIG_TPL)

all:

LIBA_TARGET     := libiot_mqtt.a

HDR_REFS        += src/infra

env:
	@echo ""
	@printf -- "-----------------------------------------------------------------\n"
	@$(foreach var,$(SHOW_ENV_VARS),$(call Dump_Var,$(var)))
	@printf -- "-----------------------------------------------------------------\n"
	@echo ""

include $(RULE_DIR)/rules.mk
