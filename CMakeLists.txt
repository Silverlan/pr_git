include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pr_git)
pr_add_module(${PROJ_NAME} SHARED "modules/git/")

pr_add_dependency(${PROJ_NAME} ishared TARGET PUBLIC)

pr_init_module(${PROJ_NAME})

set(USE_SSH OFF CACHE BOOL "Link with libssh2 to enable SSH support" FORCE)
set(BUILD_CLAR OFF CACHE BOOL "Git SSH" FORCE)
find_package(zlib REQUIRED)
add_subdirectory(third_party/libgit2)
set_target_properties(git2 PROPERTIES FOLDER third_party/libgit2)
set_target_properties(git2internal PROPERTIES FOLDER third_party/libgit2)
set_target_properties(http-parser PROPERTIES FOLDER third_party/libgit2)
if(WIN32)
	set_target_properties(pcre PROPERTIES FOLDER third_party/libgit2)
endif()

pr_add_dependency(${PROJ_NAME} git2 TARGET PUBLIC)
target_include_directories(${PROJ_NAME} PUBLIC third_party/libgit2/include/)

pr_finalize(${PROJ_NAME})
