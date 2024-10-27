cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# create project
project(avaspec_mdsplus LANGUAGES C CXX)

# set C++ standard
set(CMAKE_CXX_STANDARD 14)

# add dependencies
include(cmake/get_cpm.cmake)

# libusb is on github if not installed
# https://github.com/libusb/libusb.git
# https://github.com/libusb/libusb-cmake.git
# CPMAddPackage(
# NAME libusb
# GITHUB_REPOSITORY libusb/libusb-cmake
# GIT_TAG v1.0.27-1
# )
# creates usb-1.0 target, static library (unless option for shared is set)
CPMAddPackage(gh:libusb/libusb-cmake@1.0.27-1)  
# unfortunately, libusb-compat-0.1 does not have a cmake package
#CPMAddPackage(gh:libusb/libusb-compat-0.1@0.1.8)
include(ExternalProject)
include(GNUInstallDirs)
find_program(MAKE_EXECUTABLE NAMES gmake make mingw32-make REQUIRED)
ExternalProject_Add(
    libusb_compat
    URL https://github.com/libusb/libusb-compat-0.1/archive/v0.1.8.tar.gz
    CONFIGURE_HANDLED_BY_BUILD true
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j
    INSTALL_COMMAND ${MAKE_EXECUTABLE} install
    TEST_COMMAND ""
    BUILD_BYPRODUCTS <INSTALL_DIR>/include/usb.h <INSTALL_DIR>/lib/${CMAKE_STATIC_LIBRARY_PREFIX}usb${CMAKE_STATIC_LIBRARY_SUFFIX}
)

ExternalProject_Add_Step(libusb_compat
    bootstrap
    WORKING_DIRECTORY <SOURCE_DIR>
    COMMAND <SOURCE_DIR>/bootstrap.sh
    DEPENDEES download
    DEPENDERS configure
)
ExternalProject_Get_Property(libusb_compat INSTALL_DIR)
set(libusb_compat_LIBRARY ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}usb${CMAKE_STATIC_LIBRARY_SUFFIX})
add_library(libusb_compat::libusb_compat INTERFACE IMPORTED GLOBAL)
target_include_directories(libusb_compat::libusb_compat INTERFACE ${INSTALL_DIR}/include)
target_link_libraries(libusb_compat::libusb_compat INTERFACE "${libusb_compat_LIBRARY}") # need the quotes to expand list
add_dependencies(libusb_compat::libusb_compat usb-1.0)

# add library
add_library(avaspec SHARED 
    avaspec.cpp 
    libavaspec.cpp
    libavaspec.h
    time.cpp
    error.cpp
)
target_link_libraries(avaspec PRIVATE usb-1.0 libusb_compat::libusb_compat)
add_dependencies(avaspec libusb_compat usb-1.0)

# add executables
add_executable(avaspec_raw avaspec_raw.cpp)
target_link_libraries(avaspec_raw PRIVATE avaspec)

add_executable(avaspec_test testlib.c)
target_link_libraries(avaspec_test avaspec)

install(TARGETS avaspec_raw RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS avaspec LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS avaspec_test RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

