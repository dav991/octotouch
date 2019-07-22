cmake_minimum_required (VERSION 2.6)
project (octotouch)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

set(CMAKE_CXX_STANDARD 11)
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)

set (Octotouch_VERSION_MAJOR 1)
set (Octotouch_VERSION_MINOR 0)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-3.0)
PKG_CHECK_MODULES(LIBCONFIG REQUIRED libconfig++)
find_package(OpenSSL REQUIRED)
FIND_PACKAGE(cpprestsdk REQUIRED)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system)

INCLUDE_DIRECTORIES(${GTKMM_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${LIBCONFIG_INCLUDE_DIRS})
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
LINK_DIRECTORIES(${GTKMM_LIBRARY_DIRS})
LINK_DIRECTORIES(${LIBCONFIG_LIBRARY_DIRS})
LINK_DIRECTORIES(${LIBWEBKITGTK_LIBRARY_DIRS})
ADD_DEFINITIONS(${GTKMM_CFLAGS_OTHER})

configure_file (
  "${PROJECT_SOURCE_DIR}/inc/Version.h.in"
  "${PROJECT_BINARY_DIR}/generatedheaders/Version.h"
)

set(src
    src/main
    src/Config
    src/Activity
    src/MainActivity
    src/StatusActivity
    src/TuneActivity
    src/WebcamActivity
    )

include_directories (
    inc/
    ${CMAKE_CURRENT_BINARY_DIR}/generatedheaders
)

#print_all_variables()


file(COPY "${PROJECT_SOURCE_DIR}/glade/mainWindow.glade" DESTINATION "${PROJECT_BINARY_DIR}/glade/")
file(COPY "${PROJECT_SOURCE_DIR}/glade/statusWindow.glade" DESTINATION "${PROJECT_BINARY_DIR}/glade/")
file(COPY "${PROJECT_SOURCE_DIR}/glade/tuneWindow.glade" DESTINATION "${PROJECT_BINARY_DIR}/glade/")
file(COPY "${PROJECT_SOURCE_DIR}/glade/webcamWindow.glade" DESTINATION "${PROJECT_BINARY_DIR}/glade/")
file(COPY "${PROJECT_SOURCE_DIR}/css/style.css" DESTINATION "${PROJECT_BINARY_DIR}/css/")
file(COPY "${PROJECT_SOURCE_DIR}/octotouch.cfg" DESTINATION "${PROJECT_BINARY_DIR}/")
add_executable(octotouch ${src})

target_link_libraries(octotouch ${GTKMM_LIBRARIES})
target_link_libraries(octotouch ${LIBCONFIG_LIBRARIES})
target_link_libraries(octotouch OpenSSL::SSL)
target_link_libraries(octotouch ${Boost_LIBRARIES})
target_link_libraries(octotouch cpprestsdk::cpprest)
target_compile_options(octotouch PUBLIC ${GTKMM_CFLAGS_OTHER})