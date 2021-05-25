TARGET = x86_64-linux-musl

OUTPUT =

GCC_CONFIG += --enable-default-pie

COMMON_CONFIG += --with-debug-prefix-map=$(CURDIR)=