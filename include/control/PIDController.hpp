#pragma once 

struct PIDTerms
{
    double error = 0.0; // 误差
    double p = 0.0;     // 比例项
    double i = 0.0;     // 积分项
    double d = 0.0;     // 微分项
    double output = 0.0; // 输出
    /* data */
};


class PIDController
{
    public:
        PIDController(
            double kp,
            double ki,
            double kd,
            double minOutput = 0.0,
            double maxOutput = 100.0,
            double minIntegral = -30.0,
            double maxIntegral = 30.0
        );

        // 输入目标温度、实际温度和周期时间
        // 返回 PID 各项及最终控制输出
        PIDTerms update(
            double setpoint,
            double measured,
            double dtSeconds
        );

        // 清除积分和历史数据
        void reset();

    private:
        double kp_;
        double ki_;
        double kd_;

        double minOutput_;
        double maxOutput_;

        // 积分项限幅，防止积分无限累积
        double minIntegral_;
        double maxIntegral_;

        // 当前积分贡献
        double integralTerm_ = 0.0;

        // 保存上一次测量温度，用于计算微分项
        double previousMeasurement_ = 0.0;
        
        // 标记是否有上一次测量温度
        bool hasPreviousMeasurement_ = false;
};