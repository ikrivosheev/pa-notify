cmake_minimum_required(VERSION 3.5)

project(pa-notify VERSION 1.3.0)

add_subdirectory(src)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)
install(    
    FILES "man/pa-notify.1"
    DESTINATION share/man/man1
)

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PulseAudio volume notification")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "py.krivosheev@gmail.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ivan Krivosheev")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)
set(CPACK_DEB_COMPONENT_INSTALL YES)
set(CPACK_GENERATOR "DEB")
include(CPack)
