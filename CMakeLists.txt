cmake_minimum_required(VERSION 3.20)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/prelude.cmake)

project(
    render
    VERSION 0.0.1
    DESCRIPTION "2D Cached Software Renderer"
    HOMEPAGE_URL "https://lder.dev"
    LANGUAGES CXX
)



include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

include(cmake/sdl.cmake)

# ---- Declare library ----

add_library(
    render_lib OBJECT

    # layout engine
    src/engine/arena.cpp
    src/engine/engine.cpp

    # backends
    src/backend/SDL3/render.cpp
)

target_include_directories(
    render_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(render_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(render_exe src/main.cpp)
add_executable(render::exe ALIAS render_exe)

set_property(TARGET render_exe PROPERTY OUTPUT_NAME render)

target_compile_features(render_exe PRIVATE cxx_std_20)

target_link_libraries(render_exe PRIVATE render_lib SDL3::SDL3 SDL3_ttf::SDL3_ttf)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT render_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of render"
  )
endif()

include(cmake/dev-mode.cmake)