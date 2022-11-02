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

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX 1)
endif()

if (MSVC)
  add_compile_options(/W3)
  add_compile_options(/sdl)
else()
  add_compile_options(-Wall)
endif()

option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer" OFF)
# option(ENABLE_MEMORY_SANITIZER "Enable Memory Sanitizer" OFF)

#if(UNIX)
#  find_program(LLVM_CONFIG_BINARY NAMES llvm-config)
#
#  execute_process(
#    COMMAND "${LLVM_CONFIG_BINARY}" --libdir
#    OUTPUT_VARIABLE LOCAL_LIB_DIR
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#  )
#
#  separate_arguments(LOCAL_INCLUDE_DIR)
#
#  # Use my local LLVM install
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
#  link_libraries("-Wl,-rpath,${LOCAL_LIB_DIR}")
#endif()

if (NOT MSVC)
  add_compile_options(-Wall)
  add_compile_options(-Wextra)
  add_compile_options(-Wimplicit-fallthrough)

  #  if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
#    add_compile_options(-ferror-limit=10)

#  add_compile_options(-Weverything)
#    add_compile_options(-Wno-c++98-compat)
#    add_compile_options(-Wno-c++98-compat-pedantic)
#    add_compile_options(-Wno-padded)
#
#add_compile_options(-Wshorten-64-to-32)
    #    add_compile_options(-Wextra-semi)
    #    add_compile_options(-Wzero-as-null-pointer-constant)
    #add_compile_options(-Wshadow-uncaptured-local)
    #add_compile_options(-Wcovered-switch-default)
    #add_compile_options(-Wsign-conversion)
    #add_compile_options(-Wcovered-switch-default)
    #add_compile_options(-Wshadow)
    #add_compile_options(-Wshadow-field)
    #if (LINUX)
    #  add_compile_options(-Wno-global-constructors)
    #  add_compile_options(-Wno-exit-time-destructors)
    #  add_compile_options(-Wno-weak-vtables)
    #endif()

    #add_compile_options(-Wlifetime)

    if (ENABLE_MEMORY_SANITIZER)
      add_compile_options(-fsanitize=memory)
    endif()
    #add_compile_options(-fno-omit-frame-pointer)
    #add_compile_options(-fno-optimize-sibling-calls)
    #else()
    #endif()
endif()

# Address sanitizer settings
if (ENABLE_ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif()

if (MSVC)
  add_compile_options(/W4)
  add_compile_options(/MP)
endif()

add_executable(binums)

target_sources(binums PUBLIC
  FixedNumber.h
  Float16m7e8s1.h
  Half.h
  Int24.h
  precomp.h

  BiNums.cpp
  precomp.cpp
)

target_include_directories(binums PUBLIC
  ${LOCAL_INCLUDE_DIR}
)
