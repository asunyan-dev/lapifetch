cmake_minimum_required(VERSION 3.10)

project(lapifetch)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC_FILES src/*.cpp)

add_executable(lapifetch ${SRC_FILES})

include(GNUInstallDirs)

install(TARGETS lapifetch
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)