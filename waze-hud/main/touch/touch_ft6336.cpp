#include "touch/touch_sensor.h"
#include "config/device_config.h"
#include "display/screen_transform.h"
#include "system/shared_i2c.h"
#include "state/hud_state_store.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <atomic>
namespace waze_hud {
namespace {
QueueHandle_t events = nullptr;
std::atomic<bool> touched{false};
}
TouchSensor &TouchSensor::instance() { static TouchSensor sensor; return sensor; }
TouchOtaConfirmStatus TouchSensor::confirmStatus() const { return {}; }
bool TouchSensor::isTouched() const { return touched.load(); }
TouchEvent TouchSensor::takeEvent() {
    TouchEvent event = TouchEvent::None;
    if (events) xQueueReceive(events, &event, 0);
    return event;
}
esp_err_t TouchSensor::init() {
    esp_err_t result = initSharedI2c();
    if (result != ESP_OK) return result;
    // Polling leaves reset/interrupt pins untouched and also works on boards
    // whose touch reset is wired differently. FT6336G boots with the board.
    uint8_t reg = 0x02, status = 0;
    result = i2c_master_write_read_device(I2C_NUM_0, 0x38, &reg, 1, &status, 1, pdMS_TO_TICKS(50));
    if (result != ESP_OK) return result;
    events = xQueueCreate(16, sizeof(TouchEvent));
    if (!events) return ESP_ERR_NO_MEM;
    if (xTaskCreate(taskEntry, "hud_touch", 3072, this, 3, nullptr) != pdPASS) {
        vQueueDelete(events); events = nullptr; return ESP_ERR_NO_MEM;
    }
    ESP_LOGI("TOUCH", "FT6336G responding at 0x38; tap/hold/swipe enabled");
    return ESP_OK;
}
void TouchSensor::taskEntry(void *arg) { static_cast<TouchSensor *>(arg)->runLoop(); }
void TouchSensor::runLoop() {
    Gesture gesture;
    DeviceSettings orientation = DeviceConfig::instance().snapshot();
    unsigned failures = 0;
    bool waitForRelease = false;
    for (;;) {
        const auto cfg = DeviceConfig::instance().snapshot();
        if (cfg.portraitDisplay != orientation.portraitDisplay ||
            cfg.rotateDisplay != orientation.rotateDisplay || cfg.mirrorHud != orientation.mirrorHud) {
            gesture.cancel(); orientation = cfg; waitForRelease = true;
        }
        uint8_t reg = 0x02, data[5]{};
        const auto result = i2c_master_write_read_device(I2C_NUM_0, 0x38, &reg, 1, data, sizeof(data), pdMS_TO_TICKS(30));
        if (result == ESP_OK) {
            failures = 0;
            const unsigned count = data[0] & 0x0f;
            const int x = ((data[1] & 0x0f) << 8) | data[2];
            const int y = ((data[3] & 0x0f) << 8) | data[4];
            if (count == 0) waitForRelease = false;
            if (count > 1 || (count > 0 && (x >= 240 || y >= 320))) {
                gesture.cancel(); waitForRelease = true;
            }
            const bool down = !waitForRelease && count == 1 && (data[1] >> 6) != 1;
            touched.store(down);
            const auto point = fromNative({x, y}, cfg.portraitDisplay, cfg.rotateDisplay, cfg.mirrorHud);
            const auto event = gesture.update(down, point.x, point.y, esp_timer_get_time() / 1000);
            if (event != TouchEvent::None) {
                if (xQueueSend(events, &event, 0) == pdTRUE) HudStateStore::instance().refresh();
            }
        } else if (++failures >= 3) {
            gesture.cancel(); touched.store(false); waitForRelease = true;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
}
