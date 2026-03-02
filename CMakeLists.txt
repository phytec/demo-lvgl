cmake_minimum_required(VERSION 3.11)

project(lvgl-demo-wl)

# Set policy to allow to run the target_link_libraries cmd on targets that are
# build in another directory. Currently, the linking is not handled by
# env_support/cmake/os.cmake. This means that if a driver is enabled and it
# requires linking an external library it needs to be handled in the top-level
# project.
cmake_policy(SET CMP0079 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

set(CONFIG "default" CACHE STRING "Config name to use. Leave empty for default `lv_conf.defaults`")

set(LV_CONF_DEFAULTS_PATH "${CMAKE_SOURCE_DIR}/lv_conf.defaults")

# If CONFIG is not default, update LV_CONF_DEFAULTS_PATH to the selected config
if(NOT CONFIG STREQUAL "default")
    set(LV_CONF_DEFAULTS_PATH "${CMAKE_SOURCE_DIR}/configs/${CONFIG}.defaults")
endif()

if(NOT EXISTS "${LV_CONF_DEFAULTS_PATH}")
    message(FATAL_ERROR "Config file not found: ${LV_CONF_DEFAULTS_PATH}")
endif()
message(STATUS "Using config: ${LV_CONF_DEFAULTS_PATH}")

set(LV_BUILD_SET_CONFIG_OPTS ON CACHE BOOL
    "create CMAKE variables from lv_conf_internal.h" FORCE)

set(LV_BUILD_CONF_PATH
    "${CMAKE_BINARY_DIR}/lv_conf.h"
    CACHE PATH "path to lv_conf.h" FORCE)

set(LVGL_TEMPLATE_PATH "${CMAKE_SOURCE_DIR}/lvgl/lv_conf_template.h")
set(GENERATE_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/lvgl/scripts/generate_lv_conf.py")

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LVGL_TEMPLATE_PATH})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${LV_CONF_DEFAULTS_PATH})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${GENERATE_SCRIPT_PATH})

execute_process(
    COMMAND
        ${Python3_EXECUTABLE} ${GENERATE_SCRIPT_PATH} --template
        ${LVGL_TEMPLATE_PATH} --defaults ${LV_CONF_DEFAULTS_PATH} --config
        ${LV_BUILD_CONF_PATH}
    RESULT_VARIABLE config_result
    OUTPUT_VARIABLE config_output
    ERROR_VARIABLE config_output
)

if(NOT config_result EQUAL 0)
    message(FATAL_ERROR "Failed to generate lv_conf.h: ${config_output}")
endif()

message(STATUS "${LV_BUILD_CONF_PATH} generated successfully")

add_subdirectory(lvgl)

if (CONFIG_LV_USE_WAYLAND)
    message("Including Wayland support")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
    pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor)
    pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

    list(APPEND PKG_CONFIG_LIB ${WAYLAND_CLIENT_LIBRARIES})
    list(APPEND PKG_CONFIG_LIB ${WAYLAND_CURSOR_LIBRARIES})
    list(APPEND PKG_CONFIG_LIB ${XKBCOMMON_LIBRARIES})

    # Wayland protocols
    pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols>=1.20)
    pkg_get_variable(WAYLAND_PROTOCOLS_BASE wayland-protocols pkgdatadir)

    set(PROTOCOL_ROOT "${SYSROOT}/usr/share/wayland-protocols")

    if(NOT EXISTS ${PROTOCOL_ROOT})
        message(FATAL_ERROR "Wayland protocols not found at ${PROTOCOL_ROOT}")
    endif()

    set(PROTOCOLS_DIR "${CMAKE_CURRENT_BINARY_DIR}/protocols")
    file(MAKE_DIRECTORY ${PROTOCOLS_DIR})

    set(WAYLAND_PROTOCOLS_SRC "")

    # xdg-shell protocol
    set(XDG_SHELL_XML "${PROTOCOL_ROOT}/stable/xdg-shell/xdg-shell.xml")
    set(XDG_SHELL_HEADER "${PROTOCOLS_DIR}/wayland_xdg_shell.h")
    set(XDG_SHELL_SOURCE "${PROTOCOLS_DIR}/wayland_xdg_shell.c")

    if(NOT EXISTS ${XDG_SHELL_HEADER} OR NOT EXISTS ${XDG_SHELL_SOURCE})
        message("Generating XDG shell protocol")
        execute_process(COMMAND wayland-scanner client-header ${XDG_SHELL_XML} ${XDG_SHELL_HEADER})
        execute_process(COMMAND wayland-scanner private-code ${XDG_SHELL_XML} ${XDG_SHELL_SOURCE})
    endif()
    list(APPEND WAYLAND_PROTOCOLS_SRC ${XDG_SHELL_SOURCE})

    list(APPEND PKG_CONFIG_INC ${PROTOCOLS_DIR})
endif()

target_include_directories(lvgl PUBLIC ${PKG_CONFIG_INC})
target_link_libraries(lvgl PUBLIC ${PKG_CONFIG_LIB} m pthread rt)

add_executable(${PROJECT_NAME}
    ${WAYLAND_PROTOCOLS_SRC}
    src/main.c
    src/mouse_cursor_icon.c
)
target_include_directories(${PROJECT_NAME} PUBLIC lvgl/src lvgl/demos ${PKG_CONFIG_INC})
target_link_libraries(${PROJECT_NAME} PRIVATE
    lvgl
    lvgl::demos
    lvgl::examples
    wayland-client
    wayland-cursor
    xkbcommon
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
