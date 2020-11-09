cmake_minimum_required(VERSION 3.5)

project(pa-notify VERSION 0.0.1)

add_subdirectory(src)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)
install(    
    FILES "man/pa-notify.1"
    DESTINATION share/man/man1
)
