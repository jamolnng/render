find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )
  set(SDL_STATIC ON)
  set(SDL_SHARED OFF)
  FetchContent_MakeAvailable(SDL3)
endif()