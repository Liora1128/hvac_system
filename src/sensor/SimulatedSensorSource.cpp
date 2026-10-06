#include "sensor/SimulatedSensorSource.hpp"

#include <iostream>


bool SimulatedSensorSource::open()
{
    // 标记数据源已经打开
    opened_ = true;

    // 记录第一次更新时间
    lastUpdate_ = std::chrono::steady_clock::now();

    std::cout
        << "Simulated sensor opened successfully."
        << std::endl;

    return true;
}


bool SimulatedSensorSource::read(SensorData& data)
{
    if(!opened_)
    {
        return false;
    }


    // 当前时间
    auto now = std::chrono::steady_clock::now();

    // 距离上一次更新经过了多少秒
    float deltaTime =
        std::chrono::duration<float>(
            now - lastUpdate_
        ).count();

    lastUpdate_ = now;


    // 室内温度缓慢向外界温度靠近
    temperature_ +=
        (ambientTemperature_ - temperature_)
        * 0.002f
        * deltaTime;


    // 室内湿度缓慢向外界湿度靠近
    humidity_ +=
        (ambientHumidity_ - humidity_)
        * 0.001f
        * deltaTime;


    // 返回模拟传感器读数
    // 在真实状态基础上加一点测量噪声
    data.temperature =
        temperature_
        + temperatureNoise_(randomEngine_);

    data.humidity =
        humidity_
        + humidityNoise_(randomEngine_);


    return true;
}


void SimulatedSensorSource::close()
{
    opened_ = false;

    std::cout
        << "Simulated sensor closed."
        << std::endl;
}