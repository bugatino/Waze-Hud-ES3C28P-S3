#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
out=tools/host_preview/out
rtk mkdir -p "$out"
rtk python3 -c 'from pathlib import Path; Path("tools/host_preview/out/mulaw-all.raw").write_bytes(bytes(range(256)))'
rtk ffmpeg -v error -nostdin -y -f mulaw -ar 8000 -ac 1 -i "$out/mulaw-all.raw" -f s16le "$out/mulaw-reference.raw"
rtk clang++ -std=c++17 -O1 -g -fsanitize=address,undefined \
  -Itools/host_preview/stubs -Iwaze-hud/main -include algorithm -include cstdlib \
  tests/speech_bank_test.cpp waze-hud/main/audio/speech_message.cpp waze-hud/main/audio/speech_bank.cpp \
  -o "$out/speech_bank_test"
rtk "$out/speech_bank_test" "$out/mulaw-reference.raw" "$out"
