#include "system/shared_i2c.h"
#include "sdkconfig.h"
#if CONFIG_WAZE_HUD_BOARD_ES3C28P
#include "driver/i2c.h"
namespace waze_hud {
// Called sequentially during startup, before touch/audio workers are started.
esp_err_t initSharedI2c() {
    static bool ready = false;
    if (ready) return ESP_OK;
    i2c_config_t bus{};
    bus.mode = I2C_MODE_MASTER;
    bus.sda_io_num = GPIO_NUM_16;
    bus.scl_io_num = GPIO_NUM_15;
    bus.sda_pullup_en = GPIO_PULLUP_ENABLE;
    bus.scl_pullup_en = GPIO_PULLUP_ENABLE;
    bus.master.clk_speed = 100000;
    esp_err_t result = i2c_param_config(I2C_NUM_0, &bus);
    if (result == ESP_OK) result = i2c_driver_install(I2C_NUM_0, bus.mode, 0, 0, 0);
    ready = result == ESP_OK;
    return result;
}
}
#else
namespace waze_hud { esp_err_t initSharedI2c() { return ESP_ERR_NOT_SUPPORTED; } }
#endif
