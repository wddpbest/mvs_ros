"""
@file mvs_node_launch.py
@brief 启动MVS相机节点的启动文件
@details 该文件用于启动一个ROS2节点，该节点负责MVS相机图像数据发布
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import (
    LaunchConfiguration,
)
from launch_ros.actions import Node


def generate_launch_description():
    """
    使用可配置的参数启动MVS相机节点
    """
    mvs_camera_label_arg = DeclareLaunchArgument(
        "mvs_camera_label",
        default_value="camera0",
        description="Label of the MVS camera.",
    )
    mvs_image_topic_arg = DeclareLaunchArgument(
        "mvs_image_topic",
        default_value="image_raw",
        description="Image topic for MVS camera to publish.",
    )
    trigger_interval_arg = DeclareLaunchArgument(
        "trigger_interval_ms",
        default_value="100",
        description="Trigger interval in milliseconds.",
    )
    enable_trigger_arg = DeclareLaunchArgument(
        "enable_trigger",
        default_value="true",
        description="Enable trigger mode.",
    )
    action_device_key_arg = DeclareLaunchArgument(
        "action_device_key",
        default_value="1",
        description="Action command device key (hex value).",
    )
    action_group_key_arg = DeclareLaunchArgument(
        "action_group_key",
        default_value="1",
        description="Action command group key (hex value).",
    )
    action_group_mask_arg = DeclareLaunchArgument(
        "action_group_mask",
        default_value="4294967295",  # 0xFFFFFFFF
        description="Action command group mask (hex value).",
    )
    broadcast_ip_arg = DeclareLaunchArgument(
        "broadcast_ip",
        default_value="192.168.2.255",
        description="Broadcast IP address for action commands.",
    )

    mvs_node = Node(
        package="mvs_ros",
        executable="mvs_node",
        name="mvs_node",
        output="screen",
        parameters=[
            {
                "camera_label": LaunchConfiguration("mvs_camera_label"),
                "image_topic": LaunchConfiguration("mvs_image_topic"),
                "trigger_interval_ms": LaunchConfiguration("trigger_interval_ms"),
                "enable_trigger": LaunchConfiguration("enable_trigger"),
                "action_device_key": LaunchConfiguration("action_device_key"),
                "action_group_key": LaunchConfiguration("action_group_key"),
                "action_group_mask": LaunchConfiguration("action_group_mask"),
                "broadcast_ip": LaunchConfiguration("broadcast_ip"),
            }
        ],
    )

    return LaunchDescription(
        [
            mvs_camera_label_arg,
            mvs_image_topic_arg,
            trigger_interval_arg,
            enable_trigger_arg,
            action_device_key_arg,
            action_group_key_arg,
            action_group_mask_arg,
            broadcast_ip_arg,
            mvs_node,
        ]
    )
