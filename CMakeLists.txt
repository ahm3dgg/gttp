cmake_minimum_required (VERSION 3.10)

project ("http")

set(CMAKE_CXX_STANDARD 23)
set(ZLIB_USE_STATIC_LIBS ON) 
find_package(ZLIB REQUIRED)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB xxHASH_SRC "${PROJECT_SOURCE_DIR}/ThirdParty/xxHash/*.c")

if(MSVC)
    add_compile_options(/fsanitize=address)
    add_compile_options(/Zi)
    add_link_options(/DEBUG)
else()
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

add_executable (http ${SRC_FILES} ${xxHASH_SRC})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET http PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(http PRIVATE ws2_32 ZLIB::ZLIB)