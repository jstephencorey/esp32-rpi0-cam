#pragma once

#include "esp_err.h"

void init_sdcard(void);
esp_err_t create_directory(const char *path);
void write_text_file(const char *path, const char *text);
esp_err_t remove_directory(const char *path);
