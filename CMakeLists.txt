cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_git)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(vfilesystem)
link_external_library(sharedutils)
link_external_library(mathutil)
link_external_library(ishared)
link_external_library(libgit2)
add_external_library(luajit)
if(WIN32)
	add_external_library(lua)
endif()
add_include_dir(glm)

link_generic_module_libraries()

add_subdirectory(third_party/libgit2)
set_target_properties(git2 PROPERTIES FOLDER third_party/libgit2)
set_target_properties(git2internal PROPERTIES FOLDER third_party/libgit2)
set_target_properties(http-parser PROPERTIES FOLDER third_party/libgit2)
set_target_properties(libgit2_clar PROPERTIES FOLDER third_party/libgit2)
set_target_properties(pcre PROPERTIES FOLDER third_party/libgit2)

register_pragma_module(${PROJ_NAME})

target_include_directories(git2internal PUBLIC ${DEPENDENCY_ZLIB_BUILD_INCLUDE})

add_dependencies(pr_git git2)
