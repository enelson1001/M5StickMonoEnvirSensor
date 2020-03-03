/****************************************************************************************
 * CPHeatIndex.cpp - A content pane that displays heat index in fahrenheit
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
#include "gui/CPHeatIndex.h"

#include <smooth/core/logging/log.h>
using namespace smooth::core::logging;

namespace redstone
{
    // Class constants
    static const char* TAG = "CPHeatIndex";

    // Constructor
    CPHeatIndex::CPHeatIndex(smooth::core::Task& task_lvgl) :
            subr_queue_envir_value(SubQEnvirValue::create(2, task_lvgl, *this))

            // Create Subscriber Queue (SubQ) so this content pane can listen for
            // EnvirValue events
            // the queue will hold up to 2 items
            // the "task_lvgl" is this task which to signal when an event is available.
            // the "*this" is the class instance that will receive the events
    {
    }

    // Create the content pane
    void CPHeatIndex::create(int width, int height)
    {
        Log::info(TAG, "Creating CPHeatIndex");

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

        // create style for heat_index value
        lv_style_copy(&heat_index_label_style, &lv_style_plain);
        heat_index_label_style.text.font = &lv_font_14x14B_latin1_sup;
        heat_index_label_style.text.color = LV_COLOR_WHITE;

        // create a dynamic label for heat index measurement value
        heat_index_value_label = lv_label_create(content_container, NULL);
        lv_obj_set_style(heat_index_value_label, &heat_index_label_style);
        lv_label_set_text(heat_index_value_label, "--");
        lv_obj_align(heat_index_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // The published EnvirValue event
    void CPHeatIndex::event(const EnvirValue& event)
    {
        heat_index = event.get_heat_index_fahrenheit();
        update_heat_index_text();
    }

    // Update the heat index value label
    void CPHeatIndex::update_heat_index_text()
    {
        std::ostringstream stream;

        stream << std::fixed << std::setprecision(1) << heat_index;
        std::string heat_index_text = stream.str() + "\u00b0" + "F";
        lv_label_set_text(heat_index_value_label, heat_index_text.c_str());
        lv_obj_align(heat_index_value_label, NULL, LV_ALIGN_CENTER, 5, 0);
    }

    // Show the content pane
    void CPHeatIndex::show()
    {
        lv_obj_set_hidden(content_container, false);
    }

    // Hide the content pane
    void CPHeatIndex::hide()
    {
        lv_obj_set_hidden(content_container, true);
    }
}
