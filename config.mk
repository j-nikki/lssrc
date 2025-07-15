PROG = lssrc

PREFIX = /usr/local

CXXFLAGS := -flto -std=gnu++2c -fno-rtti -fno-exceptions -fcf-protection=none -fomit-frame-pointer -march=native -fno-stack-protector -static -masm=intel -nostdlib -nodefaultlibs -ffreestanding
CXXFLAGS += -DENABLE_MAKEFILE -DENABLE_PYPROJECT_TOML -DENABLE_CARGO_TOML -DENABLE_PACKAGE_JSON
LDFLAGS := -lgcc $(CXXFLAGS) -nostartfiles -e _start
ifdef DEBUG
CXXFLAGS += -ggdb3
else
CXXFLAGS += -Oz -ffast-math -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables  -fmerge-all-constants
LDFLAGS += -Wl,--omagic -s -z nognustack -fuse-ld=lld -Wl,--icf=all -Wl,-z,norelro -Wl,--gc-sections -Wl,--hash-style=gnu -Wl,--build-id=none
endif

CXX = clang++-21
