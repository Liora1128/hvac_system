#include <opencv2/highgui.hpp>

#include <winsock2.h>
#include <windows.h>
#include <algorithm>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>

#include <string>
#include <iostream>
#include <memory>
#include <chrono>

#include "ports/IFrameSource.hpp"
#include "camera/OpenCvCameraSource.hpp"
#include "video/VideoFileSource.hpp"
#include "display/DisplayWindow.hpp"
#include "factory/FrameSourceFactory.hpp"

#include "detector/HogPeopleDetector.hpp"
#include "display/DetectionOverlay.hpp"
#include "detector/YoloPeopleDetector.hpp"
#include "factory/PeopleDetectorFactory.hpp"
#include "filter/PeopleCountFilter.hpp"
#include "sensor/SimulatedSensorSource.hpp"

#include "protocol/ModbusTcpClient.hpp"


int main()
{
    // 打开配置文件
    std::ifstream configFile("config/config.json");
    if(!configFile.is_open())
    {
        std::cout << "failed to open config file." << std::endl;
        return 1;
    }

    // 解析json
    nlohmann::json config;
    configFile >> config;

    // 创建帧源工厂
    FrameSourceFactory frameSourceFactory;
    // 创建帧源
    std::unique_ptr<IFrameSource> frameSource = frameSourceFactory.create(config);
    if(!frameSource->open())
    {
        std::cerr << "[MAIN] Failed to open " << frameSourceFactory.getFrameSourceType() << " source." << std::endl; 
        return -1;
    }

    std::cout << "[MAIN] " << frameSourceFactory.getFrameSourceType() << " opened successfully." << std::endl;

    // 读取第一帧
    cv::Mat frame;
    if(!frameSource->read(frame))
    {
        std::cerr << "[MAIN] Failed to read frame from " << frameSourceFactory.getFrameSourceType() << "." << std::endl;
        return -1;
    }

    // 创建显示窗口
    DisplayWindow displayWindow("Building HVAC Camera", 0.8);
    if(!displayWindow.open(frame))
    {
        std::cerr << "[MAIN] Failed to open display window." << std::endl;
        return -1;
    }

    // 创建检测器工厂
    PeopleDetectorFactory peopleDetectorFactory;
    
    std::unique_ptr<IPeopleDetector> peopleDetector = 
        peopleDetectorFactory.create(config);
    
    // 防止配置错误导致返回 nullptr
    if(!peopleDetector)
    {
        std::cerr
            << "[MAIN] Failed to create people detector."
            << std::endl;

        return -1;
    }

    std::cout
        << "[MAIN] "
        << peopleDetectorFactory.getDetectorType()
        << " detector created successfully."
        << std::endl;


    // 创建检测器
    DetectionOverlay overlay;

    std::size_t windowSize = config["window_size"].get<std::size_t>();
    // 保存最近 5 帧的人数，用来稳定 YOLO 检测结果
    PeopleCountFilter peopleCountFilter(windowSize);

    // =========================
    // 创建模拟温湿度数据源
    // =========================
    // SimulatedSensorSource sensorSource;
    // if(!sensorSource.open())
    // {
    //     std::cerr << "Failed to open simulated sensor." << std::endl;
    //     return -1;
    // }

    // // 保存最近一次温湿度数据
    // SensorData sensorData;

    // // 让程序启动后立即读取一次
    // auto lastSensorRead =
    //     std::chrono::steady_clock::now()
    //     - std::chrono::seconds(1);

    ModbusTcpClient plcClient;

    if(!plcClient.connectToServer(
        "127.0.0.1",
        5020
    ))
    {
        std::cerr
            << "[MAIN] Failed to connect to PLC."
            << std::endl;

        return -1;
    }

    // 记录上一次向 PLC 写人数的时间
    auto lastPlcWrite =
        std::chrono::steady_clock::now()
        - std::chrono::seconds(1);
    auto lastPlcRead =
        std::chrono::steady_clock::now()
        - std::chrono::seconds(1);
    

    while(true)
    {
        //  检测目标人数，返回检测到的人的矩形框
        std::vector<cv::Rect> people = peopleDetector->detect(frame);

        // 统当前帧人数
        int rawCount = static_cast<int>(people.size());
        // 滤波后的人数
        int stableCount = peopleCountFilter.update(rawCount);

        // 每隔 1 秒向 PLC 写入一次人数
        auto now = std::chrono::steady_clock::now();

        if(now - lastPlcWrite >= std::chrono::seconds(1))
        {
            if(plcClient.writeSingleRegister(
                0, // 寄存器地址
                static_cast<uint16_t>(stableCount) // 写入人数
            ))
            {
                std::cout
                    << "[MAIN] Wrote stable count: "
                    << stableCount
                    << std::endl;
            }
            else
            {
                std::cerr
                    << "[MAIN] Failed to write stable count."
                    << std::endl;
            }

            lastPlcWrite = now;
        }

        std::vector<uint16_t> plcStatus;

        if(now - lastPlcRead >= std::chrono::seconds(1))
        {
            if(plcClient.readHoldingRegisters(100,2,plcStatus))
            {
                uint16_t hvacMode = plcStatus[0];
                uint16_t fanLevel = plcStatus[1];

                std::cout
                    << "[MAIN] PLC Status - HVAC Mode: "
                    << hvacMode
                    << ", Fan Level: "
                    << fanLevel
                    << std::endl;
            }
            else
            {
                std::cerr
                    << "[MAIN] Failed to read PLC status."
                    << std::endl;
            }
            lastPlcRead = now;
        }

        overlay.drawDetections(frame, people, stableCount);

        displayWindow.show(frame);

        // 按下 'q' 键退出
        if(displayWindow.pollKey(1) == 'q')
        {
            break;
        }

        if(displayWindow.pollKey(1) == 27) // ESC key
        {
            break;
        }

        if(!displayWindow.isOpen())
        {
            break;
        }

        // 读取下一帧
        if(!frameSource->read(frame))
        {
            std::cerr << "[MAIN] Failed to read frame from " << frameSourceFactory.getFrameSourceType() << "." << std::endl;
            break;
        }
    }

    displayWindow.close();
    frameSource->close();
    plcClient.disconnect();
    return 0;
}