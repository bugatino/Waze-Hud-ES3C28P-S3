#pragma once
#include <cstdint>
namespace waze_hud {
class AutoReturn {
public:
    void manualView(uint64_t now, uint64_t currentEvent) {
        until_ = now + 5000; seen_ = currentEvent; pending_ = 0;
    }
    bool update(uint64_t now, uint64_t event) {
        if (event != seen_) { seen_ = event; pending_ = event; }
        if (!event) pending_ = 0;
        if (now >= until_ && pending_) { pending_ = 0; return true; }
        return false;
    }
private:
    uint64_t until_{0}, seen_{0}, pending_{0};
};
}
