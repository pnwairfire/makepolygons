#******************************************************************************
#
#  BlueSky Framework - Controls the estimation of emissions, incorporation of
#                      meteorology, and the use of dispersion models to
#                      forecast smoke impacts from fires.
#  Copyright (C) 2003-2006  USDA Forest Service - Pacific Northwest Wildland
#                           Fire Sciences Laboratory
#  BlueSky Framework - Version 3.5.1
#  Copyright (C) 2007  USDA Forest Service - Pacific Northwest Wildland Fire
#                      Sciences Laboratory and Sonoma Technology, Inc.
#                      All rights reserved.
#
# See LICENSE.TXT for the Software License Agreement governing the use of the
# BlueSky Framework - Version 3.5.1.
#
# Contributors to the BlueSky Framework are identified in ACKNOWLEDGEMENTS.TXT
#
#******************************************************************************
#
# Required dependencies:
#   gdal-devel
#   libxslt-devel
#   proj-devel
#

##
## copied from make.inc in BSF
##

# Save the path to make.inc (so we can use relative paths below)
SRC_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# Platform tag: appended to builds made from this configuration
PLATFORM_TAG=-local

#----------------------------------------------------------------------
# Python compiling options
#----------------------------------------------------------------------
PYTHON=/usr/bin/python2.7

#----------------------------------------------------------------------
# GCC compiling options
#----------------------------------------------------------------------
GCC = gcc
GXX = g++
GCCFLAGS = -O2 -pipe
GCC_LDFLAGS = -static
CC = $(GCC)
CFLAGS = $(GCCFLAGS)

#----------------------------------------------------------------------
# Fortran 90 compiling options
#----------------------------------------------------------------------
F90 = gfortran
F77 = gfortran
FFLAGS = $(GCCFLAGS) -fall-intrinsics -fconvert=big-endian -frecord-marker=4 -ffast-math
FFLAGS_FREEFORM = -ffree-form -ffree-line-length-none
LFLAGS = $(GCC_LDFLAGS) -static -lgfortran
LIB_PGI_LFS =

#----------------------------------------------------------------------
# C compiling options for C linked with Fortran
#----------------------------------------------------------------------
PGCC = $(GCC)
PGCCFLAGS = $(GCCFLAGS) -DLITTLE -DUNDERSCORE

#----------------------------------------------------------------------
# Prerequisite libraries
#----------------------------------------------------------------------
LIB_M3API = -Bstatic -L$(SRC_ROOT)/lib/ioapi-3.0-install/lib -lioapi -Bdynamic
INCLUDE_M3API = -I$(SRC_ROOT)/lib/ioapi-3.0/ioapi

LIB_NETCDF = -Bstatic -L$(SRC_ROOT)/lib/netcdf-3.6.3-install/lib -I$(SRC_ROOT)/lib/netcdf-3.6.3-install/include -lnetcdf -Bdynamic
INCLUDE_NETCDF = -I$(SRC_ROOT)/lib/netcdf-3.6.3-install/include

LIB_GDAL = -Bstatic -L$(SRC_ROOT)/lib/gdal-1.6.0/ -lgdal -lproj -lpthread -lz -Bdynamic
INCLUDE_GDAL = -I$(SRC_ROOT)/lib/gdal-1.6.0-install/include

LIB_LIBXML = -Bstatic -L/usr/include/libxml2 -lxslt -lxml2 -Bdynamic
INCLUDE_LIBXML = -I/usr/include/libxml2 -DUSE_LIBXSLT


##
## copied from makepolygons' MAKEFILE in BSF
##

VERSION=0.1.0

INCLUDES = $(INCLUDE_GDAL) $(INCLUDE_LIBXML) $(INCLUDE_NETCDF)
LIBS = $(LIB_GDAL) $(LIB_LIBXML) $(LIB_NETCDF) -lpthread -lz -ldl

SRCS = \
	makepolygons.cpp \
	primitives.cpp \
	rasterutil.cpp \
	region.cpp \
	regionmap.cpp \
	thresholdraster.cpp \
	util.cpp

OBJS = \
	makepolygons.o \
	primitives.o \
	rasterutil.o \
	region.o \
	regionmap.o \
	thresholdraster.o \
	util.o

HEADERS = \
	primitives.h \
	raster.h \
	rasterutil.h \
	region.h \
	regionmap.h \
	thresholdraster.h \
	util.h

PACKAGE_DIR=build/utils/makepolygons/bin


## Building

all: build

%.o : %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

build: makepolygons

makepolygons: $(OBJS)
	@echo "* Building MakePolygons utility"
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS) $(GCC_LDFLAGS) $(LIBS) > build.log 2>&1
	mkdir -p $(PACKAGE_DIR)
	cp -up makepolygons $(PACKAGE_DIR)

dist: all
	@echo "* Creating MakePolygons utility distribution"
	mkdir -p dist
	mv build bluesky
	tar czf dist/makepolygons-v$(VERSION)$(PLATFORM_TAG).tar.gz bluesky/*
	mv bluesky build


## Cleaining

clean:
	@echo "* Cleaning MakePolygons utility"
	@rm -rf build
	@rm -f build.log
	@rm -f makepolygons $(OBJS)

distclean: clean
	@echo "* Cleaning MakePolygons utility distribution"
	@rm -rf dist

# purge is necessary for top level Makefile
purge: distclean
