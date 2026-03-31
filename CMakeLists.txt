cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/include/opencv4)
include_directories(/usr/include/sDepth)
include_directories(/usr/include/vpi)
include_directories(/usr/include)

link_directories(/usr/lib)
link_directories(/usr/lib/sDepth)
link_directories(/usr/lib/opencv)
link_directories(/usr/lib/vpi)

# TODO Modify the compilation file
set(SRC sample/main.cpp)

get_filename_component(EXE_NAME ${SRC} NAME_WE)
add_executable(${EXE_NAME} ${SRC})

target_link_libraries(${EXE_NAME} 
    sgmix
    sDepth
    opencv_core
    opencv_imgproc
    opencv_imgcodecs
    curl
    jsoncpp
    gstreamer-1.0
    gstapp-1.0
    glib-2.0
    gobject-2.0
    nvvpi
)
