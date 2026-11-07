// Inert fallback for non-ES3C28P builds. The S3 uses touch_ft6336.cpp.
#include "touch/touch_sensor.h"

namespace waze_hud {

TouchSensor &TouchSensor::instance() {
    static TouchSensor sensor;
    return sensor;
}

esp_err_t TouchSensor::init() { return ESP_OK; }

TouchOtaConfirmStatus TouchSensor::confirmStatus() const { return {}; }

bool TouchSensor::isTouched() const { return false; }
TouchEvent TouchSensor::takeEvent() { return TouchEvent::None; }

}  // namespace waze_hud
