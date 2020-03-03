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
#pragma once

#include <lvgl/lvgl.h>
#include <smooth/application/display/DisplaySpi.h>
#include <smooth/application/display/SH1107.h>
#include <smooth/core/io/spi/SpiDmaFixedBuffer.h>

namespace redstone
{
    class DisplayDriver
    {
        public:
            /// Constructor
            DisplayDriver();

            /// Initialize the display driver
            bool initialize();

        private:
            /// Lv tick task - Required by LittlevGL. LittlevGL uses this function to determine how much time has passed
            static void lv_tick_task(void);

            /// SH1107 Flush Callback - C style callback required by LittlevGL
            /// \param drv The display driver structure reference, not used
            /// \param area The area of the display we want to update
            /// \param color_map A pointer to the buffer memory where the graphical objects are drawn.
            // This contains the data that should be flushed to the display
            static void display_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map);

            /// Rounder callback
            /// Increase the y coordinates to the nearest page boundaries which in our case is byte boundaries
            /// \param disp_drv The display driver structure reference, not used
            /// \param area The area of the display we want to update
            static void rounder_cb(struct _disp_drv_t* disp_drv, lv_area_t* area);

            /// Set or clear a single pixel callback
            /// \param disp_drv The display driver structure reference, not used
            /// \param buf The reference to the buffer where we change the pixel
            /// \param buf_w The number of pixel columns
            /// \param x The x coordinate of the pixel to update
            /// \param y The y coordinate of the pixel to update
            /// \param color The monochrome setting for the pixel, either full color or clear
            /// \param opa The color opacity, not used in monochrome displays
            static void set_px_cb(struct _disp_drv_t* disp_drv,
                                  uint8_t* buf,
                                  lv_coord_t buf_w,
                                  lv_coord_t x,
                                  lv_coord_t y,
                                  lv_color_t color,
                                  lv_opa_t opa);

            /// Display Driver Flush
            /// \param drv The display driver structure reference, not used
            /// \param area The area of the display we want to update
            /// \param color_map A pointer to the buffer memory where the graphical objects are drawn.
            // This contains thedata that should be flushed to the display
            void display_drv_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map);

            /// Initialize the SH1107
            /// \return Returns true is successful false if initialization failed
            bool init_display();

            /// Send Page Commands
            /// Sends command to display for setting up a page pixel data transfer
            /// \param page_number The page number to send to display
            /// \param start_col The starting column position
            void send_page_commands(uint8_t page_number, uint8_t start_col);

            /// Send Page Data
            /// Sends a full or a portion of a page worth of pixel data to display
            /// \param data The pointer to the first byte in the data (pixel data)
            /// \param length The number of bytes in the data
            void send_page_data(uint8_t* data, size_t length);

            // Set the screen rotation
            void set_screen_rotation();

        private:
            static constexpr int SH1107_COLUMNS = 64;
            static constexpr int SH1107_PAGES = 16;
            static constexpr int SH1107_SEGMENTS = 128;
            static constexpr int MAX_DMA_LEN = SH1107_SEGMENTS * SH1107_PAGES; // 128 * 16 = 1024
            static constexpr int SH1107_PAGE_CMD_LEN = 64;

            spi_host_device_t spi_host;
            smooth::core::io::spi::Master spi_master;
            std::unique_ptr<smooth::application::display::DisplaySpi> display{};
            bool display_initialized{ false };

            lv_theme_t* theme;
            lv_disp_drv_t disp_drv;
            lv_disp_buf_t disp_buf;
            lv_color1_t* vdb1;

            smooth::core::io::spi::SpiDmaFixedBuffer<uint8_t, MAX_DMA_LEN> video_display_buffer1{};
            smooth::core::io::spi::SpiDmaFixedBuffer<uint8_t, SH1107_PAGE_CMD_LEN> page_commands;
    };
}
