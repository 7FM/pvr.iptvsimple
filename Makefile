.PHONY: all clean build init

all: clean build

init:
	cd .. && git clone https://github.com/xbmc/xbmc.git

build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug -DADDONS_TO_BUILD=pvr.iptvsimple -DADDON_SRC_PREFIX=../.. -DCMAKE_INSTALL_PREFIX=../../xbmc/addons -DPACKAGE_ZIP=1 ../../xbmc/cmake/addons
	cd build && $(MAKE)

clean:
	rm -rf build/
