#pragma once
#include "audio/guidance_planner.h"
#include "audio/speech_bank.h"
#include <array>
namespace waze_hud {
struct SpeechMessage {
    std::array<Prompt,16> prompts{};std::size_t count{0};
    void add(Prompt p){if(count<prompts.size())prompts[count++]=p;}
};
SpeechMessage speechMessage(const Announcement &);
}
