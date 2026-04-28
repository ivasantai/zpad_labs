#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <thread>
#include <vector>

class FaceDetector {
private:
    cv::dnn::Net net;
    float confidenceThreshold;
    int artificialDelayMs;

    std::thread worker;
    std::atomic<bool> running{false};

    mutable std::mutex dataMutex;
    std::condition_variable frameReady;
    cv::Mat latestFrame;
    bool hasNewFrame = false;
    std::vector<cv::Rect> latestFaces;

    std::mutex netMutex;

    void workerLoop();
    std::vector<cv::Rect> detectInternal(const cv::Mat& frame);

public:
    FaceDetector(const std::string& prototxtPath,
                 const std::string& modelPath,
                 float threshold = 0.5f,
                 int delayMs = 0);
    ~FaceDetector();

    bool isLoaded() const;

    void start();
    void stop();

    void submitFrame(const cv::Mat& frame);
    std::vector<cv::Rect> getFaces() const;

    // Synchronous detection for base-level mode and comparison demo.
    std::vector<cv::Rect> detect(const cv::Mat& frame);
};
