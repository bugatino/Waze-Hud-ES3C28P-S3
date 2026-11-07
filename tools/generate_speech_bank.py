#!/usr/bin/env python3
"""Generate offline Vietnamese prompts with macOS Linh + FFmpeg G.711 mu-law.

The generated C++ is checked in; ordinary firmware builds need neither tool.
8 kHz mono clips are decoded to 16 kHz stereo by the audio task on the HUD.
"""
import argparse
import json
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "waze-hud/main/audio"
PHRASES = {
    "Prepare": "Chuẩn bị", "Straight": "Đi thẳng", "Left": "Rẽ trái", "Right": "Rẽ phải",
    "SlightLeft": "Rẽ chếch trái", "SlightRight": "Rẽ chếch phải",
    "SharpLeft": "Rẽ gắt sang trái", "SharpRight": "Rẽ gắt sang phải",
    "UTurn": "Quay đầu xe", "KeepLeft": "Đi về phía bên trái", "KeepRight": "Đi về phía bên phải",
    "ExitLeft": "Ra lối bên trái", "ExitRight": "Ra lối bên phải",
    "Roundabout": "Vào vòng xuyến", "RoundaboutExit": "Lấy lối ra số", "Arrive": "Bạn đã đến nơi",
    "Ferry": "Đi lên phà", "CurrentLimit": "Giới hạn tốc độ hiện tại",
    "NextLimit": "Giới hạn tốc độ phía trước", "Kmh": "Ki lô mét một giờ",
    "CameraSpeed": "Phía trước có camera tốc độ", "CameraEnforcement": "Phía trước có camera phạt nguội",
    "CameraRedLight": "Phía trước có camera đèn đỏ", "CameraOther": "Phía trước có camera giao thông",
    "Police": "Phía trước có cảnh sát giao thông", "Hazard": "Phía trước có nguy hiểm",
    "Accident": "Phía trước có tai nạn", "RoadClosed": "Phía trước đường bị đóng",
    "Railway": "Phía trước có đường sắt", "Construction": "Phía trước có công trường",
    "NoPassing": "Bạn đang trong vùng cấm vượt", "SpeedBump": "Phía trước có gờ giảm tốc",
    "SchoolZone": "Phía trước là khu vực trường học", "TrafficLight": "Phía trước có đèn giao thông",
    "TrafficJam": "Phía trước đang ùn tắc", "GenericAlert": "Chú ý biển báo phía trước",
    "N0": "không", "N1": "một", "N2": "hai", "N3": "ba", "N4": "bốn",
    "N5": "năm", "N6": "sáu", "N7": "bảy", "N8": "tám", "N9": "chín",
    "Ten": "mười", "Tens": "mươi", "Hundred": "trăm", "Odd": "lẻ", "FinalOne": "mốt", "FinalFive": "lăm",
}

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--voice", default="Linh")
    p.add_argument("--rate", type=int, default=190)
    args = p.parse_args()
    header = '#pragma once\n#include <cstddef>\n#include <cstdint>\nnamespace waze_hud {\n'
    header += 'enum class Prompt : uint8_t {\n' + ',\n'.join(PHRASES) + ', Count\n};\n'
    header += 'struct SpeechClip { const uint8_t *mulaw; std::size_t samples; };\n'
    header += 'SpeechClip speechClip(Prompt prompt);\n}\n'
    cpp = '#include "audio/speech_bank.h"\nnamespace waze_hud {\nnamespace {\n'
    sizes = {}
    with tempfile.TemporaryDirectory(prefix="hud-speech-") as temp:
        for name, words in PHRASES.items():
            aiff, raw = Path(temp)/"clip.aiff", Path(temp)/"clip.ulaw"
            subprocess.run(["say", "-v", args.voice, "-r", str(args.rate), "-o", str(aiff), words], check=True)
            subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(aiff), "-af",
                "silenceremove=start_periods=1:start_silence=0.02:start_threshold=-45dB,areverse,"
                "silenceremove=start_periods=1:start_silence=0.05:start_threshold=-45dB,areverse,"
                "loudnorm=I=-18:TP=-3:LRA=11", "-ar", "8000", "-ac", "1", "-c:a", "pcm_mulaw", "-f", "mulaw", str(raw)], check=True)
            data = raw.read_bytes()
            assert data, name
            sizes[name] = len(data)
            cpp += f'static const uint8_t clip{name}[]={{\n'
            cpp += ',\n'.join('  '+','.join(str(b) for b in data[i:i+32]) for i in range(0,len(data),32)) + '\n};\n'
            print(name, len(data), flush=True)
    cpp += 'const SpeechClip clips[]={\n' + ',\n'.join(f'{{clip{n},sizeof(clip{n})}}' for n in PHRASES) + '\n};\n}\n'
    cpp += 'SpeechClip speechClip(Prompt p){const auto i=static_cast<std::size_t>(p);return i<static_cast<std::size_t>(Prompt::Count)?clips[i]:SpeechClip{};}\n}\n'
    (DEST/"speech_bank.h").write_text(header)
    (DEST/"speech_bank.cpp").write_text(cpp)
    (DEST/"speech_bank.json").write_text(json.dumps({"voice":args.voice,"rate":args.rate,"encoding":"G711 mu-law 8kHz mono", "bytes":sum(sizes.values()),"phrases":{n:{"text":PHRASES[n],"bytes":v} for n,v in sizes.items()}},ensure_ascii=False,indent=2)+'\n')
    print("Total embedded bytes:",sum(sizes.values()))

if __name__=="__main__":main()
