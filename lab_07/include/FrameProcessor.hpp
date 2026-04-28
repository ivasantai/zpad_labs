#pragma once
#include <opencv2/opencv.hpp>
#include "KeyProcessor.hpp"

class FrameProcessor {
private:
    int blurKernel = 5;
    int cannyThreshold1 = 50;
    int cannyThreshold2 = 150;

public:
    cv::Mat process(const cv::Mat& frame, Mode mode);
};