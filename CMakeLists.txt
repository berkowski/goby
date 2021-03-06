# t. schneider tes@mit.edu 
# base CMakeLists.txt file for goby
# for help on CMake, see http://www.cmake.org/cmake/help/documentation.html
# or type > man cmake

# this project is intended to be built out of source by using 
# > ./build.sh

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(goby)

## allows us to write custom modules or modifying existing ones
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")


option(enable_testing "Enable building of tests using CTest (if set to ON, you can run tests with 'make test')" OFF)
if(enable_testing)
  enable_testing()
  include(CTest)
endif()


set(GOBY_VERSION_MAJOR "2")
set(GOBY_VERSION_MINOR "1")
set(GOBY_VERSION_PATCH "1")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
   message("Compiling in Git source tree.")
   include(today)
   today(GOBY_VERSION_DATE)  
   execute_process(COMMAND git rev-parse --short HEAD
     OUTPUT_VARIABLE GOBY_LAST_REV)
   string(STRIP ${GOBY_LAST_REV} GOBY_LAST_REV)

   execute_process(COMMAND git diff-index --quiet HEAD
     RESULT_VARIABLE GOBY_DIRTY_REV)
   if(GOBY_DIRTY_REV EQUAL 0)
     set(GOBY_DIRTY_REV_STRING "")
   else()
     set(GOBY_DIRTY_REV_STRING "-dirty")
   endif()
   set(GOBY_VERSION_PATCH "${GOBY_VERSION_PATCH}+git${GOBY_LAST_REV}${GOBY_DIRTY_REV_STRING}")
   
else()
   message("Compiling from release tarball.")
   set(GOBY_VERSION_DATE "2015.09.21")
endif()

set(GOBY_VERSION "${GOBY_VERSION_MAJOR}.${GOBY_VERSION_MINOR}.${GOBY_VERSION_PATCH}")

# give Goby 1 series a few more soversions
set(GOBY_SOVERSION "26")

# fetch all the local directories for generated code
get_filename_component(goby_LIB_DIR lib ABSOLUTE)
get_filename_component(goby_BIN_DIR bin ABSOLUTE)
get_filename_component(goby_INC_DIR include ABSOLUTE)
get_filename_component(goby_SRC_DIR src ABSOLUTE)
get_filename_component(goby_SHARE_DIR share ABSOLUTE)
get_filename_component(goby_BUILD_DIR build ABSOLUTE)
get_filename_component(goby_SCRIPTS_DIR scripts ABSOLUTE)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall") # -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall") # -fprofile-arcs -ftest-coverage")
#set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

## set type of libraries
option(make_static_libs "Build static libraries instead of shared." OFF)

if(make_static_libs)
  set(BUILD_SHARED_LIBS OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()

## build dynamic protobuf manager with ability to read from disk (requires
## boost-filesystem)
option(make_proto_runtime_compile "Allow runtime compilation of .proto files." ON)


## set the cmake defaults for libraries and binaries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${goby_LIB_DIR} CACHE PATH 
  "Output directory for the dynamic libraries" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${goby_BIN_DIR} CACHE PATH
  "Output directory for the binaries" )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${goby_LIB_DIR} CACHE PATH 
  "Output directory for the static libraries (archives)" )

## set instructions for `make install`
file(MAKE_DIRECTORY ${goby_BIN_DIR})
install(DIRECTORY ${goby_BIN_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin 
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

install(FILES ${goby_SCRIPTS_DIR}/analyze_dccl_xml ${goby_SCRIPTS_DIR}/moos_goby_liaison DESTINATION ${CMAKE_INSTALL_PREFIX}/bin 
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

install(DIRECTORY ${goby_LIB_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY ${goby_SHARE_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/goby)
install(DIRECTORY ${goby_INC_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  FILES_MATCHING REGEX .*\\.h|.*\\.proto)

## let cmake know where the headers are
include_directories(${goby_INC_DIR})

## for MAC OS X 
if(${APPLE})
  ## MacPorts
  include_directories(/opt/local/include)
  link_directories(/opt/local/lib)
  ## Fink
  include_directories(/sw/include)
  link_directories(/sw/lib)
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${goby_BIN_DIR}" "${goby_LIB_DIR}" "${goby_INCLUDE_DIR}/goby")

## start adding subdirectories
add_subdirectory(src)

## copy to goby/include
file(GLOB_RECURSE INCLUDE_FILES RELATIVE ${goby_SRC_DIR} src/*.h src/*.proto)
foreach(I ${INCLUDE_FILES})
  configure_file(${goby_SRC_DIR}/${I} ${goby_INC_DIR}/goby/${I} @ONLY)
endforeach()

