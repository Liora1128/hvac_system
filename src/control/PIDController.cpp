#include "control/PIDController.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

PIDController::PIDController(
    double kp,
    double ki,
    double kd,
    double minOutput,
    double maxOutput,
    double minIntegral,
    double maxIntegral
)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      minOutput_(minOutput),
      maxOutput_(maxOutput),
      minIntegral_(minIntegral),
      maxIntegral_(maxIntegral)
{
    if (
        kp_ < 0.0 || ki_ < 0.0 || kd_ < 0.0 
    )
    {
        throw std::invalid_argument("PID gains must be non-negative.");
    }

    if (minOutput_ > maxOutput_ )
    {
        throw std::invalid_argument("minOutput must be less than or equal to maxOutput.");
    }
    if (minIntegral_ > maxIntegral_)
    {
        throw std::invalid_argument("minIntegral must be less than or equal to maxIntegral.");
    }
}



// 更新 PID 控制器
PIDTerms PIDController::update(
    double setpoint,
    double measured,
    double dtSeconds
)
{
    if (dtSeconds <= 0.0 || !std::isfinite(dtSeconds))
    {
        throw std::invalid_argument("dtSeconds must be positive and finite.");
    }

    // ========================================
    // 1. 计算温度误差
    // ========================================

    // 制冷系统约定：
    // 实际温度高于目标温度，误差为正，
    // 应增加制冷需求。

    const double error = measured - setpoint;

    // ========================================
    // 2. 比例项 P
    // ========================================
    const double p = kp_ * error;

    // ========================================
    // 3. 积分项 I
    // ========================================
    // 积分增量 = ki * 误差 * 时间间隔
    integralTerm_ += ki_ * error * dtSeconds;
    
    // 限幅积分贡献，避免无限累加
    integralTerm_ = std::clamp(integralTerm_, minIntegral_, maxIntegral_);
    const double i = integralTerm_;

    // ========================================
    // 4. 微分项 D
    // ========================================
    
    double temperatureRate = 0.0;

    if (hasPreviousMeasurement_)
    {
        temperatureRate = (measured - previousMeasurement_) / dtSeconds;
    }

    // 对测量温度求变化率。
    // 对于本控制器的符号约定：
    // 温度正在上升时，微分项增加制冷需求。

    const double d = kd_ * temperatureRate;

    // 合成输出
    const double rawOutput = p + i + d;
    const double output = std::clamp(rawOutput, minOutput_, maxOutput_);

    // 保留本次测量结果
    previousMeasurement_ = measured;
    hasPreviousMeasurement_ = true;
    
    return PIDTerms{error, p, i, d, output};
}


