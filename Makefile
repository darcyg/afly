all: lib/libilop-hal.so linkkit-gateway

ALIBABA_SDK=ilop_sdk
ROOMBANKER_SRC=rb
CC?=mipsel-openwrt-linux-gcc

CFLAGS=-Wall -g -O0 -I$(ALIBABA_SDK)/include -L$(ALIBABA_SDK)/lib -Llib  -I$(ALIBABA_SDK)/src/hal
LDFLAGS=-lilop-tls -lilop-sdk -lilop-hal -lpthread -lrt -ldl

lib/libilop-hal.so: $(ALIBABA_SDK)/src/hal/base64.c  $(ALIBABA_SDK)/src/hal/cJSON.c  $(ALIBABA_SDK)/src/hal/HAL_Crypt_Linux.c  $(ALIBABA_SDK)/src/hal/HAL_OS_linux.c  $(ALIBABA_SDK)/src/hal/HAL_TCP_linux.c  $(ALIBABA_SDK)/src/hal/HAL_TLS_mbedtls.c  $(ALIBABA_SDK)/src/hal/HAL_UDP_linux.c  $(ALIBABA_SDK)/src/hal/kv.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ -lilop-tls

linkkit-gateway: $(ROOMBANKER_SRC)/main.c $(ROOMBANKER_SRC)/nxp.c $(ALIBABA_SDK)/src/hal/cJSON.c
	$(CC) $(CFLAGS) -o $@ $^ -lilop-sdk $(LDFLAGS)

clean:
	rm -rf lib/libilop-hal.so linkkit-gateway

