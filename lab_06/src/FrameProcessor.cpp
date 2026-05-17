#include "FrameProcessor.hpp"

cv::Mat FrameProcessor::process(const cv::Mat& frame, Mode mode) {
    cv::Mat result = frame.clone();

    switch (mode) {
        case Mode::Normal:
            break;

        case Mode::Gray:
            cv::cvtColor(frame, result, cv::COLOR_BGR2GRAY);
            break;

        case Mode::Blur:
            cv::GaussianBlur(frame, result, cv::Size(blurKernel, blurKernel), 0);
            break;

        case Mode::Canny: {
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::Canny(gray, result, cannyThreshold1, cannyThreshold2);
            break;
        }

        case Mode::Sobel: {
            cv::Mat gray, gradX, gradY, absX, absY;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::Sobel(gray, gradX, CV_16S, 1, 0);
            cv::Sobel(gray, gradY, CV_16S, 0, 1);
            cv::convertScaleAbs(gradX, absX);
            cv::convertScaleAbs(gradY, absY);
            cv::addWeighted(absX, 0.5, absY, 0.5, 0, result);
            break;
        }

        case Mode::Invert:
            cv::bitwise_not(frame, result);
            break;
    }

    return result;
}