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

# FUTURE: Maybe a less agressive set of warnings
#if (NOT MSVC)
#  add_compile_options(-Wall)
#  add_compile_options(-Wextra)
#  add_compile_options(-Wimplicit-fallthrough)
#
#  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    add_compile_options(-Weverything)
#    add_compile_options(-Wno-c++98-compat)
#    add_compile_options(-Wno-c++98-compat-pedantic)
#    add_compile_options(-Wno-padded)
#
#    add_compile_options(-Wshorten-64-to-32)
#    add_compile_options(-Wextra-semi)
#    add_compile_options(-Wzero-as-null-pointer-constant)
#    add_compile_options(-Wshadow-uncaptured-local)
#    add_compile_options(-Wcovered-switch-default)
#    add_compile_options(-Wsign-conversion)
#    add_compile_options(-Wcovered-switch-default)
#    add_compile_options(-Wshadow)
#    add_compile_options(-Wshadow-field)
#  else()
#  endif()
#endif()

if (MSVC)
  add_compile_options(/W4)
endif()

if(UNIX)
  find_program(LLVM_CONFIG_BINARY NAMES llvm-config)

  execute_process(
    COMMAND "${LLVM_CONFIG_BINARY}" --libdir
    OUTPUT_VARIABLE LOCAL_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  separate_arguments(LOCAL_INCLUDE_DIR)
  #list(APPEND CMAKE_INSTALL_RPATH ${LOCAL_LIB_DIR})

  #add_link_options(${LOCAL_LIBCPP_LDFLAGS})

  #execute_process(
  #  COMMAND "${LLVM_CONFIG_BINARY}" --includedir
  #  OUTPUT_VARIABLE LOCAL_INCLUDE_DIR
  #  OUTPUT_STRIP_TRAILING_WHITESPACE
  #)
  #
  #separate_arguments(LOCAL_INCLUDE_DIR)

  # Use my local LLVM install
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  link_libraries("-Wl,-rpath,${LOCAL_LIB_DIR}")

endif()

#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,${LOCAL_LIB_DIR}")
#set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -Wl,-rpath,${LOCAL_LIB_DIR}")
#set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -Wl,-rpath,${LOCAL_LIB_DIR}")

add_executable(binums)

#target_compile_options(binums PUBLIC
#  -stdlib=libc++
#)
#-stdlib=libc++

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
