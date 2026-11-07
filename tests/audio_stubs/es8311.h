#pragma once
#include "driver/i2c.h"
#include "esp_err.h"
using es8311_handle_t=void *;
enum { ES8311_ADDRRES_0=0x18,ES8311_RESOLUTION_16=16 };
struct es8311_clock_config_t { bool mclk_inverted,mclk_from_mclk_pin; int mclk_frequency,sample_frequency; };
es8311_handle_t es8311_create(i2c_port_t,int);
esp_err_t es8311_init(es8311_handle_t,const es8311_clock_config_t *,int,int);
void es8311_delete(es8311_handle_t);
esp_err_t es8311_sample_frequency_config(es8311_handle_t,int,int);
esp_err_t es8311_voice_volume_set(es8311_handle_t,int,int *);
