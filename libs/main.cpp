#include "bsp.hpp"
#include <cstdio>

int main()
{
    ZlgCanDevice can;

    // 打开设备（USBCAN2 自带 2 个通道，只需打开一次）
    if (!can.OpenDevice(ZCAN_USBCAN2, 0))
    {
        printf("Failed to open device\n");
        return -1;
    }

    // 设置波特率（两个通道都设为 1 Mbps）
    can.SetBaudRate(0, 1000000);
    can.SetBaudRate(1, 1000000);

    // 配置通道
    ZlgCanDevice::ChannelConfig cfg;
    cfg.is_fd = false;
    cfg.acc_code = 0x00000000;
    cfg.acc_mask = 0xFFFFFFFF;
    cfg.filter = 0;
    cfg.timing0 = 0x00;
    cfg.timing1 = 0x14;
    cfg.mode = 0;

    if (!can.ConfigureChannel(0, cfg, true))
    {
        can.CloseDevice();
        return -2;
    }
    if (!can.ConfigureChannel(1, cfg, true))
    {
        can.CloseDevice();
        return -3;
    }

    // CAN0 发送数据
    can_frame tx{};
    tx.can_id = MAKE_CAN_ID(0x123, 0, 0, 0);  // 标准帧，ID=0x123
    tx.can_dlc = 8;
    tx.data[0] = 0x11;
    tx.data[1] = 0x22;
    tx.data[2] = 0x33;
    tx.data[3] = 0x44;
    tx.data[4] = 0x55;
    tx.data[5] = 0x66;
    tx.data[6] = 0x77;
    tx.data[7] = 0x88;

    can.Transmit(tx, 0); // 通道 0 发送

    // CAN1 接收数据
    can_frame rx{};
    if (can.Receive(rx, 1, 20)) // 通道 1 接收
    {
        // 处理 rx
        printf("ID=0x%X DLC=%d: %02X %02X %02X %02X %02X %02X %02X %02X\n",
        GET_ID(rx.can_id), rx.can_dlc,
        rx.data[0], rx.data[1], rx.data[2], rx.data[3],
        rx.data[4], rx.data[5], rx.data[6], rx.data[7]);
    }

    // 5. 关闭设备
    can.CloseDevice();

    return 0;
}