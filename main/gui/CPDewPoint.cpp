/****************************************************************************************
 * CPDewPoint.cpp - A content pane that displays dew point in fahrenheit
 *
 * Created on Jan. 04, 2020
 * Copyright (c) 2019 Ed Nelson (https://github.com/enelson1001)
 * Licensed under MIT License (see LICENSE file)
 *
 * Derivative Works
 * Smooth - A C++ framework for embedded programming on top of Espressif's ESP-IDF
 * Copyright 2019 Per Malmberg (https://gitbub.com/PerMalmberg)
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * LittlevGL - A powerful and easy-to-use embedded GUI
 * Copyright (c) 2016 Gábor Kiss-Vámosi (https://github.com/littlevgl/lvgl)
 * Licensed under MIT License
 ***************************************************************************************/
#include <sstream>
#include <iomanip>  // for set precision
#include "gui/CPDewPoint.h"

#include <smooth/core/logging/log.h>
using namespace smooth::core::logging;

namespace redstone
{
    // Class constants
    static const char* TAG = "CPDewPoint";

    // Constructor
    CPDewPoint::CPDewPoint(smooth::core::Task& task_lvgl) :
            subr_queue_envir_value(SubQEnvirValue::create(2, task_lvgl, *this))

            // Create Subscriber Queue (SubQ) so this content pane can listen for
            // EnvirValue events
            // the queue will hold up to 2 items
            // the "task_lvgl" is this task which to signal when an event is available.
            // the "*this" is the class instance that will receive the events
    {
    }

    // Create the content pane
    void CPDewPoint::create(int width, int height)
    {
        Log::info(TAG, "Creating CPDewPoint");

        // create style for the content container
        lv_style_copy(&content_container_style, &lv_style_plain);
        content_container_style.body.main_color = LV_COLOR_BLACK;
        content_container_style.body.grad_color = LV_COLOR_BLACK;

        // create a content container
        content_container = lv_cont_create(lv_scr_act(), NULL);
        lv_obj_set_size(content_container, width, height);
        lv_cont_set_layout(content_container, LV_LAYOUT_CENTER);
        lv_obj_align(content_container, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_cont_set_style(content_container, LV_CONT_STYLE_MAIN, &content_container_style);
        lv_obj_set_hidden(content_container, true);

        // create style for dew_point value
        lv_style_copy(&dew_point_label_style, &lv_style_plain);
        dew_point_label_style.text.font = &lv_font_14x14B_latin1_sup;
        dew_point_label_style.text.color = LV_COLOR_WHITE;

        // create a dynamic label for dew point measurement value
        dew_point_value_label = lv_label_create(content_container, NULL);
        lv_obj_set_style(dew_point_value_label, &dew_point_label_style);
        lv_label_set_text(dew_point_value_label, "--");
        lv_obj_align(dew_point_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // The published EnvirValue event
    void CPDewPoint::event(const EnvirValue& event)
    {
        dew_point = event.get_dew_point_fahrenheit();
        update_dew_point_text();
    }

    // Update the dew point value label
    void CPDewPoint::update_dew_point_text()
    {
        std::ostringstream stream;

        stream << std::fixed << std::setprecision(1) << dew_point;
        std::string dew_point_text = stream.str() + "\u00b0" + "F";
        lv_label_set_text(dew_point_value_label, dew_point_text.c_str());
        lv_obj_align(dew_point_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // Show the content pane
    void CPDewPoint::show()
    {
        lv_obj_set_hidden(content_container, false);
    }

    // Hide the content pane
    void CPDewPoint::hide()
    {
        lv_obj_set_hidden(content_container, true);
    }
}
