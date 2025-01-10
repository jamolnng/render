# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(renderTests LANGUAGES CXX)

# ---- Tests ----

add_executable(render_test source/render_test.cpp)
target_link_libraries(render_test PRIVATE render_lib)
target_compile_features(render_test PRIVATE cxx_std_20)

add_test(NAME render_test COMMAND render_test)

# ---- End-of-file commands ----

add_folders(Test)
