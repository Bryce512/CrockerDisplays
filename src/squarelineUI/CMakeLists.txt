SET(SOURCES ui_Screen1.c
    ui_Screen2.c
    ui_Screen3.c
    ui_theme_manager.cpp
    ui_themes.cpp
    ui.c
    ui_comp_hook.c
    ui_helpers.c)

add_library(ui ${SOURCES})
