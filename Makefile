CC=gcc
CXX=g++

CFLAGS=-Wall -Wextra -O2 -std=c99
CXXFLAGS=-Wall -Wextra -O2 -std=c++11 -pthread

EIGEN_DIR=eigen
LIBIGL_DIR=libigl/include

ifeq ($(DESTDIR),)
    DESTDIR := /usr/local/bin
endif


.PHONY: edtsurf

all: decimate off2stl edtsurf

edtsurf:
	$(MAKE) -C $@

decimate: decimate.cc
	$(CXX) $(CXXFLAGS) -I$(EIGEN_DIR) -I$(LIBIGL_DIR) $< -o $@

off2stl: off2stl.c
	$(CC) $(CFLAGS) $< -o $@ -lm

install:
	install -d $(DESTDIR)
	install -m 755 edtsurf/EDTSurf $(DESTDIR)
	install -m 755 decimate $(DESTDIR)
	install -m 755 off2stl  $(DESTDIR)
	install -m 755 pdb2stl  $(DESTDIR)
