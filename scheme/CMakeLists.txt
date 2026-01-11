file(GLOB SRC CONFIGURE_DEPENDS "*.cpp")
add_library(libscheme ${SRC})
target_include_directories(libscheme PUBLIC .)

# scheme-repl is not used for testing,
# but you may use it if you need to experiment with the code.
add_executable(scheme-repl repl/main.cpp)
target_link_libraries(scheme-repl libscheme)

file(GLOB SRC_TEST CONFIGURE_DEPENDS "tests/*.cpp")
add_catch(test_scheme ${SRC_TEST})
target_link_libraries(test_scheme PRIVATE libscheme)
