#pragma once
#include <cstdint>
namespace waze_hud {
inline int16_t decodeMulaw(uint8_t code) {
    const unsigned value=static_cast<uint8_t>(~code);
    const int magnitude=(((value&15U)<<3U)+132U)<<((value>>4U)&7U);
    return static_cast<int16_t>((value&128U)?132-magnitude:magnitude-132);
}
}
