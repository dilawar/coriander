cmake_minimum_required(VERSION 2.8)
project(coriander)

set(CORIANDER_VERSION_MAJOR 2)
set(CORIANDER_VERSION_MINOR 0)
set(CORIANDER_VERSION_PATCH 2)
set(CORIANDER_VERSION
    ${CORIANDER_VERSION_MAJOR}.${CORIANDER_VERSION_MINOR}.${CORIANDER_VERSION_PATCH}
    )

## Compiler related
set(CMAKE_BUILD_TYPE Release )
set( PACKAGE_DATA_DIR ${CMAKE_INSTALL_PREFIX}/share )
add_definitions( 
    -DVERSION="${CORIANDER_VERSION}" -DPACKAGE="coriander" 
    -DPACKAGE_DATA_DIR="${PACKAGE_DATA_DIR}"
    )

find_package( PkgConfig )

pkg_check_modules( LIBBONOBO REQUIRED libbonobo-2.0 )
pkg_check_modules( LIBGNOMECANVAS REQUIRED libgnomecanvas-2.0 )
pkg_check_modules( LIBGNOMEUI REQUIRED libgnomeui-2.0 )
pkg_check_modules( LIBGNOME REQUIRED libgnome-2.0 )

include_directories(
    ${LIBBONOBO_INCLUDE_DIRS} 
    ${LIBGNOMECANVAS_INCLUDE_DIRS} 
    ${LIBGNOMEUI_INCLUDE_DIRS} 
    ${LIBGNOME_INCLUDE_DIRS}
    )

pkg_check_modules( SDL REQUIRED sdl<=1.3.0)
include_directories( ${SDL_INCLUDE_DIRS} )

## gtk
find_package( GTK2 REQUIRED )
include_directories( ${GTK2_INCLUDE_DIRS} )

## tiff
find_package( TIFF )
if( TIFF_FOUND )
    add_definitions( -DHAVE_LIBTIFF )
endif( )

# libdc
pkg_check_modules( LIBDC REQUIRED libdc1394-2 )
pkg_check_modules( LIBRAW REQUIRED libraw1394 )
include_directories( ${LIBDC_INCLUDE_DIRS} ${LIBRAW_INCLUDE_DIRS} )

# Threads
find_package( Threads REQUIRED )

SET(CORIANDER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
FILE( GLOB CORIANDER_SRC ${CORIANDER_SOURCE_DIR}/*.c)

add_executable( coriander ${CORIANDER_SRC} )
target_link_libraries( coriander 
    ${GTK2_LIBRARIES}
    ${LIBGNOME_LIBRARIES}
    ${LIBGNOMECANVAS_LIBRARIES}
    ${LIBGNOMEUI_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LIBDC_LIBRARIES}
    ${LIBRAW_LIBRARIES}
    )
if( TIFF_FOUND )
    target_link_libraries( coriander ${TIFF_LIBRARIES} )
endif( TIFF_FOUND )

## install targets
install( TARGETS coriander 
    DESTINATION bin 
    )
install( FILES pixmaps/coriander-icon.png pixmaps/coriander-logo.png
    DESTINATION share/pixmaps/coriander/
    )

