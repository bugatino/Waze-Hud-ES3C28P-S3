#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
: "${IDF_PATH:?Set IDF_PATH to ESP-IDF 5.5.5 for its real cJSON dependency}"
json_dir="$IDF_PATH/components/json/cJSON"
out=tools/host_preview/out
rtk mkdir -p "$out"
rtk clang -std=c11 -g -fsanitize=address,undefined -I"$json_dir" \
  -c "$json_dir/cJSON.c" -o "$out/cjson.o"
rtk clang++ -std=c++17 -g -fsanitize=address,undefined \
  -Itests/config_stubs -I"$json_dir" -Itools/host_preview/stubs -Iwaze-hud/main \
  tests/device_config_test.cpp waze-hud/main/config/device_config.cpp \
  "$out/cjson.o" -o "$out/config_test"
rtk "$out/config_test"
rtk clang -std=c11 -g -fsanitize=address,undefined -Iwaze-hud/main \
  -c waze-hud/main/protocol/hlp_core.c -o "$out/hlp_core.o"
rtk clang++ -std=c++17 -g -fsanitize=address,undefined \
  -I"$json_dir" -Itools/host_preview/stubs -Iwaze-hud/main \
  tests/camera_decoder_test.cpp waze-hud/main/protocol/hlp_decoder.cpp \
  "$out/cjson.o" "$out/hlp_core.o" -o "$out/camera_decoder_test"
rtk "$out/camera_decoder_test"
rtk clang++ -std=gnu++17 -O1 -g -fsanitize=address,undefined \
  -DWAZE_HUD_HOST_PREVIEW=1 -DCONFIG_WAZE_HUD_LANES_CARD=1 \
  -include tools/host_preview/stubs/gmtime_stub.h -include algorithm -include array \
  -I"$json_dir" -Itools/host_preview/stubs -Iwaze-hud/main \
  tests/lane_renderer_test.cpp waze-hud/main/protocol/hlp_decoder.cpp \
  waze-hud/main/display/hud_renderer.cpp waze-hud/main/display/bitmap_font.cpp \
  waze-hud/main/assets/generated_assets.cpp waze-hud/main/assets/portrait_assets.cpp \
  waze-hud/main/state/trip_stats.cpp waze-hud/main/system/lunar_calendar.c \
  waze-hud/main/ota/wifi_ota_manager_stub.cpp "$out/cjson.o" "$out/hlp_core.o" \
  -o "$out/lane_renderer_test"
rtk env OUTDIR="$out" "$out/lane_renderer_test"
