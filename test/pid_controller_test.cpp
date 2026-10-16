#include "control/PIDController.hpp"

#include <cmath>
#include <iostream>

#define Kp 20.0
#define Ki 0.005
#define Kd 120.0

bool checkNear(
    const char* name,
    double actual,
    double expected
)
{
    const bool passed =
        std::abs(actual - expected) < 1e-6;

    std::cout
        << (passed ? "[PASS] " : "[FAIL] ")
        << name
        << " | 实际值: " << actual
        << " | 预期值: " << expected
        << '\n';

    return passed;
}


int main()
{
    // 当前室温模型每经过 1 秒真实时间，
    // 推进 60 秒仿真时间。
    //
    // 因此本测试使用 dtSeconds = 60。
    PIDController pid(
        Kp,   // Kp
        Ki,  // Ki
        Kd   // Kd
    );

    const double targetTemperature = 25.0;
    const double dtSeconds = 60.0;

    int failed = 0;


    // ========================================
    // 测试 1：室温 28℃，目标 25℃
    // ========================================

    const PIDTerms first = pid.update(
        targetTemperature,
        28.0,
        dtSeconds
    );

    std::cout << "\n第一次计算：\n";

    std::cout << "误差 e = " << first.error << '\n';
    std::cout << "P = " << first.p << '\n';
    std::cout << "I = " << first.i << '\n';
    std::cout << "D = " << first.d << '\n';
    std::cout << "输出 = " << first.output << "%\n";

    if (!checkNear("第一次 P", first.p, 60.0))
        ++failed;

    if (!checkNear("第一次 I", first.i, 0.9))
        ++failed;

    if (!checkNear("第一次 D", first.d, 0.0))
        ++failed;

    if (!checkNear("第一次输出", first.output, 60.9))
        ++failed;


    // ========================================
    // 测试 2：室温下降到 27.8℃
    // ========================================

    const PIDTerms second = pid.update(
        targetTemperature,
        27.8,
        dtSeconds
    );

    std::cout << "\n第二次计算：\n";

    std::cout << "误差 e = " << second.error << '\n';
    std::cout << "P = " << second.p << '\n';
    std::cout << "I = " << second.i << '\n';
    std::cout << "D = " << second.d << '\n';
    std::cout << "输出 = " << second.output << "%\n";

    if (!checkNear("第二次 P", second.p, 56.0))
        ++failed;

    if (!checkNear("第二次 I", second.i, 1.74))
        ++failed;

    if (!checkNear("第二次 D", second.d, -0.4))
        ++failed;

    if (!checkNear("第二次输出", second.output, 57.34))
        ++failed;


    // ========================================
    // 测试 3：室温降到 24℃
    // ========================================

    const PIDTerms third = pid.update(
        targetTemperature,
        24.0,
        dtSeconds
    );

    std::cout << "\n第三次计算：\n";

    std::cout << "P = " << third.p << '\n';
    std::cout << "I = " << third.i << '\n';
    std::cout << "D = " << third.d << '\n';
    std::cout << "输出 = " << third.output << "%\n";

    // 因为计算结果为负，制冷需求被限制为 0%
    if (!checkNear("第三次输出限幅", third.output, 0.0))
        ++failed;


    std::cout
        << "\n测试结束，失败项："
        << failed
        << '\n';

    return failed == 0 ? 0 : 1;
}