add_executable(loop loop.c loop_utils.c loop_animation.c)
target_link_libraries(loop common)

install(TARGETS loop DESTINATION bin)