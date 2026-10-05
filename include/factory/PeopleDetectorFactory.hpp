#pragma once

#include "ports/IPeopleDetector.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <string>


class PeopleDetectorFactory
{
public:
    // 根据 config.json 创建具体的人员检测器
    std::unique_ptr<IPeopleDetector> create(
        const nlohmann::json& config
    );

    // 获取当前检测器类型，例如 "hog" / "yolo"
    const std::string& getDetectorType() const;

private:
    // 保存当前检测器类型
    std::string detectorType_;
};