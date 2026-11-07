#pragma once
#include "esp_err.h"
#include <cstdint>
using gpio_num_t=int;
enum { GPIO_NUM_1=1,GPIO_NUM_4=4,GPIO_NUM_5=5,GPIO_NUM_7=7,GPIO_NUM_8=8,GPIO_MODE_OUTPUT=2 };
struct gpio_config_t { uint64_t pin_bit_mask; int mode; };
esp_err_t gpio_config(const gpio_config_t *);
esp_err_t gpio_set_level(gpio_num_t,int);
