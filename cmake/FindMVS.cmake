# MVS SDK路径
if (DEFINED ENV{MVCAM_SDK_PATH})
    set(MVCAM_SDK_PATH $ENV{MVCAM_SDK_PATH} CACHE PATH "MVS SDK path")
else ()
    set(MVCAM_SDK_PATH /opt/MVS CACHE PATH "MVS SDK path")
endif ()

# 搜索MVS头文件
find_path(MVS_INCLUDE_DIR
    NAMES MvCameraControl.h
    PATHS ${MVCAM_SDK_PATH}/include
)

# 搜索MVS库
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    find_library(MVS_LIBRARIES
        NAMES MvCameraControl
        PATHS ${MVCAM_SDK_PATH}/lib/64
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    find_library(MVS_LIBRARIES
        NAMES MvCameraControl
        PATHS ${MVCAM_SDK_PATH}/lib/aarch64
    )
endif ()

# 输出搜索信息
message(VERBOSE "MVCAM_SDK_PATH: ${MVCAM_SDK_PATH}")
message(VERBOSE "MVS_INCLUDE_DIR: ${MVS_INCLUDE_DIR}")
message(VERBOSE "MVS_LIBRARIES: ${MVS_LIBRARIES}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MVS
    FOUND_VAR MVS_FOUND
    REQUIRED_VARS
    MVS_INCLUDE_DIR
    MVS_LIBRARIES
)