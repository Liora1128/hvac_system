# 指定最低 CMake 版本。
# 意思是：运行这个工程至少需要 CMake 3.20。
cmake_minimum_required(VERSION 3.20)


# 定义项目名称。
# BuildingHVAC 是项目名字，可以自己改。
# LANGUAGES CXX 表示这个项目使用 C++。
project(BuildingHVAC LANGUAGES CXX)


# 指定使用 C++17 标准。
# 相当于 g++ 编译时使用 -std=c++17。
set(CMAKE_CXX_STANDARD 17)

# 要求必须使用上面指定的 C++17。
# 如果编译器不支持，不允许偷偷降级。
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ---------------------------------------------------------
# 查找 OpenCV
# ---------------------------------------------------------

# 让 CMake 自动寻找系统里安装的 OpenCV。
#
# REQUIRED 的意思是：
# 如果找不到 OpenCV，直接报错并停止配置。
#
# 找到之后，CMake 会提供一些变量，例如：
#
# OpenCV_INCLUDE_DIRS
#     OpenCV 头文件路径
#
# OpenCV_LIBS
#     需要链接的 OpenCV 库
#
find_package(OpenCV REQUIRED)
# 查找 nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)


# ---------------------------------------------------------
# 创建我们的程序
# ---------------------------------------------------------

# 创建一个名字叫 building_hvac 的可执行程序。
#
# 当前只有一个源文件：
# src/main.cpp
#
# 以后我们增加：
#
# src/camera/OpenCvCameraSource.cpp
# src/vision/HogPeopleDetector.cpp
# src/modbus/LibModbusClient.cpp
#
# 都可以继续写到这里。
add_executable(
    building_hvac

    src/main.cpp
    src/camera/OpenCvCameraSource.cpp
    src/video/VideoFileSource.cpp
    src/display/DisplayWindow.cpp
    src/factory/FrameSourceFactory.cpp
    src/detector/HogPeopleDetector.cpp
    src/display/DetectionOverlay.cpp
    src/detector/YoloPeopleDetector.cpp
    src/factory/PeopleDetectorFactory.cpp
    src/filter/PeopleCountFilter.cpp
    src/sensor/SimulatedSensorSource.cpp
    src/protocol/ModbusTcpClient.cpp
)


# ---------------------------------------------------------
# 配置头文件路径
# ---------------------------------------------------------

# 告诉 building_hvac：
# OpenCV 的头文件在哪里。
#
# 这样代码里才能正常写：
#
# #include <opencv2/opencv.hpp>
#
target_include_directories(
    building_hvac

    PRIVATE

    # 我们自己项目的头文件目录
    # 有了这个以后才能写：
    # #include "camera/OpenCvCameraSource.hpp"
    ${CMAKE_CURRENT_SOURCE_DIR}/include

    # OpenCV 的头文件目录
    ${OpenCV_INCLUDE_DIRS}
)


# ---------------------------------------------------------
# 链接 OpenCV 库
# ---------------------------------------------------------

# 编译代码只找到头文件还不够。
#
# 比如：
#
# cv::imshow()
# cv::Mat
# cv::putText()
#
# 这些函数真正的实现都在 OpenCV 的库文件里。
#
# 所以这里把 OpenCV 库链接到 building_hvac。
target_link_libraries(
    building_hvac

    PRIVATE

    ${OpenCV_LIBS}
    
    #json 库
    nlohmann_json::nlohmann_json
    ws2_32
)