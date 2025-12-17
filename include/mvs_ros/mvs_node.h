#pragma once

#include <MvCameraControl.h>
#include <image_transport/image_transport.hpp>
#include <rclcpp/rclcpp.hpp>

/**
 * @brief MVS相机节点类
 * @details 该类用于通过MVS SDK与相机通信，获取图像数据并发布为ROS图像消息
 */
class MVSNode final : public rclcpp::Node {
public:
    /**
     * @brief 构造函数
     * @param program_name 程序名称
     */
    explicit MVSNode(const std::string &program_name);

    /**
     * @brief 析构函数
     */
    ~MVSNode() override;

private:
    /**
     * @brief 打开相机
     * @return 是否成功打开相机
     */
    bool openCamera();

    /**
     * @brief 图像回调函数
     * @param data 图像数据
     * @param frame_info 图像帧信息
     * @param user 用户数据指针
     */
    static void imageCallback(unsigned char *data, MV_FRAME_OUT_INFO_EX *frame_info, void *user);

    image_transport::Publisher image_pub_; ///< 图像发布者

    void *camera_handle_{nullptr}; ///< 相机句柄
    std::string camera_label_;     ///< 相机标签
};
