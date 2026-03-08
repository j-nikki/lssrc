PROG:=lssrc

PREFIX:=/usr/local

# possible values are: PACKAGE_JSON PYPROJECT_TOML REQUIREMENTS_TXT GO_MOD CARGO_TOML POM_XML BUILD_GRADLE COMPOSER_JSON GEMFILE PUBSPEC_YAML CMAKELISTS_TXT BUILD_ZIG MAKEFILE
ENABLE:=MAKEFILE PYPROJECT_TOML CARGO_TOML PACKAGE_JSON

CXXFLAGS:=-std=gnu++2c -march=native -masm=intel -Wall -Wextra -Werror
CXXFLAGS+=$(patsubst %, -DENABLE_%, $(ENABLE))
LDFLAGS:=$(CXXFLAGS)
ifdef DEBUG
CXXFLAGS+=-ggdb3 -DDEBUG -fno-omit-frame-pointer -fno-optimize-sibling-calls \
  -fsanitize=address,undefined \
  -fsanitize-address-use-after-scope \
  -fsanitize=float-divide-by-zero \
  -fsanitize=unsigned-integer-overflow \
  -fsanitize=implicit-conversion \
  -fsanitize=local-bounds \
  -fsanitize=nullability \
  -fno-sanitize-recover=all
else
CXXFLAGS+=-flto -nostdlib -nodefaultlibs -ffreestanding -Oz -ffast-math -DNDEBUG -fomit-frame-pointer -static -fno-rtti -fno-exceptions -fcf-protection=none -fno-stack-protector -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables  -fmerge-all-constants
LDFLAGS+=-nostartfiles -e _start -lgcc -Wl,--omagic -s -z nognustack -fuse-ld=lld -Wl,--icf=all -Wl,-z,norelro -Wl,--gc-sections -Wl,--hash-style=gnu -Wl,--build-id=none
endif

CXX:=clang++-21
