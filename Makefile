-include config.mk

src := $(wildcard src/*.cpp)
obj := $(patsubst src/%.cpp, build/%.o, $(src))
json = $(patsubst src/%.cpp,build/%.json,$(src))

.PHONY: all clean install

all: build/compile_commands.json $(PROG)

clean:
	rm -rf build $(PROG)

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(PROG): $(obj)
	$(CXX) $(LDFLAGS) $^ -o $@
ifndef DEBUG
ifneq (,$(shell which sstrip))
	sstrip -z $(PROG)
endif
endif

build/compile_commands.json: $(json)
	@mkdir -p build
	jq -s '.' $^ > $@
build/%.json: src/%.cpp
	@mkdir -p build
	@echo '{"directory":"$(PWD)","command":"$(CXX) $(subst ",'\\\\\\"',$(CXXFLAGS) -DCLANGD) -c -o a.o $^","file":"$^"}'>$@

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f $(PROG) ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(PROG)
