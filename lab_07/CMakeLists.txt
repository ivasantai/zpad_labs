cmake_minimum_required(VERSION 3.10)
project(lab_07_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(include ${OpenCV_INCLUDE_DIRS})

add_executable(lab_07_app
    src/main.cpp
    src/CameraProvider.cpp
    src/KeyProcessor.cpp
    src/FrameProcessor.cpp
    src/Display.cpp
    src/FaceDetector.cpp
)

target_link_libraries(lab_07_app ${OpenCV_LIBS} Threads::Threads)
