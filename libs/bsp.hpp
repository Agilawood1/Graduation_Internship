#pragma once

#include <cstdint>
#include <cstdio>
#include "zlgcan.h"
#include "canframe.h"
#include "typedef.h"

class ZlgCanDevice
{
public:
    struct ChannelConfig
    {
        bool is_fd = false;       // false: CAN, true: CAN FD
        UINT acc_code = 0x00000000;
        UINT acc_mask = 0xFFFFFFFF;
        UINT reserved = 0;
        BYTE filter = 0;          // 过滤方式，按手册设定
        BYTE timing0 = 0x00;
        BYTE timing1 = 0x14;      // 典型值可按需要调整
        BYTE mode = 0;            // 0: normal, 1: listen-only
        UINT abit_timing = 0;     // CAN FD
        UINT dbit_timing = 0;     // CAN FD
        UINT brp = 0;             // CAN FD
    };

    ZlgCanDevice() = default;
    ~ZlgCanDevice() { CloseDevice(); }

    // 设备打开
    bool OpenDevice(UINT device_type, UINT device_index, UINT reserved = 0);

    // 通道配置（用户可选参数）
    bool ConfigureChannel(UINT can_index, const ChannelConfig& config, bool auto_start = true);

    // 配置经典 CAN 波特率（必须在 ConfigureChannel 之前调用）
    bool SetBaudRate(UINT can_index, UINT baud_rate);

    // 配置 CAN FD 仲裁段/数据段波特率（必须在 ConfigureChannel 之前调用）
    bool SetFdBaudRate(UINT can_index, UINT abit_baud_rate, UINT dbit_baud_rate);

    // 发送 CAN 报文（can_index: 0/1 通道）
    bool Transmit(const can_frame& frame, UINT can_index, UINT transmit_type = 0);

    // 接收 CAN 报文（can_index: 0/1 通道）
    bool Receive(can_frame& frame, UINT can_index, int wait_time = 10);

    // 发送 CAN FD 报文（can_index: 0/1 通道）
    bool TransmitFD(const canfd_frame& frame, UINT can_index, UINT transmit_type = 0);

    // 接收 CAN FD 报文（can_index: 0/1 通道）
    bool ReceiveFD(canfd_frame& frame, UINT can_index, int wait_time = 10);

    // 关闭设备
    bool CloseDevice();

    bool IsOpen() const { return opened_; }
    bool IsStarted(UINT can_index) const { return (can_index == 0) ? started0_ : started1_; }

    DEVICE_HANDLE GetDeviceHandle() const { return device_handle_; }
    CHANNEL_HANDLE GetChannelHandle(UINT can_index) const { return (can_index == 0) ? channel0_ : channel1_; }

private:
    CHANNEL_HANDLE& channelOf(UINT can_index) { return (can_index == 0) ? channel0_ : channel1_; }

    DEVICE_HANDLE device_handle_ = INVALID_DEVICE_HANDLE;
    CHANNEL_HANDLE channel0_ = INVALID_CHANNEL_HANDLE;
    CHANNEL_HANDLE channel1_ = INVALID_CHANNEL_HANDLE;

    bool opened_ = false;
    bool started0_ = false;
    bool started1_ = false;
};