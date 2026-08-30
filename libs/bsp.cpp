#include "bsp.hpp"  


bool ZlgCanDevice::OpenDevice(UINT device_type, UINT device_index, UINT reserved)
{
    if (device_handle_ != INVALID_DEVICE_HANDLE)
        return true;

    device_handle_ = ZCAN_OpenDevice(device_type, device_index, reserved);
    if (device_handle_ == INVALID_DEVICE_HANDLE)
        return false;

    opened_ = true;
    return true;
}

bool ZlgCanDevice::ConfigureChannel(UINT can_index, const ChannelConfig& config, bool auto_start)
{
    if (!opened_ || device_handle_ == INVALID_DEVICE_HANDLE)
        return false;

    CHANNEL_HANDLE& ch = channelOf(can_index);
    if (ch != INVALID_CHANNEL_HANDLE)
        return true; // 该通道已初始化

    ZCAN_CHANNEL_INIT_CONFIG init_config = {};
    std::memset(&init_config, 0, sizeof(init_config));

    if (config.is_fd)
    {
        init_config.can_type = 1; // TYPE_CANFD

        init_config.canfd.acc_code = config.acc_code;
        init_config.canfd.acc_mask = config.acc_mask;
        init_config.canfd.reserved = config.reserved;
        init_config.canfd.filter = config.filter;
        init_config.canfd.mode = config.mode;
        init_config.canfd.abit_timing = config.abit_timing;
        init_config.canfd.dbit_timing = config.dbit_timing;
        init_config.canfd.brp = config.brp;
    }
    else
    {
        init_config.can_type = 0; // TYPE_CAN

        init_config.can.acc_code = config.acc_code;
        init_config.can.acc_mask = config.acc_mask;
        init_config.can.reserved = config.reserved;
        init_config.can.filter = config.filter;
        init_config.can.timing0 = config.timing0;
        init_config.can.timing1 = config.timing1;
        init_config.can.mode = config.mode;
    }

    ch = ZCAN_InitCAN(device_handle_, can_index, &init_config);
    if (ch == INVALID_CHANNEL_HANDLE)
        return false;

    if (auto_start)
    {
        if (ZCAN_StartCAN(ch) != STATUS_OK)
        {
            ZCAN_ResetCAN(ch);
            ch = INVALID_CHANNEL_HANDLE;
            return false;
        }
        (can_index == 0 ? started0_ : started1_) = true;
    }

    return true;
}

bool ZlgCanDevice::SetBaudRate(UINT can_index, UINT baud_rate)
{
    if (!opened_ || device_handle_ == INVALID_DEVICE_HANDLE)
        return false;

    char path[64];
    char value[16];
    std::snprintf(path, sizeof(path), "%u/baud_rate", can_index);
    std::snprintf(value, sizeof(value), "%u", baud_rate);

    return ZCAN_SetValue(device_handle_, path, value) == 1;
}

bool ZlgCanDevice::SetFdBaudRate(UINT can_index, UINT abit_baud_rate, UINT dbit_baud_rate)
{
    if (!opened_ || device_handle_ == INVALID_DEVICE_HANDLE)
        return false;

    char path[64];
    char value[16];

    std::snprintf(path, sizeof(path), "%u/canfd_abit_baud_rate", can_index);
    std::snprintf(value, sizeof(value), "%u", abit_baud_rate);
    if (ZCAN_SetValue(device_handle_, path, value) != 1)
        return false;

    std::snprintf(path, sizeof(path), "%u/canfd_dbit_baud_rate", can_index);
    std::snprintf(value, sizeof(value), "%u", dbit_baud_rate);
    if (ZCAN_SetValue(device_handle_, path, value) != 1)
        return false;

    return true;
}

bool ZlgCanDevice::Transmit(const can_frame& frame, UINT can_index, UINT transmit_type)
{
    CHANNEL_HANDLE ch = channelOf(can_index);
    if (ch == INVALID_CHANNEL_HANDLE)
        return false;

    ZCAN_Transmit_Data tx = {};
    tx.frame = frame;
    tx.transmit_type = transmit_type;

    return ZCAN_Transmit(ch, &tx, 1) == 1;
}

bool ZlgCanDevice::Receive(can_frame& frame, UINT can_index, int wait_time)
{
    CHANNEL_HANDLE ch = channelOf(can_index);
    if (ch == INVALID_CHANNEL_HANDLE)
        return false;

    ZCAN_Receive_Data rx[1] = {};
    UINT ret = ZCAN_Receive(ch, rx, 1, wait_time);

    if (ret != 1)
        return false;

    frame = rx[0].frame;
    return true;
}

bool ZlgCanDevice::TransmitFD(const canfd_frame& frame, UINT can_index, UINT transmit_type)
{
    CHANNEL_HANDLE ch = channelOf(can_index);
    if (ch == INVALID_CHANNEL_HANDLE)
        return false;

    ZCAN_TransmitFD_Data tx = {};
    tx.frame = frame;
    tx.transmit_type = transmit_type;

    return ZCAN_TransmitFD(ch, &tx, 1) == 1;
}

bool ZlgCanDevice::ReceiveFD(canfd_frame& frame, UINT can_index, int wait_time)
{
    CHANNEL_HANDLE ch = channelOf(can_index);
    if (ch == INVALID_CHANNEL_HANDLE)
        return false;

    ZCAN_ReceiveFD_Data rx[1] = {};
    UINT ret = ZCAN_ReceiveFD(ch, rx, 1, wait_time);

    if (ret != 1)
        return false;

    frame = rx[0].frame;
    return true;
}

bool ZlgCanDevice::CloseDevice()
{
    if (channel0_ != INVALID_CHANNEL_HANDLE)
    {
        ZCAN_ResetCAN(channel0_);
        channel0_ = INVALID_CHANNEL_HANDLE;
    }
    if (channel1_ != INVALID_CHANNEL_HANDLE)
    {
        ZCAN_ResetCAN(channel1_);
        channel1_ = INVALID_CHANNEL_HANDLE;
    }

    if (device_handle_ != INVALID_DEVICE_HANDLE)
    {
        ZCAN_CloseDevice(device_handle_);
        device_handle_ = INVALID_DEVICE_HANDLE;
    }

    opened_ = false;
    started0_ = false;
    started1_ = false;
    return true;
}