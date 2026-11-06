cmake_minimum_required(VERSION 3.25)
project(FruitNinjaXbox360 C)

if(NOT XSDK)
    message(FATAL_ERROR "Configure this target with xenosworks/xsdk/cmake/xsdk.cmake")
endif()

add_executable(fruit-ninja-xbox360 main.c)
set_target_properties(fruit-ninja-xbox360 PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
)
target_compile_options(fruit-ninja-xbox360 PRIVATE -Wall -Wextra -Werror)

xsdk_add_xex(fruit-ninja-xbox360
    TITLE_ID 0x46524E58
    NAME "Fruit Ninja Xbox 360"
    VERSION 0.1.0.0
)
