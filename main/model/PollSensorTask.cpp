/****************************************************************************************
 * PollSensorTask.cpp - A task that runs periodically to retreive DHT12 measurements.
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
#include "model/PollSensorTask.h"
#include <smooth/core/ipc/Publisher.h>

using namespace std::chrono;
using namespace smooth::core;
using namespace smooth::core::ipc;
using namespace smooth::application::sensor;

namespace redstone
{
    // Class constants
    static const char* TAG = "PollSensorTask";

    // Constructor
    PollSensorTask::PollSensorTask() :
            smooth::core::Task("PollSensorTask", 3300, 10, seconds(30)),

            // The Task Name = "PollSensorTask"
            // The stack size is 3300 bytes
            // The priority is set to 10
            // The tick interval is 30 sec

            i2c_master(I2C_NUM_0,                       // I2C Port 0
                       GPIO_NUM_13,                     // SCL pin
                       false,                           // SCL internal pullup NOT enabled
                       GPIO_NUM_25,                     // SDA pin
                       false,                           // SDA internal pullup NOT enabled
                       100 * 1000)                      // clock frequency - 100kHz
    {
    }

    // Initialize the Task
    void PollSensorTask::init()
    {
        dht12_initialized = init_i2c_dht12();
        Log::info(TAG, "DHT12 intialization --- {}", dht12_initialized ? "Succeeded" : "Failed");
    }

    // Initialize the I2C DHT12 device
    bool PollSensorTask::init_i2c_dht12()
    {
        bool res = true;
        auto device = i2c_master.create_device<DHT12>(0x5C);   // DHT12 i2c device address  0x5c

        Log::info(TAG, "Scanning for DHT12");

        if (device->is_present())
        {
            Log::warning(TAG, "DHT12 found");
            sensor = std::move(device);
        }
        else
        {
            Log::error(TAG, "DHT12 not present");
            res = false;
        }

        return res;
    }

    // The task tick event happens every 30 seconds
    void PollSensorTask::tick()
    {
        if (dht12_initialized)
        {
            float temperature, humidity;
            sensor->read_measurements(humidity, temperature);

            envir_value.set_temperture_degree_C(temperature);
            envir_value.set_relative_humidity(humidity);

            Publisher<EnvirValue>::publish(envir_value);
        }
    }
}
