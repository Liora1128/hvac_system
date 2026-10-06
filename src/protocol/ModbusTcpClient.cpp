#include "protocol/ModbusTcpClient.hpp"

#include <iostream>
#include <ws2tcpip.h>

ModbusTcpClient::ModbusTcpClient()
{
    // 初始化 Winsock
    WSADATA wsaData;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(result != 0)
    {
        std::cerr
            << "[ModbusTcpClient] WSAStartup failed: "
            << result
            << std::endl;
    }
    else
    {
        wsaInitialized_ = true;
    }
}

ModbusTcpClient::~ModbusTcpClient()
{
    // 断开连接
    disconnect();

    // 清理 Winsock
    if(wsaInitialized_)
    {
        WSACleanup();
    }
}

bool ModbusTcpClient::connectToServer(
    const std::string& ip,
    unsigned short port
)
{
    if(!wsaInitialized_)
    {
        return false;
    }

    // 创建套接字
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_ == INVALID_SOCKET)
    {
        std::cerr
            << "[ModbusTcpClient] Failed to create socket: "
            << WSAGetLastError()
            << std::endl;
        return false;
    }

    // 设置服务器地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // 连接服务器
    int result = connect(
        socket_,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    );

    if(result == SOCKET_ERROR)
    {
        std::cerr
            << "[ModbusTcpClient] Failed to connect to server: "
            << WSAGetLastError()
            << std::endl;

        closesocket(socket_);
        socket_ = INVALID_SOCKET;

        return false;
    }

    std::cout
        << "[ModbusTcpClient] Connected to PLC: "
        << ip
        << ":"
        << port
        << std::endl;

    return true;
}

void ModbusTcpClient::disconnect()
{
    if(socket_ != INVALID_SOCKET)
    {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;

        std::cout
            << "[ModbusTcpClient] Disconnected from PLC."
            << std::endl;
    }
}


bool ModbusTcpClient::isConnected() const
{
    return socket_ != INVALID_SOCKET;
}


bool ModbusTcpClient::sendAll(
    const uint8_t* data,
    int length
)
{
    int totalSent = 0;

    while(totalSent < length)
    {
        int sent = send(
            socket_,
            reinterpret_cast<const char*>(data + totalSent),
            length - totalSent,
            0
        );

        if(sent == SOCKET_ERROR || sent == 0)
        {
            std::cerr
                << "[ModbusTcpClient] Socket send failed."
                << std::endl;

            return false;
        }

        totalSent += sent;
    }

    return true;
}


bool ModbusTcpClient::recvAll(
    uint8_t* data,
    int length
)
{
    int totalReceived = 0;

    while(totalReceived < length)
    {
        int received = recv(
            socket_,
            reinterpret_cast<char*>(data + totalReceived),
            length - totalReceived,
            0
        );

        if(received == SOCKET_ERROR || received == 0)
        {
            std::cerr
                << "[ModbusTcpClient] Socket receive failed."
                << std::endl;

            return false;
        }

        totalReceived += received;
    }

    return true;
}

bool ModbusTcpClient::readHoldingRegisters(
    uint16_t startAddress,
    uint16_t quantity,
    std::vector<uint16_t>& values
)
{
    // Modbus 规定 FC03 一次最多读取 125 个寄存器
    if(quantity == 0 || quantity > 125)
    {
        std::cerr << "[ModbusTcpClient] Invalid register quantity." << std::endl;
        return false;
    }

    // 本次请求的事务 ID
    uint16_t currentTransactionId = transaction_ID_++;

    // ==============================
    // 构造 Modbus TCP 请求
    // ==============================
    //
    // 00 01   Transaction ID
    // 00 00   Protocol ID
    // 00 06   Length
    // 01      Unit ID
    // 03      Function Code
    // 00 00   Start Address
    // 00 03   Quantity
    //
    uint8_t request[12];

    // Transaction ID
    request[0] = static_cast<uint8_t>(currentTransactionId >> 8);
    request[1] = static_cast<uint8_t>(currentTransactionId & 0xFF);

    // Protocol ID = 0，Modbus TCP 固定为 0
    request[2] = 0x00;
    request[3] = 0x00;

    // Length = Unit ID + PDU = 6 字节
    request[4] = 0x00;
    request[5] = 0x06;

    // Unit ID
    request[6] = 0x01;

    // Function Code = 03
    request[7] = 0x03;

    // 起始寄存器地址
    request[8] =
        static_cast<uint8_t>(startAddress >> 8);

    request[9] =
        static_cast<uint8_t>(startAddress & 0xFF);

    // 读取多少个寄存器
    request[10] =
        static_cast<uint8_t>(quantity >> 8);

    request[11] =
        static_cast<uint8_t>(quantity & 0xFF);


    // ==============================
    // 发送请求
    // ==============================
    if(!sendAll(request, sizeof(request)))
    {
        return false;
    }


    // ==============================
    // 先读取 7 字节 MBAP Header
    // ==============================
    uint8_t header[7];

    if(!recvAll(header, 7))
    {
        return false;
    }


    // 检查 Transaction ID
    uint16_t responseTransactionId =
        (static_cast<uint16_t>(header[0]) << 8)
        | header[1];

    if(responseTransactionId != currentTransactionId)
    {
        std::cerr
            << "[ModbusTcpClient] Transaction ID mismatch."
            << std::endl;

        return false;
    }


    // Modbus Length 字段
    uint16_t length =
        (static_cast<uint16_t>(header[4]) << 8)
        | header[5];

    // length 包含 Unit ID，
    // 但 Unit ID 已经在 header[6] 里读掉了
    int remainingLength =
        static_cast<int>(length) - 1;

    if(remainingLength <= 0)
    {
        return false;
    }


    // ==============================
    // 接收剩下的 PDU
    // ==============================
    std::vector<uint8_t> response(
        remainingLength
    );

    if(!recvAll(
        response.data(),
        remainingLength
    ))
    {
        return false;
    }


    // ==============================
    // 检查 Modbus 异常响应
    // ==============================
    //
    // 正常：
    // 03 ...
    //
    // 异常：
    // 83 exception_code
    //
    if(response[0] & 0x80)
    {
        std::cerr
            << "[ModbusTcpClient] Modbus exception code: "
            << static_cast<int>(response[1])
            << std::endl;

        return false;
    }


    // 必须是 FC03
    if(response[0] != 0x03)
    {
        std::cerr
            << "[ModbusTcpClient] Unexpected function code."
            << std::endl;

        return false;
    }


    // response[1] = 后面寄存器数据一共有多少字节
    uint8_t byteCount = response[1];

    if(byteCount != quantity * 2)
    {
        std::cerr
            << "[ModbusTcpClient] Unexpected byte count."
            << std::endl;

        return false;
    }


    // ==============================
    // 解析寄存器
    // ==============================
    values.clear();

    for(uint16_t i = 0; i < quantity; ++i)
    {
        int index = 2 + i * 2;

        uint16_t value =
            (static_cast<uint16_t>(response[index]) << 8)
            | response[index + 1];

        values.push_back(value);
    }

    return true;
}



bool ModbusTcpClient::writeMultipleRegisters(
    uint16_t startAddress,
    const std::vector<uint16_t>& values
)
{
    // Modbus 规定 FC16 一次最多写入 123 个寄存器
    if(values.empty() || values.size() > 123)
    {
        std::cerr << "[ModbusTcpClient] Invalid register quantity." << std::endl;
        return false;
    }

    uint16_t quantity = static_cast<uint16_t>(values.size());
    // 本次请求的事务 ID
    uint16_t currentTransactionId = transaction_ID_++;

    // 每个寄存器占 2 个字节
    uint8_t byteCount = static_cast<uint8_t>(quantity * 2);

    // Modbus TCP Length：
    // Unit ID 1
    // Function Code 1
    // Start Address 2
    // Quantity 2
    // Byte Count 1
    // Register Data quantity*2

    uint16_t length = 1 + 1 + 2 + 2 + 1 + byteCount;

    // 整个请求：
    // 7字节 MBAP
    // +
    // 6字节固定PDU
    // +
    // 寄存器数据

    std::vector<uint8_t> request(7 + 6 + byteCount);

    // =========================
    // MBAP Header
    // =========================

    // Transaction ID

    request[0] = static_cast<uint8_t>(currentTransactionId >> 8);
    request[1] = static_cast<uint8_t>(currentTransactionId & 0xFF);

    // Protocol ID = 0
    request[2] = 0x00;
    request[3] = 0x00;

    // Length
    request[4] = static_cast<uint8_t>(length >> 8);
    request[5] = static_cast<uint8_t>(length & 0xFF);

    // Unit ID
    request[6] = 0x01;

    // =========================
    // PDU
    // Function Code = 16
    request[7] = 0x10;
    // Start Address
    request[8] = static_cast<uint8_t>(startAddress >> 8);
    request[9] = static_cast<uint8_t>(startAddress & 0xFF);

    // Quantity
    request[10] = static_cast<uint8_t>(quantity >> 8);
    request[11] = static_cast<uint8_t>(quantity & 0xFF);

    // Byte Count
    request[12] = byteCount;

    // =========================
    // 填入寄存器数据
    // =========================
    for(uint16_t i = 0; i < quantity; ++i)
    {
        request[13 + i * 2] =
            static_cast<uint8_t>(values[i] >> 8);

        request[14 + i * 2] =
            static_cast<uint8_t>(values[i] & 0xFF);
    }

    // 发送完整 Modbus TCP 请求
    if(!sendAll(
        request.data(),
        static_cast<int>(request.size())
    ))
    {
        return false;
    }


    // =========================
    // 接收 7 字节 MBAP Header
    // =========================

    uint8_t header[7];

    if(!recvAll(header, 7))
    {
        return false;
    }


    uint16_t responseTransactionId =
        (static_cast<uint16_t>(header[0]) << 8)
        | header[1];

    if(responseTransactionId != currentTransactionId)
    {
        std::cerr
            << "[ModbusTcpClient] Transaction ID mismatch."
            << std::endl;

        return false;
    }


    uint16_t responseLength =
        (static_cast<uint16_t>(header[4]) << 8)
        | header[5];

    int remainingLength =
        static_cast<int>(responseLength) - 1;

    if(remainingLength <= 0)
    {
        return false;
    }


    // 接收剩余 PDU
    std::vector<uint8_t> response(
        remainingLength
    );

    if(!recvAll(
        response.data(),
        remainingLength
    ))
    {
        return false;
    }


    // Modbus 异常响应
    if(response[0] & 0x80)
    {
        std::cerr
            << "[ModbusTcpClient] Modbus exception code: "
            << static_cast<int>(response[1])
            << std::endl;

        return false;
    }


    if(response[0] != 0x10)
    {
        std::cerr
            << "[ModbusTcpClient] Unexpected function code."
            << std::endl;

        return false;
    }


    // PLC 会把起始地址和写入数量返回给我们
    uint16_t responseAddress =
        (static_cast<uint16_t>(response[1]) << 8)
        | response[2];

    uint16_t responseQuantity =
        (static_cast<uint16_t>(response[3]) << 8)
        | response[4];


    if(responseAddress != startAddress ||
       responseQuantity != quantity)
    {
        std::cerr
            << "[ModbusTcpClient] Write response mismatch."
            << std::endl;

        return false;
    }

    return true;
}


bool ModbusTcpClient::writeSingleRegister(
    uint16_t address,
    uint16_t value
)
{
    uint16_t currentTransactionId =
        transaction_ID_++;

    // FC06 请求固定 12 字节
    uint8_t request[12];

    // =========================
    // MBAP Header
    // =========================

    // Transaction ID
    request[0] =
        static_cast<uint8_t>(currentTransactionId >> 8);

    request[1] =
        static_cast<uint8_t>(currentTransactionId & 0xFF);

    // Protocol ID，Modbus TCP 固定为 0
    request[2] = 0x00;
    request[3] = 0x00;

    // Length：
    // Unit ID(1) + PDU(5) = 6
    request[4] = 0x00;
    request[5] = 0x06;

    // Unit ID
    request[6] = 0x01;

    // =========================
    // PDU
    // =========================

    // Function Code = 0x06
    request[7] = 0x06;

    // 寄存器地址
    request[8] =
        static_cast<uint8_t>(address >> 8);

    request[9] =
        static_cast<uint8_t>(address & 0xFF);

    // 写入值
    request[10] =
        static_cast<uint8_t>(value >> 8);

    request[11] =
        static_cast<uint8_t>(value & 0xFF);


    // 发送 Modbus TCP 请求
    if(!sendAll(request, sizeof(request)))
    {
        return false;
    }


    // =========================
    // 接收 MBAP Header
    // =========================

    uint8_t header[7];

    if(!recvAll(header, 7))
    {
        return false;
    }


    // 检查 Transaction ID
    uint16_t responseTransactionId =
        (static_cast<uint16_t>(header[0]) << 8)
        | header[1];

    if(responseTransactionId != currentTransactionId)
    {
        std::cerr
            << "[ModbusTcpClient] Transaction ID mismatch."
            << std::endl;

        return false;
    }


    // 获取后面还有多少字节
    uint16_t length =
        (static_cast<uint16_t>(header[4]) << 8)
        | header[5];

    int remainingLength =
        static_cast<int>(length) - 1;

    if(remainingLength <= 0)
    {
        return false;
    }


    std::vector<uint8_t> response(
        remainingLength
    );

    if(!recvAll(
        response.data(),
        remainingLength
    ))
    {
        return false;
    }


    // 异常响应，例如 0x86
    if(response[0] & 0x80)
    {
        std::cerr
            << "[ModbusTcpClient] Modbus exception code: "
            << static_cast<int>(response[1])
            << std::endl;

        return false;
    }


    // 正常必须还是 FC06
    if(response[0] != 0x06)
    {
        std::cerr
            << "[ModbusTcpClient] Unexpected function code."
            << std::endl;

        return false;
    }


    // FC06 正常响应会把：
    // 地址 + 写入值
    // 原样返回

    uint16_t responseAddress =
        (static_cast<uint16_t>(response[1]) << 8)
        | response[2];

    uint16_t responseValue =
        (static_cast<uint16_t>(response[3]) << 8)
        | response[4];


    if(responseAddress != address ||
       responseValue != value)
    {
        std::cerr
            << "[ModbusTcpClient] Write response mismatch."
            << std::endl;

        return false;
    }


    return true;
}