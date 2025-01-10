install(
    TARGETS render_exe
    RUNTIME COMPONENT render_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
