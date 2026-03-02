/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 PHYTEC Messtechnik GmbH
 */

#include "lvgl.h"

int main(void)
{
    /* LVGL init */
    lv_init();

    /* Wayland display init */
    lv_display_t *disp = lv_wayland_window_create(1280, 800, "window", NULL);
    if (disp == NULL) {
        LV_LOG_ERROR("Failed creating wayland window");
        return 1;
    }
    lv_wayland_window_set_fullscreen(disp, true);

    /* Set a cursor for the touchscreen */
    lv_indev_t *touchscreen = lv_wayland_get_touchscreen(disp);
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t *cursor_obj = lv_img_create(lv_scr_act()); /* Create an image object for the cursor  */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon); /* Set the image source */
    lv_indev_set_cursor(touchscreen, cursor_obj); /* Connect the image object to the driver */

    /* Search connected keyboard and attach it to the driver */
    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(lv_wayland_get_keyboard(disp), g);
    lv_indev_set_group(lv_wayland_get_pointeraxis(disp), g);

    /* Create a Demo */
    lv_demo_widgets();

    uint32_t idle_time;
    while (true) {
        idle_time = lv_wayland_timer_handler();

        if (idle_time != 0) {
            usleep(idle_time * 1000);
        }
        if (!lv_wayland_window_is_open(NULL)) {
            break;
        }
    }

    return 0;
}
