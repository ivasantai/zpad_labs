#include "KeyProcessor.hpp"

Mode KeyProcessor::processKey(int key, Mode currentMode) {
    switch (key) {
        case '0': return Mode::Normal;
        case '1': return Mode::Gray;
        case '2': return Mode::Blur;
        case '3': return Mode::Canny;
        case '4': return Mode::Sobel;
        case '5': return Mode::Invert;
        default: return currentMode;
    }
}