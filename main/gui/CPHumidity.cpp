/****************************************************************************************
 * CPHumidity.cpp - A content pane that displays humidity
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
#include "gui/CPHumidity.h"

#include <smooth/core/logging/log.h>
using namespace smooth::core::logging;

namespace redstone
{
    // Class constants
    static const char* TAG = "CPHumidity";

    // Constructor
    CPHumidity::CPHumidity(smooth::core::Task& task_lvgl) :
            subr_queue_envir_value(SubQEnvirValue::create(2, task_lvgl, *this))

            // Create Subscriber Queue (SubQ) so this content pane can listen for
            // EnvirValue events
            // the queue will hold up to 2 items
            // the "task_lvgl" is this task which to signal when an event is available.
            // the "*this" is the class instance that will receive the events
    {
    }

    // Create the content pane
    void CPHumidity::create(int width, int height)
    {
        Log::info(TAG, "Creating CPHumidity");

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

        // create style for humidity value
        lv_style_copy(&humidity_label_style, &lv_style_plain);
        humidity_label_style.text.font = &lv_font_14x14B_latin1_sup;
        humidity_label_style.text.color = LV_COLOR_WHITE;

        // create a dynamic label for humidity measurement value
        humidity_value_label = lv_label_create(content_container, NULL);
        lv_obj_set_style(humidity_value_label, &humidity_label_style);
        lv_label_set_text(humidity_value_label, "--");
        lv_obj_align(humidity_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // The published EnvirValue event
    void CPHumidity::event(const EnvirValue& event)
    {
        humidity = event.get_relative_humidity();
        update_humidity_text();
    }

    // Update the humidity value label
    void CPHumidity::update_humidity_text()
    {
        std::ostringstream stream;

        stream << std::fixed << std::setprecision(0) << humidity;
        std::string humidity_text = stream.str() + " %RH";
        lv_label_set_text(humidity_value_label, humidity_text.c_str());
        lv_obj_align(humidity_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // Show the content pane
    void CPHumidity::show()
    {
        lv_obj_set_hidden(content_container, false);
    }

    // Hide the content pane
    void CPHumidity::hide()
    {
        lv_obj_set_hidden(content_container, true);
    }
}
