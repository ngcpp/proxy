cmake_minimum_required(VERSION 3.5)

project(report_generator)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/read_dependencies.cmake)
proxy_read_dependencies("${CMAKE_CURRENT_SOURCE_DIR}/dependencies.json")

include(FetchContent)
# The policy uses the download time for timestamp, instead of the timestamp in the archive. This
# allows for proper rebuilds when a projects URL changes.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
  nlohmann_json
  URL ${PROXY_NLOHMANN_JSON_URL}
  URL_HASH SHA256=${PROXY_NLOHMANN_JSON_SHA256}
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(report_generator main.cpp)

target_link_libraries(report_generator PRIVATE nlohmann_json::nlohmann_json)

target_compile_features(report_generator PRIVATE cxx_std_20)
if(MSVC)
  target_compile_options(report_generator PRIVATE /W4 /WX)
else()
  target_compile_options(
    report_generator
    PRIVATE -Wall -Wextra -Wpedantic -Werror
  )
endif()
