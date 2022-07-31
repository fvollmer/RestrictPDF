# ToDo: make compatible with MSVC (only tested with mingw-w64)
cmake_minimum_required(VERSION 3.20)

project(RestrictPDF VERSION 0.1.2)

if(NOT QPDF_PATH)
    set(QPDF_PATH ${CMAKE_CURRENT_LIST_DIR}/qpdf)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.rc.in main.rc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.manifest main.manifest COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icon.ico icon.ico COPYONLY)

add_library(qpdf SHARED IMPORTED)
set_target_properties(
    qpdf 
    PROPERTIES 
        IMPORTED_LOCATION "${QPDF_PATH}/bin/qpdf28.dll"
        IMPORTED_IMPLIB "${QPDF_PATH}/lib/libqpdf.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${QPDF_PATH}/include")

add_executable(${PROJECT_NAME} WIN32)
target_sources(
    ${PROJECT_NAME}
    PRIVATE 
        main.cpp
        fileopendialog.hpp fileopendialog.cpp
        util.hpp util.cpp 
        parameters.hpp 
        ccoinitializer.hpp ccoinitializer.cpp 
        main.manifest main.rc)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} qpdf )
target_link_libraries(${PROJECT_NAME} qpdf shlwapi)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        LINK_FLAGS "-municode")

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
