add_executable(pcat pcat.c)
target_link_libraries(pcat common)

install(TARGETS pcat DESTINATION bin)
