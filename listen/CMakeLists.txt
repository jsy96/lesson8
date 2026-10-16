cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(listen)
spiffs_create_partition_image(assets ${CMAKE_CURRENT_SOURCE_DIR}/assets FLASH_IN_PROJECT)
