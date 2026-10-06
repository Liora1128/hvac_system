#pragma once

#include <winsock2.h>
#include <string>
#include <vector>
#include <cstdint>

class ModbusTcpClient
{
    public:
        ModbusTcpClient();
        ~ModbusTcpClient();

        // 连接 Modbus TCP Server
        bool connectToServer(
            const std::string& ip,
            unsigned short port
        );

        // 读取保持寄存器
        bool readHoldingRegisters(
            uint16_t startAddress,
            uint16_t quantity,
            std::vector<uint16_t>& values
        );

        // 写入单个保持寄存器
        bool writeSingleRegister(
            uint16_t address,
            uint16_t value
        );

        // 写入多个保持寄存器
        bool writeMultipleRegisters(
            uint16_t startAddress,
            const std::vector<uint16_t>& values
        );

        // 断开连接
        void disconnect();

        // 当前是否已连接
        bool isConnected() const;

    private:
        // Windows Socket 句柄
        SOCKET socket_ = INVALID_SOCKET;

        // Winsock 是否初始化成功
        bool wsaInitialized_ = false;

        // Modbus TCP Transaction ID
        uint16_t transaction_ID_ = 1;

        // 保证完整发送指定数量的字节
        bool sendAll(const uint8_t* data, int length);

        // 保证完整接收指定数量的字节
        bool recvAll(uint8_t* data, int length);
};