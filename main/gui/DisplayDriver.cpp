/****************************************************************************************
 * DisplayDriver.h - A LittlevGL Display Driver for SH1107 - Resolution: 64×128
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
#include "gui/DisplayDriver.h"
#include <esp_freertos_hooks.h>
#include <smooth/core/logging/log.h>

using namespace smooth::core::io::spi;
using namespace smooth::application::display;
using namespace smooth::core::logging;
using namespace std::chrono;

namespace redstone
{
    // Class Constants
    static const char* TAG = "DisplayDriver";

    // Constructor
    DisplayDriver::DisplayDriver() :
            spi_host(VSPI_HOST),            // Use VSPI as host

            spi_master(
                spi_host,                   // host VSPI
                DMA_1,                      // use DMA
                GPIO_NUM_23,                // mosi gpio pin
                GPIO_NUM_NC,                // miso gpio pin - none
                GPIO_NUM_18,                // clock gpio pin
                MAX_DMA_LEN)                // max transfer size
    {
    }

    // Initialize the display driver
    bool DisplayDriver::initialize()
    {
        Log::info(TAG, "Initializing Lvgl SH1107 Display Driver ........");

        display_initialized = init_display();

        if (display_initialized)
        {
            // set screen rotation
            set_screen_rotation();

            // initialize LittlevGL graphics library
            lv_init();

            // The video_display_buffer1 is used by LittlevGL to draw screen content.
            // Verfiy that DMA buffer - video_display_buffer1 has been allocated.
            if (video_display_buffer1.is_buffer_allocated())
            {
                // initialize a display buffer
                vdb1 = reinterpret_cast<lv_color1_t*>(video_display_buffer1.data());
                lv_disp_buf_init(&disp_buf, vdb1, NULL, MAX_DMA_LEN);

                // initialize and register a display driver
                lv_disp_drv_init(&disp_drv);
                disp_drv.buffer = &disp_buf;
                disp_drv.flush_cb = display_flush_cb;
                disp_drv.set_px_cb = set_px_cb;
                disp_drv.rounder_cb = rounder_cb;
                disp_drv.user_data = this;
                lv_disp_drv_register(&disp_drv);

                // Set the mono system theme
                theme = lv_theme_mono_init(0, NULL);
                lv_theme_set_current(theme);
            }
            else
            {
                display_initialized = false;
            }
        }

        return display_initialized;
    }

    // Initialize the SH1107
    bool DisplayDriver::init_display()
    {
        auto device = spi_master.create_device<DisplaySpi>(
                        GPIO_NUM_14,            // chip select gpio pin
                        GPIO_NUM_27,            // data command gpio pin
                        //GPIO_NUM_33,            // reset gpio pin
                        0,                      // spi command_bits
                        0,                      // spi address_bits,
                        0,                      // bits_between_address_and_data_phase,
                        0,                      // spi_mode = 0,
                        128,                    // spi positive_duty_cycle,
                        0,                      // spi cs_ena_posttrans,
                        SPI_MASTER_FREQ_8M,     // spi-sck = 8MHz
                        0,                      // full duplex (4-wire)
                        1,                      // queue_size,
                        true,                   // use pre-trans callback
                        true);                  // use post-trans callback

        bool res = device->init(spi_host);

        if (res)
        {
            //device->hw_reset(true, milliseconds(5), milliseconds(120));
            // add reset pin - pullup=false, pulldown=false, active_high=false
            device->add_reset_pin(std::make_unique<DisplayPin>(GPIO_NUM_33, false, false, false));
            device->hw_reset(true, milliseconds(5), milliseconds(120));  // reset chip

            // initialize the display
            res &= device->send_cmds(sh1107_init_cmds_1.data(), sh1107_init_cmds_1.size());
            display = std::move(device);
        }
        else
        {
            Log::error(TAG, "Initializing of SPI Device: SH1107 --- FAILED");
        }

        return res;
    }

    // Set screen rotation
    void DisplayDriver::set_screen_rotation()
    {
        bool res = true;

        // the default screen oreintation via init_cmds is portrait, check to see if
        // lv_config file has oreintation set for landscape
        if (LV_HOR_RES_MAX > LV_VER_RES_MAX)
        {
            res = display->send_cmd(SH1107Cmd::CommonOutputScanDirLandscape);
        }

        if (!res)
        {
            Log::error(TAG, "Setting screen rotation --- FAILED");
        }
    }

    // A class instance callback to flush the display buffer and thereby write colors to screen
    void DisplayDriver::display_drv_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map)
    {
        uint8_t start_col;
        uint8_t end_col;
        uint8_t start_page;
        uint8_t end_page;

        if (LV_VER_RES_MAX > LV_HOR_RES_MAX)
        {
            // Portrait
            start_col = area->x1;
            end_col = area->x2;
            start_page = area->y1 >> 3; // start row on page boundary
            end_page = area->y2 >> 3;   // end row on page boundary
        }
        else
        {
            // Landscape
            start_col = area->y1;
            end_col = area->y2;
            start_page = area->x1 >> 3; // start row on page boundary
            end_page = area->x2 >> 3;   // end row on page boundary
        }

        for (uint8_t page = start_page; page <= end_page; page++)
        {
            send_page_commands(page, start_col);
            send_page_data(reinterpret_cast<uint8_t*>(color_map), static_cast<uint8_t>(end_col - start_col + 1));
            color_map += SH1107_COLUMNS;
        }

        // Inform the lvgl graphics library that we are ready for flushing buffer
        lv_disp_t* disp = lv_refr_get_disp_refreshing();
        lv_disp_flush_ready(&disp->driver);
    }

    // To send a page of pixel data we have to send a command to set the upper column
    // address bits and then send a command to set the lower column address bits
    // and then send a command to set the page address before sending the pixel data itself.
    void DisplayDriver::send_page_commands(uint8_t page_number, uint8_t start_col)
    {
        // set page address and column address
        page_commands[0] = SH1107Cmd::UpperColumnAddress | ((start_col >> 4) & 0x0F);
        page_commands[1] = SH1107Cmd::LowerColumnAddress | (start_col & 0x0F);
        page_commands[2] = SH1107Cmd::PageAddress0 | page_number;

        if (!display->send_cmds(page_commands.data(), 3))
        {
            Log::error(TAG, "Failed to send page commands");
        }
    }

    // Send pixel data
    void DisplayDriver::send_page_data(uint8_t* data, size_t length)
    {
        if (!display->send_data(data, length))
        {
            Log::error(TAG, "Failed to send page data");
        }
    }

    // The "C" style callback, rounder increase the y1, y2 to byte boundary
    void IRAM_ATTR DisplayDriver::rounder_cb(struct _disp_drv_t* disp_drv, lv_area_t* area)
    {
        if (LV_VER_RES_MAX > LV_HOR_RES_MAX)
        {
            // Portrait
            area->y1 = area->y1 & ~0x7;
            area->y2 = area->y2 | 0x7;
        }
        else
        {
            // Landscape
            area->x1 = (area->x1 & (~0x7));
            area->x2 = (area->x2 & (~0x7)) + 7;
        }
    }

    // The "C" style callback, set_px is used to set or clear single pixel
    // in buf (video display buffer)c from supplied x,y coordinates
    void IRAM_ATTR DisplayDriver::set_px_cb(struct _disp_drv_t* disp_drv,
                                            uint8_t* buf,
                                            lv_coord_t buf_w,
                                            lv_coord_t x,
                                            lv_coord_t y,
                                            lv_color_t color,
                                            lv_opa_t opa)
    {
        uint16_t buf_index;
        uint8_t bit_index;

        if (LV_VER_RES_MAX > LV_HOR_RES_MAX)
        {
            // Potrait
            buf_index = x + (SH1107_COLUMNS * (y >> 3));
            bit_index = y & 0x07;
        }
        else
        {
            // Landscape
            buf_index = y + (SH1107_COLUMNS * (x >> 3));
            bit_index = x & 0x07;
        }

        if (color.full == 0)
        {
            // clear the bit
            buf[buf_index] &= static_cast<uint8_t>(~(1 << bit_index));
        }
        else
        {
            // set the bit
            buf[buf_index] |= static_cast<uint8_t>(1 << bit_index);
        }
    }

    // The "C" style callback required by LittlevGL
    void IRAM_ATTR DisplayDriver::display_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area,
                                                   lv_color_t* color_map)
    {
        DisplayDriver* driver = reinterpret_cast<DisplayDriver*>(drv->user_data);
        driver->display_drv_flush(drv, area, color_map);
    }
}
