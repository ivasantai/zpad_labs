#pragma once

enum class Mode {
    Normal,
    Gray,
    Blur,
    Canny,
    Sobel,
    Invert,
    Face
};

class KeyProcessor {
public:
    Mode processKey(int key, Mode currentMode);
};
