cmake_minimum_required (VERSION 2.8)

project (gecko_media)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

include_directories(
  include
)

add_library(gecko_core OBJECT
  GeckoMedia.cpp
  MediaFormatReader.cpp
)

add_library(gecko_media_cmake STATIC
  $<TARGET_OBJECTS:gecko_core>
)

install(TARGETS gecko_media_cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}
)
