set(waze_hud_extra_requires "")
if(IDF_TARGET STREQUAL "esp32s3")
    # ES3C28P speaker path: ES8311 codec over legacy I2C + I2S output.
    list(APPEND waze_hud_extra_requires driver esp_driver_i2s espressif__es8311)
endif()
# WiFi OTA remains disabled. Classic CYD touch stays disabled; ES3C28P uses
# its I2C FT6336G, never the XPT2046 SPI driver whose pins conflict with PSRAM.
set(waze_hud_board_sources "ota/wifi_ota_manager_stub.cpp" "touch/touch_sensor_stub.cpp")
if(CONFIG_WAZE_HUD_BOARD_ES3C28P)
    set(waze_hud_board_sources "ota/wifi_ota_manager_stub.cpp" "touch/touch_ft6336.cpp")
endif()

idf_component_register(
    SRCS
        "app_main.cpp"
        "audio/overspeed_audio.cpp"
        "audio/overspeed_voice.cpp"
        "bluetooth/ble_transport.cpp"
        "protocol/hlp_core.c"
        "protocol/hlp_protocol.cpp"
        "protocol/hlp_decoder.cpp"
        "state/hud_state_store.cpp"
        "state/trip_stats.cpp"
        "display/display_driver.cpp"
        "display/hud_renderer.cpp"
        "display/bitmap_font.cpp"
        "assets/generated_assets.cpp"
        "config/device_config.cpp"
        "system/system_status.cpp"
        "system/shared_i2c.cpp"
        "system/lunar_calendar.c"
        ${waze_hud_board_sources}
    EMBED_FILES
        "assets/clock_optimus.rgb565"
        "assets/clock_bumblebee.rgb565"
    INCLUDE_DIRS "."
    REQUIRES bt esp_adc esp_lcd esp_driver_gpio esp_driver_ledc esp_driver_spi esp_hw_support nvs_flash json esp_netif
             ${waze_hud_extra_requires}
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall -Wextra -Werror=return-type
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    WAZE_HUD_FIRMWARE_VERSION="${PROJECT_VER}"
)
