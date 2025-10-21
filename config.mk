PROG:=lssrc

PREFIX:=/usr/local

# possible values are: PACKAGE_JSON PYPROJECT_TOML REQUIREMENTS_TXT GO_MOD CARGO_TOML POM_XML BUILD_GRADLE COMPOSER_JSON GEMFILE PUBSPEC_YAML CMAKELISTS_TXT BUILD_ZIG MAKEFILE
ENABLE:=MAKEFILE PYPROJECT_TOML CARGO_TOML PACKAGE_JSON

CXXFLAGS:=-flto -std=gnu++2c -fno-rtti -fno-exceptions -fcf-protection=none -fomit-frame-pointer -march=native -fno-stack-protector -static -masm=intel -nostdlib -nodefaultlibs -ffreestanding -Wall -Wextra -Werror
CXXFLAGS+=$(patsubst %, -DENABLE_%, $(ENABLE))
LDFLAGS:=-lgcc $(CXXFLAGS) -nostartfiles -e _start
ifdef DEBUG
CXXFLAGS+=-ggdb3
else
CXXFLAGS+=-Oz -ffast-math -DNDEBUG -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables  -fmerge-all-constants
LDFLAGS+=-Wl,--omagic -s -z nognustack -fuse-ld=lld -Wl,--icf=all -Wl,-z,norelro -Wl,--gc-sections -Wl,--hash-style=gnu -Wl,--build-id=none
endif

CXX:=clang++-21
