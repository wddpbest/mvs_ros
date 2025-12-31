"""
@file dual_mvs_node_launch.py
@brief 启动双MVS相机节点的启动文件
@details 该文件用于启动两个ROS2节点，每个节点负责MVS相机图像数据发布
"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,
    TextSubstitution,
)
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import PushRosNamespace


def generate_launch_description():
    """
    使用可配置的参数启动MVS相机节点
    """
    mvs_camera0_label_arg = DeclareLaunchArgument(
        "mvs_camera0_label",
        default_value="camera0",
        description="Label of the MVS camera0.",
    )
    mvs_camera1_label_arg = DeclareLaunchArgument(
        "mvs_camera1_label",
        default_value="camera1",
        description="Label of the MVS camera1.",
    )
    trigger_interval_arg = DeclareLaunchArgument(
        "trigger_interval_ms",
        default_value="100",
        description="Trigger interval in milliseconds for both cameras.",
    )
    enable_trigger_arg = DeclareLaunchArgument(
        "enable_trigger",
        default_value="true",
        description="Enable trigger mode for both cameras.",
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

    mvs_camera0_node = GroupAction(
        [
            PushRosNamespace(LaunchConfiguration("mvs_camera0_label")),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution(
                        [
                            FindPackageShare("mvs_ros"),
                            TextSubstitution(text="launch"),
                            TextSubstitution(text="mvs_node_launch.py"),
                        ]
                    )
                ),
                launch_arguments={
                    "mvs_camera_label": LaunchConfiguration("mvs_camera0_label"),
                    "trigger_interval_ms": LaunchConfiguration("trigger_interval_ms"),
                    "enable_trigger": LaunchConfiguration("enable_trigger"),
                    "action_device_key": LaunchConfiguration("action_device_key"),
                    "action_group_key": LaunchConfiguration("action_group_key"),
                    "action_group_mask": LaunchConfiguration("action_group_mask"),
                    "broadcast_ip": LaunchConfiguration("broadcast_ip"),
                }.items(),
            ),
        ]
    )
    mvs_camera1_node = GroupAction(
        [
            PushRosNamespace(LaunchConfiguration("mvs_camera1_label")),
            IncludeLaunchDescription(
                PythonLaunchDescriptionSource(
                    PathJoinSubstitution(
                        [
                            FindPackageShare("mvs_ros"),
                            TextSubstitution(text="launch"),
                            TextSubstitution(text="mvs_node_launch.py"),
                        ]
                    )
                ),
                launch_arguments={
                    "mvs_camera_label": LaunchConfiguration("mvs_camera1_label"),
                    "trigger_interval_ms": LaunchConfiguration("trigger_interval_ms"),
                    "enable_trigger": LaunchConfiguration("enable_trigger"),
                    "action_device_key": LaunchConfiguration("action_device_key"),
                    "action_group_key": LaunchConfiguration("action_group_key"),
                    "action_group_mask": LaunchConfiguration("action_group_mask"),
                    "broadcast_ip": LaunchConfiguration("broadcast_ip"),
                }.items(),
            ),
        ]
    )

    return LaunchDescription(
        [
            mvs_camera0_label_arg,
            mvs_camera1_label_arg,
            trigger_interval_arg,
            enable_trigger_arg,
            action_device_key_arg,
            action_group_key_arg,
            action_group_mask_arg,
            broadcast_ip_arg,
            mvs_camera0_node,
            mvs_camera1_node,
        ]
    )
