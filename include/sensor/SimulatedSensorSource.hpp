#pragma once

#include "ports/ISensorSource.hpp"

#include <chrono>
#include <random>


class SimulatedSensorSource : public ISensorSource
{
    public:
        bool open() override;
        bool read(SensorData& data) override;
        void close() override;

    private:
        bool opened_ = false;

        float temperature_ = 28.0f;
        float humidity_ = 60.0f;

        float ambientTemperature_ = 30.0f;
        float ambientHumidity_ = 65.0f;

        std::chrono::steady_clock::time_point lastUpdate_;

        std::mt19937 randomEngine_{std::random_device{}()};

        std::normal_distribution<float> temperatureNoise_{
            0.0f,
            0.03f
        };

        std::normal_distribution<float> humidityNoise_{
            0.0f,
            0.1f
        };
};