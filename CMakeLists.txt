cmake_minimum_required(VERSION 3.13)

project(BiNums LANGUAGES C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the build artifacts directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/$<CONFIG>/lib")

if (MSVC)
  add_compile_options(/W3)
  add_compile_options(/sdl)
else()
  add_compile_options(-Wall)
endif()

if (MSVC)
endif()

if(UNIX)
  find_program(LLVM_CONFIG_BINARY NAMES llvm-config)

  execute_process(
    COMMAND "${LLVM_CONFIG_BINARY}" --libdir
    OUTPUT_VARIABLE LOCAL_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  separate_arguments(LOCAL_INCLUDE_DIR)

  # Use my local LLVM install
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  link_libraries("-Wl,-rpath,${LOCAL_LIB_DIR}")
endif()

add_executable(binums)

target_sources(binums PUBLIC
  BiNums.cpp
  FixedNumber.h
  Half.h
  precomp.h
  Float16m7e8s1.h
  Int24.h
  precomp.cpp
)

target_include_directories(binums PUBLIC
  ${LOCAL_INCLUDE_DIR}
)
