add_executable(pa-notify main.c)

set_target_properties(pa-notify PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(PULSEAUDIO REQUIRED IMPORTED_TARGET libpulse)
pkg_search_module(PULSEMAINLOOPGLIB REQUIRED IMPORTED_TARGET libpulse-mainloop-glib)
pkg_search_module(LIBNOTIFY REQUIRED IMPORTED_TARGET libnotify)

target_compile_options(pa-notify PRIVATE -Wall -Wextra)

target_link_libraries(pa-notify
    PRIVATE
    m
    PkgConfig::GLIB
    PkgConfig::PULSEAUDIO
    PkgConfig::PULSEMAINLOOPGLIB
    PkgConfig::LIBNOTIFY
)
