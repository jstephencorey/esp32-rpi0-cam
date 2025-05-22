#pragma once

#include "esp_err.h"

// Ideally this should be unique to the device.
// You could retrieve ESP32 chip ID at runtime if desired. todo change this maybe
#define DEVICE_ID "esp32_001"

esp_err_t upload_photos_from_dir(const char *dir_path);
