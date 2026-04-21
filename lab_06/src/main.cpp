#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "CameraProvider.hpp"
#include "KeyProcessor.hpp"
#include "FrameProcessor.hpp"
#include "Display.hpp"

int main() {
    CameraProvider camera(0);
    if (!camera.isOpened()) {
        std::cerr << "Cannot open camera" << std::endl;
        return 1;
    }

    KeyProcessor keyProcessor;
    FrameProcessor frameProcessor;
    Display display("Lab 06 OpenCV");

    Mode currentMode = Mode::Normal;
    int frameCount = 0;
    double fps = 0.0;
    auto lastTime = cv::getTickCount();

    while (true) {
        cv::Mat frame = camera.getFrame();
        if (frame.empty()) {
            std::cerr << "Empty frame" << std::endl;
            break;
        }

        cv::Mat processed = frameProcessor.process(frame, currentMode);

        frameCount++;
        double currentTime = cv::getTickCount();
        double elapsed = (currentTime - lastTime) / cv::getTickFrequency();
        if (elapsed >= 1.0) {
            fps = frameCount / elapsed;
            frameCount = 0;
            lastTime = currentTime;
        }

        cv::putText(processed, "Keys: 0-Normal 1-Gray 2-Blur 3-Canny 4-Sobel 5-Invert Q-Quit",
                    cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        cv::putText(processed, "FPS: " + std::to_string((int)fps),
                    cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        display.show(processed);

        int key = cv::waitKey(1);
        if (key == 'q' || key == 27) {
            break;
        }

        currentMode = keyProcessor.processKey(key, currentMode);
    }

    cv::destroyAllWindows();
    return 0;
}