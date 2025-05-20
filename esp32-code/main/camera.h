#pragma once

#include "esp_err.h"

esp_err_t init_camera(void);
esp_err_t capture_and_save_photo(const char *filepath);
