ROOTDIR=$(shell pwd)
WORKDIR=$(ROOTDIR)/build

#targets			+= libilop-hal.so
targets			+= afly

.PHONY: targets

all : $(targets)


srcs				+= $(ROOTDIR)/libs/osi/src/cond.c
srcs				+= $(ROOTDIR)/libs/osi/src/mutex.c
srcs				+= $(ROOTDIR)/libs/osi/src/list.c
srcs				+= $(ROOTDIR)/libs/osi/src/lockqueue.c
#srcs				+= $(ROOTDIR)/libs/osi/src/serial.c
#srcs				+= $(ROOTDIR)/libs/osi/src/tcp.c

srcs				+= $(ROOTDIR)/libs/osi/src/nameval.c
srcs				+= $(ROOTDIR)/libs/osi/src/log.c
#srcs				+= $(ROOTDIR)/libs/osi/src/crc.c
srcs				+= $(ROOTDIR)/libs/osi/src/time_utils.c
srcs				+= $(ROOTDIR)/libs/osi/src/timer.c
srcs				+= $(ROOTDIR)/libs/osi/src/hex.c
srcs				+= $(ROOTDIR)/libs/osi/src/parse_argv.c
srcs				+= $(ROOTDIR)/libs/osi/src/file_io.c
srcs				+= $(ROOTDIR)/libs/osi/src/file_event.c
#srcs				+= $(ROOTDIR)/libs/osi/src/filesystem_monitor.c
#srcs				+= $(ROOTDIR)/libs/osi/src/hashmap.c
#srcs				+= $(ROOTDIR)/libs/osi/src/base64.c
#srcs				+= $(ROOTDIR)/libs/osi/src/md5.c
#srcs				+= $(ROOTDIR)/libs/osi/src/des.cpp
#srcs				+= $(ROOTDIR)/libs/osi/src/util.c


srcs				+= $(ROOTDIR)/libs/third/json/src/dump.c
srcs				+= $(ROOTDIR)/libs/third/json/src/error.c
srcs				+= $(ROOTDIR)/libs/third/json/src/hashtable.c
srcs				+= $(ROOTDIR)/libs/third/json/src/hashtable_seed.c
srcs				+= $(ROOTDIR)/libs/third/json/src/json_parser.c
srcs				+= $(ROOTDIR)/libs/third/json/src/load.c
srcs				+= $(ROOTDIR)/libs/third/json/src/memory.c
srcs				+= $(ROOTDIR)/libs/third/json/src/pack_unpack.c
srcs				+= $(ROOTDIR)/libs/third/json/src/strbuffer.c
srcs				+= $(ROOTDIR)/libs/third/json/src/strconv.c
srcs				+= $(ROOTDIR)/libs/third/json/src/utf.c
srcs				+= $(ROOTDIR)/libs/third/json/src/value.c


#srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/api.c
#srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/ip_acl.c
#srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/version.c
#srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/ember.c
#srcs				+= $(ROOTDIR)/libs/third/libhttpd/src/protocol.c

srcs	      := $(subst .cpp,.c,$(srcs))

objs 		 		:= $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(srcs)))


aflysrcs		:= $(ROOTDIR)/main.c
aflysrcs		+= $(ROOTDIR)/rbx/src/cmd.c
aflysrcs		+= $(ROOTDIR)/rbx/src/afly.c
aflysrcs		+= $(ROOTDIR)/rbx/src/afly_profile.c
aflysrcs		+= $(ROOTDIR)/rbx/src/product.c
aflysrcs		+= $(ROOTDIR)/rbx/src/product_sub.c
aflysrcs		+= $(ROOTDIR)/rbx/src/platform.c
aflysrcs		+= $(ROOTDIR)/rbx/src/nxpx.c
#aflysrcs		+= $(ROOTDIR)/rbx/src/zigbee.c
aflysrcs		+= $(ROOTDIR)/rbx/src/uproto.c
aflysrcs		+= $(ROOTDIR)/rbx/src/schedule.c

aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/base64.c  
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_Crypt_Linux.c  
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_TCP_linux.c    
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_UDP_linux.c
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/cJSON.c   
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_OS_linux.c     
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_TLS_mbedtls.c  
aflysrcs		+= $(ROOTDIR)/ilop_sdk/src/hal/kv.c

aflysrcs		+= $(srcs)
aflysrcs    := $(subst .cpp,.c,$(aflysrcs))

aflyobjs 		:= $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(aflysrcs)))
aflyobjs		+= $(ROOTDIR)/ilop_sdk/lib/libilop-tls.a

ilopsrcs 		:= $(ROOTDIR)/ilop_sdk/src/hal/base64.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/cJSON.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_Crypt_Linux.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_OS_linux.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_TCP_linux.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_TLS_mbedtls.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/HAL_UDP_linux.c
ilopsrcs 		+= $(ROOTDIR)/ilop_sdk/src/hal/kv.c

ilopobjs		:= $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.o,$(ilopsrcs)))
ilopobjs		+= $(ROOTDIR)/ilop_sdk/lib/libilop-tls.a


-include $(ROOTDIR)/make/arch.mk
-include $(ROOTDIR)/make/rules.mk


#$(eval $(call LinkLio,libilop-hal.so,$(ilopobjs)))
$(eval $(call LinkApp,afly,$(aflyobjs)))



run :
	./build/afly

scp : 
	scp -P2204 ./build/afly root@192.168.10.230:/root
	#scp -P2200 ./build/libilop-hal.so root@192.168.10.230:/usr/lib
	#scp -P2200 ./ilop_sdk/lib/libilop-sdk.so root@192.168.10.230:/usr/lib
	#scp -P2200 ./files/linkkit.hal root@192.168.10.230:/etc/config/dusun/afly
