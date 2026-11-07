#pragma once
#include "config/device_config.h"
#include "state/hud_state.h"
#include <array>
#include <cstdint>

namespace waze_hud {
enum class SpeechKind : uint8_t { None, Maneuver, CurrentLimit, NextLimit, Alert, NoPassing, Overspeed };
struct Announcement {
    SpeechKind kind{SpeechKind::None};
    uint32_t source{0};
    uint8_t stage{0},priority{0};
    int value{0},exit{0};
    Maneuver maneuver{Maneuver::None};
    AlertKind alert{AlertKind::None};
    uint64_t key() const {return (uint64_t(kind)<<56U)|(uint64_t(source)<<8U)|stage;}
};

// Reconciles the latest state; no FIFO of stale turn instructions. Playback
// completion, rather than selection, acknowledges an announcement.
class GuidancePlanner {
public:
    explicit GuidancePlanner(uint32_t repeatMs=4000):repeatMs_(repeatMs){}
    Announcement next(const HudState &,const DeviceSettings &,uint64_t nowMs);
    bool canContinue(const Announcement &,const HudState &,const DeviceSettings &,uint64_t nowMs);
    void complete(const Announcement &,uint64_t nowMs);
private:
    struct AlertSlot {
        AlertKind kind{AlertKind::None};int value{0},distance{-1};
        uint32_t serial{0};uint64_t seen{0};bool spoken{false},present{false};
    };
    void observe(const HudState &,const DeviceSettings &,uint64_t nowMs);
    int candidates(const HudState &,const DeviceSettings &,uint64_t nowMs,std::array<Announcement,12> &) const;
    uint32_t session_{0},serial_{0},turnSerial_{0},limitSerial_{0},limitSpoken_{0},zoneSerial_{0},zoneSpoken_{0};
    uint64_t turnHash_{0},overSince_{0},lastOverSpoken_{0};
    int turnDistance_{-1},limit_{0};uint8_t turnSpoken_{0};
    bool fresh_{false},zone_{false},over_{false},overSpoken_{false};
    std::array<AlertSlot,8> alerts_{};
    uint32_t repeatMs_{4000};
};
}
