from pymodbus.server import StartTcpServer
from pymodbus.simulator import SimData, SimDevice, DataType


# ==============================
# PLC Holding Registers
# ==============================
#
# 先创建 200 个保持寄存器：
#
# 地址 0 ~ 199
#
# 后面规划：
#
# 0   -> 人数
# 1   -> 温度 × 10
# 2   -> 湿度 × 10
#
# 100 -> HVAC 模式
# 101 -> 风机档位
# 102 -> 压缩机状态
#
holding_registers = [
    SimData(
        address=0,                  # 起始地址
        count=200,                  # 一共 200 个寄存器
        values=0,                   # 初始值全部为 0
        datatype=DataType.REGISTERS
    )
]


# ==============================
# 创建模拟 PLC
# ==============================
#
# simdata 四个位置依次是：
#
# (
#     Coils,
#     Discrete Inputs,
#     Holding Registers,
#     Input Registers
# )
#
plc = SimDevice(
    id=1,

    # 直接传一个列表，表示使用共享寄存器模型
    simdata=holding_registers
)


if __name__ == "__main__":
    print("================================")
    print(" Modbus TCP PLC Simulator")
    print("================================")
    print("IP      : 127.0.0.1")
    print("Port    : 5020")
    print("Device  : 1")
    print()
    print("Holding Register Map:")
    print("HR[0]   -> People Count")
    print("HR[1]   -> Temperature x10")
    print("HR[2]   -> Humidity x10")
    print("HR[100] -> HVAC Mode")
    print("HR[101] -> Fan Level")
    print("HR[102] -> Compressor State")
    print()
    print("Waiting for Modbus TCP client...")

    # 启动 Modbus TCP Server
    StartTcpServer(
        context=plc,
        address=("127.0.0.1", 5020)
    )