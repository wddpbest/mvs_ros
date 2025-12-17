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
                }.items(),
            ),
        ]
    )

    return LaunchDescription(
        [
            mvs_camera0_label_arg,
            mvs_camera1_label_arg,
            mvs_camera0_node,
            mvs_camera1_node,
        ]
    )
