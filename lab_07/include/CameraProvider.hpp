#pragma once
#include <opencv2/opencv.hpp>

class CameraProvider {
private:
    cv::VideoCapture cap;

public:
    CameraProvider(int cameraIndex = 0);
    bool isOpened() const;
    cv::Mat getFrame();
};