cmake_minimum_required (VERSION 2.6)
project (octolcd)

set (Octotouch_VERSION_MAJOR 1)
set (Octotouch_VERSION_MINOR 0)

FIND_PACKAGE(PkgConfig REQUIRED)
#PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)
PKG_CHECK_MODULES(GTKMM REQUIRED gtkmm-3.0)

#INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GTKMM_INCLUDE_DIRS})
#LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})
LINK_DIRECTORIES(${GTKMM_LIBRARY_DIRS})
#ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
ADD_DEFINITIONS(${GTKMM_CFLAGS_OTHER})

configure_file (
  "${PROJECT_SOURCE_DIR}/inc/Version.h.in"
  "${PROJECT_BINARY_DIR}/generatedheaders/Version.h"
)

set(src
    src/main
    src/MainActivity
    src/Activity)

include_directories (
    inc/
    ${CMAKE_CURRENT_BINARY_DIR}/generatedheaders
)

file(COPY "${PROJECT_SOURCE_DIR}/glade/mainWindow.glade" DESTINATION "${PROJECT_BINARY_DIR}/glade/")
file(COPY "${PROJECT_SOURCE_DIR}/css/style.css" DESTINATION "${PROJECT_BINARY_DIR}/css/")
add_executable(octolcd ${src} )
#target_link_libraries(octolcd PUBLIC ${GTK3_LIBRARIES})
target_link_libraries(octolcd PUBLIC ${GTKMM_LIBRARIES})
#target_compile_options(octolcd PUBLIC ${GTK3_CFLAGS_OTHER})
target_compile_options(octolcd PUBLIC ${GTKMM_CFLAGS_OTHER})
