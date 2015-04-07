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

# Note:  what's in make.inc would have to be copied here if bluesky
# kml were moved into it's own repo
include ../../make.inc

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
