MODULE_NAME := examples
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

DEPENDS             := src/ref-impl/hal
DEPENDS             += src/ref-impl/tls

HDR_REFS            += src

LDFLAGS             := -Bstatic
LDFLAGS             += -liot_sdk
LDFLAGS             += -liot_hal
CFLAGS              := $(filter-out -ansi,$(CFLAGS))
ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
LDFLAGS             += -liot_tls
endif
ifneq (,$(filter -D_PLATFORM_IS_WINDOWS_,$(CFLAGS)))
LDFLAGS             += -lws2_32
CFLAGS              := $(filter-out -DCOAP_COMM_ENABLED,$(CFLAGS))
endif

ifneq (,$(filter -DMQTT_COMM_ENABLED,$(CFLAGS)))
TARGET                          += mqtt-example mqtt-example-rrpc mqtt-example-multithread
SRCS_mqtt-example               := mqtt/mqtt-example.c
SRCS_mqtt-example-rrpc          := mqtt/mqtt_example_rrpc.c
SRCS_mqtt-example-multithread   := mqtt/mqtt_example_multithread.c

    ifneq (,$(filter -DOTA_ENABLED,$(CFLAGS)))
    ifneq (,$(filter -DOTA_SIGNAL_CHANNEL=1,$(CFLAGS)))
    TARGET                      += ota_mqtt-example
    SRCS_ota_mqtt-example       := uOTA/ota_mqtt-example.c
    endif
    endif

    ifneq (,$(filter -DOTA_ENABLED,$(CFLAGS)))
    TARGET                      += uota_app-example
    SRCS_uota_app-example       := uOTA/uota_app-example.c
    endif

    ifneq (,$(filter -DMQTT_SHADOW,$(CFLAGS)))
    TARGET                      += shadow-example
    SRCS_shadow-example         := device-shadow/shadow-example.c
    endif

    ifneq (,$(filter -DSUPPORT_ITLS,$(CFLAGS)))

    LDFLAGS += \
        -litls \
        -lid2client \
        -lkm \
        -lplat_gen \
        -lalicrypto \
        -lmbedcrypto \

    endif

endif

ifneq (,$(filter -DCOAP_COMM_ENABLED,$(CFLAGS)))
TARGET              += coap-example

SRCS_coap-example   := coap/coap-example.c
SRCS                += coap/coap-example.c

    ifneq (,$(filter -DOTA_ENABLED,$(CFLAGS)))
    ifneq (,$(filter -DOTA_SIGNAL_CHANNEL=2,$(CFLAGS)))
    TARGET                  += ota_coap-example
    SRCS_ota_coap-example   := uOTA/ota_coap-example.c
    endif
    endif
endif

ifneq (,$(filter -DHTTP_COMM_ENABLED,$(CFLAGS)))
TARGET              += http-example
SRCS_http-example   := http/http-example.c
endif

ifneq (,$(filter -DHTTP2_COMM_ENABLED,$(CFLAGS)))
TARGET              += http2-example
SRCS_http2-example  := http2/http2-example.c
endif

ifneq (,$(filter -DSUBDEVICE_ENABLED,$(CFLAGS)))
TARGET                  += subdev-example
SRCS_subdev-example     += subdev/subdev-example.c \
                           subdev/subdev_example_api.c
endif

ifneq (,$(filter -DSDK_ENHANCE,$(CFLAGS)))

    ifneq (,$(filter -DCONFIG_DM_DEVTYPE_SINGLE,$(CFLAGS)))
    TARGET                          += linkkit-example-solo
    SRCS_linkkit-example-solo       := linkkit/linkkit_example_solo.c
    endif
    
    ifneq (,$(filter -DCONFIG_DM_DEVTYPE_GATEWAY,$(CFLAGS)))
    TARGET                          += linkkit-example-gw
    SRCS_linkkit-example-gw         += linkkit/linkkit_example_gateway.c \
                                       linkkit/light.c \
                                       linkkit/cJSON.c

    TARGET                          += linkkit-example-gw-test
    SRCS_linkkit-example-gw-test    += linkkit/linkkit_example_gateway_test.c \
                                       linkkit/light.c \
                                       linkkit/cJSON.c
    endif

    TARGET += linkkit-example-countdown
    SRCS_linkkit-example-countdown = linkkit/linkkit_example_cntdown.c
    
    TARGET += linkkit-example-sched
    SRCS_linkkit-example-sched = linkkit/linkkit_example_sched.c
 
endif

ifneq (,$(filter -D_PLATFORM_IS_WINDOWS_,$(CFLAGS)))
    TARGET                          := mqtt-example
    SRCS                            := mqtt/mqtt-example.c
endif

env:
	@echo ""
	@printf -- "-----------------------------------------------------------------\n"
	@$(foreach var,$(SHOW_ENV_VARS),$(call Dump_Var,$(var)))
	@printf -- "-----------------------------------------------------------------\n"
	@echo ""

include $(RULE_DIR)/rules.mk
