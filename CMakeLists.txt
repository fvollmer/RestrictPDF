# ToDo: make compatible with MSVC (only tested with mingw-w64)
cmake_minimum_required(VERSION 3.23)

project(RestrictPDF)

if(NOT QPDF_PATH)
    set(QPDF_PATH ${CMAKE_CURRENT_LIST_DIR}/qpdf)
endif()

add_library(qpdf SHARED IMPORTED)
set_target_properties(
    qpdf 
    PROPERTIES 
        IMPORTED_LOCATION "${QPDF_PATH}/bin/qpdf28.dll"
        IMPORTED_IMPLIB "${QPDF_PATH}/lib/libqpdf.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${QPDF_PATH}/include")

add_executable(RestrictPDF WIN32)
target_sources(RestrictPDF PRIVATE main.cpp main.manifest main.rc)
target_link_libraries(RestrictPDF qpdf)

set_target_properties(
    RestrictPDF
    PROPERTIES
        CXX_STANDARD 17
        LINK_FLAGS "-municode")

target_compile_definitions(RestrictPDF PRIVATE -DUNICODE -D_UNICODE)
target_compile_options(RestrictPDF PRIVATE -Wall -Wextra)
