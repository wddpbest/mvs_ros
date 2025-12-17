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

    mvs_node = Node(
        package="mvs_ros",
        executable="mvs_node",
        name="mvs_node",
        output="screen",
        parameters=[
            {
                "camera_label": LaunchConfiguration("mvs_camera_label"),
                "image_topic": LaunchConfiguration("mvs_image_topic"),
            }
        ],
    )

    return LaunchDescription(
        [
            mvs_camera_label_arg,
            mvs_image_topic_arg,
            mvs_node,
        ]
    )
