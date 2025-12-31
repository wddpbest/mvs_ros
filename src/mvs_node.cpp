#include "mvs_ros/mvs_node.h"

#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/image.hpp>

#define MV_CHECK(logger, func, ...)                                                                                    \
    do {                                                                                                               \
        const auto ret = func(__VA_ARGS__);                                                                            \
        if (ret != MV_OK) {                                                                                            \
            RCLCPP_ERROR(logger, "MVS SDK execute error: " #func " = %d", ret);                                        \
        }                                                                                                              \
    } while (0)

MVSNode::MVSNode(const std::string &program_name) : Node(program_name) {
    // 获取参数
    declare_parameter<std::string>("camera_label", "");
    declare_parameter<std::string>("image_topic", "");
    declare_parameter<int>("trigger_interval_ms", 100);
    declare_parameter<bool>("enable_trigger", true);
    declare_parameter<int>("action_device_key", 1);
    declare_parameter<int>("action_group_key", 1);
    declare_parameter<int>("action_group_mask", 0xFFFFFFFF);
    declare_parameter<std::string>("broadcast_ip", "192.168.2.255");

    camera_label_          = get_parameter("camera_label").as_string();
    const auto image_topic = get_parameter("image_topic").as_string();

    // 获取触发间隔参数并验证
    const auto trigger_interval_param = get_parameter("trigger_interval_ms").as_int();
    if (trigger_interval_param <= 0) {
        RCLCPP_WARN(get_logger(), "Invalid trigger_interval_ms: %ld, using default value 100ms", trigger_interval_param);
        trigger_interval_ms_ = 100;
    } else {
        trigger_interval_ms_ = static_cast<unsigned int>(trigger_interval_param);
    }

    // 获取触发相关参数
    enable_trigger_ = get_parameter("enable_trigger").as_bool();

    // 获取Action Command参数并验证
    const auto action_device_key_param = get_parameter("action_device_key").as_int();
    if (action_device_key_param < 0) {
        RCLCPP_WARN(get_logger(), "Invalid action_device_key: %ld, using default value 1", action_device_key_param);
        action_device_key_ = 1;
    } else {
        action_device_key_ = static_cast<unsigned int>(action_device_key_param);
    }

    const auto action_group_key_param = get_parameter("action_group_key").as_int();
    if (action_group_key_param < 0) {
        RCLCPP_WARN(get_logger(), "Invalid action_group_key: %ld, using default value 1", action_group_key_param);
        action_group_key_ = 1;
    } else {
        action_group_key_ = static_cast<unsigned int>(action_group_key_param);
    }

    const auto action_group_mask_param = get_parameter("action_group_mask").as_int();
    if (action_group_mask_param < 0) {
        RCLCPP_WARN(get_logger(), "Invalid action_group_mask: %ld, using default value 0xFFFFFFFF", action_group_mask_param);
        action_group_mask_ = 0xFFFFFFFF;
    } else {
        action_group_mask_ = static_cast<unsigned int>(action_group_mask_param);
    }

    broadcast_ip_ = get_parameter("broadcast_ip").as_string();

    // 初始化MVS SDK
    MV_CHECK(get_logger(), MV_CC_Initialize);
    RCLCPP_INFO(get_logger(), "MVS SDK initialized successfully!");

    // 打开相机
    RCLCPP_INFO(get_logger(), "Trying to open %s...", camera_label_.c_str());
    if (!openCamera()) {
        RCLCPP_FATAL(get_logger(), "Fail to open %s!", camera_label_.c_str());
        return;
    }
    RCLCPP_INFO(get_logger(), "%s starts to grab image!", camera_label_.c_str());

    // 设置服务质量（QoS）
    rclcpp::QoS qos(rclcpp::QoSInitialization(RMW_QOS_POLICY_HISTORY_KEEP_LAST, 1000));
    qos.reliability(rclcpp::ReliabilityPolicy::Reliable);
    qos.durability(rclcpp::DurabilityPolicy::SystemDefault);

    // 创建图像发布
    RCLCPP_INFO(get_logger(), "%s image topic: %s", camera_label_.c_str(), image_topic.c_str());
    image_pub_ = image_transport::create_publisher(this, image_topic, qos.get_rmw_qos_profile());

    // 创建触发定时器
    if (enable_trigger_) {
        RCLCPP_INFO(get_logger(), "Trigger mode enabled with interval: %d ms", trigger_interval_ms_);
        RCLCPP_INFO(get_logger(), "Action Command config - Device Key: 0x%X, Group Key: 0x%X, Group Mask: 0x%X, Broadcast IP: %s",
                    action_device_key_, action_group_key_, action_group_mask_, broadcast_ip_.c_str());

        trigger_timer_ = create_wall_timer(
            std::chrono::milliseconds(trigger_interval_ms_),
            std::bind(&MVSNode::triggerTimerCallback, this)
        );
    }
}

MVSNode::~MVSNode() {
    RCLCPP_INFO(get_logger(), "Closing %s...", camera_label_.c_str());

    // 停止触发定时器
    if (trigger_timer_) {
        trigger_timer_->cancel();
        trigger_timer_.reset();
    }

    if (camera_handle_) {
        MV_CHECK(get_logger(), MV_CC_StopGrabbing, camera_handle_);
        MV_CHECK(get_logger(), MV_CC_CloseDevice, camera_handle_);
        MV_CHECK(get_logger(), MV_CC_DestroyHandle, camera_handle_);
        camera_handle_ = nullptr;
    }

    // 释放MVS SDK资源
    MV_CHECK(get_logger(), MV_CC_Finalize);
    RCLCPP_INFO(get_logger(), "MVS SDK resources released");

    RCLCPP_INFO(get_logger(), "%s closed!", camera_label_.c_str());
}

bool MVSNode::openCamera() {
    // 枚举设备
    MV_CC_DEVICE_INFO_LIST device_info_list = {};
    MV_CHECK(get_logger(), MV_CC_EnumDevices, MV_GIGE_DEVICE | MV_USB_DEVICE, &device_info_list);

    // 搜索与相机名称匹配的设备
    for (size_t i = 0; i < device_info_list.nDeviceNum; ++i) {
        const auto device_info       = device_info_list.pDeviceInfo[i];
        const auto user_defined_name = [&]() -> const char * {
            switch (device_info->nTLayerType) {
                case MV_GIGE_DEVICE:
                    return reinterpret_cast<const char *>(device_info->SpecialInfo.stGigEInfo.chUserDefinedName);
                case MV_USB_DEVICE:
                    return reinterpret_cast<const char *>(device_info->SpecialInfo.stUsb3VInfo.chUserDefinedName);
                default:
                    return nullptr;
            }
        }();

        // 跳过不支持的设备类型
        if (!user_defined_name) {
            RCLCPP_WARN(get_logger(), "Unsupported device type: %d", device_info->nTLayerType);
            continue;
        }

        // 搜索到匹配的设备，打印设备信息并尝试打开
        if (user_defined_name == camera_label_) {
            // 打印设备信息
            if (device_info->nTLayerType == MV_GIGE_DEVICE) {
                const auto &gige_info = device_info->SpecialInfo.stGigEInfo;
                RCLCPP_INFO(get_logger(), "%s: GIGE, %p, %d.%d.%d.%d", user_defined_name, gige_info.chModelName,
                            (gige_info.nCurrentIp >> 24) & 0xff, (gige_info.nCurrentIp >> 16) & 0xff,
                            (gige_info.nCurrentIp >> 8) & 0xff, gige_info.nCurrentIp & 0xff);
            } else {
                RCLCPP_INFO(get_logger(), "%s: USB, %p", user_defined_name,
                            device_info->SpecialInfo.stUsb3VInfo.chModelName);
            }

            // 尝试打开设备
            MV_CHECK(get_logger(), MV_CC_CreateHandle, &camera_handle_, device_info);
            MV_CHECK(get_logger(), MV_CC_OpenDevice, camera_handle_);
            MV_CHECK(get_logger(), MV_CC_RegisterImageCallBackEx, camera_handle_, &MVSNode::imageCallback, this);
            MV_CHECK(get_logger(), MV_CC_StartGrabbing, camera_handle_);

            break;
        }
    }

    if (!camera_handle_) {
        return false;
    }
    return true;
}

void MVSNode::imageCallback(unsigned char *data, MV_FRAME_OUT_INFO_EX *frame_info, void *user) {
    const auto node = static_cast<MVSNode *>(user);
    auto image_msg  = std::make_unique<sensor_msgs::msg::Image>();

    // 检查图像数据大小
    if (frame_info->nFrameLen > image_msg->data.max_size()) {
        RCLCPP_ERROR_ONCE(node->get_logger(), "Image bytes exceed max available size!");
        return;
    }

    // 获取设备时间戳
    const auto dev_timestamp = static_cast<int64_t>(frame_info->nDevTimeStampHigh) << 32ll |
                               static_cast<int64_t>(frame_info->nDevTimeStampLow);

    // 填充图像属性信息
    image_msg->header.frame_id = node->camera_label_;
    image_msg->header.stamp    = rclcpp::Time(dev_timestamp);
    image_msg->is_bigendian    = false;
    image_msg->width           = frame_info->nWidth;
    image_msg->height          = frame_info->nHeight;
    if (frame_info->enPixelType == PixelType_Gvsp_BayerRG8) {
        image_msg->step     = frame_info->nWidth * 1;
        image_msg->encoding = sensor_msgs::image_encodings::BAYER_RGGB8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: BAYER_RGGB8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_BayerBG8) {
        image_msg->step     = frame_info->nWidth * 1;
        image_msg->encoding = sensor_msgs::image_encodings::BAYER_BGGR8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: BAYER_BGGR8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_BayerGR8) {
        image_msg->step     = frame_info->nWidth * 1;
        image_msg->encoding = sensor_msgs::image_encodings::BAYER_GRBG8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: BAYER_GRBG8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_BayerGB8) {
        image_msg->step     = frame_info->nWidth * 1;
        image_msg->encoding = sensor_msgs::image_encodings::BAYER_GBRG8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: BAYER_GBRG8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_Mono8) {
        image_msg->step     = frame_info->nWidth * 1;
        image_msg->encoding = sensor_msgs::image_encodings::MONO8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: MONO8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_RGB8_Packed) {
        image_msg->step     = frame_info->nWidth * 3;
        image_msg->encoding = sensor_msgs::image_encodings::RGB8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: RGB8");
    } else if (frame_info->enPixelType == PixelType_Gvsp_BGR8_Packed) {
        image_msg->step     = frame_info->nWidth * 3;
        image_msg->encoding = sensor_msgs::image_encodings::BGR8;
        RCLCPP_INFO_ONCE(node->get_logger(), "Pixel format: BGR8");
    } else {
        RCLCPP_ERROR_ONCE(node->get_logger(), "Unsupported pixel format: %d",
                          static_cast<int>(frame_info->enPixelType));
        return;
    }

    // 拷贝图像数据
    image_msg->data.resize(image_msg->height * image_msg->step);
    if (frame_info->nFrameLen < image_msg->data.size()) {
        RCLCPP_ERROR(node->get_logger(), "Image bytes less than expected size: %u < %zu", frame_info->nFrameLen,
                     image_msg->data.size());
        return;
    }
    std::copy_n(data, image_msg->data.size(), image_msg->data.data());

    // 发布图像
    node->image_pub_.publish(std::move(image_msg));
}

void MVSNode::triggerTimerCallback() {
    MV_ACTION_CMD_INFO action_cmd_info = {};
    MV_ACTION_CMD_RESULT_LIST action_cmd_results = {};

    // 配置Action Command参数
    action_cmd_info.nDeviceKey = action_device_key_;
    action_cmd_info.nGroupKey = action_group_key_;
    action_cmd_info.nGroupMask = action_group_mask_;
    action_cmd_info.pBroadcastAddress = broadcast_ip_.c_str();
    action_cmd_info.nTimeOut = 0;  // 0表示不需要ACK

    // 发送Action Command
    MV_CHECK(get_logger(), MV_GIGE_IssueActionCommand, &action_cmd_info, &action_cmd_results);
}

std::string getFileName(const std::string &file) {
    const std::string separator = "/";
    const auto last_separator   = file.find_last_of(separator);
    if (last_separator == std::string::npos) {
        return {};
    }
    return file.substr(last_separator + 1);
}

int main(int argc, char **argv) {
    // 启动ROS节点
    rclcpp::init(argc, argv);
    const auto program_name = getFileName(argv[0]);
    rclcpp::spin(std::make_shared<MVSNode>(program_name));
    rclcpp::shutdown();

    return 0;
}
