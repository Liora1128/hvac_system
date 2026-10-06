#pragma once


// 一次温湿度采集结果
struct SensorData
{
    float temperature = 0.0f;   // 温度 ℃
    float humidity = 0.0f;      // 湿度 %
};


class ISensorSource
{
public:
    virtual ~ISensorSource() = default;

    // 初始化数据源
    virtual bool open() = 0;

    // 读取一次温湿度数据
    virtual bool read(SensorData& data) = 0;

    // 关闭数据源
    virtual void close() = 0;
};