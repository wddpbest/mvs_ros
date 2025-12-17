cmake_minimum_required(VERSION 3.15)
project(mvs_ros)

# 查找第三方库
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(MVS REQUIRED)

# 查找ROS包
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(image_transport REQUIRED)

# 设置编译选项
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -march=native")

# 添加可执行文件
add_executable(mvs_node
    src/mvs_node.cpp
)
target_include_directories(mvs_node PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
    ${MVS_INCLUDE_DIR}
)
target_link_libraries(mvs_node
    ${MVS_LIBRARIES}
)
ament_target_dependencies(mvs_node
    rclcpp
    sensor_msgs
    image_transport
)

# 安装可执行文件
install(TARGETS mvs_node
    DESTINATION lib/${PROJECT_NAME}
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION include
)

# 安装启动文件
install(DIRECTORY
    launch
    DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(
    MVS
    rclcpp
    sensor_msgs
    image_transport
)
ament_package()