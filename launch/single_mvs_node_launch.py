"""
@file single_mvs_node_launch.py
@brief 启动单MVS相机节点的启动文件
@details 该文件用于启动一个ROS2节点，该节点负责MVS相机图像数据发布
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
    mvs_camera0_trigger_interval_arg = DeclareLaunchArgument(
        "mvs_camera0_trigger_interval",
        default_value="100",
        description="Trigger interval in milliseconds for camera0.",
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
                    "trigger_interval_ms": LaunchConfiguration("mvs_camera0_trigger_interval"),
                }.items(),
            ),
        ]
    )

    return LaunchDescription(
        [
            mvs_camera0_label_arg,
            mvs_camera0_trigger_interval_arg,
            mvs_camera0_node,
        ]
    )
