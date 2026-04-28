# Laboratory Work No. 7 — Computer Vision and Multithreading in C++

## What Was Implemented

The project is based on Laboratory Work No. 6. A face detection mode was added using OpenCV DNN and the `ResNet-10 SSD` model.

Mode keys:

- `0` — normal video
- `1` — Gray
- `2` — Gaussian Blur
- `3` — Canny
- `4` — Sobel
- `5` — Invert
- `F` — Face detection
- `Q` or `Esc` — exit

## Architecture

Main classes:

- `CameraProvider` — gets frames from the camera.
- `KeyProcessor` — handles key presses and switches modes.
- `FrameProcessor` — performs the old processing modes from Laboratory Work No. 6.
- `Display` — shows the result in an OpenCV window.
- `FaceDetector` — loads the Caffe neural network and performs face detection.

`FaceDetector` contains:

- `std::thread worker` — a background thread for inference;
- `std::mutex dataMutex` — protects shared data: the latest frame and face coordinates;
- `std::condition_variable frameReady` — wakes the worker thread when a new frame appears;
- `std::atomic<bool> running` — a safe flag for stopping the thread.

In multithreaded mode, the UI thread does not wait for `net.forward()`. It only sends the latest frame to `FaceDetector` and draws the last known face coordinates. Therefore, FPS remains high, while the rectangle may slightly lag during fast movements.

## Important Fix

`run.sh` now starts the program from the project root, not from the `build/` folder. This fixes the model loading error:

```text
Can't open "models/deploy.prototxt"
```

The application also checks both `models/` and `../models/`, so it can find the model files when launched from different directories.

## Installing Dependencies and Models

Ubuntu/Debian/Kali:

```bash
cd lab_07
./preinstall.sh
```

The script installs `build-essential`, `cmake`, `libopencv-dev`, `curl` and downloads two model files into the `models/` folder:

- `models/deploy.prototxt`
- `models/res10_300x300_ssd_iter_140000.caffemodel`

If the models are already downloaded, the script will not download them again.

If `apt update` shows a warning from an external repository, for example Microsoft VS Code, the script continues. That warning is not related to this laboratory work.

## Build

```bash
cd lab_07
./build.sh
```

After the build, the executable file will be here:

```bash
build/lab_07_app
```

## Run

Default launch with camera `0` and asynchronous detection:

```bash
./run.sh
```

Launch with another camera, for example `1`:

```bash
./run.sh --camera 1
```

Launch with custom paths to the model:

```bash
./run.sh --proto models/deploy.prototxt --model models/res10_300x300_ssd_iter_140000.caffemodel
```

## Demonstration of the Difference Between Single-Threaded and Multithreaded Modes

For demonstration, an artificial detector delay can be added, as required in the task.

Single-threaded mode: `net.forward()` is executed in the main UI thread, so the video slows down.

```bash
./run.sh --sync-face --delay-ms 500
```

Multithreaded mode: inference runs in the worker thread, so the video remains smooth, while the rectangle may update with a delay.

```bash
./run.sh --delay-ms 500
```

After launch, press `F` to enable face detection.

## Useful Options

```bash
./run.sh --help
```

Program options:

- `--camera <index>` — camera index, default is `0`.
- `--sync-face` — perform face detection in the main thread.
- `--delay-ms <value>` — artificial detector delay for demonstration, for example `500`.
- `--proto <path>` — path to `deploy.prototxt`.
- `--model <path>` — path to `.caffemodel`.

## Project Structure

```text
lab_07/
├── CMakeLists.txt
├── README.md
├── build.sh
├── preinstall.sh
├── run.sh
├── include/
│   ├── CameraProvider.hpp
│   ├── Display.hpp
│   ├── FaceDetector.hpp
│   ├── FrameProcessor.hpp
│   └── KeyProcessor.hpp
└── src/
    ├── CameraProvider.cpp
    ├── Display.cpp
    ├── FaceDetector.cpp
    ├── FrameProcessor.cpp
    ├── KeyProcessor.cpp
    └── main.cpp
```
