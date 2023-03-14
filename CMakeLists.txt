cmake_minimum_required(VERSION 3.15.0)

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
  add_compile_options(/W4)
  add_compile_options(/sdl)
  add_compile_options(/MP)
else()
  add_compile_options(-Wall)
  add_compile_options(-Wextra)
endif()

option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

if (MSVC)
  option(ENABLE_TESTING "Enable unit tests" ON)
else()
  option(ENABLE_TESTING "Enable unit tests" OFF)
endif()

# Address sanitizer settings
if (ENABLE_ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address)
  if (NOT MSVC)
    add_link_options(-fsanitize=address)
  endif()
endif()

add_executable(binums)

target_sources(binums PUBLIC
  Common.h
  FixedNumber.h
  Float16m7e8s1.h
  Half.h
  Int24.h
  precomp.h

  BiNums.cpp
  BiNumsMain.cpp
  precomp.cpp
)

target_include_directories(binums PUBLIC
  ${LOCAL_INCLUDE_DIR}
)

if (ENABLE_TESTING)
  add_executable(binumstest)

  target_sources(binumstest PUBLIC
    Common.h
    FixedNumber.h
    Float16m7e8s1.h
    Half.h
    Int24.h
    precomp.h

    BiNums.cpp
    BiNumsTest.cpp
    precomp.cpp
  )

  target_include_directories(binumstest PUBLIC
    ${LOCAL_INCLUDE_DIR}
  )
endif()
