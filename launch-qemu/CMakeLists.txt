add_executable(launch-qemu main.c)

set_target_properties(launch-qemu PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("Debug configuration.")
else()
    message("Release configuration.")
endif()

set(CMAKE_SYSTEM_NAME CMAKE_HOST_SYSTEM_NAME)
set(CMAKE_SYSTEM_VERSION CMAKE_HOST_SYSTEM_VERSION)
set(CMAKE_SYSTEM_PROCESSOR CMAKE_HOST_SYSTEM_PROCESSOR)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_ASM-ATT_COMPILER "as")

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ALWAYS)