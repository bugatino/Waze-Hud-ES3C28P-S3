#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
rtk mkdir -p tools/host_preview/out
rtk clang++ -std=gnu++17 -O1 -g -fsanitize=address,undefined \
  -DWAZE_HUD_HOST_PREVIEW=1 -DCONFIG_WAZE_HUD_LANES_CARD=1 \
  -include tools/host_preview/stubs/gmtime_stub.h -include algorithm -include array \
  -Itools/host_preview/stubs -Iwaze-hud/main \
  tools/host_preview/preview.cpp waze-hud/main/display/hud_renderer.cpp \
  waze-hud/main/display/bitmap_font.cpp waze-hud/main/assets/generated_assets.cpp \
  waze-hud/main/assets/portrait_assets.cpp \
  waze-hud/main/state/trip_stats.cpp waze-hud/main/system/lunar_calendar.c \
  waze-hud/main/ota/wifi_ota_manager_stub.cpp -o tools/host_preview/out/preview
rtk env OUTDIR=tools/host_preview/out \
  CLOCK_OPTIMUS=waze-hud/main/assets/clock_optimus.rgb565 \
  CLOCK_BUMBLEBEE=waze-hud/main/assets/clock_bumblebee.rgb565 \
  tools/host_preview/out/preview
rtk clang++ -std=c++17 -include initializer_list -Iwaze-hud/main \
  tests/gesture_transform_test.cpp -o tools/host_preview/out/gesture_test
rtk tools/host_preview/out/gesture_test
rtk clang++ -std=c++17 -Iwaze-hud/main tests/pcm_auto_return_test.cpp -o tools/host_preview/out/pcm_auto_return_test
rtk tools/host_preview/out/pcm_auto_return_test
