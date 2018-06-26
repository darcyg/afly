all: libilop-hal.so linkkit-gateway

ALIBABA_SDK=ilop_sdk
ROOMBANKER_SRC=rb
CC?=mipsel-openwrt-linux-gcc
INC?=
INC+=-I$(ALIBABA_SDK)/include
LIB?=
LIB+=-L$(ALIBABA_SDK)/lib
#LIB+=-Llib
LIB+=-L./

CFLAGS=-Wall -g -O0 $(INC) $(LIB) -I$(ALIBABA_SDK)/src/hal
LDFLAGS=-lubus -ljson-c -lblobmsg_json -lubox  -ldusun -losi -lilop-tls -lilop-sdk -lilop-hal -lpthread -lrt -ldl

libilop-hal.so: $(ALIBABA_SDK)/src/hal/base64.c  $(ALIBABA_SDK)/src/hal/cJSON.c  $(ALIBABA_SDK)/src/hal/HAL_Crypt_Linux.c  $(ALIBABA_SDK)/src/hal/HAL_OS_linux.c  $(ALIBABA_SDK)/src/hal/HAL_TCP_linux.c  $(ALIBABA_SDK)/src/hal/HAL_TLS_mbedtls.c  $(ALIBABA_SDK)/src/hal/HAL_UDP_linux.c  $(ALIBABA_SDK)/src/hal/kv.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ -lilop-tls
#	mkdir -p lib
#	mv $@ lib/

linkkit-gateway: $(ROOMBANKER_SRC)/main.c $(ROOMBANKER_SRC)/nxp.c $(ROOMBANKER_SRC)/translation.c $(ROOMBANKER_SRC)/linkkit.c $(ROOMBANKER_SRC)/cmd.c $(ALIBABA_SDK)/src/hal/cJSON.c
	$(CC) $(CFLAGS) -o $@ $^ -lilop-sdk $(LDFLAGS)

clean:
	rm -f libilop-hal.so linkkit-gateway

