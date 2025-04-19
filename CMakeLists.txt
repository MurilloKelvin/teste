cmake_minimum_required(VERSION 3.10)
project(PDS2-20251-TF-Web-Searcher CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_library(GUMBO_LIBRARY NAMES gumbo)

# Adicione caminhos manuais como fallback
if(NOT GUMBO_LIBRARY)
    set(GUMBO_LIBRARY "E:/original/clionprojects/pds2-20251-tf-web-searcher/vcpkg/packages/gumbo_x64-mingw-static/lib/libgumbo.a")
endif()
if(NOT CURL_LIBRARIES)
    set(CURL_LIBRARIES "E:/original/clionprojects/pds2-20251-tf-web-searcher/vcpkg/packages/curl_x64-mingw-static/lib/libcurl.a")
endif()

include_directories(${CURL_INCLUDE_DIRS} include)

add_executable(PDS2-20251-TF-Web-Searcher
        main.cpp
        src/scraper.cpp
        src/logger.cpp
        src/config.cpp
)

target_link_libraries(PDS2-20251-TF-Web-Searcher ${CURL_LIBRARIES} ${GUMBO_LIBRARY})