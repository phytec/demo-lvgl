/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023 PHYTEC Messtechnik GmbH
 */

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/wayland/wayland.h"
#include <unistd.h>

int main(void)
{
    /*LVGL init*/
    lv_init();

    /* Wayland display init */
    lv_wayland_init();
    lv_disp_t * disp = lv_wayland_create_window(1280, 800, "window", NULL);
    lv_wayland_window_set_fullscreen(disp, true);

    lv_indev_t * mouse = lv_wayland_get_touchscreen(disp);

    /*Set a cursor for the mouse*/
    LV_IMG_DECLARE(mouse_cursor_icon)
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    lv_indev_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/

    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);

    lv_indev_t * cur_drv = NULL;
    for (;;) {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv) {
            break;
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(cur_drv, g);
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_ENCODER) {
            lv_indev_set_group(cur_drv, g);
        }
    }

    /*Create a Demo*/
    lv_demo_widgets();

    while(1) {
        lv_tick_inc(5);
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}
