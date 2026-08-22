# Config
cmake_minimum_required(VERSION 3.28)
# set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(CD-Player LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# target_compile_features(main PRIVATE cxx_std_17)

# Conan-managed packages
find_package(cJSON REQUIRED)
find_package(CURL REQUIRED)
find_package(miniaudio REQUIRED)

# System packages
include(FetchContent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCDIO REQUIRED IMPORTED_TARGET libcdio)

# Build
add_executable(main src/main.cpp)
target_sources(main PRIVATE
  src/main.cpp
  src/audio.cpp
  src/musicbrainz.cpp
  src/logger.cpp
  include/audio.hpp
  include/musicbrainz.hpp
  include/logger.hpp
)
target_include_directories(main PRIVATE include)
target_link_libraries(main PRIVATE
  PkgConfig::LIBCDIO
  CURL::libcurl
  cjson::cjson
  miniaudio::miniaudio
)

# if(WIN32)
#     # Windows native audio APIs (WASAPI, DirectSound)
#     target_link_libraries(main PRIVATE ole32 user32 winmm)
# elseif(APPLE)
#     # macOS CoreAudio framework
#     find_library(COREAUDIO_LIB CoreAudio)
#     find_library(AUDIOUNIT_LIB AudioUnit)
#     find_library(COREFOUNDATION_LIB CoreFoundation)
#     target_link_libraries(main PRIVATE ${COREAUDIO_LIB} ${AUDIOUNIT_LIB} ${COREFOUNDATION_LIB})
# elseif(UNIX AND NOT APPLE)
#     # Linux audio APIs (ALSA, PulseAudio) & threading
#     find_package(Threads REQUIRED)
#     target_link_libraries(main PRIVATE Threads::Threads ${CMAKE_DL_LIBS})

#     # ALSA is miniaudio's default backend on Linux
#     find_package(ALSA REQUIRED)
#     target_link_libraries(main PRIVATE ALSA::ALSA)
# endif()
