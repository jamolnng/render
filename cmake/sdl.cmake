find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE TRUE
  )

  FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
    GIT_TAG main
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE TRUE
  )

  set(SDL_STATIC ON)
  set(SDL_SHARED OFF)
  set(SDL_STATIC_PIC ON)
  set(BUILD_SHARED_LIBS FALSE)
  FetchContent_MakeAvailable(SDL3 SDL3_ttf)
endif()