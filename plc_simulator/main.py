import asyncio
import math

from pymodbus import pymodbus_apply_logging_config
from pymodbus.server import ModbusTcpServer
from pymodbus.simulator import SimData, SimDevice, DataType


# ============================================================
# PyModbus 日志
# ============================================================

# 不显示每一条 Modbus 报文，只显示警告和错误
pymodbus_apply_logging_config("WARNING")


# ============================================================
# PLC 控制参数
# ============================================================

# 初始室温与外界温度
INITIAL_TEMPERATURE = 28.0
AMBIENT_TEMPERATURE = 30.0

# 每增加一人，对模拟平衡温度的影响
PEOPLE_HEAT_GAIN = 0.12

# 每增加 1% 风机输出，对模拟平衡温度的降温影响
COOLING_GAIN_PER_PERCENT = 0.075

# 模拟时间：现实运行 1 秒，相当于模拟 60 秒
SIM_STEP_SECONDS = 60.0

# 室温模型的时间常数，数值越大，温度变化越慢
THERMAL_TIME_CONSTANT_SECONDS = 900.0

# 保留你原有的温度启停阈值
COOLING_ON_TEMP = 26.0
COOLING_OFF_TEMP = 24.5

# 制冷启动温度
#
# 当前没有制冷时：
# 温度 >= 26℃ 才允许启动
COOLING_ON_TEMP = 26.0


# 制冷停止温度
#
# 当前正在制冷时：
# 温度 <= 24.5℃ 才停止
COOLING_OFF_TEMP = 24.5


# ============================================================
# PLC 寄存器规划
# ============================================================
#
# HR[0]
#   C++ 写入的人数
#
# HR[100]
#   HVAC 状态
#   0 = 停止
#   1 = 运行
#
# HR[101]
#   风机档位
#   0 = 关闭
#   1 = 低速
#   2 = 中速
#   3 = 高速
#
# 创建 HR[0] ~ HR[199]
# 初始值全部为 0
# ============================================================
inital_registers = [0] * 200

# HR[102] 保存室温，放大 100 倍
# 28.00℃ 对应 2800

inital_registers[102] = int(INITIAL_TEMPERATURE * 100)
plc = SimDevice(
    1,  # Device ID / Unit ID
    SimData(
        0,
        datatype=DataType.REGISTERS,
        values=inital_registers
    )
)


# ============================================================
# 状态转文字
# ============================================================

def hvac_mode_text(mode: int) -> str:
    """把 HVAC 数值状态转换成中文。"""

    if mode == 0:
        return "停止"

    if mode == 1:
        return "制冷运行"

    return "未知"


def fan_level_text(level: int) -> str:
    """把风机档位转换成中文。"""

    levels = {
        0: "关闭",
        1: "低速",
        2: "中速",
        3: "高速"
    }

    return levels.get(level, "未知")


# ============================================================
# PLC 控制程序
# ============================================================

async def plc_control_task(server: ModbusTcpServer):

    # 记录上一次状态，避免重复打印
    last_state = None

    # 记住当前制冷状态，实现温度滞回控制
    cooling_on = False

    # 模拟房间实际温度
    temperature = INITIAL_TEMPERATURE

    # 一阶热惯性模型的更新系数
    alpha = 1.0 - math.exp(-SIM_STEP_SECONDS / THERMAL_TIME_CONSTANT_SECONDS)

    while(True):
        # 1. 读取 C++ 写入的人数 HR[0]
        valuaes = await server.async_getValues(1, 3, 0, count = 1)
        people_count = valuaes[0]

        # 2. 读取 C++ 写入的风机需求 HR[103]
        command_values = await server.async_getValues(
            1, 3, 103, count=1
        )
        requested_fan_percent = max(0, min(100, int(command_values[0])))

        # 3. 制冷启停：保留原来的温度滞回逻辑
        if people_count == 0:
            cooling_on = False

        elif (
            not cooling_on
            and temperature >= COOLING_ON_TEMP
        ):
            cooling_on = True

        elif (
            cooling_on
            and temperature <= COOLING_OFF_TEMP
        ):
            cooling_on = False

        hvac_mode = 1 if cooling_on else 0

        # 无人或制冷未运行时，不产生有效风机输出
        if people_count > 0 and cooling_on:
            actual_fan_percent = requested_fan_percent
        else:
            actual_fan_percent = 0
        
        equilibrium_temperature = (
            AMBIENT_TEMPERATURE
            + PEOPLE_HEAT_GAIN * people_count
            - COOLING_GAIN_PER_PERCENT * actual_fan_percent
        ) 

        temperature += alpha * (
            equilibrium_temperature - temperature
        )
        
        await server.async_setValues(
            1, 3, 100, [hvac_mode]
        )

        # HR[101]：实际风机输出百分比
        await server.async_setValues(
            1, 3, 101, [
                round(actual_fan_percent)
            ]
        )

        # HR[102]：温度放大 100 倍
        await server.async_setValues(
            1, 3, 102, [
                round(temperature * 100)
            ]
        )

        # 6. 状态改变时打印
        current_state = (
            people_count,
            round(temperature, 2),
            hvac_mode,
            actual_fan_percent
        )

        if current_state != last_state:
            print()
            print("================================")
            print("          PLC 状态变化")
            print("================================")
            print(f"当前人数   : {people_count} 人")
            print(f"室内温度   : {temperature:.2f} ℃")
            print(f"HVAC 状态  : {hvac_mode}")
            print(f"风机输出   : {actual_fan_percent}%")
            print("================================")

            last_state = current_state

            # 每秒执行一次 PLC 仿真
        await asyncio.sleep(1)





# ============================================================
# 主程序
# ============================================================

async def main():

    print()
    print("================================")
    print("       智慧楼宇 PLC 模拟器")
    print("================================")

    print("IP        : 127.0.0.1")
    print("Port      : 5020")
    print("Device ID : 1")

    print()

    print("寄存器映射：")
    print("HR[0]     : 人数")
    print("HR[100]   : HVAC状态")
    print("HR[101]   : 风机档位")

    print()

    print(
        f"制冷启动温度 : "
        f"{COOLING_ON_TEMP:.1f} ℃"
    )

    print(
        f"制冷停止温度 : "
        f"{COOLING_OFF_TEMP:.1f} ℃"
    )

    print()

    print(
        "PLC 已启动，等待 C++ 客户端连接..."
    )


    # ========================================================
    # 创建 Modbus TCP Server
    # ========================================================

    server = ModbusTcpServer(
        plc,
        address=(
            "127.0.0.1",
            5020
        )
    )


    # ========================================================
    # 创建 PLC 控制任务
    # ========================================================

    control_task = asyncio.create_task(
        plc_control_task(server)
    )


    try:

        # 启动 Modbus TCP Server
        await server.serve_forever()


    finally:

        # Server 停止时，同时停止控制程序
        control_task.cancel()


# ============================================================
# 程序入口
# ============================================================

if __name__ == "__main__":
    asyncio.run(main())