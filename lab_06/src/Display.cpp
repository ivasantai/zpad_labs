#include "Display.hpp"

Display::Display(const std::string& name) : windowName(name) {
    cv::namedWindow(windowName);
}

void Display::show(const cv::Mat& frame) {
    cv::imshow(windowName, frame);
}