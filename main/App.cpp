/****************************************************************************************
 * App.cpp - The Application class
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

//*****************************************************************************************************************
// Typical output on M5Stick-Mono
//
//  MemStat: Mem type |  8-bit free | Smallest block | Minimum free | 32-bit free | Smallest block | Minimum free
//  MemStat: INTERNAL |      163216 |         113804 |       162708 |      213528 |         113804 |       213012
//  MemStat:      DMA |      163216 |         113804 |       162708 |      163216 |         113804 |       162708
//  MemStat:   SPIRAM |           0 |              0 |            0 |           0 |              0 |            0
//  MemStat:
//  MemStat:             Name |      Stack |  Min free stack |  Max used stack
//  MemStat:         LvglTask |       4096 |             604 |            3492
//  MemStat:   PollSensorTask |       3300 |             928 |            2372
//  MemStat: SocketDispatcher |      20480 |           18400 |            2080
//  MemStat:         MainTask |      16384 |           12692 |            3692
//
// Esp32-IDF version: v4.0-beta2
// Toolchain version: xtensa-esp32-elf-gcc (crosstool-NG esp32-2019r1) 8.2.0
// Lvgl version: v6.1.2 SHA1: 2ead4959
// Smooth version: master SHA1: b4bf80b4
// Bin file size: 1,120,928 bytes 
//******************************************************************************************************************
#include "App.h"
#include <smooth/core/task_priorities.h>
#include <smooth/core/logging/log.h>
#include <smooth/core/SystemStatistics.h>

using namespace smooth::core;
using namespace std::chrono;

namespace redstone
{
    // Class Constants
    static const char* TAG = "APP";

    // Constructor                                     
    App::App() : Application(APPLICATION_BASE_PRIO, seconds(60))
    {
    }

    // Initialize the application
    void App::init()
    {
        Log::warning(TAG, "============ Starting APP  ===========");
        Application::init();
        lvgl_task.start();
        poll_sensor_task.start();
    }

    // Tick event happens every 60 seconds
    void App::tick()
    {
        Log::warning(TAG, "============ M5StickMonoEnvir Tick  =============");

        if (!heap_caps_check_integrity_all(true))
        {
            Log::error(TAG, "========= Heap Corrupted  ===========");
        }

        SystemStatistics::instance().dump();
    }
}
