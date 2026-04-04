add_executable(mercury main.cpp)

target_link_libraries(mercury PRIVATE mercury::headers mercury::warnings)
