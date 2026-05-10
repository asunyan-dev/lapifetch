cmake_minimum_required(VERSION 3.10)

project(lapifetch)

add_executable(lapifetch
    src/main.cpp
)

include(GNUInstallDirs)

install(TARGETS lapifetch
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)