#pragma once
#include <cstdlib>
#include <cstdint>
namespace waze_hud {
enum class TouchEvent : uint8_t { None, Tap, Hold, VolumeUp, VolumeDown, Release };
class Gesture {
public:
    TouchEvent update(bool down, int x, int y, uint64_t ms) {
        if (!down) {
            if (!active_) return TouchEvent::None;
            active_ = false;
            if (swiped_) return TouchEvent::Release;
            if (cancelled_ || held_ || ms - started_ < 35 || ms - started_ > 700)
                return TouchEvent::None;
            return TouchEvent::Tap;
        }
        if (!active_) {
            active_ = true; swiped_ = held_ = cancelled_ = false;
            x0_ = x; y0_ = lastY_ = y; started_ = ms;
            return TouchEvent::None;
        }
        if (std::abs(x - x0_) > 22 && !swiped_) cancelled_ = true;
        if (!cancelled_ && !held_ && std::abs(y - lastY_) >= 20) {
            swiped_ = true;
            const auto event = y < lastY_ ? TouchEvent::VolumeUp : TouchEvent::VolumeDown;
            lastY_ = y;
            return event;
        }
        if (!cancelled_ && !swiped_ && !held_ && ms - started_ >= 800) {
            held_ = true;
            return TouchEvent::Hold;
        }
        return TouchEvent::None;
    }
    void cancel() { active_ = false; }
private:
    bool active_{false}, swiped_{false}, held_{false}, cancelled_{false};
    int x0_{0}, y0_{0}, lastY_{0};
    uint64_t started_{0};
};
}
