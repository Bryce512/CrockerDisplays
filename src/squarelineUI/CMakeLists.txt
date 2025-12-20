SET(SOURCES ui_Screen1.c
    ui_Screen2.c
    ui_Screen3.c
    ui_theme_manager.cpp
    ui_themes.cpp
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_backpacks_png.c
    ui_img_rightarrow2_png.c
    ui_img_cog_png.c)

add_library(ui ${SOURCES})
