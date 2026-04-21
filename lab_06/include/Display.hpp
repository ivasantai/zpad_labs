#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class Display {
private:
    std::string windowName;

public:
    Display(const std::string& name = "Lab 06");
    void show(const cv::Mat& frame);
};