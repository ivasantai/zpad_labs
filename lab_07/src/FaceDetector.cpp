#include "FaceDetector.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>

FaceDetector::FaceDetector(const std::string& prototxtPath,
                           const std::string& modelPath,
                           float threshold,
                           int delayMs)
    : confidenceThreshold(threshold), artificialDelayMs(delayMs) {
    try {
        net = cv::dnn::readNetFromCaffe(prototxtPath, modelPath);
    } catch (const cv::Exception& e) {
        std::cerr << "FaceDetector: cannot load neural network: " << e.what() << std::endl;
    }
}

FaceDetector::~FaceDetector() {
    stop();
}

bool FaceDetector::isLoaded() const {
    return !net.empty();
}

void FaceDetector::start() {
    if (!isLoaded() || running.load()) {
        return;
    }

    running.store(true);
    worker = std::thread(&FaceDetector::workerLoop, this);
}

void FaceDetector::stop() {
    if (!running.exchange(false)) {
        return;
    }

    frameReady.notify_all();
    if (worker.joinable()) {
        worker.join();
    }
}

void FaceDetector::submitFrame(const cv::Mat& frame) {
    if (frame.empty() || !isLoaded()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        // Keep only the newest frame. This prevents the worker from processing
        // an ever-growing queue when inference is slower than camera FPS.
        latestFrame = frame.clone();
        hasNewFrame = true;
    }
    frameReady.notify_one();
}

std::vector<cv::Rect> FaceDetector::getFaces() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return latestFaces;
}

std::vector<cv::Rect> FaceDetector::detect(const cv::Mat& frame) {
    return detectInternal(frame);
}

void FaceDetector::workerLoop() {
    while (running.load()) {
        cv::Mat frameForDetection;

        {
            std::unique_lock<std::mutex> lock(dataMutex);
            frameReady.wait(lock, [this] {
                return !running.load() || hasNewFrame;
            });

            if (!running.load()) {
                break;
            }

            frameForDetection = latestFrame.clone();
            hasNewFrame = false;
        }

        std::vector<cv::Rect> faces = detectInternal(frameForDetection);

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            latestFaces = std::move(faces);
        }
    }
}

std::vector<cv::Rect> FaceDetector::detectInternal(const cv::Mat& frame) {
    std::vector<cv::Rect> faces;

    if (frame.empty() || !isLoaded()) {
        return faces;
    }

    cv::Mat blob = cv::dnn::blobFromImage(
        frame,
        1.0,
        cv::Size(300, 300),
        cv::Scalar(104.0, 177.0, 123.0),
        false,
        false
    );

    cv::Mat detections;
    {
        // OpenCV DNN Net is not used concurrently by the UI thread and worker.
        std::lock_guard<std::mutex> lock(netMutex);
        net.setInput(blob);
        detections = net.forward();
    }

    if (artificialDelayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(artificialDelayMs));
    }

    cv::Mat detectionsMatrix(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
    const cv::Rect frameRect(0, 0, frame.cols, frame.rows);

    for (int i = 0; i < detectionsMatrix.rows; ++i) {
        float confidence = detectionsMatrix.at<float>(i, 2);
        if (confidence < confidenceThreshold) {
            continue;
        }

        int x1 = static_cast<int>(detectionsMatrix.at<float>(i, 3) * frame.cols);
        int y1 = static_cast<int>(detectionsMatrix.at<float>(i, 4) * frame.rows);
        int x2 = static_cast<int>(detectionsMatrix.at<float>(i, 5) * frame.cols);
        int y2 = static_cast<int>(detectionsMatrix.at<float>(i, 6) * frame.rows);

        cv::Rect face(cv::Point(x1, y1), cv::Point(x2, y2));
        face = face & frameRect;

        if (face.area() > 0) {
            faces.push_back(face);
        }
    }

    return faces;
}
