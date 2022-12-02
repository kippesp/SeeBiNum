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

option(USE_CCACHE "Enable ccache" ON)

if (USE_CCACHE)
  find_program(ccache_exe ccache)
  if(ccache_exe)
    message(STATUS "Found ccache: ${ccache_exe}")
    file(COPY_FILE
      ${ccache_exe} ${CMAKE_BINARY_DIR}/cl.exe
      ONLY_IF_DIFFERENT)

    set(CMAKE_VS_GLOBALS
      "CLToolExe=cl.exe"
      "CLToolPath=${CMAKE_BINARY_DIR}"
      "TrackFileAccess=false"
      "UseMultiToolTask=true"
      "DebugInformationFormat=OldStyle"
    )
  endif()
endif()

if (MSVC)
  add_compile_options(/W3)
  add_compile_options(/sdl)
else()
  add_compile_options(-Wall)
endif()

option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)

if (NOT MSVC)
  add_compile_options(-Wall)
  add_compile_options(-Wextra)
endif()

# Address sanitizer settings
if (ENABLE_ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address)
  if (NOT MSVC)
    add_link_options(-fsanitize=address)
  endif()
endif()

if (MSVC)
  add_compile_options(/W4)
  add_compile_options(/MP)
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

add_executable(binumstest)

message(STATUS "FLAGS (before): ${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "FLAGS (after): ${CMAKE_CXX_FLAGS_DEBUG}")

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

target_include_directories(binums PUBLIC
  ${LOCAL_INCLUDE_DIR}
)

target_include_directories(binumstest PUBLIC
  ${LOCAL_INCLUDE_DIR}
)
