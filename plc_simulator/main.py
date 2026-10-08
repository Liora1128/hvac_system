import asyncio

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

# 模拟 PLC 从现场温度传感器读取到的温度
# 目前先固定，下一步再做动态温度模型
TEMPERATURE = 28.0


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

plc = SimDevice(
    1,  # Device ID / Unit ID
    SimData(
        0,
        datatype=DataType.REGISTERS,
        values=[0] * 200
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

    # 保存上一次状态
    # 状态没变化时不重复打印
    last_state = None


    # ----------------------------------------
    # 制冷状态需要记忆
    # ----------------------------------------
    #
    # 这就是实现“死区”的关键。
    #
    # False：
    # 当前没有制冷
    #
    # True：
    # 当前正在制冷
    #
    cooling_on = False


    while True:

        # ====================================================
        # 1. 读取 HR[0]：C++ 写进来的人数
        # ====================================================

        values = await server.async_getValues(
            1,          # Device ID
            3,          # Holding Register
            0,          # HR[0]
            count=1
        )

        people_count = values[0]


        # ====================================================
        # 2. 模拟 PLC 本地温度传感器
        # ====================================================

        temperature = TEMPERATURE


        # ====================================================
        # 3. 根据人数决定风机档位
        # ====================================================

        if people_count == 0:

            fan_level = 0

        elif people_count <= 3:

            # 1 ~ 3 人
            fan_level = 1

        elif people_count <= 8:

            # 4 ~ 8 人
            fan_level = 2

        else:

            # 9 人及以上
            fan_level = 3


        # ====================================================
        # 4. HVAC 温度死区控制
        # ====================================================

        # ----------------------------------------
        # 无人
        # ----------------------------------------
        if people_count == 0:

            cooling_on = False


        # ----------------------------------------
        # 有人
        # ----------------------------------------
        else:

            # 当前没有制冷
            #
            # 温度达到 26℃ 才启动制冷
            if (
                not cooling_on
                and temperature >= COOLING_ON_TEMP
            ):
                cooling_on = True


            # 当前正在制冷
            #
            # 必须降到 24.5℃ 才停止
            elif (
                cooling_on
                and temperature <= COOLING_OFF_TEMP
            ):
                cooling_on = False


        # True -> 1
        # False -> 0
        hvac_mode = 1 if cooling_on else 0


        # ====================================================
        # 5. 写入 PLC 控制结果
        # ====================================================

        # HR[100] = HVAC状态
        await server.async_setValues(
            1,
            3,
            100,
            [hvac_mode]
        )


        # HR[101] = 风机档位
        await server.async_setValues(
            1,
            3,
            101,
            [fan_level]
        )


        # ====================================================
        # 6. 状态变化时打印
        # ====================================================

        current_state = (
            people_count,
            temperature,
            hvac_mode,
            fan_level
        )


        if current_state != last_state:

            print()
            print("================================")
            print("          PLC 状态变化")
            print("================================")

            print(
                f"当前人数 : "
                f"{people_count} 人"
            )

            print(
                f"当前温度 : "
                f"{temperature:.1f} ℃"
            )

            print(
                f"HVAC状态 : "
                f"{hvac_mode_text(hvac_mode)}"
            )

            print(
                f"风机档位 : "
                f"{fan_level_text(fan_level)}"
            )

            print("================================")

            last_state = current_state


        # ====================================================
        # 7. PLC 扫描周期
        # ====================================================
        #
        # 这里为了仿真方便，每秒执行一次控制程序。
        #
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