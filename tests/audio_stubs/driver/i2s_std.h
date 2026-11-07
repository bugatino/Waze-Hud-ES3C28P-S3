#pragma once
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include <cstddef>
using i2s_chan_handle_t=void *;
enum { I2S_NUM_0=0,I2S_ROLE_MASTER=1,I2S_MCLK_MULTIPLE_256=256,I2S_DATA_BIT_WIDTH_16BIT=16,I2S_SLOT_MODE_STEREO=2,I2S_GPIO_UNUSED=-1 };
struct i2s_chan_config_t { bool auto_clear; };
struct i2s_std_config_t {
    struct { int rate; int mclk_multiple; } clk_cfg;
    struct { int width; int mode; } slot_cfg;
    struct { int mclk,bclk,ws,dout,din; } gpio_cfg;
};
#define I2S_CHANNEL_DEFAULT_CONFIG(a,b) i2s_chan_config_t{}
#define I2S_STD_CLK_DEFAULT_CONFIG(rate) {rate,256}
#define I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits,mode) {bits,mode}
esp_err_t i2s_new_channel(const i2s_chan_config_t *,i2s_chan_handle_t *,void *);
esp_err_t i2s_channel_init_std_mode(i2s_chan_handle_t,const i2s_std_config_t *);
esp_err_t i2s_channel_enable(i2s_chan_handle_t);
esp_err_t i2s_channel_write(i2s_chan_handle_t,const void *,std::size_t,std::size_t *,TickType_t);
