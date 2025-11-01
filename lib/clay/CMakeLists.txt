cmake_minimum_required(VERSION 3.20)

add_library(clay)
add_library(clay::clay ALIAS clay)
target_compile_features(clay PUBLIC cxx_std_20)

target_sources(clay PRIVATE src/clay/clay.cpp)
target_include_directories(clay SYSTEM PUBLIC src)
install(FILES "LICENSE.md" DESTINATION "license/clay")