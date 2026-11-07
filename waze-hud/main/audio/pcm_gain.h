#pragma once
#include <algorithm>
#include <cstdint>
namespace waze_hud {
inline int16_t scalePcm(int16_t sample, unsigned volume) {
    // Keep both operands signed before multiplying negative PCM samples.
    const int32_t gain = static_cast<int32_t>(std::min(volume, 100U));
    return static_cast<int16_t>(static_cast<int32_t>(sample) * gain / 100);
}
}
