# Copyright (C) 2007-2010 Glenn Randers-Pehrson

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

# needed packages
# find_package(ZLIB REQUIRED)
set(ZLIB_INCLUDE_DIR ../zlib)
set(ZLIB_LIBRARY zlib)
include_directories(${ZLIB_INCLUDE_DIR})

# OUR SOURCES
set(libpng_sources
  png.h
  pngconf.h
  pngpriv.h
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)

add_definitions(-DPNG_CONFIGURE_LIBPNG -DPNG_NO_MODULEDEF -D_CRT_SECURE_NO_DEPRECATE -DPNG_NO_STDIO -DPNG_NO_CONSOLE_IO)

# NOW BUILD OUR TARGET
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ZLIB_INCLUDE_DIR})

add_library(libpng STATIC ${libpng_sources})
target_link_libraries(libpng ${ZLIB_LIBRARY})
