message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(cJSON)
find_package(miniaudio)
find_package(CURL)

set(CONANDEPS_LEGACY  cjson::cjson  miniaudio::miniaudio  CURL::libcurl )