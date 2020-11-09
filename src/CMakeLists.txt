add_executable(pa-notify main.c)

set_target_properties(pa-notify PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(PULSEAUDIO REQUIRED libpulse)

target_link_libraries(pa-notify 
    ${GLIB_LDFLAGS}
    ${PULSEAUDIO_LDFLAGS}
)

target_include_directories(
    pa-notify
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${PULSEAUDIO_INCLUDE_DIRS}
)
