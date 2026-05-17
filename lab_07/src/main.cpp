#include <opencv2/opencv.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "CameraProvider.hpp"
#include "Display.hpp"
#include "FaceDetector.hpp"
#include "FrameProcessor.hpp"
#include "KeyProcessor.hpp"

struct AppConfig {
    int cameraIndex = 0;
    bool syncFace = false;
    int delayMs = 0;
    std::string prototxtPath = "models/deploy.prototxt";
    std::string modelPath = "models/res10_300x300_ssd_iter_140000.caffemodel";
    bool customProtoPath = false;
    bool customModelPath = false;
};

static void printHelp(const char* appName) {
    std::cout
        << "Usage: " << appName << " [options]\n\n"
        << "Options:\n"
        << "  --camera <index>     Camera index, default: 0\n"
        << "  --sync-face          Run face detection in the main UI thread\n"
        << "  --delay-ms <value>   Add artificial detector delay for demo, e.g. 500\n"
        << "  --proto <path>       Path to deploy.prototxt\n"
        << "  --model <path>       Path to .caffemodel\n"
        << "  --help               Show this help\n";
}

static bool parseArgs(int argc, char** argv, AppConfig& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto requireValue = [&](const std::string& option) -> bool {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << option << std::endl;
                return false;
            }
            return true;
        };

        if (arg == "--help" || arg == "-h") {
            printHelp(argv[0]);
            return false;
        } else if (arg == "--sync-face") {
            config.syncFace = true;
        } else if (arg == "--camera") {
            if (!requireValue(arg)) return false;
            config.cameraIndex = std::stoi(argv[++i]);
        } else if (arg == "--delay-ms") {
            if (!requireValue(arg)) return false;
            config.delayMs = std::max(0, std::stoi(argv[++i]));
        } else if (arg == "--proto") {
            if (!requireValue(arg)) return false;
            config.prototxtPath = argv[++i];
            config.customProtoPath = true;
        } else if (arg == "--model") {
            if (!requireValue(arg)) return false;
            config.modelPath = argv[++i];
            config.customModelPath = true;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printHelp(argv[0]);
            return false;
        }
    }

    return true;
}

static bool fileExists(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return file.good();
}

static std::string findExistingPath(const std::vector<std::string>& candidates) {
    for (const auto& candidate : candidates) {
        if (fileExists(candidate)) {
            return candidate;
        }
    }
    return candidates.empty() ? std::string() : candidates.front();
}

static void resolveDefaultModelPaths(AppConfig& config, const char* appName) {
    // The app can be launched from the project root with ./run.sh or directly
    // from build/. Check both locations so the model files are found reliably.
    namespace fs = std::filesystem;

    fs::path executableDir = fs::absolute(appName).parent_path();

    if (!config.customProtoPath) {
        config.prototxtPath = findExistingPath({
            "models/deploy.prototxt",
            "../models/deploy.prototxt",
            (executableDir / "../models/deploy.prototxt").lexically_normal().string(),
            (executableDir / "models/deploy.prototxt").lexically_normal().string()
        });
    }

    if (!config.customModelPath) {
        config.modelPath = findExistingPath({
            "models/res10_300x300_ssd_iter_140000.caffemodel",
            "../models/res10_300x300_ssd_iter_140000.caffemodel",
            (executableDir / "../models/res10_300x300_ssd_iter_140000.caffemodel").lexically_normal().string(),
            (executableDir / "models/res10_300x300_ssd_iter_140000.caffemodel").lexically_normal().string()
        });
    }
}

static void drawFaces(cv::Mat& frame, const std::vector<cv::Rect>& faces) {
    for (const auto& face : faces) {
        cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    }
}

static std::string modeName(Mode mode) {
    switch (mode) {
        case Mode::Normal: return "Normal";
        case Mode::Gray: return "Gray";
        case Mode::Blur: return "Blur";
        case Mode::Canny: return "Canny";
        case Mode::Sobel: return "Sobel";
        case Mode::Invert: return "Invert";
        case Mode::Face: return "Face";
    }
    return "Unknown";
}

int main(int argc, char** argv) {
    AppConfig config;
    if (!parseArgs(argc, argv, config)) {
        return 0;
    }

    resolveDefaultModelPaths(config, argv[0]);

    CameraProvider camera(config.cameraIndex);
    if (!camera.isOpened()) {
        std::cerr << "Cannot open camera with index " << config.cameraIndex << std::endl;
        return 1;
    }

    KeyProcessor keyProcessor;
    FrameProcessor frameProcessor;
    Display display("Lab 07 OpenCV + DNN Face Detection");

    std::cout << "Face detector prototxt: " << config.prototxtPath << std::endl;
    std::cout << "Face detector model: " << config.modelPath << std::endl;

    FaceDetector faceDetector(config.prototxtPath, config.modelPath, 0.5f, config.delayMs);
    if (!faceDetector.isLoaded()) {
        std::cerr << "Face detector is disabled. Run ./preinstall.sh or check model paths.\n";
    } else if (!config.syncFace) {
        faceDetector.start();
    }

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

        cv::Mat processed;

        if (currentMode == Mode::Face) {
            processed = frame.clone();

            std::vector<cv::Rect> faces;
            if (faceDetector.isLoaded()) {
                if (config.syncFace) {
                    // Base-level comparison: inference blocks the UI loop.
                    faces = faceDetector.detect(frame);
                } else {
                    // Advanced solution: UI gives the newest frame to the worker
                    // and immediately uses the last known detection result.
                    faceDetector.submitFrame(frame);
                    faces = faceDetector.getFaces();
                }
            }

            drawFaces(processed, faces);
        } else {
            processed = frameProcessor.process(frame, currentMode);
        }

        ++frameCount;
        double currentTime = cv::getTickCount();
        double elapsed = (currentTime - lastTime) / cv::getTickFrequency();
        if (elapsed >= 1.0) {
            fps = frameCount / elapsed;
            frameCount = 0;
            lastTime = cv::getTickCount();
        }

        if (processed.channels() == 1) {
            cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGR);
        }

        cv::putText(processed, "Keys: 0-Normal 1-Gray 2-Blur 3-Canny 4-Sobel 5-Invert F-Face Q-Quit",
                    cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        cv::putText(processed, "Mode: " + modeName(currentMode) + " | FPS: " + std::to_string(static_cast<int>(fps)),
                    cv::Point(10, 55), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        if (currentMode == Mode::Face) {
            std::string detectorMode = config.syncFace ? "main thread" : "worker thread";
            std::ostringstream info;
            info << "Detector: " << detectorMode << " | delay: " << config.delayMs << " ms";
            cv::putText(processed, info.str(), cv::Point(10, 85),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        }

        display.show(processed);

        int key = cv::waitKey(1);
        if (key == 'q' || key == 'Q' || key == 27) {
            break;
        }

        currentMode = keyProcessor.processKey(key, currentMode);
    }

    faceDetector.stop();
    cv::destroyAllWindows();
    return 0;
}
